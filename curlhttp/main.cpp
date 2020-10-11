/*
#include <iostream>
#include <fstream>

#include "curlhttp/curl_handle.hpp"
#include "curlhttp/response_t.hpp"
#include "curlhttp/http_request.hpp"


using namespace curlhttp;


void test_get(){
    get_request<nullbuf_t> request(nullbuf, "https://google.com");
    request.perform();

    auto p = make_request<method_t::get>(std::cout, "https://postman-echo.com/get");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->query.push_back({"q", "term"});
    p->perform();
}


void test_post(){
    auto p = make_request<method_t::post>(std::cout, "https://postman-echo.com/post");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->data.push_back({"q", "term"});
    p->perform();
}


void test_head(){
    head_request request("https://google.com");
    request.perform();

    auto p = make_request<method_t::head>("https://example.net");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->perform();
}


void test_trace(){
    auto p = make_request<method_t::trace>("https://postman-echo.com/trace");
    p->throw_http_errors = false;
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->perform();
}


void test_options(){
    std::cout << std::unitbuf;
    options_request<std::ostream> request{std::cout, "https://example.net"};
    request.set_option(CURLOPT_VERBOSE, true);
    request.perform();
}


void test_delete(){
    std::string s{"{\"message\":\"Hello World!\"}"};
    buffer_t<std::string> input(s);
    auto p = make_request<method_t::delete_>(std::cout, input, "https://postman-echo.com/delete");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->headers.push_back({"Content-Type", "application/json"});
    p->perform();
}


void test_put(){
    std::string s{"{\"message\":\"Hello World!\"}"};
    buffer_t<std::string> input(s);
    auto p = make_request<method_t::put>(std::cout, input, "https://postman-echo.com/put");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->headers.push_back({"Content-Type", "application/json"});
    p->perform();
}


void test_special_post(){
    std::string s{"{\"message\":\"Hello World!\"}"};
    buffer_t<std::string> input(s);
    auto p = make_request<method_t::special_post>(std::cout, input, "https://postman-echo.com/post");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->headers.push_back({"Content-Type", "application/json"});
    p->perform();
}


void test_patch(){
    std::string s{"{\"message\":\"Hello World!\"}"};
    buffer_t<std::string> input(s);
    auto p = make_request<method_t::patch>(std::cout, input, "https://postman-echo.com/patch");
    p->set_option(CURLOPT_VERBOSE, true);
    p->headers.push_back({"DNT", "1"});
    p->headers.push_back({"Content-Type", "application/json"});
    p->perform();
}


static const std::vector<url_t> urls = {
    "https://www.mozilla.org/en-US/firefox/central/",
    "https://support.mozilla.org/en-US/questions/1225249",
    "https://git-scm.com/book/en/v2/Customizing-Git-Git-Configuration",
    "https://unix.stackexchange.com/questions/203048/crypttab-and-veracrypt",
    "https://wallup.net/wp-content/uploads/2018/09/27/195020-psychedelic-bokeh.jpg",
    "https://soundcloud.com/",
    "https://www.youtube.com/",
    "https://www.facebook.com/?_fb_noscript=1",
    "https://lu.indeed.com/",
    "https://en.jobs.lu/",
    "https://lu.linkedin.com/",
    "https://www.c-plusplus.net/forum/",
    "https://www.archlinux.org/",
    "https://mail.google.com/mail/u/0/#inbox",
    "https://www.linuxquestions.org/questions/linux-newbie-8/write-permissions-in-etc-fstab-for-vfat-filesystem-376215/",
    "https://stackoverflow.com/questions/5343068/is-there-a-way-to-cache-github-credentials-for-pushing-commits",
    "https://bbs.archlinux.org/viewtopic.php?id=65534",
    "https://unix.stackexchange.com/questions/36477/how-do-i-prevent-gnome-suspending-while-i-finish-a-compilation-job",
    "https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html",
    "https://www.bleepingcomputer.com/news/security/mozilla-firefox-to-let-you-export-saved-passwords-in-plain-text/",
    "https://www.esante.lu/dsp-patient/com/ideoFront?controller=web.Main#2019_12_17_02_37_e7c895fb-2b81-499b-877a_RUB_SYNTHESE_PATIENT/portlet.ConteneurPortlet//base64:eyJpZCI6IlNZTlRIRVNFX1BBVElFTlQiLCJ3ZWJDb250ZXh0Ijp7ImNsZVBhdGllbnQiOnsiZG9tYWluZSI6eyJpZGVudGlmaWFudCI6IjEuMy4xODIuMi40LjIiLCJ0eXBlIjoiSVNPIn0sImlkZW50aWZpYW50IjoiNjk1NzU2NzY5NiJ9LCJjbGVQYXRpZW50QXV0cmVzRG9tYWluZXMiOlt7ImRvbWFpbmUiOnsiaWRlbnRpZmlhbnQiOiIxLjMuMTgyLjMuMS4xLjE0MzI2NTYyLjIiLCJ0eXBlIjoiSVNPIn0sImlkZW50aWZpYW50IjoiMjA5NzQ3MSJ9LHsiZG9tYWluZSI6eyJpZGVudGlmaWFudCI6IjEuMy4xODIuNC40IiwidHlwZSI6IklTTyJ9LCJpZGVudGlmaWFudCI6IjE5OTYwMzA2MTE1MTYifSx7ImRvbWFpbmUiOnsiaWRlbnRpZmlhbnQiOiIxLjMuMTgyLjIuMi4yLjAiLCJ0eXBlIjoiSVNPIn0sImlkZW50aWZpYW50IjoiMTQwMDQyNjUwMCJ9XSwiZXRhYmxpc3NlbWVudElkIjoxfX0=",
    "https://de.doctena.lu/praxis/Centre_Medical_de_Steinsel-362105",
    "https://access.redhat.com/solutions/1543373",
    "https://wiki.archlinux.de/title/VirtualBox",
    "https://wiki.archlinux.org/index.php/GDM#Log-in_screen_background_image",
    "https://www.microsoft.com/de-de/software-download/windows10ISO",
    "https://www.cyberciti.biz/faq/add-remove-list-linux-kernel-modules/",
    "https://freenode.net/kb/answer/registration",
    "https://doc.qt.io/qt-5/highdpi.html",
    "https://www.thomas-krenn.com/en/wiki/64_bit_guests_in_VirtualBox",
    "https://stackoverflow.com/questions/25288194/dont-display-pushd-popd-stack-across-several-bash-scripts-quiet-pushd-popd",
    "https://askubuntu.com/questions/663187/how-can-i-run-a-program-on-startup-minimized",
    "https://superuser.com/questions/1189467/how-to-copy-text-to-the-clipboard-when-using-wayland",
    "https://askubuntu.com/questions/3697/how-do-i-install-fonts",
    "https://en.cppreference.com/w/cpp/error/system_error",
    "https://wiki.archlinux.org/index.php/XDG_user_directories",
    "https://extensions.gnome.org/",
    "https://strftime.org/",
    "https://www.qtcentre.org/threads/31564-How-to-compile-on-multiple-cores",
    "https://curl.haxx.se/libcurl/c/curl_url_get.html",
    "https://en.cppreference.com/w/cpp/memory/shared_ptr",
    "http://www.cplusplus.com/reference/system_error/error_code/error_code/",
    "https://curl.haxx.se/libcurl/c/curl_url_set.html",
    "https://wiki.archlinux.org/index.php/Network_configuration#Change_interface_name",
    "https://www.netflix.com/browse",
    "https://unix.stackexchange.com/questions/445890/get-network-interface-name-and-mac-address-on-command-line",
    "https://stackoverflow.com/questions/4603886/c-operator-lookup-rules-koenig-lookup",
    "https://stackoverflow.com/questions/12846565/regextype-with-find-command",
    "https://superuser.com/questions/608554/how-can-i-find-a-path-by-providing-only-part-of-the-path",
    "https://jonasjacek.github.io/colors/",
    "https://plumbum.readthedocs.io/en/latest/colors.html",
    "https://www.dict.cc/",
    "https://www.dict.cc/?s=streng",
    "https://ideone.com/H4Uxh6",
    "https://mywiki.wooledge.org/BashPitfalls#pf27",
    "http://mywiki.wooledge.org/BashFAQ/082",
    "https://ideone.com/Pd0EfZ",
    "http://linuxcommand.org/lc3_adv_tput.php",
    "https://ideone.com/RyvxVm",
    "https://www.dict.cc/?s=mangle",
    "https://www.dict.cc/?s=astonished",
    "https://www.cyberciti.biz/faq/linux-find-process-name/",
    "https://linuxhint.com/associative_array_bash/",
    "https://stackoverflow.com/questions/1527049/how-can-i-join-elements-of-an-array-in-bash",
    "https://wiki.archlinux.org/index.php/MySQL",
    "https://wiki.archlinux.org/index.php/MariaDB",
    "https://www.dropbox.com/de/",
    "https://www.dict.cc/?s=bulk",
    "https://wiki.archlinux.org/index.php/desktop_entries#Application_entry",
    "https://crontab.guru/#*_*_*_*_*",
    "https://www.dict.cc/?s=nudge",
    "https://unix.stackexchange.com/questions/363331/why-is-there-no-easy-way-to-list-all-processes-in-a-given-process-group",
    "https://www.dict.cc/?s=verdient",
    "https://curl.haxx.se/mail/lib-2018-10/0112.html",
    "https://github.com/curl/curl/issues/5698",
    "https://stackoverflow.com/questions/45523425/getting-all-the-values-of-an-array-with-jq",
    "https://gist.github.com/chilts/7229605",
    "https://curl.haxx.se/libcurl/c/curl_url_dup.html",
    "https://curl.haxx.se/libcurl/c/curl_url.html",
    "https://curl.haxx.se/libcurl/c/curl_global_init.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html",
    "https://en.cppreference.com/w/cpp/error/nested_exception",
    "https://curl.haxx.se/libcurl/c/curl_easy_perform.html",
    "https://en.cppreference.com/w/cpp/io/ios_base/seekdir",
    "https://curl.haxx.se/libcurl/c/CURLOPT_NOSIGNAL.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_HEADERFUNCTION.html",
    "https://curl.haxx.se/libcurl/c/curl_easy_reset.html",
    "https://curl.haxx.se/libcurl/c/",
    "https://www.informit.com/articles/article.aspx?p=376878",
    "https://curl.haxx.se/libcurl/c/CURLOPT_READFUNCTION.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_SEEKFUNCTION.html",
    "https://curl.haxx.se/libcurl/c/CURLINFO_EFFECTIVE_URL.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_FOLLOWLOCATION.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_CURLU.html",
    "https://www.computerhope.com/unix/bash/mapfile.htm",
    "https://en.cppreference.com/w/cpp/algorithm/equal",
    "https://tools.ietf.org/html/rfc2616",
    "https://tools.ietf.org/html/rfc7540",
    "https://www.computerhope.com/unix.htm",
    "https://www.ionos.de/digitalguide/hosting/hosting-technik/http-header/",
    "https://stackoverflow.com/questions/21960365/invalid-operands-of-types-stdistream-and-char-to-binary-operator",
    "https://developer.mozilla.org/en-US/docs/Glossary/Response_header",
    "https://en.wikipedia.org/wiki/List_of_HTTP_status_codes",
    "https://www.dict.cc/?s=overzealous",
    "https://curl.haxx.se/libcurl/c/curl_easy_upkeep.html",
    "https://ideone.com/",
    "https://httpstatuses.com/",
    "https://stackoverflow.com/questions/39023152/curl-kill-download-on-condition",
    "https://en.cppreference.com/w/cpp/error/rethrow_exception",
    "https://curl.haxx.se/libcurl/c/curl_slist_append.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_CUSTOMREQUEST.html",
    "https://cpp-netlib.org/0.13.0/index.html",
    "https://docs.postman-echo.com/?version=latest",
    "https://github.com/cpp-netlib/cpp-netlib/blob/master/libs/network/example/http_client.cpp",
    "https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/PATCH",
    "https://curl.haxx.se/libcurl/c/CURLOPT_UPLOAD.html",
    "https://curl.haxx.se/mail/lib-2005-04/0141.html",
    "https://www.reddit.com/dev/api/",
    "https://stackoverflow.com/questions/22543724/how-to-send-http-delete-request-using-libcurl",
    "https://stackoverflow.com/questions/6581285/is-a-response-body-allowed-for-a-http-delete-request",
    "https://stackoverflow.com/questions/299628/is-an-entity-body-allowed-for-an-http-delete-request",
    "https://developer.mozilla.org/de/docs/Web/HTTP/Methods/DELETE",
    "https://curl.haxx.se/libcurl/c/CURLOPT_NOBODY.html",
    "https://en.cppreference.com/w/cpp/error/generic_category",
    "http://www.cplusplus.com/reference/system_error/error_category/message/",
    "https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/TRACE",
    "https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/OPTIONS",
    "https://stackoverflow.com/questions/14481850/how-to-send-a-http-options-request-from-the-command-line/14489636",
    "http://www.cplusplus.com/reference/system_error/error_category/",
    "https://en.cppreference.com/w/cpp/string/basic_string_view",
    "https://i.imgur.com/4Pl6Xda.jpg",
    "https://en.cppreference.com/w/cpp/string/basic_string_view/basic_string_view",
    "https://curl.haxx.se/libcurl/c/curl_version.html",
    "https://curl.haxx.se/libcurl/c/curl_version_info.html",
    "https://curl.haxx.se/libcurl/c/curl_easy_setopt.html",
    "https://curl.haxx.se/libcurl/c/CURLOPT_REQUEST_TARGET.html"
};


int main(){
    std::size_t counter{};

    for(const auto& url : urls){
        auto p = make_request<method_t::get>(nullbuf, url);

        auto print_error_callback = [p](const std::error_code& code){
            std::cerr << p->url.string() << ": " << code.message() << '\n';
        };

        p->throw_easy_errors = false;
        p->throw_http_errors = false;

        p->easy_error_callback = print_error_callback;
        p->http_error_callback = print_error_callback;

        p->done_callback = [&, p]{
            std::cerr << '[' << ++counter << '/' << urls.size() << "]: " << p->url.string() << '\n';
        };

        p->response_callback = [p](const response_t& ){
            std::cout << "Status code: " << (int)p->get_status_code() << '\n';
            return false;
        };

        p->perform();
    }
}
*/


