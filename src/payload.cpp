module c2server.payload;

namespace c2server {

   PayloadStore::PayloadStore(std::string value)
       : value_{std::move(value)} {}

   std::string PayloadStore::get() const {
      std::lock_guard lock{mtx_};
      return value_;
   }

   void PayloadStore::set(std::string_view sv) {
      std::lock_guard lock{mtx_};
      value_ = sv;
   }

} // namespace c2server
