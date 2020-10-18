#ifndef CURLHTTP_ASYNC_HANDLE_HPP
#define CURLHTTP_ASYNC_HANDLE_HPP


#include <memory>
#include <unordered_map>
#include <thread>

#ifndef _WIN32
    #include <sys/select.h>
#else
    #include <WinSock2.h>
#endif

#include "detail.hpp"
#include "curl_base.hpp"
#include "option_t.hpp"

namespace curlhttp{

    class async_handle{
    public:
        enum class autoremove_t : char{
            none, remove_failed, remove_success, remove_all
        };

        struct settings_t{
            curl_base* request;
            std::function<void()> done_callback;
        };

        struct prototype_t{
            using easy_option_ptr = std::shared_ptr<basic_easy_option>;

            std::function<void(const std::error_code& )> easy_error_callback;
            std::function<void()> done_callback;
            std::vector<easy_option_ptr> default_options;
            bool throw_easy_errors{true};

            void apply(curl_base& ref) const{
                ref.easy_error_callback = easy_error_callback;
                ref.done_callback = done_callback;
                ref.throw_easy_errors = throw_easy_errors;

                for(auto& opt : default_options)
                    opt->apply(ref);
            }
        };

        using error_callback_t = std::function<void(const std::error_code& )>;
        using done_callback_t = std::function<void(curl_base& )>;

        prototype_t prototype;
        error_callback_t multi_error_callback;
        done_callback_t done_callback;
        autoremove_t autoremove{autoremove_t::none};
        bool throw_multi_errors = true;

        async_handle()
            : handle{curl_multi_init()} {}

        async_handle(async_handle&& ) = default;
        async_handle& operator= (async_handle&& ) = default;

        virtual ~async_handle() {}

        template<typename T>
        void set_option(CURLMoption option, T&& value){
            multi_error_checker(::curl_multi_setopt, option, std::forward<T>(value));
        }

        void set_option(const std::shared_ptr<basic_multi_option>& opt){
            opt->apply(*this);
        }

        template<typename T>
        void add(T& request){
            CURLMcode code;

            if((code = curl_multi_add_handle(handle.get(), request.native())) == CURLM_OK){
                requests[request.native()] = {std::addressof(request), {}};
                apply(request);
            }

            else
                throw curl_multi_error{make_multi_error_code(code)};
        }

        template<typename T, typename Function>
        void add(T& request, Function&& callback = {}){
            CURLMcode code;

            if((code = curl_multi_add_handle(handle.get(), request.native())) == CURLM_OK){
                requests[request.native()] = {
                    std::addressof(request),
                    std::bind(std::forward<Function>(callback), std::ref(request))
                };

                apply(request);
            }

            else
                throw curl_multi_error{make_multi_error_code(code)};
        }

        virtual void apply(curl_base& request){
            prototype.apply(request);
        }

        virtual void remove(curl_base& request){
            if(curl_multi_remove_handle(handle.get(), request.native()) == CURLM_OK)
                requests.erase(request.native());
        }

        virtual void init(){
            for(auto& item : requests)
                item.second.request->init();
        }

        virtual void perform(){
            struct timeval timeout;

            fd_set fdread;
            fd_set fdwrite;
            fd_set fdexcep;

            long ms = -1;
            int still_running;
            int maxfd = -1;
            int rc;

            init();
            multi_error_checker(curl_multi_perform, &still_running);
            process_events();

            while(still_running > 0){
                FD_ZERO(&fdread);
                FD_ZERO(&fdwrite);
                FD_ZERO(&fdexcep);

                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                multi_error_checker(curl_multi_timeout, &ms);

                if(ms >= 0){
                    timeout.tv_sec = ms / 1000;

                    if(timeout.tv_sec > 1)
                        timeout.tv_sec = 1;
                    else
                        timeout.tv_usec = (ms % 1000) * 1000;
                }

                multi_error_checker(curl_multi_fdset, &fdread, &fdwrite, &fdexcep, &maxfd);

                if(maxfd < 0){
                    struct timeval wait = {0, 100 * 1000};
                    rc = select(0, 0, 0, 0, &wait);
                }

                else
                    rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

                if(rc > 0){
                    multi_error_checker(curl_multi_perform, &still_running);
                    process_events();
                }
            }
        }

        virtual void reset(){
            while(requests.size())
                remove(*requests.begin()->second.request);

            handle.reset(curl_multi_init());
            prototype = {};

            multi_error_callback = {};
            done_callback = {};

            autoremove = autoremove_t::none;
            throw_multi_errors = true;
        }

        template<typename T>
        void reuse(T& request){
            multi_error_checker(curl_multi_remove_handle, request.native());
            multi_error_checker(curl_multi_add_handle, request.native());
        }

        void reuse(){
            for(auto& p : requests){
                multi_error_checker(curl_multi_remove_handle, p.second.request->native());
                multi_error_checker(curl_multi_add_handle, p.second.request->native());
            }
        }

    protected:
        std::unordered_map<CURL*, settings_t> requests;

    private:
        std::unique_ptr<CURLM, detail::CURLM_deleter> handle;

        template<typename Function, typename... Args>
        void multi_error_checker(Function&& callback, Args&&... arguments){
            CURLMcode code = std::forward<Function>(callback)(handle.get(), std::forward<Args>(arguments)...);

            if(code != CURLM_OK){
                std::error_code ec{make_multi_error_code(code)};

                if(multi_error_callback)
                    multi_error_callback(ec);

                if(throw_multi_errors)
                    throw curl_multi_error{ec};
            }
        }

        void remove(CURL* request){
            if(curl_multi_remove_handle(handle.get(), request) == CURLM_OK)
                requests.erase(request);
        }

        void handle_removal(CURL* key, bool failed){
            if(autoremove == autoremove_t::none)
                return;

            else if(autoremove == autoremove_t::remove_all)
                remove(key);

            else if(failed && autoremove == autoremove_t::remove_failed)
                remove(key);

            else if(!failed && autoremove == autoremove_t::remove_success)
                remove(key);
        }

        void process_event(CURL* key, CURLcode result){
            auto& settings = requests[key];

            if(settings.request->callback_exception){
                handle_removal(key, true);
                std::rethrow_exception(settings.request->callback_exception);
            }

            if(result != CURLE_OK)
                settings.request->handle_easy_error(make_error_code(result));

            settings.request->exit();

            if(settings.done_callback)
                settings.done_callback();

            if(done_callback)
                done_callback(*settings.request);

            handle_removal(key, false);
        }

        void process_events(){
            CURLMsg* message;
            int nmessages;

            while((message = curl_multi_info_read(handle.get(), &nmessages))){
                if(message->msg == CURLMSG_DONE)
                    process_event(message->easy_handle, message->data.result);
            }
        }
    };


    template<typename T>
    void multi_option<T>::apply(async_handle& handle) const{
        handle.set_option(this->option, this->value);
    }

}


#endif
