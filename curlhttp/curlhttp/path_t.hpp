#ifndef CURLHTTP_PATH_T_HPP
#define CURLHTTP_PATH_T_HPP


#include <string>
#include <algorithm>


namespace curlhttp{

    class path_t{
    public:
        inline static const std::string separators = "/\\";

        path_t() {}

        path_t(const char* s)
            : path{s} {}

        path_t(const std::string& p)
            : path{p} {}

        path_t(const path_t& ) = default;
        path_t& operator= (const path_t& ) = default;

        path_t(path_t&& ) = default;
        path_t& operator= (path_t&& ) = default;

        path_t& operator= (const std::string& s){
            path = s;
            return *this;
        }

        path_t operator/ (const path_t& p) const{
            path_t temp{*this};
            return temp /= p;
        }

        path_t& operator/= (const path_t& p){
            if(!path.size())
                path += (p.path.size() && separators.find(p.path.front()) != separators.npos ? "/" : "") + p.path;

            else{
                if(path.size() && separators.find(path.back()) != separators.npos
                    && p.path.size() && separators.find(p.path.front()) != separators.npos)
                    path += p.path.substr(1);

                else if(path.size() && separators.find(path.back()) == separators.npos
                        && p.path.size() && separators.find(p.path.front()) == separators.npos)
                    path += '/' + p.path;

                else
                    path += p.path;
            }

            return *this;
        }

        void remove_trailing_slashes(){
            while(path.size() && separators.find(path.back()) != separators.npos)
                path.pop_back();
        }

        std::string filename() const{
            if(!path.size())
                return "";

            std::size_t pos = path.find_last_of(separators);

            if(pos == path.npos)
                return path;

            return path.substr(++pos);
        }

        path_t parent() const{
            std::size_t sepcount = std::count_if(path.begin(), path.end(), [](char c){
                return separators.find(c) != separators.npos;
            });

            if(!sepcount)
                return "";

            if(sepcount == 1 && path.size() == 1)
                return "/";

            std::size_t pos = path.find_last_of(separators);
            return path.substr(0, pos);
        }

        void pop_back(){
            path.pop_back();
        }

        void clear(){
            path.clear();
        }

        const std::string& string() const{
            return path;
        }

        bool operator!= (const path_t& rhs) const{
            return path != rhs.path;
        }

        bool operator== (const path_t& rhs) const{
            return !operator!=(rhs);
        }

    private:
        std::string path;
    };


    inline std::ostream& operator<< (std::ostream& out, const path_t& path){
        return out << path.string();
    }


    inline path_t operator/ (const std::string& s, const path_t& path){
        path_t result{s};
        return result /= path;
    }
}


#endif
