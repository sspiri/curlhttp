#ifndef CURLHTTP_DEFAULT_SEEKER_HPP
#define CURLHTTP_DEFAULT_SEEKER_HPP


#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>

#include "detail.hpp"
#include "buffer_t.hpp"
#include "nullbuf_t.hpp"


namespace curlhttp{

    template<typename T>
    struct default_seeker {};


    template<>
    struct default_seeker<nullbuf_t>{
        int operator()(const nullbuf_t& , curl_off_t , int ){
            return CURL_SEEKFUNC_OK;
        }
    };


    template<typename T>
    struct default_seeker<buffer_t<T>>{
        int operator()(buffer_t<T>& buffer, curl_off_t offset, int origin){
            switch(origin){
                case SEEK_SET:
                    buffer.read = (std::size_t)offset;
                    return CURL_SEEKFUNC_OK;

                case SEEK_CUR:
                    buffer.read += (std::size_t)offset;
                    return CURL_SEEKFUNC_OK;

                case SEEK_END:
                    buffer.read = buffer.container.size() - offset;
                    return CURL_SEEKFUNC_OK;
            }

            return CURL_SEEKFUNC_FAIL;
        }
    };


    template<>
    struct default_seeker<std::iostream>{
        int operator()(std::iostream& stream, curl_off_t offset, int origin){
            if(stream.seekg(offset, detail::curlseek2std(origin)))
                return CURL_SEEKFUNC_OK;

            return CURL_SEEKFUNC_FAIL;
        }
    };


    template<>
    struct default_seeker<std::istream>{
        int operator()(std::istream& stream, curl_off_t offset, int origin){
            if(stream.seekg(offset, detail::curlseek2std(origin)))
                return CURL_SEEKFUNC_OK;

            return CURL_SEEKFUNC_FAIL;
        }
    };


    template<>
    struct default_seeker<std::fstream>{
        int operator()(std::fstream& stream, curl_off_t offset, int origin){
            if(stream.seekg(offset, detail::curlseek2std(origin)))
                return CURL_SEEKFUNC_OK;

            return CURL_SEEKFUNC_FAIL;
        }
    };


    template<>
    struct default_seeker<std::ifstream>{
        int operator()(std::iostream& stream, curl_off_t offset, int origin){
            if(stream.seekg(offset, detail::curlseek2std(origin)))
                return CURL_SEEKFUNC_OK;

            return CURL_SEEKFUNC_FAIL;
        }
    };


    template<>
    struct default_seeker<std::stringstream>{
        int operator()(std::iostream& stream, curl_off_t offset, int origin){
            if(stream.seekg(offset, detail::curlseek2std(origin)))
                return CURL_SEEKFUNC_OK;

            return CURL_SEEKFUNC_FAIL;
        }
    };


    template<>
    struct default_seeker<std::istringstream>{
        int operator()(std::iostream& stream, curl_off_t offset, int origin){
            if(stream.seekg(offset, detail::curlseek2std(origin)))
                return CURL_SEEKFUNC_OK;

            return CURL_SEEKFUNC_FAIL;
        }
    };

}


#endif
