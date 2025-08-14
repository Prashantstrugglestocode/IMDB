#include "imdb/table.hpp"
#include "imdb/types.hpp"
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace imdb {

Table::Table(const std::string& name) : table_name_(name) {}

std::optional<size_t> Table::find_column_index(const std::string& column_name) const {
    for (size_t i = 0; i < columns_.size(); ++i) if (columns_[i].name == column_name) return i;
    return std::nullopt;
}

void Table::add_column(const std::string& name, ColumnType type) {
    if (column_exists(name)) throw std::runtime_error("column exists");
    columns_.push_back(Column{name, type});
    if (!rows_.empty()) {
        Value def = (type == ColumnType::Int) ? Value(int64_t{0}) : Value(std::string{""});
        for (auto& r : rows_) r.values.push_back(def);
    }
}

bool Table::remove_column(const std::string& name) {
    auto idx = find_column_index(name);
    if (!idx) return false;
    size_t i = *idx;
    columns_.erase(columns_.begin() + i);
    for (auto& r : rows_) if (r.values.size() > i) r.values.erase(r.values.begin() + i);
    return true;
}

bool Table::validate_row(const std::vector<Value>& values) const {
    if (values.size() != columns_.size()) return false;
    for (size_t i = 0; i < values.size(); ++i)
        if (!value_matches_type(values[i], columns_[i].type)) return false;
    return true;
}

bool Table::insert_row(const std::vector<Value>& values) {
    if (!validate_row(values)) return false;
    rows_.push_back(Row{values});
    return true;
}

bool Table::insert_row(const Row& row) { return insert_row(row.values); }

std::vector<Row> Table::select_all() const { return rows_; }

std::vector<Row> Table::select_where(const std::string& column_name, const Value& value) const {
    auto idx = find_column_index(column_name);
    if (!idx) return {};
    size_t i = *idx;
    std::vector<Row> out;
    for (const auto& r : rows_) if (i < r.values.size() && r.values[i] == value) out.push_back(r);
    return out;
}

std::vector<Row> Table::select_where_custom(std::function<bool(const Row&, size_t)> predicate) const {
    std::vector<Row> out;
    for (size_t i = 0; i < rows_.size(); ++i) if (predicate(rows_[i], i)) out.push_back(rows_[i]);
    return out;
}

size_t Table::update_where(const std::string& column_name, const Value& old_value,
                           const std::string& update_column, const Value& new_value) {
    auto sidx = find_column_index(column_name);
    auto uidx = find_column_index(update_column);
    if (!sidx || !uidx) return 0;
    if (!value_matches_type(new_value, columns_[*uidx].type)) return 0;
    size_t n = 0;
    for (auto& r : rows_) {
        if (*sidx < r.values.size() && r.values[*sidx] == old_value) {
            r.values[*uidx] = new_value;
            ++n;
        }
    }
    return n;
}

size_t Table::delete_where(const std::string& column_name, const Value& value) {
    auto idx = find_column_index(column_name);
    if (!idx) return 0;
    size_t i = *idx;
    size_t before = rows_.size();
    rows_.erase(std::remove_if(rows_.begin(), rows_.end(),
                               [&](const Row& r){ return i < r.values.size() && r.values[i] == value; }),
                rows_.end());
    return before - rows_.size();
}

void Table::clear_all_rows() { rows_.clear(); }
bool Table::column_exists(const std::string& column_name) const { return find_column_index(column_name).has_value(); }

void Table::print_table() const {
    std::cout << "\n=== Table: " << table_name_ << " ===\n";
    if (columns_.empty()) { std::cout << "No columns defined.\n"; return; }
    const int W = 18;
    for (size_t i=0;i<columns_.size();++i){ std::cout<<std::setw(W)<<columns_[i].name; if(i+1<columns_.size()) std::cout<<" | "; }
    std::cout<<"\n";
    for (size_t i=0;i<columns_.size();++i){ std::cout<<std::string(W,'-'); if(i+1<columns_.size()) std::cout<<"-+-"; }
    std::cout<<"\n";
    for (const auto& r: rows_){
        for (size_t i=0;i<columns_.size();++i){
            std::string cell = (i<r.values.size()) ? value_to_string(r.values[i]) : "";
            std::cout<<std::setw(W)<<cell; if(i+1<columns_.size()) std::cout<<" | ";
        }
        std::cout<<"\n";
    }
    std::cout<<"\nRows: "<<rows_.size()<<"\n\n";
}

void Table::print_schema() const {
    std::cout << "\n=== Schema for Table: " << table_name_ << " ===\n";
    const int Wn=20, Wt=10;
    std::cout<<std::setw(Wn)<<"Column Name"<<" | "<<std::setw(Wt)<<"Type"<<"\n";
    std::cout<<std::string(Wn,'-')<<"-+-"<<std::string(Wt,'-')<<"\n";
    for (const auto& c: columns_) std::cout<<std::setw(Wn)<<c.name<<" | "<<std::setw(Wt)<<type_name(c.type)<<"\n";
    std::cout<<"\nTotal columns: "<<columns_.size()<<"\n\n";
}

std::vector<Row> Table::select_range(size_t start_row, size_t count) const {
    if (start_row >= rows_.size()) return {};
    size_t end = std::min(start_row + count, rows_.size());
    std::vector<Row> out; out.reserve(end - start_row);
    for (size_t i = start_row; i < end; ++i) out.push_back(rows_[i]);
    return out;
}

std::optional<Row> Table::find_first_where(const std::string& column_name, const Value& value) const {
    auto idx = find_column_index(column_name);
    if (!idx) return std::nullopt;
    size_t i = *idx;
    for (const auto& r : rows_) if (i < r.values.size() && r.values[i] == value) return r;
    return std::nullopt;
}

std::optional<int64_t> Table::sum_column(const std::string& column_name) const {
    auto idx = find_column_index(column_name);
    if (!idx) return std::nullopt;
    size_t i = *idx;
    if (columns_[i].type != ColumnType::Int) return std::nullopt;
    int64_t s = 0;
    for (const auto& r : rows_) if (i < r.values.size()) s += std::get<int64_t>(r.values[i]);
    return s;
}

std::optional<int64_t> Table::count_where(const std::string& column_name, const Value& value) const {
    auto idx = find_column_index(column_name);
    if (!idx) return std::nullopt;
    size_t i = *idx; int64_t c = 0;
    for (const auto& r : rows_) if (i < r.values.size() && r.values[i] == value) ++c;
    return c;
}

std::optional<Value> Table::min_value(const std::string& column_name) const {
    auto idx = find_column_index(column_name);
    if (!idx || rows_.empty()) return std::nullopt;
    size_t i = *idx; Value m = rows_[0].values[i];
    for (size_t r = 1; r < rows_.size(); ++r) {
        if (i >= rows_[r].values.size()) continue;
        const Value& cur = rows_[r].values[i];
        if (columns_[i].type == ColumnType::Int) { if (std::get<int64_t>(cur) < std::get<int64_t>(m)) m = cur; }
        else { if (std::get<std::string>(cur) < std::get<std::string>(m)) m = cur; }
    }
    return m;
}

std::optional<Value> Table::max_value(const std::string& column_name) const {
    auto idx = find_column_index(column_name);
    if (!idx || rows_.empty()) return std::nullopt;
    size_t i = *idx; Value M = rows_[0].values[i];
    for (size_t r = 1; r < rows_.size(); ++r) {
        if (i >= rows_[r].values.size()) continue;
        const Value& cur = rows_[r].values[i];
        if (columns_[i].type == ColumnType::Int) { if (std::get<int64_t>(cur) > std::get<int64_t>(M)) M = cur; }
        else { if (std::get<std::string>(cur) > std::get<std::string>(M)) M = cur; }
    }
    return M;
}

static std::vector<std::string> parse_csv_line(const std::string& line){
    std::vector<std::string> f; std::string cur; bool q=false;
    for (size_t i=0;i<line.size();++i){
        char c=line[i];
        if(q){
            if(c=='"'){
                if(i+1<line.size() && line[i+1]=='"'){ cur.push_back('"'); ++i; }
                else q=false;
            } else cur.push_back(c);
        } else {
            if(c==','){ f.push_back(cur); cur.clear(); }
            else if(c=='"') q=true;
            else if(c=='\r') {}
            else cur.push_back(c);
        }
    }
    f.push_back(cur);
    return f;
}

size_t Table::import_csv(const std::string& path, bool header){
    std::ifstream in(path);
    if(!in.is_open()){ std::cout<<"IMPORT CSV: cannot open file: "<<path<<"\n"; return 0; }
    if (columns_.empty()){ std::cout<<"IMPORT CSV: table has no columns\n"; return 0; }
    size_t inserted=0; std::string line; bool first=true;
    while(std::getline(in,line)){
        if(first && header){ first=false; continue; }
        if(line.empty()) continue;
        auto fields=parse_csv_line(line);
        if(fields.size()!=columns_.size()) continue;
        std::vector<Value> vals; vals.reserve(columns_.size());
        for(size_t i=0;i<columns_.size();++i){
            if(columns_[i].type==ColumnType::Int){
                try{ vals.push_back(static_cast<int64_t>(std::stoll(fields[i]))); }
                catch(...){ vals.push_back(int64_t{0}); }
            }else{
                vals.push_back(fields[i]);
            }
        }
        if(insert_row(vals)) ++inserted;
    }
    return inserted;
}

static std::string csv_escape(const std::string& s){
    bool need = s.find_first_of(",\"\n\r") != std::string::npos;
    if(!need) return s;
    std::string t="\"";
    for(char c: s){ if(c=='"') t.push_back('"'); t.push_back(c); }
    t.push_back('"'); return t;
}

bool Table::export_csv(const std::string& path) const{
    std::ofstream out(path);
    if(!out.is_open()) return false;
    for(size_t i=0;i<columns_.size();++i){ out<<csv_escape(columns_[i].name); if(i+1<columns_.size()) out<<","; }
    out<<"\n";
    for(const auto& r: rows_){
        for(size_t i=0;i<columns_.size();++i){
            std::string s = (i<r.values.size()) ? value_to_string(r.values[i]) : "";
            out<<csv_escape(s); if(i+1<columns_.size()) out<<",";
        }
        out<<"\n";
    }
    return true;
}

}