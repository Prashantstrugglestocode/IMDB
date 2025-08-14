#pragma once
#include "table.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace imdb {
    
    class Database {
    private:
        std::string database_name_;
        std::unordered_map<std::string, std::unique_ptr<Table>> tables_;
        
    public:
       
        explicit Database(const std::string& name);
        
      
        ~Database() = default;
        
        
        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
        
        
        Database(Database&& other) noexcept = default;
        Database& operator=(Database&& other) noexcept = default;
        
       
        bool create_table(const std::string& table_name);
        bool drop_table(const std::string& table_name);
        Table* get_table(const std::string& table_name);
        const Table* get_table(const std::string& table_name) const;
        
        
        bool table_exists(const std::string& table_name) const;
        std::vector<std::string> get_table_names() const;
        size_t get_table_count() const noexcept { return tables_.size(); }
        std::string get_database_name() const { return database_name_; }
        
        
        void print_database_info() const;
        void print_all_tables() const;
        
        
        void clear_all_tables();
        size_t get_total_rows() const;
        
        
        bool rename_table(const std::string& old_name, const std::string& new_name);
        std::vector<std::string> search_tables_by_column(const std::string& column_name) const;
        
        
        bool create_tables(const std::vector<std::string>& table_names);
        size_t drop_tables(const std::vector<std::string>& table_names);
    };
}