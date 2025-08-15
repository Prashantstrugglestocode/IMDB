#pragma once
#include "table.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace imdb {

class Database {
private:
    std::string database_name;
    std::unordered_map<std::string, std::unique_ptr<Table>> tables;

public:
    explicit Database(const std::string& name);
    ~Database() = default;

    bool create_table(const std::string& table_name);
    bool drop_table(const std::string& table_name);
    Table* get_table(const std::string& table_name);
    const Table* get_table(const std::string& table_name) const;

    bool table_exists(const std::string& table_name) const;
    std::vector<std::string> get_table_names() const;

    void clear_all_tables();
    size_t get_total_rows() const;

    bool rename_table(const std::string& old_name, const std::string& new_name);

    bool inner_join(const std::string& left_table,
                    const std::string& left_col,
                    const std::string& right_table,
                    const std::string& right_col,
                    std::vector<std::string>& out_headers,
                    std::vector<std::vector<Value>>& out_rows) const;
};

}