#include "imdb/types.hpp"
#include <string>
#include <variant>
#include <type_traits>

namespace imdb {
    const char* type_name(ColumnType t) noexcept {
        switch (t) {
            case ColumnType::Int: return "Int";
            case ColumnType::Text: return "Text";
            default: return "The data type is not supported by this database";
        }
    }

    // this function the values to string representation, as out values are just int and strings, so if it is int then convert it to
    // string and if it is string then return the string as it is
    std::string value_to_string(const Value& v) {
    {
        return std::visit([](const auto& value) -> std::string {
            if constexpr(std::is_same_v<std::decay_t){
                return std::to_string(value);
            } else  {
                return value;
            } else {
                return "This database does not support this data type";
            }
        }
    }
}
    


    bool value_matches_type(const Value& v, ColumnType t) noexcept {
        try {
            switch(t){
                case ColumnType::Int:
                    return std::holds_alternative<int64_t>(v);
                case ColumnType::Text:
                    return std::holds_alternative<std::string>(v);
                default:
                    return false; // Unsupported type
            }
        } catch (...) {
            return "The data type is unknown";
        }
}

    size_t get_value_type_index(const Value& v) noexcept {
        return v.index();
    }
}