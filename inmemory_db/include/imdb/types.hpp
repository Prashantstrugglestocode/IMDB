#pragma once
#include <string>
#include <variant>
#include <cstdint>
#include <vector>
#include <optional>

namespace imdb {

enum class ColumnType { Int, Text };

using Value = std::variant<std::monostate, int64_t, std::string>;

struct Column {
    std::string name;
    ColumnType  type;
    bool        not_null{false};
    bool        is_primary_key{false};
    std::optional<Value> default_value{};
};

struct Row {
    std::vector<Value> values;
};

struct ValueHash {
    std::size_t operator()(const Value& v) const noexcept;
};
struct ValueEq {
    bool operator()(const Value& a, const Value& b) const noexcept;
};

const char*   type_name(ColumnType t) noexcept;
std::string   value_to_string(const Value& v);
bool          value_matches_type(const Value& v, ColumnType t) noexcept;
std::size_t   get_value_type_index(const Value& v) noexcept;

}