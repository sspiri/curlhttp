#ifndef CURLHTTP_DEFAULT_WRITER_T_HPP
#define CURLHTTP_DEFAULT_WRITER_T_HPP


#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>

#include "nullbuf_t.hpp"


namespace curlhttp{

    template<typename T>
    struct default_writer;


    template<>
    struct default_writer<nullbuf_t>{
        std::size_t operator()(const nullbuf_t& , const char* , std::size_t size, std::size_t nmemb){
            return size *= nmemb;
        }
    };


    template<typename T, typename... Args, template<typename, typename...> class Container>
    struct default_writer<Container<T, Args...>>{
        std::size_t operator()(Container<T, Args...>& container, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            container.insert(container.end(), buffer, buffer + buffer_size);
            return buffer_size;
        }
    };


    template<>
    struct default_writer<std::iostream>{
        std::size_t operator()(std::iostream& stream, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            stream.write(buffer, (std::streamsize)buffer_size);
            return buffer_size;
        }
    };


    template<>
    struct default_writer<std::ostream>{
        std::size_t operator()(std::ostream& stream, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            stream.write(buffer, (std::streamsize)buffer_size);
            return buffer_size;
        }
    };


    template<>
    struct default_writer<std::fstream>{
        std::size_t operator()(std::fstream& stream, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            stream.write(buffer, (std::streamsize)buffer_size);
            return buffer_size;
        }
    };


    template<>
    struct default_writer<std::ofstream>{
        std::size_t operator()(std::ofstream& stream, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            stream.write(buffer, (std::streamsize)buffer_size);
            return buffer_size;
        }
    };


    template<>
    struct default_writer<std::stringstream>{
        std::size_t operator()(std::stringstream& stream, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            stream.write(buffer, (std::streamsize)buffer_size);
            return buffer_size;
        }
    };


    template<>
    struct default_writer<std::ostringstream>{
        std::size_t operator()(std::ostringstream& stream, const char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            stream.write(buffer, (std::streamsize)buffer_size);
            return buffer_size;
        }
    };
}


#endif
