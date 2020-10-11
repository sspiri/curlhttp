#ifndef CURLHTTP_HTTP_MANAGER_HPP
#define CURLHTTP_HTTP_MANAGER_HPP


#include "async_handle.hpp"
#include "http_request.hpp"


namespace curlhttp{

    class http_manager : public async_handle{
    public:
        struct http_prototype_t{
            using http_error_callback_t = std::function<void(const std::error_code& code)>;
            using status_code_callback_t = std::function<bool(status_code)>;
            using response_callback_t = std::function<bool(const response_t& )>;

            std::string user_agent{get_default_user_agent()};
            http_error_callback_t http_error_callback;
            status_code_callback_t status_code_callback;
            response_callback_t response_callback;
            bool throw_http_errors{true};

            template<typename T>
            void apply(T& request) const{
                request.user_agent = user_agent;

                request.http_error_callback = http_error_callback;
                request.status_code_callback = status_code_callback;
                request.response_callback = response_callback;
                request.throw_http_errors = throw_http_errors;
            }
        };

        http_prototype_t http_prototype;

        http_manager() {}

        http_manager(http_manager&& ) = default;
        http_manager& operator= (http_manager&& ) = default;

        virtual ~http_manager() {}

        template<typename T, typename... Args>
        T& make_rx_buffer(Args&&... arguments){
            auto bufp = buffer_ptr{new T(std::forward<Args>(arguments)...), [](void* p){
                delete (T*)p;
            }};

            rx_buffers.push_back(std::move(bufp));
            return *(T*)rx_buffers.back().get();
        }

        template<typename T, typename... Args>
        T& make_tx_buffer(Args&&... arguments){
            auto bufp = buffer_ptr{new T(std::forward<Args>(arguments)...), [](void* p){
                delete (T*)p;
            }};

            tx_buffers.push_back(std::move(bufp));
            return *(T*)tx_buffers.back().get();
        }

        void remove(curl_base& request) override{
            async_handle::remove(request);

            auto it = std::find_if(handles.begin(), handles.end(), [&request](const curl_ptr& p){
                return p->native() == request.native();
            });

            if(it != handles.end())
                handles.erase(it);

            for(std::size_t n{}; n < rx_buffers.size(); ++n){
                if(request.rx_buffer_ptr() == rx_buffers[n].get()){
                    rx_buffers.erase(rx_buffers.begin() + n);
                    break;
                }
            }

            for(std::size_t n{}; n < tx_buffers.size(); ++n){
                if(request.tx_buffer_ptr() == tx_buffers[n].get()){
                    tx_buffers.erase(tx_buffers.begin() + n);
                    break;
                }
            }
        }

        /*** TRACE ***/

        std::shared_ptr<trace_request> trace(const std::string& url){
            auto p = std::make_shared<trace_request>(url);
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename Function>
        std::shared_ptr<trace_request> trace(const std::string& url, Function&& callback){
            auto p = std::make_shared<trace_request>(url);
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** HEAD ***/

        std::shared_ptr<head_request> head(const std::string& url){
            auto p = std::make_shared<head_request>(make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename Function>
        std::shared_ptr<head_request> head(const std::string& url, Function&& callback){
            auto p = std::make_shared<head_request>(make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** GET ***/

        template<typename RX>
        std::shared_ptr<get_request<RX>> get(RX& rx_buffer, const std::string& url){
            auto p = std::make_shared<get_request<RX>>(rx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename Function>
        std::shared_ptr<get_request<RX>> get(RX& rx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<get_request<RX>>(rx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** POST ***/

        template<typename RX>
        std::shared_ptr<post_request<RX>> post(RX& rx_buffer, const std::string& url){
            auto p = std::make_shared<post_request<RX>>(rx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename Function>
        std::shared_ptr<post_request<RX>> post(RX& rx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<post_request<RX>>(rx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** OPTIONS ***/

        template<typename RX>
        std::shared_ptr<options_request<RX>> options(RX& rx_buffer, const std::string& url){
            auto p = std::make_shared<options_request<RX>>(rx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename Function>
        std::shared_ptr<options_request<RX>> options(RX& rx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<options_request<RX>>(rx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** PUT ***/

        template<typename RX, typename TX>
        std::shared_ptr<put_request<RX, TX>> put(RX& rx_buffer, TX& tx_buffer, const std::string& url){
            auto p = std::make_shared<put_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename TX, typename Function>
        std::shared_ptr<put_request<RX, TX>> put(RX& rx_buffer, TX& tx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<put_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** DELETE ***/

        template<typename RX, typename TX>
        std::shared_ptr<delete_request<RX, TX>> delete_(RX& rx_buffer, TX& tx_buffer, const std::string& url){
            auto p = std::make_shared<delete_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename TX, typename Function>
        std::shared_ptr<delete_request<RX, TX>> delete_(RX& rx_buffer, TX& tx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<delete_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** PATCH ***/

        template<typename RX, typename TX>
        std::shared_ptr<patch_request<RX, TX>> patch(RX& rx_buffer, TX& tx_buffer, const std::string& url){
            auto p = std::make_shared<patch_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename TX, typename Function>
        std::shared_ptr<patch_request<RX, TX>> patch(RX& rx_buffer, TX& tx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<patch_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        /*** SPECIAL POST ***/

        template<typename RX, typename TX>
        std::shared_ptr<special_post_request<RX, TX>> special_post(RX& rx_buffer, TX& tx_buffer, const std::string& url){
            auto p = std::make_shared<special_post_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p);
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

        template<typename RX, typename TX, typename Function>
        std::shared_ptr<special_post_request<RX, TX>> special_post(RX& rx_buffer, TX& tx_buffer, const std::string& url, Function&& callback){
            auto p = std::make_shared<special_post_request<RX, TX>>(rx_buffer, tx_buffer, make_url(url));
            add(*p, std::forward<Function>(callback));
            http_prototype.apply(*p);
            handles.push_back(p);
            return p;
        }

    protected:
        virtual url_t make_url(const std::string& s) const{
            return s;
        }

    private:
        using buffer_ptr = std::unique_ptr<void, void(*)(void*)>;
        using curl_ptr = std::shared_ptr<curl_base>;

        std::vector<buffer_ptr> rx_buffers, tx_buffers;
        std::vector<curl_ptr> handles;
    };

}


#endif
