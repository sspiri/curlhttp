#ifndef SIZE_GETTER_HPP
#define SIZE_GETTER_HPP


#include <iostream>
#include <fstream>
#include <sstream>

#include "nullbuf_t.hpp"


namespace curlhttp{
    template<typename T>
    struct size_getter;


    template<>
    struct size_getter<nullbuf_t>{
        std::size_t operator()(const nullbuf_t& ) const{
            return 0;
        }
    };


    template<typename T, typename... Args, template<typename, typename...> class Container>
    struct size_getter<Container<T, Args...>>{
        std::size_t operator()(const Container<T, Args...>& container) const{
            return container.size();
        }
    };


    template<>
    struct size_getter<std::iostream>{
        std::size_t operator()(std::iostream& stream) const{
            auto current = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto size = stream.tellg() - current;
            stream.seekg(current, std::ios::beg);
            return size;
        }
    };


    template<>
    struct size_getter<std::istream>{
        std::size_t operator()(std::istream& stream) const{
            auto current = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto size = stream.tellg() - current;
            stream.seekg(current, std::ios::beg);
            return size;
        }
    };


    template<>
    struct size_getter<std::fstream>{
        std::size_t operator()(std::fstream& stream) const{
            auto current = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto size = stream.tellg() - current;
            stream.seekg(current, std::ios::beg);
            return size;
        }
    };


    template<>
    struct size_getter<std::ifstream>{
        std::size_t operator()(std::ifstream& stream) const{
            auto current = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto size = stream.tellg() - current;
            stream.seekg(current, std::ios::beg);
            return size;
        }
    };


    template<>
    struct size_getter<std::stringstream>{
        std::size_t operator()(std::stringstream& stream) const{
            auto current = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto size = stream.tellg() - current;
            stream.seekg(current, std::ios::beg);
            return size;
        }
    };


    template<>
    struct size_getter<std::istringstream>{
        std::size_t operator()(std::istringstream& stream) const{
            auto current = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto size = stream.tellg() - current;
            stream.seekg(current, std::ios::beg);
            return size;
        }
    };
}


#endif
