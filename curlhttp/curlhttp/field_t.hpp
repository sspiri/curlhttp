#ifndef CURLHTTP_FIELD_T_HPP
#define CURLHTTP_FIELD_T_HPP


#include <string>
#include "curl_error.hpp"


namespace curlhttp{

    struct field_t{
        std::string name, value;

        void parse_query(const std::string& s){
            clear();
            std::size_t pos = s.find('=');

            if(pos == s.npos)
                throw curl_error{make_error_code(CURLE_READ_ERROR), "Cannot parse query"};

            name = s.substr(0, pos);
            value = s.substr(++pos);
        }

        void parse_header(const std::string& s){
            clear();
            std::size_t pos = s.find(": ");

            if(pos == s.npos && (pos = s.find(":\t")) == s.npos)
                throw curl_error{make_error_code(CURLE_READ_ERROR), "Cannot parse header"};

            name = s.substr(0, pos);
            value = s.substr(pos += 2);
        }

        void clear(){
            name.clear();
            value.clear();
        }

        bool operator!= (const field_t& rhs) const{
            return name != rhs.name || value != rhs.value;
        }

        bool operator== (const field_t& rhs) const{
            return !operator!=(rhs);
        }
    };

}


#endif
