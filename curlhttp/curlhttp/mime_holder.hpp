#ifndef MIME_HOLDER_HPP
#define MIME_HOLDER_HPP


#include "curl_base.hpp"


namespace curlhttp{
    class basic_mime_data;


    class mime_holder{
    public:
        std::vector<std::shared_ptr<basic_mime_data>> data;

        explicit mime_holder(curl_base& parent)
            : mime{curl_mime_init(parent.native())} {}

        mime_holder(mime_holder&& ) = default;
        mime_holder& operator= (mime_holder&& ) = default;

        void setup();

        curl_mime* native() const{
            return mime.get();
        }

    private:
        std::unique_ptr<curl_mime, detail::curl_mime_deleter> mime;
    };
}


#endif
