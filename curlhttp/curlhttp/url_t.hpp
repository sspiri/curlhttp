#ifndef CURLHTTP_URL_T_HPP
#define CURLHTTP_URL_T_HPP


#include <memory>
#include <string>
#include <optional>

#include "detail.hpp"
#include "curl_error.hpp"
#include "path_t.hpp"
#include "query_t.hpp"


namespace curlhttp{

    class url_t{
    public:
        url_t()
            : url{curl_url(), detail::CURLU_deleter{}} {}

        url_t(const std::string& uri, CURLUPart part = CURLUPART_URL, unsigned flags = 0)
            : url{curl_url(), detail::CURLU_deleter{}}{

            set(part, uri, flags);
        }

        url_t(const char* uri, CURLUPart part = CURLUPART_URL, unsigned flags = 0)
            : url_t{std::string{uri}, part, flags} {}

        url_t(const url_t& rhs)
            : url{curl_url_dup(rhs.native()), detail::CURLU_deleter{}} {}

        url_t& operator= (const url_t& rhs){
            url = {curl_url_dup(rhs.native()), detail::CURLU_deleter{}};
            return *this;
        }

        url_t(url_t&& ) = default;
        url_t& operator= (url_t&& ) = default;

        url_t& operator= (const std::string& uri){
            set(CURLUPART_URL, uri);
            return *this;
        }

        url_t& operator= (const char* uri){
            set(CURLUPART_URL, uri);
            return *this;
        }

        void set(CURLUPart part, const std::string& s, unsigned flags = 0){
            CURLUcode code = curl_url_set(url.get(), part, s.c_str(), flags);

            if(code)
                throw curl_url_error{make_url_error_code(code)};
        }

        std::optional<std::string> get(CURLUPart part, unsigned flags = 0) const{
            char* result{};
            CURLUcode code = curl_url_get(url.get(), part, &result, flags);

            if(code){
                curl_free(result);
                return {};
            }

            std::string res{result};
            curl_free(result);

            return res;
        }

        bool is_valid(CURLUPart part = CURLUPART_URL) const{
            char* unused;
            auto code = curl_url_get(url.get(), part, &unused, 0);
            curl_free(unused);
            return code == CURLUE_OK;
        }

        bool is_absolute() const{
            return is_valid() && (bool)get(CURLUPART_HOST);
        }

        bool is_relative() const{
            return !is_absolute() && (bool)get(CURLUPART_PATH);
        }

        std::string string(int flags = 0) const{
            if(is_absolute())
                return *get(CURLUPART_URL, flags);

            else if(is_relative()){
                auto path = get(CURLUPART_PATH, flags);
                auto query = get(CURLUPART_QUERY, flags);
                auto fragment = get(CURLUPART_FRAGMENT, flags);

                return *path + (query ? *query : "") + (fragment ? *fragment : "");
            }

            return "";
        }

        path_t path(int flags = 0) const{
            if(auto p = get(CURLUPART_PATH, flags))
                return *p;

            return path_t{};
        }

        std::string query_string(int flags = 0) const{
            if(auto p = get(CURLUPART_QUERY, flags))
                return *p;

            return "";
        }

        query_t query(int flags = 0) const{
            if(auto p = get(CURLUPART_QUERY, flags)){
                std::string s = *p + '&';
                std::size_t beg{}, pos{};
                field_t field;
                query_t query;

                while((pos = s.find('&', pos)) != s.npos){
                    std::string sub = s.substr(beg, pos - beg);

                    if(!sub.size())
                        break;

                    field.parse_query(sub);
                    beg = ++pos;

                    query.push_back(std::move(field));
                }

                return query;
            }

            return query_t{};
        }

        std::string fragment(int flags = 0) const{
            if(auto p = get(CURLUPART_FRAGMENT, flags))
                return *p;

            return "";
        }

        void clear(CURLUPart part = CURLUPART_URL){
            if(part == CURLUPART_URL)
                url.reset(curl_url(), detail::CURLU_deleter{});

            else
                curl_url_set(url.get(), part, nullptr, 0);
        }

        CURLU* native() const{
            return url.get();
        }

        bool operator!= (const url_t& rhs) const{
            return !operator==(rhs);
        }

        bool operator== (const url_t& rhs) const{
            return is_valid() && rhs.is_valid() && string() == rhs.string();
        }

    private:
        std::shared_ptr<CURLU> url;
    };

}


#endif
