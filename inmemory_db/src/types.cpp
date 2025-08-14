#include "imdb/types.hpp"
#include <string>
#include <variant>
#include <sstream>
#include <functional>

namespace imdb {

const char* type_name(ColumnType t) noexcept {
    switch (t) {
        case ColumnType::Int:  return "Int";
        case ColumnType::Text: return "Text";
    }
    return "Unknown";
}

std::string value_to_string(const Value& v) {
    if (std::holds_alternative<std::monostate>(v)) return "NULL";
    if (auto p = std::get_if<int64_t>(&v)) return std::to_string(*p);
    return std::get<std::string>(v);
}

bool value_matches_type(const Value& v, ColumnType t) noexcept {
    if (std::holds_alternative<std::monostate>(v)) return true;
    switch (t) {
        case ColumnType::Int:  return std::holds_alternative<int64_t>(v);
        case ColumnType::Text: return std::holds_alternative<std::string>(v);
    }
    return false;
}

std::size_t get_value_type_index(const Value& v) noexcept { return v.index(); }

std::size_t ValueHash::operator()(const Value& v) const noexcept {
    if (std::holds_alternative<std::monostate>(v)) return 0x9e3779b97f4a7c15ULL;
    if (auto p = std::get_if<int64_t>(&v))         return std::hash<int64_t>{}(*p) ^ 0x12345678ULL;
    return std::hash<std::string>{}(std::get<std::string>(v)) ^ 0x87654321ULL;
}

bool ValueEq::operator()(const Value& a, const Value& b) const noexcept { return a == b; }

}