#include <iostream>

#include "curlhttp/http_request.hpp"
#include "curlhttp/async_handle.hpp"
#include "curlhttp/http_manager.hpp"
#include "curlhttp/resource_manager.hpp"
#include "curlhttp/multipart_request.hpp"


using namespace curlhttp;


const std::vector<std::string> ips = {
    "173.44.37.82",
    "173.44.37.82",
    "128.199.202.122",
    "128.199.202.122",
    "111.230.138.177",
    "111.230.138.177",
    "125.34.21.155",
    "125.34.21.155",
    "115.213.60.57",
    "115.213.60.57",
    "47.112.230.91",
    "47.112.230.91",
    "115.150.58.194",
    "115.150.58.194",
    "120.79.56.27",
    "120.79.56.27",
    "120.79.8.169",
    "120.79.8.169",
    "113.195.165.41",
    "41.165.195.113",
    "47.244.17.134",
    "47.244.17.134",
    "49.232.166.21",
    "49.232.166.21",
    "120.79.72.64",
    "120.79.72.64",
    "171.35.169.55",
    "55.169.35.171",
    "181.101.57.248",
    "181.3.208.32",
    "113.110.202.216",
    "113.110.202.216",
    "186.126.167.237",
    "207.97.174.134",
    "207.97.174.134",
    "47.56.193.27",
    "47.56.193.27",
    "219.147.112.150",
    "150.112.147.219",
    "221.232.146.211",
    "221.232.146.211",
    "181.3.110.186",
    "47.114.8.133",
    "47.114.8.133",
    "47.116.108.45",
    "47.116.108.45",
    "60.31.213.115",
    "60.31.213.115",
    "114.236.90.5",
    "114.236.90.5",
    "8.129.220.60",
    "8.129.220.60",
    "78.189.231.24",
    "78.189.231.24",
    "185.183.98.117"
};



