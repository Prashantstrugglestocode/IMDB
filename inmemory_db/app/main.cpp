#include "imdb/database.hpp"
#include "imdb/table.hpp"
#include "imdb/types.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cctype>
#include <optional>

using namespace imdb;

static std::string trim_quotes(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') return s.substr(1, s.size() - 2);
    return s;
}

static std::string to_upper(std::string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = std::toupper(static_cast<unsigned char>(s[i]));
    return s;
}

static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> out;
    std::string current;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (in_quotes) {
            current.push_back(c);
            if (c == '"') in_quotes = false;
        } else {
            if (c == '"') {
                if (!current.empty()) {
                    out.push_back(current);
                    current.clear();
                }
                current.push_back(c);
                in_quotes = true;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    out.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
    }
    if (!current.empty()) out.push_back(current);
    return out;
}

static std::optional<int64_t> to_int64(const std::string& s) {
    try {
        size_t p = 0;
        long long v = std::stoll(s, &p);
        if (p == s.size()) return static_cast<int64_t>(v);
    } catch (...) {}
    return std::nullopt;
}

static ColumnType parse_type(const std::string& t) {
    std::string u = to_upper(t);
    if (u == "INT" || u == "INTEGER") return ColumnType::Int;
    return ColumnType::Text;
}

static Value parse_value_token(const std::string& token, std::optional<ColumnType> expected) {
    std::string t = token;
    if (!t.empty() && t.front() == '"' && t.back() == '"') t = trim_quotes(t);
    if (expected.has_value()) {
        if (*expected == ColumnType::Int) {
            auto x = to_int64(t);
            if (x.has_value()) return *x;
            return t;
        } else {
            return t;
        }
    }
    auto x = to_int64(t);
    if (x.has_value()) return *x;
    return t;
}

static void print_banner() {
    std::cout << "\n=============================================\n";
    std::cout << "  In-Memory Database CLI\n";
    std::cout << "  Type HELP for commands, EXIT to quit\n";
    std::cout << "=============================================\n\n";
}

