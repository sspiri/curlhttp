#ifndef CURLHTTP_HTTP_REQUEST_HPP
#define CURLHTTP_HTTP_REQUEST_HPP


#include <filesystem>

#include "curl_handle.hpp"
#include "status_code.hpp"
#include "method_t.hpp"
#include "utility.hpp"
#include "size_getter.hpp"


namespace curlhttp{

    inline std::string get_default_user_agent(){
        return std::string{"curl/"} + curl_version_info(CURLVERSION_NOW)->version;
    }


    /*** HTTP REQUEST ***/

    template<method_t Method, typename RX, typename TX,
             typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    class http_request : public curl_handle<RX, TX, Writer, Reader, Seeker>{
    public:
        static constexpr method_t method = Method;
        inline static const std::string default_user_agent = get_default_user_agent();

        using status_code_callback_t = std::function<bool(status_code)>;
        using response_callback_t = std::function<bool(const response_t& )>;

        using curl_handle<RX, TX, Writer, Reader, Seeker>::curl_handle;

        http_request(http_request&& ) = default;
        http_request& operator= (http_request&& ) = default;

        virtual ~http_request() {}

        std::vector<field_t> headers;
        std::string user_agent{default_user_agent};
        curl_base::error_callback_t http_error_callback;
        status_code_callback_t status_code_callback;
        response_callback_t response_callback;
        bool throw_http_errors{true};

        void init() override{
            curl_handle<RX, TX, Writer, Reader, Seeker>::init();
            code = (status_code)0;

            curl_base::set_option(CURLOPT_FOLLOWLOCATION, true);
            curl_base::set_option(CURLOPT_USERAGENT, user_agent.c_str());

            setup_headers();
        }

        void exit() override{
            curl_handle<RX, TX, Writer, Reader, Seeker>::exit();

            if((bool)code)
                handle_status_code();
        }

        void reset() override{
            curl_handle<RX, TX, Writer, Reader, Seeker>::reset();

            headers.clear();
            user_agent = default_user_agent;

            http_error_callback = {};
            status_code_callback = {};
            response_callback = {};

            throw_http_errors = true;
        }

        void load_cookies(const std::filesystem::path& path){
            curl_base::set_option(CURLOPT_COOKIEFILE, path.c_str());
        }

        void save_cookies(const std::filesystem::path& path){
            curl_base::set_option(CURLOPT_COOKIEJAR, path.c_str());
        }

        void set_cookie(const std::string& cookie){
            curl_base::set_option(CURLOPT_COOKIELIST, cookie.c_str());
        }

        status_code get_status_code() const{
            return code;
        }

    protected:
        void setup_header_download() override{
            curl_base::set_option(CURLOPT_HEADERDATA, this);
            curl_base::set_option(CURLOPT_HEADERFUNCTION, &http_request::write_header_callback);
        }

    private:
        status_code code;
        std::unique_ptr<curl_slist, detail::curl_slist_deleter> headers_list;

        void setup_headers(){
            for(const auto& field : headers)
                headers_list.reset(curl_slist_append(headers_list.release(), (field.name + ": " + field.value).c_str()));

            curl_base::set_option(CURLOPT_HTTPHEADER, headers_list.get());
        }

        void handle_status_code(){
            if(is_client_error(code) || is_server_error(code)){
                auto ec = make_http_error_code(code);

                if(http_error_callback)
                    http_error_callback(ec);

                if(throw_http_errors)
                    throw http_error{ec};
            }
        }

        bool get_status_code(const std::string_view& status_line){
            std::size_t pos = status_line.find_first_of(" \t");

            if(pos != status_line.npos && pos < status_line.size() - 1 && std::isdigit(status_line[++pos])){
                code = (status_code)std::stoul(status_line.substr(pos).data());

                if(status_code_callback)
                    return status_code_callback(code);
            }

            return true;
        }


        static std::size_t write_header_callback(char* buffer, std::size_t sz, std::size_t nmemb, http_request* this_) try{
            if(this_->rx_callback && !this_->rx_callback(std::string_view{buffer, sz * nmemb}))
                return curl_base::default_write_abort;

            std::size_t size = sz * nmemb;

            if(!std::strncmp(buffer, "HTTP/", 5)){
                if(!this_->get_status_code(std::string_view{buffer, size}))
                    return curl_base::default_write_abort;
            }

            else if(size == 2 && !std::strncmp(buffer, "\r\n", 2)){
                size = this_->string_writer(this_->response_buffer, buffer, sz, nmemb);

                this_->response_headers.push_back(this_->response_buffer);
                this_->response_buffer.clear();

                if(this_->response_callback){
                    std::stringstream stream{this_->response_headers.back()};
                    response_t response;
                    stream >> response;

                    if(!this_->response_callback(response))
                        return curl_base::default_write_abort;
                }

                return size;
            }

            return this_->string_writer(this_->response_buffer, buffer, sz, nmemb);
        }

        catch(...){
            this_->callback_exception = std::current_exception();
            return curl_base::default_write_abort;
        }
    };


