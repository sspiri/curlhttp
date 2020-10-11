#ifndef CURLHTTP_DEFAULT_READER_HPP
#define CURLHTTP_DEFAULT_READER_HPP


#include <cstddef>
#include <cstring>

#include <iostream>
#include <fstream>
#include <sstream>

#include "buffer_t.hpp"
#include "nullbuf_t.hpp"
#include "size_getter.hpp"


namespace curlhttp{

    template<typename T>
    struct default_reader;


    template<>
    struct default_reader<nullbuf_t>{
        std::size_t operator()(const nullbuf_t& , char* , std::size_t size, std::size_t nmemb){
            return size *= nmemb;
        }
    };


    template<typename T>
    struct default_reader<buffer_t<T>>{
        std::size_t operator()(buffer_t<T>& buffer, char* cbuf, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(buffer.size(), buffer_size);
            std::memcpy(cbuf, &buffer.container[buffer.read], nread);
            buffer.read += nread;
            return nread;
        }
    };


    template<>
    struct default_reader<std::iostream>{
        std::size_t operator()(std::iostream& stream, char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(size_getter<std::iostream>{}(stream), buffer_size);

            stream.read(buffer, (std::streamsize)nread);
            return nread;
        }
    };


    template<>
    struct default_reader<std::istream>{
        std::size_t operator()(std::istream& stream, char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(size_getter<std::istream>{}(stream), buffer_size);

            stream.read(buffer, (std::streamsize)nread);
            return nread;
        }
    };


    template<>
    struct default_reader<std::fstream>{
        std::size_t operator()(std::fstream& stream, char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(size_getter<std::fstream>{}(stream), buffer_size);

            stream.read(buffer, (std::streamsize)nread);
            return nread;
        }
    };


    template<>
    struct default_reader<std::ifstream>{
        std::size_t operator()(std::ifstream& stream, char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(size_getter<std::ifstream>{}(stream), buffer_size);

            stream.read(buffer, (std::streamsize)nread);
            return nread;
        }
    };


    template<>
    struct default_reader<std::stringstream>{
        std::size_t operator()(std::stringstream& stream, char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(size_getter<std::stringstream>{}(stream), buffer_size);

            stream.read(buffer, (std::streamsize)nread);
            return nread;
        }
    };


    template<>
    struct default_reader<std::istringstream>{
        std::size_t operator()(std::istringstream& stream, char* buffer, std::size_t size, std::size_t nmemb){
            std::size_t buffer_size = size * nmemb;
            std::size_t nread = std::min(size_getter<std::istringstream>{}(stream), buffer_size);

            stream.read(buffer, (std::streamsize)nread);
            return nread;
        }
    };
}


#endif