static void print_rows(const Table* table, const std::vector<Row>& rows) {
    if (!table) { std::cout << "No table.\n"; return; }
    auto columns = table->get_columns();
    if (columns.empty()) { std::cout << "No columns.\n"; return; }
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

static void print_join(const std::vector<std::string>& headers,
                       const std::vector<std::vector<Value>>& rows) {
    if (headers.empty()) { std::cout << "(empty)\n"; return; }
    const int width = 18;
    for (size_t i = 0; i < headers.size(); i++) {
        std::cout << std::setw(width) << headers[i];
        if (i + 1 < headers.size()) std::cout << " | ";
    }
    std::cout << "\n";
    for (size_t i = 0; i < headers.size(); i++) {
        std::cout << std::string(width, '-');
        if (i + 1 < headers.size()) std::cout << "-+-";
    }
    std::cout << "\n";
    for (size_t r = 0; r < rows.size(); r++) {
        for (size_t c = 0; c < headers.size(); c++) {
            std::string cell = "";
            if (c < rows[r].size()) cell = value_to_string(rows[r][c]);
            std::cout << std::setw(width) << cell;
            if (c + 1 < headers.size()) std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << "\nRows: " << rows.size() << "\n\n";
}

static void print_help() {
    const int a = 32;
    auto line = [](){ std::cout << std::string(70, '-') << "\n"; };
    std::cout << "\n";
    line();
    std::cout << std::left << std::setw(a) << "HELP" << "Show this help\n";
    std::cout << std::left << std::setw(a) << "TABLES" << "List all tables\n";
    std::cout << std::left << std::setw(a) << "CREATE TABLE <name>" << "Create table\n";
    std::cout << std::left << std::setw(a) << "DROP TABLE <name>" << "Drop table\n";
    std::cout << std::left << std::setw(a) << "ADD COLUMN <table> <col> <type>" << "Add column (INT or TEXT)\n";
    std::cout << std::left << std::setw(a) << "ADD CONSTRAINT <table> PRIMARY KEY <col>" << "Set primary key\n";
    std::cout << std::left << std::setw(a) << "ADD CONSTRAINT <table> NOT NULL <col>" << "Set not-null on column\n";
    std::cout << std::left << std::setw(a) << "INSERT <table> <values...>" << "Insert row\n";
    std::cout << std::left << std::setw(a) << "SELECT ALL <table>" << "Show all rows\n";
    std::cout << std::left << std::setw(a) << "SELECT WHERE <table> <col> = <val>" << "Filter rows\n";
    std::cout << std::left << std::setw(a) << "UPDATE <table> <col> <val> <set_col> <new_val>" << "Update rows\n";
    std::cout << std::left << std::setw(a) << "DELETE FROM <table> <col> <val>" << "Delete rows\n";
    std::cout << std::left << std::setw(a) << "JOIN <t1> <c1> <t2> <c2>" << "Inner join and print\n";
    std::cout << std::left << std::setw(a) << "PRINT TABLE <table>" << "Print table\n";
    std::cout << std::left << std::setw(a) << "PRINT SCHEMA <table>" << "Print schema\n";
    std::cout << std::left << std::setw(a) << "IMPORT CSV <table> \"path\" [HEADER]" << "Import CSV\n";
    std::cout << std::left << std::setw(a) << "EXIT" << "Quit\n";
    line();
    std::cout << "Use quotes for names or values with spaces.\n";
    line();
    std::cout << "\n";
}

int main() {
    Database db("DB");
    print_banner();
    std::cout << "Type HELP to see commands.\n\n";

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;
        if (input.empty()) continue;

        std::vector<std::string> tokens = tokenize(input);
        if (tokens.empty()) continue;

        std::string cmd = to_upper(tokens[0]);

        if (cmd == "HELP" || cmd == "?") { print_help(); continue; }
        if (cmd == "EXIT" || cmd == "QUIT") { std::cout << "Goodbye!\n"; break; }

        if (cmd == "TABLES") {
            auto names = db.get_table_names();
            if (names.empty()) { std::cout << "(no tables)\n"; continue; }
            for (size_t i = 0; i < names.size(); i++) std::cout << " - " << names[i] << "\n";
            std::cout << "\n";
            continue;
        }

        if (cmd == "CREATE" && tokens.size() >= 3 && to_upper(tokens[1]) == "TABLE") {
            std::string table_name = trim_quotes(tokens[2]);
            bool ok = db.create_table(table_name);
            if (ok) std::cout << "OK\n"; else std::cout << "ERR: table exists?\n";
            continue;
        }

        if (cmd == "DROP" && tokens.size() >= 3 && to_upper(tokens[1]) == "TABLE") {
            std::string table_name = trim_quotes(tokens[2]);
            bool ok = db.drop_table(table_name);
            if (ok) std::cout << "OK\n"; else std::cout << "ERR: no such table\n";
            continue;
        }

        if (cmd == "ADD" && tokens.size() >= 5 && to_upper(tokens[1]) == "COLUMN") {
            std::string table_name = trim_quotes(tokens[2]);
            std::string col_name = trim_quotes(tokens[3]);
            std::string col_type = tokens[4];
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            try {
                tbl->add_column(col_name, parse_type(col_type));
                std::cout << "OK\n";
            } catch (...) {
                std::cout << "ERR: column exists\n";
            }
            continue;
        }

        if (cmd == "ADD" && tokens.size() >= 6 && to_upper(tokens[1]) == "CONSTRAINT") {
            std::string table_name = trim_quotes(tokens[2]);
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            std::string word3 = to_upper(tokens[3]);
            if (word3 == "PRIMARY" && tokens.size() >= 6 && to_upper(tokens[4]) == "KEY") {
                std::string col_name = trim_quotes(tokens[5]);
                bool ok = tbl->set_primary_key(col_name);
                if (ok) std::cout << "OK\n"; else std::cout << "ERR\n";
                continue;
            }
            if (word3 == "NOT" && tokens.size() >= 6 && to_upper(tokens[4]) == "NULL") {
                std::string col_name = trim_quotes(tokens[5]);
                bool ok = tbl->set_not_null(col_name, true);
                if (ok) std::cout << "OK\n"; else std::cout << "ERR\n";
                continue;
            }
            std::cout << "ERR\n";
            continue;
        }

        if (cmd == "INSERT" && tokens.size() >= 3) {
            std::string table_name = trim_quotes(tokens[1]);
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = tbl->get_columns();
            if (cols.empty()) { std::cout << "ERR: define columns first\n"; continue; }
            if (tokens.size() - 2 < cols.size()) { std::cout << "ERR: need " << cols.size() << " values\n"; continue; }
            std::vector<Value> values;
            values.reserve(cols.size());
            for (size_t i = 0; i < cols.size(); i++) {
                values.push_back(parse_value_token(tokens[2 + i], cols[i].type));
            }
            bool ok = tbl->insert_row(values);
            if (ok) std::cout << "OK\n"; else std::cout << "ERR\n";
            continue;
        }

        if (cmd == "SELECT" && tokens.size() >= 3 && to_upper(tokens[1]) == "ALL") {
            std::string table_name = trim_quotes(tokens[2]);
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            print_rows(tbl, tbl->select_all());
            continue;
        }

        if (cmd == "SELECT" && tokens.size() >= 6 && to_upper(tokens[1]) == "WHERE") {
            std::string table_name = trim_quotes(tokens[2]);
            std::string col_name = trim_quotes(tokens[3]);
            std::string equal_sign = tokens[4];
            std::string value_token = tokens[5];
            if (equal_sign != "=") { std::cout << "ERR\n"; continue; }
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = tbl->get_columns();
            std::optional<ColumnType> col_type;
            for (size_t i = 0; i < cols.size(); i++) {
                if (cols[i].name == col_name) { col_type = cols[i].type; break; }
            }
            if (!col_type) { std::cout << "ERR: no such column\n"; continue; }
            Value v = parse_value_token(value_token, col_type);
            print_rows(tbl, tbl->select_where(col_name, v));
            continue;
        }

        if (cmd == "UPDATE" && tokens.size() >= 6) {
            std::string table_name = trim_quotes(tokens[1]);
            std::string search_col = trim_quotes(tokens[2]);
            std::string search_val_token = tokens[3];
            std::string update_col = trim_quotes(tokens[4]);
            std::string new_val_token = tokens[5];
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = tbl->get_columns();
            std::optional<ColumnType> search_type;
            std::optional<ColumnType> update_type;
            for (size_t i = 0; i < cols.size(); i++) {
                if (cols[i].name == search_col) search_type = cols[i].type;
                if (cols[i].name == update_col) update_type = cols[i].type;
            }
            if (!search_type || !update_type) { std::cout << "ERR: no such column\n"; continue; }
            Value search_value = parse_value_token(search_val_token, search_type);
            Value new_value = parse_value_token(new_val_token, update_type);
            size_t n = tbl->update_where(search_col, search_value, update_col, new_value);
            std::cout << "UPDATED " << n << "\n";
            continue;
        }

        if (cmd == "DELETE" && tokens.size() >= 5 && to_upper(tokens[1]) == "FROM") {
            std::string table_name = trim_quotes(tokens[2]);
            std::string col_name = trim_quotes(tokens[3]);
            std::string value_token = tokens[4];
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            auto cols = tbl->get_columns();
            std::optional<ColumnType> col_type;
            for (size_t i = 0; i < cols.size(); i++) {
                if (cols[i].name == col_name) col_type = cols[i].type;
            }
            if (!col_type) { std::cout << "ERR: no such column\n"; continue; }
            Value v = parse_value_token(value_token, col_type);
            size_t n = tbl->delete_where(col_name, v);
            std::cout << "DELETED " << n << "\n";
            continue;
        }

        if (cmd == "JOIN" && tokens.size() >= 5) {
            std::string t1 = trim_quotes(tokens[1]);
            std::string c1 = trim_quotes(tokens[2]);
            std::string t2 = trim_quotes(tokens[3]);
            std::string c2 = trim_quotes(tokens[4]);
            std::vector<std::string> headers;
            std::vector<std::vector<Value>> rows;
            bool ok = db.inner_join(t1, c1, t2, c2, headers, rows);
            if (!ok) { std::cout << "ERR\n"; continue; }
            print_join(headers, rows);
            continue;
        }

        if (cmd == "PRINT" && tokens.size() >= 3 && to_upper(tokens[1]) == "TABLE") {
            std::string table_name = trim_quotes(tokens[2]);
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            tbl->print_table();
            continue;
        }

        if (cmd == "PRINT" && tokens.size() >= 3 && to_upper(tokens[1]) == "SCHEMA") {
            std::string table_name = trim_quotes(tokens[2]);
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            tbl->print_schema();
            continue;
        }

        if (cmd == "IMPORT" && tokens.size() >= 4 && to_upper(tokens[1]) == "CSV") {
            std::string table_name = trim_quotes(tokens[2]);
            std::string path = trim_quotes(tokens[3]);
            bool header = false;
            if (tokens.size() >= 5 && to_upper(tokens[4]) == "HEADER") header = true;
            Table* tbl = db.get_table(table_name);
            if (!tbl) { std::cout << "ERR: no such table\n"; continue; }
            size_t n = tbl->import_csv(path, header);
            std::cout << "IMPORTED " << n << "\n";
            continue;
        }

        std::cout << "ERR: unknown command. Type HELP.\n";
    }
    return 0;
}