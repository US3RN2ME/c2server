export module c2server.payload;

import std;

export namespace c2server {

   struct PayloadStoreBase {
      virtual ~PayloadStoreBase() = default;
      virtual std::string get() const = 0;
      virtual void set(std::string_view) = 0;
   };

   struct PayloadStore final : PayloadStoreBase {
      explicit PayloadStore(std::string value);

      std::string get() const override;
      void set(std::string_view) override;

   private:
      mutable std::mutex mtx_;
      std::string value_;
   };

} // namespace c2server
