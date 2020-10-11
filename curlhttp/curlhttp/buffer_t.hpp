#ifndef CURLHTTP_BUFFER_T_HPP
#define CURLHTTP_BUFFER_T_HPP


#include <cstddef>


namespace curlhttp{

    template<typename T>
    struct buffer_t{
        const T& container;
        std::size_t read{};

        explicit buffer_t(const T& cont)
            : container{cont} {}

        std::size_t pending() const{
            return container.size() - read;
        }

        std::size_t size() const{
            return container.size();
        }
    };

}


#endif
