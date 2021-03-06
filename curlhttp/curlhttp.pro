CONFIG += console c++17
CONFIG -= app_bundle qt

unix:QMAKE_CXXFLAGS += -std=c++17
unix:LIBS += -lcurl

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

win32:DEFINES+="CURL_STATICLIB"
win32:LIBS += -L$$PWD/../../curl/lib/ -llibcurl_a -lws2_32 -lwldap32 -lcrypt32 -lnormaliz -ladvapi32
win32:INCLUDEPATH += $$PWD/../../curl/include
win32:DEPENDPATH += $$PWD/../../curl/include
