export module c2server.payload;

import std;

export namespace c2server {

   /** @brief Abstract storage interface for a mutable payload. */
   struct PayloadStoreBase {
      /** @brief Destroy a payload store. */
      virtual ~PayloadStoreBase() = default;
      /**
       * @brief Read the stored payload.
       *
       * @return Stored payload value.
       */
      virtual std::string get() const = 0;
      /**
       * @brief Replace the stored payload.
       *
       * @param value New payload value.
       */
      virtual void set(std::string_view value) = 0;
   };

   /** @brief Thread-safe in-memory payload storage. */
   struct PayloadStore final : PayloadStoreBase {
      /**
       * @brief Construct a payload store.
       *
       * @param value Initial payload value.
       */
      explicit PayloadStore(std::string value);

      /**
       * @brief Read the stored payload.
       *
       * @return Stored payload value.
       */
      std::string get() const override;
      /**
       * @brief Replace the stored payload.
       *
       * @param value New payload value.
       */
      void set(std::string_view value) override;

   private:
      mutable std::mutex mtx_;
      std::string value_;
   };

} // namespace c2server
