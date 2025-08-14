#include "imdb/database.hpp"
#include "imdb/table.hpp"
#include "imdb/types.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <optional>

using namespace imdb;

static std::string trim_quotes(const std::string& s) {
    if (s.size() >= 2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.size()-2);
    return s;
}

static std::string to_upper(std::string s) {
    for (auto& c : s) c = std::toupper(static_cast<unsigned char>(c));
    return s;
}

static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool in_quotes = false;
    for (char ch : line) {
        if (in_quotes) {
            cur.push_back(ch);
            if (ch == '"') in_quotes = false;
        } else {
            if (ch == '"') {
                if (!cur.empty()) { out.push_back(cur); cur.clear(); }
                cur.push_back(ch);
                in_quotes = true;
            } else if (std::isspace(static_cast<unsigned char>(ch))) {
                if (!cur.empty()) { out.push_back(cur); cur.clear(); }
            } else {
                cur.push_back(ch);
            }
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

static std::optional<int64_t> to_i64(const std::string& s) {
    try { size_t p = 0; long long v = std::stoll(s, &p); if (p == s.size()) return static_cast<int64_t>(v); }
    catch (...) {}
    return std::nullopt;
}

static ColumnType parse_type(const std::string& t) {
    std::string u = to_upper(t);
    if (u == "INT" || u == "INTEGER") return ColumnType::Int;
    return ColumnType::Text;
}

static Value parse_value_token(const std::string& raw, std::optional<ColumnType> expected = std::nullopt) {
    std::string t = raw;
    if (!t.empty() && t.front()=='"' && t.back()=='"') t = trim_quotes(t);
    if (expected.has_value()) {
        if (*expected == ColumnType::Int) {
            if (auto v = to_i64(t)) return *v;
            return t;
        } else {
            return t;
        }
    }
    if (auto v = to_i64(t)) return *v;
    return t;
}

static void print_banner() {
    std::cout << "\n=============================================\n";
    std::cout << "  In-Memory Database CLI\n";
    std::cout << "  Type HELP for commands, EXIT to quit\n";
    std::cout << "=============================================\n\n";
}

static void print_help() {
    const int A = 28;
    auto line = [](){ std::cout << std::string(70, '-') << "\n"; };
    std::cout << "\n";
    line();
    std::cout << std::left << std::setw(A) << "HELP" << "Show this help\n";
    std::cout << std::left << std::setw(A) << "TABLES" << "List all tables\n";
    std::cout << std::left << std::setw(A) << "CREATE TABLE <name>" << "Create a new table\n";
    std::cout << std::left << std::setw(A) << "DROP TABLE <name>" << "Remove a table\n";
    std::cout << std::left << std::setw(A) << "ADD COLUMN <table> <col> <type>" << "Add a column (INT or TEXT)\n";
    std::cout << std::left << std::setw(A) << "INSERT <table> <values...>" << "Insert a row (match column order)\n";
    std::cout << std::left << std::setw(A) << "SELECT ALL <table>" << "Show all rows\n";
    std::cout << std::left << std::setw(A) << "SELECT WHERE <table> <col> = <val>" << "Filter rows by equality\n";
    std::cout << std::left << std::setw(A) << "UPDATE <table> <col> <val> <set_col> <new_val>" << "Update rows\n";
    std::cout << std::left << std::setw(A) << "DELETE FROM <table> <col> <val>" << "Delete rows\n";
    std::cout << std::left << std::setw(A) << "PRINT TABLE <table>" << "Pretty print a table\n";
    std::cout << std::left << std::setw(A) << "PRINT SCHEMA <table>" << "Show table schema\n";
    std::cout << std::left << std::setw(A) << "IMPORT CSV <table> \"path\" [HEADER]" << "Import rows from CSV\n";
    std::cout << std::left << std::setw(A) << "EXIT" << "Quit the program\n";
    line();
    std::cout << "Notes:\n";
    std::cout << "• Use quotes for names/values with spaces.\n";
    std::cout << "• INSERT values must match column count and types.\n";
    std::cout << "• CSV import expects columns in the table’s column order.\n";
    line();
    std::cout << "\n";
}

static void print_rows(const Table* tbl, const std::vector<Row>& rows) {
    if (!tbl) { std::cout << "No table.\n"; return; }
    auto cols = tbl->get_columns();
    if (cols.empty()) { std::cout << "No columns.\n"; return; }
    const int W = 18;
    for (size_t i=0;i<cols.size();++i){ std::cout<<std::setw(W)<<cols[i].name; if(i+1<cols.size()) std::cout<<" | "; }
    std::cout << "\n";
    for (size_t i=0;i<cols.size();++i){ std::cout<<std::string(W,'-'); if(i+1<cols.size()) std::cout<<"-+-"; }
    std::cout << "\n";
    for (const auto& r: rows){
        for (size_t i=0;i<cols.size();++i){
            std::string cell = (i<r.values.size()) ? value_to_string(r.values[i]) : "";
            std::cout<<std::setw(W)<<cell; if(i+1<cols.size()) std::cout<<" | ";
        }
        std::cout << "\n";
    }
    std::cout << "\nRows: " << rows.size() << "\n\n";
}

int main() {
    Database db("DB");
    print_banner();
    std::cout << "Type HELP to see commands.\n\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        auto toks = tokenize(line);
        if (toks.empty()) continue;

        std::string cmd = to_upper(toks[0]);

        if (cmd == "HELP" || cmd == "?") { print_help(); continue; }
        if (cmd == "EXIT" || cmd == "QUIT") { std::cout << "Goodbye!\n"; break; }

        if (cmd == "TABLES") {
            auto names = db.get_table_names();
            if (names.empty()) { std::cout << "(no tables)\n"; continue; }
            std::cout << "Tables:\n";
            for (auto& n : names) std::cout << " - " << n << "\n";
            std::cout << "\n";
            continue;
        }

        if (cmd == "CREATE" && toks.size() >= 3 && to_upper(toks[1]) == "TABLE") {
            std::string tname = trim_quotes(toks[2]);
            bool ok = db.create_table(tname);
            std::cout << (ok ? "OK\n" : "ERR: table exists?\n");
            continue;
        }

        if (cmd == "DROP" && toks.size() >= 3 && to_upper(toks[1]) == "TABLE") {
            std::string tname = trim_quotes(toks[2]);
            bool ok = db.drop_table(tname);
            std::cout << (ok ? "OK\n" : "ERR: no such table\n");
            continue;
        }

        if (cmd == "ADD" && toks.size() >= 5 && to_upper(toks[1]) == "COLUMN") {
            std::string tname = trim_quotes(toks[2]);
            std::string cname = trim_quotes(toks[3]);
            std::string ctype = toks[4];
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            try { t->add_column(cname, parse_type(ctype)); std::cout << "OK\n"; }
            catch (const std::exception& e) { std::cout << "ERR: " << e.what() << "\n"; }
            continue;
        }

        if (cmd == "INSERT" && toks.size() >= 3) {
            std::string tname = trim_quotes(toks[1]);
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = t->get_columns();
            if (cols.empty()) { std::cout << "ERR: define columns first\n"; continue; }
            if (toks.size() - 2 < cols.size()) { std::cout << "ERR: need " << cols.size() << " values\n"; continue; }
            std::vector<Value> vals; vals.reserve(cols.size());
            for (size_t i=0;i<cols.size();++i) vals.push_back(parse_value_token(toks[2+i], cols[i].type));
            bool ok = t->insert_row(vals);
            std::cout << (ok ? "OK\n" : "ERR: type/arity mismatch\n");
            continue;
        }

        if (cmd == "SELECT" && toks.size() >= 3 && to_upper(toks[1]) == "ALL") {
            std::string tname = trim_quotes(toks[2]);
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            print_rows(t, t->select_all());
            continue;
        }

        if (cmd == "SELECT" && toks.size() >= 6 && to_upper(toks[1]) == "WHERE") {
            std::string tname = trim_quotes(toks[2]);
            std::string col   = trim_quotes(toks[3]);
            std::string eq    = toks[4];
            std::string valtk = toks[5];
            if (eq != "=") { std::cout << "ERR: use '='\n"; continue; }
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = t->get_columns();
            auto it = std::find_if(cols.begin(), cols.end(), [&](const Column& c){ return c.name == col; });
            if (it == cols.end()) { std::cout << "ERR: no such column\n"; continue; }
            Value v = parse_value_token(valtk, it->type);
            print_rows(t, t->select_where(col, v));
            continue;
        }

        if (cmd == "UPDATE" && toks.size() >= 6) {
            std::string tname = trim_quotes(toks[1]);
            std::string sc    = trim_quotes(toks[2]);
            std::string svtk  = toks[3];
            std::string uc    = trim_quotes(toks[4]);
            std::string nvtk  = toks[5];
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = t->get_columns();
            auto it1 = std::find_if(cols.begin(), cols.end(), [&](const Column& c){ return c.name == sc; });
            auto it2 = std::find_if(cols.begin(), cols.end(), [&](const Column& c){ return c.name == uc; });
            if (it1 == cols.end() || it2 == cols.end()) { std::cout << "ERR: no such column\n"; continue; }
            Value sv = parse_value_token(svtk, it1->type);
            Value nv = parse_value_token(nvtk, it2->type);
            size_t n = t->update_where(sc, sv, uc, nv);
            std::cout << "UPDATED " << n << "\n";
            continue;
        }

        if (cmd == "DELETE" && toks.size() >= 5 && to_upper(toks[1]) == "FROM") {
            std::string tname = trim_quotes(toks[2]);
            std::string col   = trim_quotes(toks[3]);
            std::string vtok  = toks[4];
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = t->get_columns();
            auto it = std::find_if(cols.begin(), cols.end(), [&](const Column& c){ return c.name == col; });
            if (it == cols.end()) { std::cout << "ERR: no such column\n"; continue; }
            Value v = parse_value_token(vtok, it->type);
            size_t n = t->delete_where(col, v);
            std::cout << "DELETED " << n << "\n";
            continue;
        }

        if (cmd == "PRINT" && toks.size() >= 3 && to_upper(toks[1]) == "TABLE") {
            std::string tname = trim_quotes(toks[2]);
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            t->print_table();
            continue;
        }

        if (cmd == "PRINT" && toks.size() >= 3 && to_upper(toks[1]) == "SCHEMA") {
            std::string tname = trim_quotes(toks[2]);
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            t->print_schema();
            continue;
        }

        if (cmd == "IMPORT" && toks.size() >= 4 && to_upper(toks[1]) == "CSV") {
            std::string tname  = trim_quotes(toks[2]);
            std::string path   = trim_quotes(toks[3]);
            bool header = (toks.size() >= 5 && to_upper(toks[4]) == "HEADER");
            Table* t = db.get_table(tname);
            if (!t) { std::cout << "ERR: no such table\n"; continue; }
            size_t n = t->import_csv(path, header);
            std::cout << "IMPORTED " << n << "\n";
            continue;
        }

        std::cout << "ERR: unknown command. Type HELP.\n";
    }
    return 0;
}