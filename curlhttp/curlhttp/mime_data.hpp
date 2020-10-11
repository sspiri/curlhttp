#ifndef CURLHTTP_MIME_DATA_HPP
#define CURLHTTP_MIME_DATA_HPP


#include <filesystem>

#include "default_reader.hpp"
#include "default_seeker.hpp"
#include "size_getter.hpp"
#include "mime_holder.hpp"


namespace curlhttp{
    enum class encoder_t : char{
        none, binary, bit8, bit7, base64, quoted_printable
    };

    inline std::string encoder_string(encoder_t enc){
        switch(enc){
            case encoder_t::none:
                return "";
            case encoder_t::binary:
                return "binary";
            case encoder_t::bit8:
                return "8bit";
            case encoder_t::bit7:
                return "7bit";
            case encoder_t::base64:
                return "base64";
            case encoder_t::quoted_printable:
                return "quoted-printable";
        }

        assert(((void)"curl specific implementation error", false));
    }


    class basic_mime_data{
    public:
        std::string name;
        std::string type;
        encoder_t encoder{encoder_t::none};
        std::unique_ptr<mime_holder> sub;

        explicit basic_mime_data(curl_mime* parent)
            : part{curl_mime_addpart(parent)} {}

        virtual void setup(){
            curl_mime_name(part, name.c_str());
            curl_mime_type(part, type.c_str());
            curl_mime_encoder(part, encoder_string(encoder).c_str());

            if(sub){
                sub->setup();
                curl_mime_subparts(part, sub->native());
            }

            else
                curl_mime_subparts(part, 0);
        }

        virtual ~basic_mime_data() {}

    protected:
        curl_mimepart* part;
    };


    class file_data final : public basic_mime_data{
    public:
        std::filesystem::path filepath;
        std::string filename;

        file_data(const std::filesystem::path& fp, curl_mime* parent)
            : basic_mime_data{parent}, filepath{fp} {}

        void setup() override{
            basic_mime_data::setup();

            curl_mime_filedata(part, filepath.c_str());

            if(filename.size())
                curl_mime_filename(part, filename.c_str());
            else
                curl_mime_filename(part, filepath.filename().c_str());
        }
    };


    template<typename TX, typename Reader = default_reader<TX>, typename Seeker = default_seeker<TX>>
    class mime_data final : public basic_mime_data{
    public:
        using tx_buffer_t = TX;

        using reader_t = Reader;
        using seeker_t = Seeker;

        TX& tx_buffer;

        mime_data(TX& tx, curl_mime* parent)
            : basic_mime_data{parent}, tx_buffer{tx} {}

        void setup() override{
            basic_mime_data::setup();
            curl_mime_data_cb(part, size_getter<TX>{}(tx_buffer), &mime_data::read_callback, &mime_data::seek_callback, 0, this);
        }

    private:
        reader_t reader;
        seeker_t seeker;

        static std::size_t read_callback(char* buffer, std::size_t size, std::size_t nmemb, void* voidp){
            auto* this_ = (mime_data*)voidp;
            return this_->reader(this_->tx_buffer, buffer, size, nmemb);
        }

        static int seek_callback(void* voidp, curl_off_t offset, int origin){
            auto* this_ = (mime_data*)voidp;
            return this_->seeker(this_->tx_buffer, offset, origin);
        }
    };

}


#endif
