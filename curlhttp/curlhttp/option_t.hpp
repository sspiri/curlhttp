#ifndef CURLHTTP_OPTION_T_HPP
#define CURLHTTP_OPTION_T_HPP


#include <utility>
#include <memory>
#include <curl/curl.h>


namespace curlhttp{
    class curl_base;
    class async_handle;


    template<typename T, typename Handle>
    struct basic_option{
        using option_t = T;
        using handle_t = Handle;

        option_t option;

        explicit basic_option(option_t opt)
            : option{opt} {}

        virtual ~basic_option() {}

        virtual void apply(handle_t& ) const = 0;

        bool operator!= (const basic_option& rhs) const{
            return option != rhs.option;
        }

        bool operator== (const basic_option& rhs) const{
            return !operator!= (rhs);
        }
    };


    struct basic_easy_option : public basic_option<CURLoption, curl_base>{
        using basic_option<CURLoption, curl_base>::basic_option;

        virtual ~basic_easy_option() {}
    };


    struct basic_multi_option : public basic_option<CURLMoption, async_handle>{
        using basic_option<CURLMoption, async_handle>::basic_option;

        virtual ~basic_multi_option() {}
    };


    template<typename T>
    struct easy_option : basic_easy_option{
        T value;

        easy_option(option_t opt, T&& val)
            : basic_easy_option{opt}, value{std::forward<T>(val)} {}

        void apply(handle_t& handle) const override;

        bool operator!= (const easy_option& rhs) const{
            return basic_easy_option::operator!=(rhs) || value != rhs.value;
        }

        bool operator== (const easy_option& rhs) const{
            return !operator!=(rhs);
        }
    };


    template<typename T>
    struct multi_option : basic_multi_option{
        T value;

        multi_option(option_t opt, T&& val)
            : basic_multi_option{opt}, value{std::forward<T>(val)} {}

        void apply(handle_t& handle) const override;

        bool operator!= (const multi_option& rhs) const{
            return basic_multi_option::operator!=(rhs) || value != rhs.value;
        }

        bool operator== (const multi_option& rhs) const{
            return !operator!=(rhs);
        }
    };


    template<typename T>
    std::shared_ptr<easy_option<T>> make_easy_option(CURLoption option, T&& value){
        return std::make_shared<easy_option<T>>(option, std::forward<T>(value));
    }


    template<typename T>
    std::shared_ptr<multi_option<T>> make_multi_option(CURLMoption option, T&& value){
        return std::make_shared<multi_option<T>>(option, std::forward<T>(value));
    }

}


#endif
