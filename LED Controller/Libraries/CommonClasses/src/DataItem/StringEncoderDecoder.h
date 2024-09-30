
template <typename T>
class StringEncoderDecoder
{
    public:
        StringEncoderDecoder(){}
        virtual ~StringEncoderDecoder(){}
        T DecodeFromString(String str) const
        {
            std::string stdStr = str.c_str();
            std::istringstream iss(stdStr);
            T value;
            iss >> value;
            return value;
        }

        String EncodeToString(T value) const
        {
            std::ostringstream oss;
            oss << value;

            if (oss.fail())
            {
                ESP_LOGE("EncodeToString", "Failed to encode value to string");
                return String();
            }
            return String(oss.str().c_str());
        }
};