#ifndef CURLHTTP_DETAIL_HPP
#define CURLHTTP_DETAIL_HPP


#include <ios>
#include <istream>
#include <string>
#include <vector>

#include <cassert>
#include <curl/curl.h>

#include "nullbuf_t.hpp"


namespace curlhttp{
namespace detail{

    struct CURLU_deleter{
        void operator()(CURLU* p){
            curl_url_cleanup(p);
        }
    };


    struct CURL_deleter{
        void operator()(CURL* p){
            curl_easy_cleanup(p);
        }
    };


    struct CURLM_deleter{
        void operator()(CURLM* p){
            curl_multi_cleanup(p);
        }
    };


    struct CURLSH_deleter{
        void operator()(CURLSH* p){
            curl_share_cleanup(p);
        }
    };


    struct curl_slist_deleter{
        void operator()(curl_slist* list){
            curl_slist_free_all(list);
        }
    };


    struct curl_mime_deleter{
        void operator()(curl_mime* mime){
            curl_mime_free(mime);
        }
    };


    inline std::ios::seekdir curlseek2std(int origin){
        switch(origin){
        case SEEK_SET:
            return std::ios::beg;
        case SEEK_CUR:
            return std::ios::cur;
        case SEEK_END:
            return std::ios::end;
        }

        assert(((void)"curl specific implementation error", false));
    }

}
}


#endif