    /*** GET REQUEST ***/

    template<typename RX, typename Writer>
    class http_request<method_t::get, RX, nullbuf_t, Writer>
        : public http_request<method_t::none, RX, nullbuf_t, Writer>{
    public:
        query_t query;

        http_request(RX& buffer, const url_t& url)
            : http_request<method_t::none, RX, nullbuf_t, Writer>{buffer, nullbuf, url} {}

        http_request(http_request&& ) = default;
        http_request& operator= (http_request&& ) = default;

        virtual ~http_request() {}

        void init() override{
            setup_query();
            http_request<method_t::none, RX, nullbuf_t, Writer>::init();
            curl_base::set_option(CURLOPT_HTTPGET, true);
        }

        void reset() override{
            http_request<method_t::none, RX, nullbuf_t, Writer>::reset();
            query.clear();
        }

    private:
        void setup_query(){
            std::string q = this->url.query_string();

            if(q.size() && query.size())
                q += '&';

            q += query_string(query);
            this->url.set(CURLUPART_QUERY, q);
        }
    };


    /*** OPTIONS REQUEST ***/

    template<typename RX, typename Writer>
    class http_request<method_t::options, RX, nullbuf_t, Writer>
        : public http_request<method_t::none, RX, nullbuf_t, Writer>{
    public:
        std::string target{"*"};

        http_request(RX& buffer, const url_t& url)
            : http_request<method_t::none, RX, nullbuf_t, Writer>{buffer, nullbuf, url} {}

        http_request(http_request&& ) = default;
        http_request& operator= (http_request&& ) = default;

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, RX, nullbuf_t, Writer>::init();
            curl_base::set_option(CURLOPT_CUSTOMREQUEST, "OPTIONS");
            curl_base::set_option(CURLOPT_REQUEST_TARGET, target.c_str());
        }

