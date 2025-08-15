#include "imdb/database.hpp"
#include <algorithm>
#include <utility>
#include <optional>

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

static std::optional<size_t> find_index_by_name(const std::vector<Column>& cols, const std::string& name) {
    for (size_t i = 0; i < cols.size(); i++) if (cols[i].name == name) return i;
    return std::nullopt;
}

bool Database::inner_join(const std::string& left_table,
                          const std::string& left_col,
                          const std::string& right_table,
                          const std::string& right_col,
                          std::vector<std::string>& out_headers,
                          std::vector<std::vector<Value>>& out_rows) const {
    out_headers.clear();
    out_rows.clear();

    const Table* lt = get_table(left_table);
    const Table* rt = get_table(right_table);
    if (!lt || !rt) return false;

    auto lcols = lt->get_columns();
    auto rcols = rt->get_columns();

    auto li = find_index_by_name(lcols, left_col);
    auto ri = find_index_by_name(rcols, right_col);
    if (!li || !ri) return false;

    if (lcols[*li].type != rcols[*ri].type) return false;

    out_headers.reserve(lcols.size() + rcols.size());
    for (const auto& c : lcols) out_headers.push_back(left_table + "." + c.name);
    for (const auto& c : rcols) out_headers.push_back(right_table + "." + c.name);

    auto lrows = lt->select_all();
    auto rrows = rt->select_all();

    for (const auto& lr : lrows) {
        if (*li >= lr.values.size()) continue;
        const Value& lv = lr.values[*li];
        if (!value_matches_type(lv, lcols[*li].type)) continue;

        for (const auto& rr : rrows) {
            if (*ri >= rr.values.size()) continue;
            const Value& rv = rr.values[*ri];
            if (!value_matches_type(rv, rcols[*ri].type)) continue;

            if (lv == rv) {
                std::vector<Value> combined;
                combined.reserve(lr.values.size() + rr.values.size());
                combined.insert(combined.end(), lr.values.begin(), lr.values.end());
                combined.insert(combined.end(), rr.values.begin(), rr.values.end());
                out_rows.push_back(std::move(combined));
            }
        }
    }
    return true;
}

}