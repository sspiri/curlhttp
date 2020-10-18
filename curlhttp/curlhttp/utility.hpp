#ifndef CURLHTTP_UTILITY_HPP
#define CURLHTTP_UTILITY_HPP


#include <locale>
#include <algorithm>
#include <string>

#include <curl/curl.h>

#include "field_t.hpp"
#include "query_t.hpp"


namespace curlhttp{

    inline std::string escape(const std::string& s){
        char* p = curl_easy_escape(nullptr, s.c_str(), (int)s.size());
        std::string result{p};
        curl_free(p);
        return result;
    }


    inline std::string unescape(const std::string& s){
        int length;
        char* p = curl_easy_unescape(nullptr, s.c_str(), (int)s.size(), &length);
        std::string result{p};
        curl_free(p);
        result.resize(length);
        return result;
    }


    inline bool icase_compare(const std::string& s1, const std::string& s2, const std::locale& loc = std::locale{}){
        return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(), [&loc](char c1, char c2){
            return std::toupper(c1, loc) == std::toupper(c2, loc);
        });
    }


    inline std::string query_string(const std::vector<field_t>& fields){
        std::string s;

        for(const auto& field : fields)
            s += field.name + '=' + field.value + '&';

        if(s.size())
            s.pop_back();

        return s;
    }


    inline std::tuple<std::string, std::string, std::string> split_relative_url(const std::string& s){
        std::size_t path_end = s.find('?');
        std::size_t query_end = s.find('#', path_end == s.npos ? 0 : path_end);

        std::string path = s.substr(0, path_end);
        std::string query, fragment;

        if(path_end != s.npos)
            query = s.substr(path_end + 1, query_end == s.npos ? s.npos : query_end - path_end - 1);

        if(query_end != s.npos)
            fragment = s.substr(query_end + 1);

        return std::make_tuple(std::move(path), std::move(query), std::move(fragment));
    }


    inline void append_query(query_t& query, const query_t& append){
        query.reserve(query.size() + append.size());
        std::copy(append.begin(), append.end(), std::back_inserter<query_t>(query));
    }

}


#endif
