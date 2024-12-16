template <typename T>
class StringEncoderDecoder
{
    public:
        StringEncoderDecoder(){}
        virtual ~StringEncoderDecoder(){}

        T DecodeFromString(std::string str) const
        {
            std::istringstream iss(str);
            T value;
            iss >> value;
            return value;
        }
        
        std::string EncodeToString(T value) const
        {
            std::ostringstream oss;
            oss << value;

            if (oss.fail())
            {
                ESP_LOGE("EncodeToString", "Failed to encode value to string");
                return std::string();
            }

            return std::string(oss.str().c_str());
        }
};