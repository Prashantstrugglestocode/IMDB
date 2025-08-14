#pragma once
#include "types.hpp"
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace imdb {
class Table {
private:
    std::string table_name_;
    std::vector<Column> columns_;
    std::vector<Row> rows_;
    std::optional<size_t> find_column_index(const std::string& column_name) const;

public:
    explicit Table(const std::string& name);
    ~Table() = default;

    Table(const Table& other) = default;
    Table& operator=(const Table& other) = default;
    Table(Table&& other) noexcept = default;
    Table& operator=(Table&& other) noexcept = default;

    void add_column(const std::string& name, ColumnType type);
    bool remove_column(const std::string& name);

    bool insert_row(const std::vector<Value>& values);
    bool insert_row(const Row& row);

    std::vector<Row> select_all() const;
    std::vector<Row> select_where(const std::string& column_name, const Value& value) const;
    std::vector<Row> select_where_custom(std::function<bool(const Row&, size_t)> predicate) const;

    size_t update_where(const std::string& column_name, const Value& old_value,
                        const std::string& update_column, const Value& new_value);

    size_t delete_where(const std::string& column_name, const Value& value);
    void clear_all_rows();

    size_t row_count() const noexcept { return rows_.size(); }
    size_t column_count() const noexcept { return columns_.size(); }
    std::string get_table_name() const { return table_name_; }
    std::vector<Column> get_columns() const { return columns_; }

    void print_table() const;
    void print_schema() const;

    std::vector<Row> select_range(size_t start_row, size_t count) const;
    std::optional<Row> find_first_where(const std::string& column_name, const Value& value) const;

    bool validate_row(const std::vector<Value>& values) const;
    bool column_exists(const std::string& column_name) const;

    std::optional<int64_t> sum_column(const std::string& column_name) const;
    std::optional<int64_t> count_where(const std::string& column_name, const Value& value) const;
    std::optional<Value> min_value(const std::string& column_name) const;
    std::optional<Value> max_value(const std::string& column_name) const;

    size_t import_csv(const std::string& path, bool header);
    bool export_csv(const std::string& path) const;
};
}