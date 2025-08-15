#include "imdb/table.hpp"
#include "imdb/types.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace imdb {

Table::Table(const std::string& name) : table_name(name), primary_key_index(std::nullopt) {}

std::optional<size_t> Table::find_column_index(const std::string& column_name) const {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == column_name) return i;
    }
    return std::nullopt;
}

bool Table::is_null_value(const Value& v) const {
    return std::holds_alternative<std::monostate>(v);
}

void Table::add_column(const std::string& name, ColumnType type) {
    if (find_column_index(name).has_value()) throw std::runtime_error("column exists");
    Column c;
    c.name = name;
    c.type = type;
    c.not_null = false;
    c.is_primary_key = false;
    columns.push_back(c);

    if (!rows.empty()) {
        Value default_value;
        if (type == ColumnType::Int) default_value = static_cast<int64_t>(0);
        else default_value = std::string("");
        for (size_t r = 0; r < rows.size(); r++) {
            rows[r].values.push_back(default_value);
        }
    }
}

bool Table::remove_column(const std::string& name) {
    auto idx = find_column_index(name);
    if (!idx) return false;
    if (primary_key_index && *primary_key_index == *idx) return false;

    size_t column_index = *idx;
    std::vector<Column> new_columns;
    for (size_t i = 0; i < columns.size(); i++) {
        if (i != column_index) new_columns.push_back(columns[i]);
    }
    columns.swap(new_columns);

    for (size_t r = 0; r < rows.size(); r++) {
        std::vector<Value> new_values;
        for (size_t i = 0; i < rows[r].values.size(); i++) {
            if (i != column_index) new_values.push_back(rows[r].values[i]);
        }
        rows[r].values.swap(new_values);
    }

    if (primary_key_index && *primary_key_index > column_index) {
        primary_key_index = *primary_key_index - 1;
    }
    return true;
}

bool Table::insert_row(const std::vector<Value>& values) {
    if (values.size() != columns.size()) return false;

    for (size_t i = 0; i < values.size(); i++) {
        if (!value_matches_type(values[i], columns[i].type)) return false;
        if (columns[i].not_null && is_null_value(values[i])) return false;
    }

    if (primary_key_index) {
        const Value& key_value = values[*primary_key_index];
        if (is_null_value(key_value)) return false;
        for (size_t r = 0; r < rows.size(); r++) {
            if (rows[r].values[*primary_key_index] == key_value) return false;
        }
    }

    Row row;
    row.values = values;
    rows.push_back(row);
    return true;
}

bool Table::insert_row(const Row& row) {
    return insert_row(row.values);
}

std::vector<Row> Table::select_all() const {
    return rows;
}

std::vector<Row> Table::select_where(const std::string& column_name, const Value& value) const {
    std::vector<Row> result;
    auto idx = find_column_index(column_name);
    if (!idx) return result;
    size_t column_index = *idx;

    for (size_t r = 0; r < rows.size(); r++) {
        if (column_index < rows[r].values.size() && rows[r].values[column_index] == value) {
            result.push_back(rows[r]);
        }
    }
    return result;
}

size_t Table::update_where(const std::string& column_name, const Value& old_value,
                           const std::string& update_column, const Value& new_value) {
    auto src_idx = find_column_index(column_name);
    auto upd_idx = find_column_index(update_column);
    if (!src_idx || !upd_idx) return 0;

    size_t source_index = *src_idx;
    size_t update_index = *upd_idx;

    if (!value_matches_type(new_value, columns[update_index].type)) return 0;
    if (columns[update_index].not_null && is_null_value(new_value)) return 0;

    size_t updated_count = 0;

    for (size_t r = 0; r < rows.size(); r++) {
        bool match = false;
        if (source_index < rows[r].values.size() && rows[r].values[source_index] == old_value) {
            match = true;
        }
        if (!match) continue;

        if (primary_key_index && update_index == *primary_key_index) {
            if (is_null_value(new_value)) continue;
            bool clash = false;
            for (size_t k = 0; k < rows.size(); k++) {
                if (k == r) continue;
                if (rows[k].values[update_index] == new_value) {
                    clash = true;
                    break;
                }
            }
            if (clash) continue;
        }

        rows[r].values[update_index] = new_value;
        updated_count++;
    }

    return updated_count;
}

size_t Table::delete_where(const std::string& column_name, const Value& value) {
    auto idx = find_column_index(column_name);
    if (!idx) return 0;
    size_t column_index = *idx;

    size_t before = rows.size();
    std::vector<Row> kept;
    for (size_t r = 0; r < rows.size(); r++) {
        bool match = false;
        if (column_index < rows[r].values.size() && rows[r].values[column_index] == value) {
            match = true;
        }
        if (!match) kept.push_back(rows[r]);
    }
    rows.swap(kept);
    return before - rows.size();
}

void Table::clear_all_rows() {
    rows.clear();
}

