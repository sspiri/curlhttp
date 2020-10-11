#ifndef CURL_ERROR_HPP
#define CURL_ERROR_HPP


#include <system_error>
#include <curl/curl.h>
#include "status_code.hpp"


namespace curlhttp{

    static struct curl_easy_category_t : std::error_category{
        const char* name() const noexcept override{
            return "curl_easy";
        }

        virtual std::string message(int value) const override{
            return curl_easy_strerror((CURLcode)value);
        }
    } curl_easy_category;


    static struct curl_multi_category_t : std::error_category{
        const char* name() const noexcept override{
            return "curl_multi";
        }

        virtual std::string message(int value) const override{
            return curl_multi_strerror((CURLMcode)value);
        }
    } curl_multi_category;


    static struct curl_share_category_t : std::error_category{
        const char* name() const noexcept override{
            return "curl_share";
        }

        virtual std::string message(int value) const override{
            return curl_share_strerror((CURLSHcode)value);
        }
    } curl_share_category;


    static struct curl_url_category_t : std::error_category{
        const char* name() const noexcept override{
            return "curl_url";
        }

        virtual std::string message(int value) const override{
            switch((CURLUcode)value){
                case CURLUE_OK:                 return "No error";
                case CURLUE_BAD_HANDLE:         return "Bad url handle";
                case CURLUE_BAD_PARTPOINTER:    return "Bad partpointer";
                case CURLUE_MALFORMED_INPUT:    return "Malformed input";
                case CURLUE_BAD_PORT_NUMBER:    return "Bad port number";
                case CURLUE_UNSUPPORTED_SCHEME: return "Unsupported scheme";
                case CURLUE_URLDECODE:          return "Url decoding error";
                case CURLUE_OUT_OF_MEMORY:      return "Out of memory";
                case CURLUE_USER_NOT_ALLOWED:   return "User not allowed";
                case CURLUE_UNKNOWN_PART:       return "Unknown url part";
                case CURLUE_NO_HOST:            return "No url hostname specified";
                case CURLUE_NO_PORT:            return "No url port specified";
                case CURLUE_NO_SCHEME:          return "No url scheme specified";
                case CURLUE_NO_USER:            return "No url user specified";
                case CURLUE_NO_QUERY:           return "No url query specified";
                case CURLUE_NO_OPTIONS:         return "No url options specified";
                case CURLUE_NO_FRAGMENT:        return "No url fragment specified";
                case CURLUE_NO_PASSWORD:        return "No url password specified";

                default:
                    return "Unknown url error";
            }
        }
    } curl_url_category;


    static struct http_category_t : std::error_category{
        const char* name() const noexcept override{
            return "http";
        }

        virtual std::string message(int value) const override{
            return status_code_string((status_code)value);
        }
    } http_category;

    struct curl_error : std::system_error{
        using std::system_error::system_error;

        virtual ~curl_error() {}
    };


    struct curl_multi_error : curl_error{
        using curl_error::curl_error;
    };


    struct curl_share_error : curl_error{
        using curl_error::curl_error;
    };


    struct curl_url_error : curl_error{
        using curl_error::curl_error;
    };


    struct http_error : curl_error{
        using curl_error::curl_error;
    };



    inline std::error_code make_error_code(CURLcode code){
        return std::error_code{code, curl_easy_category};
    }

    inline std::error_code make_multi_error_code(CURLMcode code){
        return std::error_code{code, curl_multi_category};
    }

    inline std::error_code make_share_error_code(CURLSHcode code){
        return std::error_code{code, curl_share_category};
    }

    inline std::error_code make_url_error_code(CURLUcode code){
        return std::error_code{code, curl_url_category};
    }

    inline std::error_code make_http_error_code(status_code code){
        return std::error_code{(int)code, http_category};
    }
}


#endif
