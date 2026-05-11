class vector_string
{
public:
    constexpr vector_string() = default;
    constexpr vector_string(std::string_view str) {
        _data.resize(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            _data[i] = str[i];
        }
    }
    constexpr vector_string(const vector_string& that) = default;
    constexpr vector_string(vector_string&& that) = default;
    constexpr vector_string& operator=(const vector_string& that) = default;
    constexpr vector_string& operator=(vector_string&& that) = default;
    constexpr vector_string& operator+=(std::string const& str) {
        *this += std::string_view(str);
        return *this;
    }
    constexpr vector_string& operator+=(const char* str) {
        *this += std::string_view(str);
        return *this;
    }
    constexpr vector_string& operator+=(const vector_string& that) {
        _data.reserve(_data.size() + that._data.size());
        for (auto c : that._data) {
            _data.push_back(c);
        }
        return *this;
    }
    constexpr vector_string& operator+=(std::string_view str) {
        _data.reserve(_data.size() + str.size());
        for (auto c : str) {
            _data.push_back(c);
        }
        return *this;
    }
    constexpr void reserve(size_t size) {
        _data.reserve(size);
    }
    constexpr const std::vector<char>& data() const {
        return _data;
    }
private:
    std::vector<char> _data;
};