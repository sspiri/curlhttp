#ifndef CURLHTTP_RESPONSE_T_HPP
#define CURLHTTP_RESPONSE_T_HPP


#include <string>
#include <vector>

#include "utility.hpp"
#include "status_code.hpp"
#include "field_t.hpp"


namespace curlhttp{

    struct response_t{
        std::string version;
        status_code code;
        std::string message;
        std::vector<field_t> fields;

        void clear(){
            version = "";
            message = "";
            code = (status_code)0;
            fields.clear();
        }

        std::string string() const{
            std::string s = version + ' ' + std::to_string((int)code) + ' ' + status_code_string(code) + "\r\n";

            for(const auto& field : fields)
                s += field.name + ": " + field.value + "\r\n";

            return s += "\r\n";
        }

        std::ptrdiff_t find(const std::string& key) const{
            for(std::size_t n{}; n < fields.size(); ++n){
                if(icase_compare(fields[n].name, key))
                    return (std::ptrdiff_t)n;
            }

            return -1;
        }

        bool operator!= (const response_t& rhs) const{
            return version != rhs.version || code != rhs.code || message != rhs.message || fields != rhs.fields;
        }

        bool operator== (const response_t& rhs) const{
            return !operator!=(rhs);
        }
    };


    inline std::istream& operator>> (std::istream& stream, response_t& response){
        response.clear();

        if(!(stream >> response.version >> (int&)response.code && std::getline(stream, response.message)))
            return stream;

        std::string line;
        field_t field;

        while(std::getline(stream, line)){
            if(line == "\r")
                break;

            field.parse_header(line);
            response.fields.push_back(std::move(field));
        }

        return stream;
    }

}


#endif
