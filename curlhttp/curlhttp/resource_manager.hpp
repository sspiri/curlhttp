#ifndef CURLHTTP_RESOURCE_MANAGER_HPP
#define CURLHTTP_RESOURCE_MANAGER_HPP


#include "http_manager.hpp"
#include "utility.hpp"


namespace curlhttp{

    class resource_manager : public http_manager{
    public:
        explicit resource_manager(const url_t& endp)
            : endpoint{endp}, share{curl_share_init()} {}

        resource_manager(resource_manager&& ) = default;
        resource_manager& operator= (resource_manager&& ) = default;

        virtual ~resource_manager() {}

        void init() override{
            setup_share();

            for(auto& item : requests){
                item.second.request->init();
                item.second.request->set_option(CURLOPT_SHARE, share.get());
            }
        }

        void remove(curl_base& request) override{
            http_manager::remove(request);
            request.set_option(CURLOPT_SHARE, 0);
        }

        url_t generate_url(const std::string& rel) const{
            url_t result{endpoint};
            std::string path, query, fragment;

            std::tie(path, query, fragment) = split_relative_url(rel);

            if(path.size()){
                auto temp = result.path();
                temp /= path;
                result.set(CURLUPART_PATH, temp.string());
            }

            if(query.size()){
                auto temp = result.query_string();
                temp += (temp.size() ? '&' + query : query);
                result.set(CURLUPART_QUERY, temp);
            }

            if(fragment.size())
                result.set(CURLUPART_FRAGMENT, fragment);

            return result;
        }

        CURLSH* native_share() const{
            return share.get();
        }

    protected:
        url_t make_url(const std::string& rel) const override{
            return generate_url(rel);
        }

    private:
        url_t endpoint;
        std::unique_ptr<CURLSH, detail::CURLSH_deleter> share;

        template<typename Function, typename... Args>
        void share_error_checker(Function&& callback, Args&&... arguments){
            CURLSHcode code = std::forward<Function>(callback)(share.get(), std::forward<Args>(arguments)...);

            if(code != CURLSHE_OK)
                    throw curl_share_error{make_share_error_code(code)};
        }

        void setup_share(){
            share_error_checker(curl_share_setopt, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
            share_error_checker(curl_share_setopt, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
            share_error_checker(curl_share_setopt, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
            share_error_checker(curl_share_setopt, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
        }
    };

}


#endif
