#ifndef CURLHTTP_CURL_HANDLE_HPP
#define CURLHTTP_CURL_HANDLE_HPP


#include <string_view>

#include "curl_base.hpp"
#include "default_writer.hpp"
#include "default_reader.hpp"
#include "default_seeker.hpp"
#include "response_t.hpp"


namespace curlhttp{

    template<typename RX, typename TX,
             typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    class curl_handle : public curl_base{
    public:
        using rx_buffer_t = RX;
        using tx_buffer_t = TX;

        using writer_t = Writer;
        using reader_t = Reader;
        using seeker_t = Seeker;

        using rx_callback_t = std::function<bool(const std::string_view& )>;
        using tx_callback_t = std::function<bool(const std::string_view& )>;

        rx_callback_t rx_callback;
        tx_callback_t tx_callback;

        rx_buffer_t& rx_buffer;
        tx_buffer_t& tx_buffer;

        curl_handle(rx_buffer_t& rx, tx_buffer_t& tx, const url_t& url)
            : curl_base{url}, rx_buffer{rx}, tx_buffer{tx} {}

        curl_handle(curl_handle&& ) = default;
        curl_handle& operator= (curl_handle&& ) = default;

        virtual ~curl_handle() {}

        void reset() override{
            curl_base::reset();

            rx_callback = {};
            tx_callback = {};

            response_headers.clear();
        }

        const std::vector<std::string> get_response_headers() const{
            return response_headers;
        }

        void* rx_buffer_ptr() const override{
            return std::addressof(rx_buffer);
        }
        void* tx_buffer_ptr() const override{
            return std::addressof(tx_buffer);
        }

    protected:
        std::vector<std::string> response_headers;
        std::string response_buffer;
        default_writer<std::string> string_writer;

        void setup_download() override{
            set_option(CURLOPT_WRITEDATA, this);
            set_option(CURLOPT_WRITEFUNCTION, &curl_handle::write_callback);
        }

        void setup_header_download() override{
            set_option(CURLOPT_HEADERDATA, this);
            set_option(CURLOPT_HEADERFUNCTION, &curl_handle::write_header_callback);
        }

        void setup_upload() override{
            set_option(CURLOPT_READDATA, this);
            set_option(CURLOPT_READFUNCTION, &curl_handle::read_callback);
        }

        void setup_seeking() override{
            set_option(CURLOPT_SEEKDATA, this);
            set_option(CURLOPT_SEEKFUNCTION, &curl_handle::seek_callback);
        }

    private:
        writer_t writer;
        reader_t reader;
        seeker_t seeker;

        static std::size_t write_callback(char* buffer, std::size_t sz, std::size_t nmemb, curl_handle* this_) try{
            if(this_->rx_callback && !this_->rx_callback(std::string_view{buffer, sz * nmemb}))
                return default_write_abort;

            return this_->writer(this_->rx_buffer, buffer, sz, nmemb);
        }

        catch(...){
            this_->callback_exception = std::current_exception();
            return default_write_abort;
        }


        static std::size_t write_header_callback(char* buffer, std::size_t sz, std::size_t nmemb, curl_handle* this_) try{
            if(this_->rx_callback && !this_->rx_callback(std::string_view{buffer, sz * nmemb}))
                return default_write_abort;

            std::size_t size = sz * nmemb;

            if(size == 2 && !std::strncmp(buffer, "\r\n", 2)){
                size = this_->string_writer(this_->response_buffer, buffer, sz, nmemb);

                this_->response_headers.push_back(this_->response_buffer);
                this_->response_buffer.clear();

                return size;
            }

            return this_->string_writer(this_->response_buffer, buffer, sz, nmemb);
        }

        catch(...){
            this_->callback_exception = std::current_exception();
            return default_write_abort;
        }


        static std::size_t read_callback(char* buffer, std::size_t sz, std::size_t nmemb, curl_handle* this_) try{
            std::size_t size = this_->reader(this_->tx_buffer, buffer, sz, nmemb);

            if(this_->tx_callback && !this_->tx_callback(std::string_view{buffer, size}))
                return default_read_abort;

            return size;
        }

        catch(...){
            this_->callback_exception = std::current_exception();
            return default_read_abort;
        }


        static int seek_callback(curl_handle* this_, curl_off_t offset, int origin) try{
            return this_->seeker(this_->tx_buffer, offset, origin);
        }

        catch(...){
            this_->callback_exception = std::current_exception();
            return CURL_SEEKFUNC_FAIL;
        }
    };

}


#endif
