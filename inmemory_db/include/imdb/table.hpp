#pragma once
#include "types.hpp"
#include <vector>
#include <string>
#include <optional>

namespace imdb {

class Table {
private:
    std::string table_name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    std::optional<size_t> primary_key_index;

    std::optional<size_t> find_column_index(const std::string& column_name) const;
    bool is_null_value(const Value& v) const;

public:
    explicit Table(const std::string& name);
    ~Table() = default;

    void add_column(const std::string& name, ColumnType type);
    bool remove_column(const std::string& name);

    bool insert_row(const std::vector<Value>& values);
    bool insert_row(const Row& row);

    std::vector<Row> select_all() const;
    std::vector<Row> select_where(const std::string& column_name, const Value& value) const;

    size_t update_where(const std::string& column_name, const Value& old_value,
                        const std::string& update_column, const Value& new_value);

    size_t delete_where(const std::string& column_name, const Value& value);

    void clear_all_rows();

    size_t row_count() const noexcept { return rows.size(); }
    size_t column_count() const noexcept { return columns.size(); }
    std::string get_table_name() const { return table_name; }
    std::vector<Column> get_columns() const { return columns; }

    void print_table() const;
    void print_schema() const;

    bool set_primary_key(const std::string& column_name);
    bool set_not_null(const std::string& column_name, bool value);

    size_t import_csv(const std::string& path, bool header);
    bool export_csv(const std::string& path) const;
};

} 