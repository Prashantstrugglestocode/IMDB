#include "imdb/types.hpp"
#include <variant>
#include <string>

namespace imdb {

const char* type_name(ColumnType t) noexcept {
    if (t == ColumnType::Int) return "Int";
    if (t == ColumnType::Text) return "Text";
    return "Unknown";
}

std::string value_to_string(const Value& v) {
    if (std::holds_alternative<std::monostate>(v)) return "NULL";
    if (auto p = std::get_if<int64_t>(&v)) return std::to_string(*p);
    return std::get<std::string>(v);
}

bool value_matches_type(const Value& v, ColumnType t) noexcept {
    if (std::holds_alternative<std::monostate>(v)) return true;
    if (t == ColumnType::Int) return std::holds_alternative<int64_t>(v);
    if (t == ColumnType::Text) return std::holds_alternative<std::string>(v);
    return false;
}

}