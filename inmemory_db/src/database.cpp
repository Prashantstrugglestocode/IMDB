#include "imdb/database.hpp"
#include "imdb/table.hpp"
#include <algorithm>
#include <utility>

namespace imdb {

Database::Database(const std::string& name) : database_name_(name) {}

bool Database::create_table(const std::string& table_name) {
    if (table_exists(table_name)) return false;
    tables_[table_name] = std::make_unique<Table>(table_name);
    return true;
}

bool Database::drop_table(const std::string& table_name) {
    auto it = tables_.find(table_name);
    if (it == tables_.end()) return false;
    tables_.erase(it);
    return true;
}

Table* Database::get_table(const std::string& table_name) {
    auto it = tables_.find(table_name);
    if (it != tables_.end()) return it->second.get();
    return nullptr;
}

const Table* Database::get_table(const std::string& table_name) const {
    auto it = tables_.find(table_name);
    if (it != tables_.end()) return it->second.get();
    return nullptr;
}

bool Database::table_exists(const std::string& table_name) const {
    return tables_.find(table_name) != tables_.end();
}

std::vector<std::string> Database::get_table_names() const {
    std::vector<std::string> names;
    names.reserve(tables_.size());
    for (const auto& p : tables_) names.push_back(p.first);
    std::sort(names.begin(), names.end());
    return names;
}

void Database::print_database_info() const {}
void Database::print_all_tables() const {}

void Database::clear_all_tables() {
    tables_.clear();
}

size_t Database::get_total_rows() const {
    size_t total = 0;
    for (const auto& p : tables_) total += p.second->row_count();
    return total;
}

bool Database::rename_table(const std::string& old_name, const std::string& new_name) {
    auto it = tables_.find(old_name);
    if (it == tables_.end()) return false;
    if (table_exists(new_name)) return false;
    auto ptr = std::move(it->second);
    tables_.erase(it);
    tables_[new_name] = std::move(ptr);
    return true;
}

std::vector<std::string> Database::search_tables_by_column(const std::string& column_name) const {
    std::vector<std::string> res;
    for (const auto& p : tables_) if (p.second->column_exists(column_name)) res.push_back(p.first);
    std::sort(res.begin(), res.end());
    return res;
}

bool Database::create_tables(const std::vector<std::string>& table_names) {
    bool all=true;
    for (auto& n: table_names) if (!create_table(n)) all=false;
    return all;
}

size_t Database::drop_tables(const std::vector<std::string>& table_names) {
    size_t c=0;
    for (auto& n: table_names) if (drop_table(n)) ++c;
    return c;
}

}