int main(){
    http_manager manager;

    manager.prototype.throw_easy_errors = false;
    manager.http_prototype.throw_http_errors = false;

    auto error_callback = [](const std::error_code& ec){
        if(ec.value() != CURLE_OPERATION_TIMEDOUT)
            std::cerr << ec.message() << '\n';
    };

    manager.prototype.easy_error_callback = error_callback;
    manager.http_prototype.http_error_callback = error_callback;

    for(const auto& ip : ips){
        auto get = manager.get(nullbuf, "https://google.com", [ip](auto& request){
            if(request.get_status_code() == status_code::ok)
                std::cerr << ip << '\n';
        });

        get->timeout_callback = [](){
            std::cerr << "Timed out.\n";
        };

        get->set_option(CURLOPT_PROXY, ("socks5://" + ip + ":1080").c_str());
        get->set_option(CURLOPT_TIMEOUT_MS, 5000);
    }

    manager.perform();


    /*
    resource_manager manager{"https://ipinfo.io"};

    manager.prototype.default_options.push_back(make_easy_option(CURLOPT_VERBOSE, true));
    manager.http_prototype.throw_http_errors = false;

    manager.get(manager.make_rx_buffer<std::string>(), "/ip", [](auto& request){
        std::cerr << request.rx_buffer << '\n';
    });

    manager.perform();
    */
}
