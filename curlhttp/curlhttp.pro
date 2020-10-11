TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle qt

QMAKE_CXXFLAGS += -std=c++17
LIBS += -lcurl

SOURCES += \
        main.cpp

HEADERS += \
    curlhttp/async_handle.hpp \
    curlhttp/buffer_t.hpp \
    curlhttp/curl_base.hpp \
    curlhttp/curl_error.hpp \
    curlhttp/curl_handle.hpp \
    curlhttp/curlhttp.hpp \
    curlhttp/default_reader.hpp \
    curlhttp/default_seeker.hpp \
    curlhttp/default_writer.hpp \
    curlhttp/detail.hpp \
    curlhttp/field_t.hpp \
    curlhttp/html.hpp \
    curlhttp/http_manager.hpp \
    curlhttp/http_request.hpp \
    curlhttp/json.hpp \
    curlhttp/method_t.hpp \
    curlhttp/mime_data.hpp \
    curlhttp/mime_holder.hpp \
    curlhttp/multipart_request.hpp \
    curlhttp/nullbuf_t.hpp \
    curlhttp/option_t.hpp \
    curlhttp/path_t.hpp \
    curlhttp/query_t.hpp \
    curlhttp/resource_manager.hpp \
    curlhttp/response_t.hpp \
    curlhttp/size_getter.hpp \
    curlhttp/status_code.hpp \
    curlhttp/url_t.hpp \
    curlhttp/utility.hpp
