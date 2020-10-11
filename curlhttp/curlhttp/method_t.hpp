#ifndef METHOD_T_HPP
#define METHOD_T_HPP


namespace curlhttp{
    enum class method_t : char{
        none, get, head, options, post, special_post, put, patch, delete_, trace
    };
}


#endif