        void reset() override{
            http_request<method_t::none, RX, nullbuf_t, Writer>::reset();
            target.clear();
        }
    };



    /*** HEAD REQUEST ***/


    template<>
    class http_request<method_t::head, nullbuf_t, nullbuf_t>
        : public http_request<method_t::none, nullbuf_t, nullbuf_t>{
    public:
        explicit http_request(const url_t& url)
            : http_request<method_t::none, nullbuf_t, nullbuf_t>{nullbuf, nullbuf, url} {}

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, nullbuf_t, nullbuf_t>::init();
            curl_base::set_option(CURLOPT_NOBODY, true);
        }
    };



    /*** TRACE REQUEST ***/


    template<>
    class http_request<method_t::trace, nullbuf_t, nullbuf_t>
        : public http_request<method_t::none, nullbuf_t, nullbuf_t>{
    public:
        explicit http_request(const url_t& url)
            : http_request<method_t::none, nullbuf_t, nullbuf_t>{nullbuf, nullbuf, url} {}

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, nullbuf_t, nullbuf_t>::init();
            curl_base::set_option(CURLOPT_CUSTOMREQUEST, "TRACE");
        }
    };



    /*** POST REQUEST ***/

    template<typename RX, typename Writer>
    class http_request<method_t::post, RX, nullbuf_t, Writer>
        : public http_request<method_t::none, RX, nullbuf_t, Writer>{
    public:
        std::vector<field_t> data;

        http_request(RX& buffer, const url_t& url)
            : http_request<method_t::none, RX, nullbuf_t, Writer>{buffer, nullbuf, url} {}

        http_request(http_request&& ) = default;
        http_request& operator= (http_request&& ) = default;

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, RX, nullbuf_t, Writer>::init();
            setup_post_fields();
        }

        void reset() override{
            http_request<method_t::none, RX, nullbuf_t, Writer>::reset();
            data.clear();
        }

    private:
        void setup_post_fields(){
            std::string s = query_string(data);

            curl_base::set_option(CURLOPT_POSTFIELDSIZE_LARGE, s.size());
            curl_base::set_option(CURLOPT_COPYPOSTFIELDS, s.c_str());
        }
    };



    /*** SPECIAL POST REQUEST ***/

    template<typename RX, typename TX, typename Writer, typename Reader, typename Seeker>
    class http_request<method_t::special_post, RX, TX, Writer, Reader, Seeker>
        : public http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{
    public:
        http_request(RX& rx, TX& tx, const url_t& url)
            : http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{rx, tx, url} {}

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, RX, TX, Writer, Reader, Seeker>::init();

            curl_base::set_option(CURLOPT_UPLOAD, true);
            curl_base::set_option(CURLOPT_INFILESIZE_LARGE, size_getter<TX>{}(this->tx_buffer));
            curl_base::set_option(CURLOPT_CUSTOMREQUEST, "POST");
        }
    };



    /*** PUT REQUEST ***/

    template<typename RX, typename TX, typename Writer, typename Reader, typename Seeker>
    class http_request<method_t::put, RX, TX, Writer, Reader, Seeker>
        : public http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{
    public:
        http_request(RX& rx, TX& tx, const url_t& url)
            : http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{rx, tx, url} {}

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, RX, TX, Writer, Reader, Seeker>::init();

            curl_base::set_option(CURLOPT_UPLOAD, true);
            curl_base::set_option(CURLOPT_INFILESIZE_LARGE, size_getter<TX>{}(this->tx_buffer));
            curl_base::set_option(CURLOPT_CUSTOMREQUEST, "PUT");
        }
    };



    /*** DELETE REQUEST ***/

    template<typename RX, typename TX, typename Writer, typename Reader, typename Seeker>
    class http_request<method_t::delete_, RX, TX, Writer, Reader, Seeker>
        : public http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{
    public:
        http_request(RX& rx, TX& tx, const url_t& url)
            : http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{rx, tx, url} {}

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, RX, TX, Writer, Reader, Seeker>::init();

            curl_base::set_option(CURLOPT_UPLOAD, true);
            curl_base::set_option(CURLOPT_INFILESIZE_LARGE, size_getter<TX>{}(this->tx_buffer));
            curl_base::set_option(CURLOPT_CUSTOMREQUEST, "DELETE");
        }
    };



    /*** PATCH REQUEST ***/

    template<typename RX, typename TX, typename Writer, typename Reader, typename Seeker>
    class http_request<method_t::patch, RX, TX, Writer, Reader, Seeker>
        : public http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{
    public:
        http_request(RX& rx, TX& tx, const url_t& url)
            : http_request<method_t::none, RX, TX, Writer, Reader, Seeker>{rx, tx, url} {}

        virtual ~http_request() {}

        void init() override{
            http_request<method_t::none, RX, TX, Writer, Reader, Seeker>::init();

            curl_base::set_option(CURLOPT_UPLOAD, true);
            curl_base::set_option(CURLOPT_INFILESIZE_LARGE, size_getter<TX>{}(this->tx_buffer));
            curl_base::set_option(CURLOPT_CUSTOMREQUEST, "PATCH");
        }
    };



    /*** REQUEST ALIASES ***/

    using head_request = http_request<method_t::head, nullbuf_t, nullbuf_t>;
    using trace_request = http_request<method_t::trace, nullbuf_t, nullbuf_t>;

    template<typename RX, typename Writer = default_writer<RX>>
    using get_request = http_request<method_t::get, RX, nullbuf_t, Writer>;

    template<typename RX, typename Writer = default_writer<RX>>
    using options_request = http_request<method_t::options, RX, nullbuf_t, Writer>;

    template<typename RX, typename Writer = default_writer<RX>>
    using post_request = http_request<method_t::post, RX, nullbuf_t, Writer>;

    template<typename RX, typename TX, typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    using put_request = http_request<method_t::put, RX, TX, Writer, Reader, Seeker>;

    template<typename RX, typename TX, typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    using special_post = http_request<method_t::special_post, RX, TX, Writer, Reader, Seeker>;

    template<typename RX, typename TX, typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    using delete_request = http_request<method_t::delete_, RX, TX, Writer, Reader, Seeker>;

    template<typename RX, typename TX, typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    using patch_request = http_request<method_t::patch, RX, TX, Writer, Reader, Seeker>;

    template<typename RX, typename TX, typename Writer = default_writer<RX>, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    using special_post_request = http_request<method_t::special_post, RX, TX, Writer, Reader, Seeker>;



    /*** HELPER FUNCTIONS ***/


    template<method_t Method>
    std::shared_ptr<http_request<Method, nullbuf_t, nullbuf_t>> make_request(const url_t& url){
        return std::make_shared<http_request<Method, nullbuf_t, nullbuf_t>>(url);
    }

    template<method_t Method, typename RX>
    std::shared_ptr<http_request<Method, RX, nullbuf_t>> make_request(RX& buffer, const url_t& url){
        return std::make_shared<http_request<Method, RX, nullbuf_t>>(buffer, url);
    }

    template<method_t Method, typename RX, typename TX>
    std::shared_ptr<http_request<Method, RX, TX>> make_request(RX& rx, TX& tx, const url_t& url){
        return std::make_shared<http_request<Method, RX, TX>>(rx, tx, url);
    }

}


#endif
