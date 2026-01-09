#ifndef V2_TYPES_H
#define V2_TYPES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace gempp {

// Simple exception class
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& msg) : std::runtime_error(msg) {}
};

// String utilities
class StringUtils {
public:
    static std::vector<std::string> split(const std::string& str, char delimiter, bool skip_empty = true) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);

        while (std::getline(tokenStream, token, delimiter)) {
            if (!skip_empty || !token.empty()) {
                tokens.push_back(token);
            }
        }

        return tokens;
    }

    static std::string trim(const std::string& str) {
        size_t start = 0;
        while (start < str.length() && std::isspace(str[start])) {
            ++start;
        }

        size_t end = str.length();
        while (end > start && std::isspace(str[end - 1])) {
            --end;
        }

        return str.substr(start, end - start);
    }

    static int toInt(const std::string& str, bool* ok = nullptr) {
        try {
            size_t pos;
            int value = std::stoi(str, &pos);
            if (ok) *ok = (pos == str.length());
            return value;
        } catch (...) {
            if (ok) *ok = false;
            return 0;
        }
    }

    static std::string fromInt(int value) {
        return std::to_string(value);
    }
};

} // namespace gempp

#endif // V2_TYPES_H
