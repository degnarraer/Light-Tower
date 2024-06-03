
template <typename T>
class StringEncoderDecoder
{
    public:
        StringEncoderDecoder(){}
        virtual ~StringEncoderDecoder(){}

        T DecodeFromString(String str) {
            std::string stdStr = str.c_str();
            std::istringstream iss(stdStr);
            T value;
            iss >> value;
            return value;
        }

        String EncodeToString(T value)
        {
            std::ostringstream oss;
            oss << value;
            if (oss.fail())
            {
                assert((false) && "Failed to encode value to string");
            }
            return String(oss.str().c_str());
        }
};