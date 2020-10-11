#ifndef CURLHTTP_CURL_BASE_HPP
#define CURLHTTP_CURL_BASE_HPP


#include <memory>
#include <functional>
#include <curl/curl.h>

#include "detail.hpp"
#include "url_t.hpp"
#include "curl_error.hpp"
#include "option_t.hpp"
#include<iostream>

namespace curlhttp{

    class curl_base{
        friend class async_handle;

    public:
        static constexpr std::size_t default_write_abort = CURL_MAX_WRITE_SIZE + 1;
        static constexpr std::size_t default_read_abort = CURL_MAX_READ_SIZE + 1;

        using callback_t = std::function<void()>;
        using error_callback_t = std::function<void(const std::error_code& )>;

        url_t url;
        callback_t done_callback, timeout_callback;
        error_callback_t easy_error_callback;
        bool throw_easy_errors{true};

        virtual void init(){
            callback_exception = {};

            setup_upload();
            setup_header_download();
            setup_download();
            setup_seeking();

            set_option(CURLOPT_HEADER, false);
            set_option(CURLOPT_URL, url.string().c_str());
        }

        virtual void reset(){
            curl_easy_reset(handle.get());
            url.clear();

            done_callback = {};
            timeout_callback = {};
            easy_error_callback = {};

            throw_easy_errors = true;
            last_error = std::error_code{};
        }

        virtual void perform(){
            init();

            last_error = make_error_code(curl_easy_perform(handle.get()));

            if(callback_exception)
                std::rethrow_exception(callback_exception);

            if(last_error.value() != CURLE_OK)
                handle_easy_error(last_error);

            exit();
        }

        virtual void exit(){
            char* result;
            get_info(CURLINFO_EFFECTIVE_URL, result);
            url = result;

            if(done_callback && last_error.value() == CURLE_OK)
                done_callback();

            else if(timeout_callback && last_error.value() == CURLE_OPERATION_TIMEDOUT)
                timeout_callback();
        }

        const std::error_code& get_last_error() const{
            return last_error;
        }

        template<typename T>
        void set_option(CURLoption option, const T& value){
            easy_error_checker(::curl_easy_setopt, option, value);
        }

        void set_option(const std::shared_ptr<basic_easy_option>& opt){
            opt->apply(*this);
        }

        template<typename T>
        void get_info(CURLINFO info, T& result){
            easy_error_checker(::curl_easy_getinfo, info, std::addressof(result));
        }

        virtual void* rx_buffer_ptr() const = 0;
        virtual void* tx_buffer_ptr() const = 0;

        CURL* native() const{
            return handle.get();
        }

    protected:
        std::exception_ptr callback_exception;
        std::unique_ptr<CURL, detail::CURL_deleter> handle;

        curl_base(const url_t& uri)
            : url{uri}, handle{curl_easy_init()} {}

        curl_base(curl_base&& ) = default;
        curl_base& operator= (curl_base&& ) = default;

        virtual ~curl_base() {}

        void handle_easy_error(const std::error_code& ec){
            if(easy_error_callback)
                easy_error_callback(ec);

            if(ec.value() != CURLE_OPERATION_TIMEDOUT && throw_easy_errors)
                throw curl_error{ec};
        }

        template<typename Function, typename... Args>
        void easy_error_checker(Function&& callback, Args&&... arguments){
            CURLcode code = std::forward<Function>(callback)(handle.get(), std::forward<Args>(arguments)...);
            last_error = make_error_code(code);

            if(code != CURLE_OK)
                handle_easy_error(make_error_code(code));
        }

        virtual void setup_upload() = 0;
        virtual void setup_header_download() = 0;
        virtual void setup_download() = 0;
        virtual void setup_seeking() = 0;

    private:
        std::error_code last_error;
    };



    template<typename T>
    void easy_option<T>::apply(curl_base& handle) const{
        handle.set_option(this->option, this->value);
    }

}


#endif
