#include "imdb/database.hpp"
#include <algorithm>
#include <utility>

namespace imdb {

Database::Database(const std::string& name) : database_name(name) {}

bool Database::create_table(const std::string& table_name) {
    if (table_exists(table_name)) return false;
    tables[table_name] = std::make_unique<Table>(table_name);
    return true;
}

bool Database::drop_table(const std::string& table_name) {
    auto it = tables.find(table_name);
    if (it == tables.end()) return false;
    tables.erase(it);
    return true;
}

Table* Database::get_table(const std::string& table_name) {
    auto it = tables.find(table_name);
    if (it == tables.end()) return nullptr;
    return it->second.get();
}

const Table* Database::get_table(const std::string& table_name) const {
    auto it = tables.find(table_name);
    if (it == tables.end()) return nullptr;
    return it->second.get();
}

bool Database::table_exists(const std::string& table_name) const {
    return tables.find(table_name) != tables.end();
}

std::vector<std::string> Database::get_table_names() const {
    std::vector<std::string> names;
    names.reserve(tables.size());
    for (const auto& pair : tables) names.push_back(pair.first);
    std::sort(names.begin(), names.end());
    return names;
}

void Database::clear_all_tables() {
    tables.clear();
}

size_t Database::get_total_rows() const {
    size_t total = 0;
    for (const auto& pair : tables) total += pair.second->row_count();
    return total;
}

bool Database::rename_table(const std::string& old_name, const std::string& new_name) {
    auto it = tables.find(old_name);
    if (it == tables.end()) return false;
    if (table_exists(new_name)) return false;
    auto ptr = std::move(it->second);
    tables.erase(it);
    tables[new_name] = std::move(ptr);
    return true;
}

} 