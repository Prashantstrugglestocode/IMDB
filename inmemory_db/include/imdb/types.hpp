#pragma once
#include <string>
#include <variant>
#include <cstdint>
#include <vector>

namespace imdb {

enum class ColumnType { Int, Text };

using Value = std::variant<std::monostate, int64_t, std::string>;

struct Column {
    std::string name;
    ColumnType type;
    bool not_null = false;
    bool is_primary_key = false;
};

struct Row {
    std::vector<Value> values;
};

const char* type_name(ColumnType t) noexcept;
std::string value_to_string(const Value& v);
bool value_matches_type(const Value& v, ColumnType t) noexcept;

}