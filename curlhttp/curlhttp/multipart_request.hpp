#ifndef CURLHTTP_MULTIPART_REQUEST_HPP
#define CURLHTTP_MULTIPART_REQUEST_HPP


#include "http_request.hpp"
#include "mime_data.hpp"
#include "mime_holder.hpp"


namespace curlhttp{

    template<typename RX, typename Writer = default_writer<RX>>
    class multipart_request : public post_request<RX, Writer>{
    public:
        mime_holder mimes;

        multipart_request(RX& buffer, const url_t& url)
            : post_request<RX, Writer>{buffer, url}, mimes{*this} {}

        void init() override{
            post_request<RX, Writer>::init();
            curl_base::set_option(CURLOPT_MIMEPOST, mimes.native());

            for(auto& mime : mimes.data)
                mime->setup();
        }

        std::shared_ptr<file_data> create_file_data(const std::filesystem::path& filepath){
            auto p = std::make_shared<file_data>(filepath, mimes.native());
            mimes.data.push_back(p);
            return p;
        }

        template<typename TX>
        std::shared_ptr<mime_data<TX>> create_data(TX& buffer){
            auto p = std::make_shared<mime_data<TX>>(buffer, mimes.native());
            mimes.data.push_back(p);
            return p;
        }

    private:
        std::unique_ptr<curl_mime, detail::curl_mime_deleter> mime;
    };


    void mime_holder::setup(){
        for(auto& m : data)
            m->setup();
    }

}


#endif