void Table::print_table() const {
    std::cout << "\n=== Table: " << table_name << " ===\n";
    if (columns.empty()) {
        std::cout << "No columns defined.\n";
        return;
    }

    const int width = 18;
    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << std::setw(width) << columns[i].name;
        if (i + 1 < columns.size()) std::cout << " | ";
    }
    std::cout << "\n";
    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << std::string(width, '-');
        if (i + 1 < columns.size()) std::cout << "-+-";
    }
    std::cout << "\n";

    for (size_t r = 0; r < rows.size(); r++) {
        for (size_t i = 0; i < columns.size(); i++) {
            std::string cell = "";
            if (i < rows[r].values.size()) cell = value_to_string(rows[r].values[i]);
            std::cout << std::setw(width) << cell;
            if (i + 1 < columns.size()) std::cout << " | ";
        }
        std::cout << "\n";
    }

    std::cout << "\nRows: " << rows.size() << "\n\n";
}

void Table::print_schema() const {
    std::cout << "\n=== Schema for Table: " << table_name << " ===\n";
    const int wn = 20, wt = 10, wnn = 8, wpk = 12;

    std::cout << std::setw(wn) << "Column Name" << " | "
              << std::setw(wt) << "Type" << " | "
              << std::setw(wnn) << "NotNull" << " | "
              << std::setw(wpk) << "PrimaryKey" << "\n";

    std::cout << std::string(wn, '-') << "-+-"
              << std::string(wt, '-') << "-+-"
              << std::string(wnn, '-') << "-+-"
              << std::string(wpk, '-') << "\n";

    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << std::setw(wn) << columns[i].name << " | "
                  << std::setw(wt) << type_name(columns[i].type) << " | "
                  << std::setw(wnn) << (columns[i].not_null ? "yes" : "no") << " | "
                  << std::setw(wpk) << (columns[i].is_primary_key ? "yes" : "no") << "\n";
    }
    std::cout << "\n";
}

bool Table::set_primary_key(const std::string& column_name) {
    auto idx = find_column_index(column_name);
    if (!idx) return false;
    size_t i = *idx;

    for (size_t r = 0; r < rows.size(); r++) {
        if (is_null_value(rows[r].values[i])) return false;
    }
    for (size_t a = 0; a < rows.size(); a++) {
        for (size_t b = a + 1; b < rows.size(); b++) {
            if (rows[a].values[i] == rows[b].values[i]) return false;
        }
    }

    if (primary_key_index) {
        columns[*primary_key_index].is_primary_key = false;
    }
    primary_key_index = i;
    columns[i].is_primary_key = true;
    columns[i].not_null = true;
    return true;
}

bool Table::set_not_null(const std::string& column_name, bool value) {
    auto idx = find_column_index(column_name);
    if (!idx) return false;
    size_t i = *idx;

    if (value) {
        for (size_t r = 0; r < rows.size(); r++) {
            if (is_null_value(rows[r].values[i])) return false;
        }
    }
    columns[i].not_null = value;
    return true;
}

static std::vector<std::string> parse_csv_line_simple(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool in_quotes = false;

    for (size_t k = 0; k < line.size(); k++) {
        char c = line[k];
        if (in_quotes) {
            if (c == '"') {
                if (k + 1 < line.size() && line[k + 1] == '"') {
                    current.push_back('"');
                    k++;
                } else {
                    in_quotes = false;
                }
            } else {
                current.push_back(c);
            }
        } else {
            if (c == '"') {
                in_quotes = true;
            } else if (c == ',') {
                fields.push_back(current);
                current.clear();
            } else if (c == '\r') {
            } else {
                current.push_back(c);
            }
        }
    }
    fields.push_back(current);
    return fields;
}

size_t Table::import_csv(const std::string& path, bool header) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cout << "IMPORT CSV: cannot open file: " << path << "\n";
        return 0;
    }
    if (columns.empty()) {
        std::cout << "IMPORT CSV: table has no columns\n";
        return 0;
    }

    size_t inserted = 0;
    std::string line;
    bool first_line = true;

    while (std::getline(in, line)) {
        if (first_line && header) {
            first_line = false;
            continue;
        }
        if (line.empty()) continue;

        std::vector<std::string> fields = parse_csv_line_simple(line);
        if (fields.size() != columns.size()) continue;

        std::vector<Value> values;
        values.reserve(columns.size());

        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].type == ColumnType::Int) {
                try {
                    int64_t x = std::stoll(fields[i]);
                    values.push_back(x);
                } catch (...) {
                    values.push_back(static_cast<int64_t>(0));
                }
            } else {
                values.push_back(fields[i]);
            }
        }

        if (insert_row(values)) inserted++;
    }
    return inserted;
}

static std::string csv_escape(const std::string& s) {
    bool need = false;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            need = true;
            break;
        }
    }
    if (!need) return s;

    std::string out;
    out.push_back('"');
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

bool Table::export_csv(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) return false;

    for (size_t i = 0; i < columns.size(); i++) {
        out << csv_escape(columns[i].name);
        if (i + 1 < columns.size()) out << ",";
    }
    out << "\n";

    for (size_t r = 0; r < rows.size(); r++) {
        for (size_t i = 0; i < columns.size(); i++) {
            std::string s = "";
            if (i < rows[r].values.size()) s = value_to_string(rows[r].values[i]);
            out << csv_escape(s);
            if (i + 1 < columns.size()) out << ",";
        }
        out << "\n";
    }
    return true;
}

} 