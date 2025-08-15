#include <catch2/catch_test_macros.hpp>
#include "imdb/database.hpp"
#include "imdb/table.hpp"
#include "imdb/types.hpp"
#include <filesystem>
#include <fstream>

using namespace imdb;
namespace fs = std::filesystem;

static fs::path write_csv() {
    fs::path dir = "inmemory_db/tests/sample";
    fs::create_directories(dir);
    fs::path p = dir / "new_house.csv";
    std::ofstream out(p.string());
    out << "id,address,city,price,bedrooms\n";
    out << "1,One,Sunnyvale,1000000,3\n";
    out << "2,Two,San Jose,850000,2\n";
    out << "3,Three,MountainView,1200000,4\n";
    return p;
}

TEST_CASE("CSV_IMPORT_CHECK") {
    Database db("T");
    db.create_table("new_house");
    Table* t = db.get_table("new_house");
    t->add_column("id", ColumnType::Int);
    t->add_column("address", ColumnType::Text);
    t->add_column("city", ColumnType::Text);
    t->add_column("price", ColumnType::Int);
    t->add_column("bedrooms", ColumnType::Int);
    REQUIRE(t->set_primary_key("id"));
    REQUIRE(t->set_not_null("address", true));
    auto csv = write_csv();
    size_t n = t->import_csv(csv.string(), true);
    REQUIRE(n == 3);
    REQUIRE(t->row_count() == 3);
}

TEST_CASE("pk_duplicate_insert_and_update") {
    Database db("T");
    db.create_table("new_house");
    Table* t = db.get_table("new_house");
    t->add_column("id", ColumnType::Int);
    t->add_column("address", ColumnType::Text);
    t->add_column("city", ColumnType::Text);
    t->add_column("price", ColumnType::Int);
    t->add_column("bedrooms", ColumnType::Int);
    REQUIRE(t->set_primary_key("id"));
    REQUIRE(t->set_not_null("address", true));
    auto csv = write_csv();
    t->import_csv(csv.string(), true);
    bool ok = t->insert_row({ int64_t(1), std::string("new jersey"), std::string("no idea"), int64_t(1), int64_t(1) });
    REQUIRE_FALSE(ok);
    size_t changed = t->update_where("id", int64_t(2), "id", int64_t(1));
    REQUIRE(changed == 0);
    changed = t->update_where("id", int64_t(2), "id", Value(std::monostate{}));
    REQUIRE(changed == 0);
    changed = t->update_where("id", int64_t(2), "id", int64_t(20));
    REQUIRE(changed == 1);
    auto a = t->select_where("id", int64_t(2));
    auto b = t->select_where("id", int64_t(20));
    REQUIRE(a.size() == 0);
    REQUIRE(b.size() == 1);
}

TEST_CASE("not_null_insert_and_update") {
    Database db("T");
    db.create_table("new_house");
    Table* t = db.get_table("new_house");
    t->add_column("id", ColumnType::Int);
    t->add_column("address", ColumnType::Text);
    t->add_column("city", ColumnType::Text);
    t->add_column("price", ColumnType::Int);
    t->add_column("bedrooms", ColumnType::Int);
    REQUIRE(t->set_primary_key("id"));
    REQUIRE(t->set_not_null("address", true));
    auto csv = write_csv();
    t->import_csv(csv.string(), true);
    bool ok = t->insert_row({ int64_t(10), Value(std::monostate{}), std::string("Z"), int64_t(1), int64_t(1) });
    REQUIRE_FALSE(ok);
    size_t changed = t->update_where("id", int64_t(1), "address", Value(std::monostate{}));
    REQUIRE(changed == 0);
    REQUIRE(t->set_not_null("city", true));
    ok = t->insert_row({ int64_t(50), std::string("A"), Value(std::monostate{}), int64_t(1), int64_t(1) });
    REQUIRE_FALSE(ok);
    changed = t->update_where("id", int64_t(1), "city", Value(std::monostate{}));
    REQUIRE(changed == 0);
}

TEST_CASE("select_delete_remove_column") {
    Database db("T");
    db.create_table("new_house");
    Table* t = db.get_table("new_house");
    t->add_column("id", ColumnType::Int);
    t->add_column("address", ColumnType::Text);
    t->add_column("city", ColumnType::Text);
    t->add_column("price", ColumnType::Int);
    t->add_column("bedrooms", ColumnType::Int);
    REQUIRE(t->set_primary_key("id"));
    REQUIRE(t->set_not_null("address", true));
    auto csv = write_csv();
    t->import_csv(csv.string(), true);
    auto s1 = t->select_where("city", std::string("Sunnyvale"));
    REQUIRE(s1.size() == 1);
    size_t removed = t->delete_where("id", int64_t(3));
    REQUIRE(removed == 1);
    REQUIRE(t->row_count() == 2);
    REQUIRE_FALSE(t->remove_column("id"));
    REQUIRE(t->remove_column("bedrooms"));
    REQUIRE(t->column_count() == 4);
}

TEST_CASE("set_pk_rejects_bad_existing_data") {
    Database db("T");
    db.create_table("houses");
    Table* t = db.get_table("houses");
    t->add_column("id", ColumnType::Int);
    t->add_column("address", ColumnType::Text);
    REQUIRE(t->insert_row({ int64_t(1), std::string("A") }));
    REQUIRE(t->insert_row({ int64_t(1), std::string("B") }));
    REQUIRE_FALSE(t->set_primary_key("id"));
    db.create_table("houses2");
    Table* t2 = db.get_table("houses2");
    t2->add_column("id", ColumnType::Int);
    t2->add_column("address", ColumnType::Text);
    REQUIRE(t2->insert_row({ Value(std::monostate{}), std::string("C") }));
    REQUIRE_FALSE(t2->set_primary_key("id"));
}

TEST_CASE("inner_join_basic") {
    Database db("T");
    db.create_table("a");
    db.create_table("b");
    Table* a = db.get_table("a");
    Table* b = db.get_table("b");
    a->add_column("id", ColumnType::Int);
    a->add_column("name", ColumnType::Text);
    b->add_column("id", ColumnType::Int);
    b->add_column("note", ColumnType::Text);
    REQUIRE(a->insert_row({ int64_t(1), std::string("x") }));
    REQUIRE(a->insert_row({ int64_t(2), std::string("y") }));
    REQUIRE(b->insert_row({ int64_t(1), std::string("p") }));
    REQUIRE(b->insert_row({ int64_t(3), std::string("q") }));
    std::vector<std::string> headers;
    std::vector<std::vector<Value>> rows;
    REQUIRE(db.inner_join("a", "id", "b", "id", headers, rows));
    REQUIRE(rows.size() == 1);
    REQUIRE(headers.size() == 4);
}

TEST_CASE("inner_join_no_match") {
    Database db("T");
    db.create_table("a");
    db.create_table("b");
    Table* a = db.get_table("a");
    Table* b = db.get_table("b");
    a->add_column("id", ColumnType::Int);
    a->add_column("name", ColumnType::Text);
    b->add_column("id", ColumnType::Int);
    b->add_column("note", ColumnType::Text);
    a->insert_row({ int64_t(1), std::string("x") });
    a->insert_row({ int64_t(2), std::string("y") });
    b->insert_row({ int64_t(3), std::string("z") });
    std::vector<std::string> headers;
    std::vector<std::vector<Value>> rows;
    REQUIRE(db.inner_join("a", "id", "b", "id", headers, rows));
    REQUIRE(headers.size() == 4);
    REQUIRE(rows.size() == 0);
}

TEST_CASE("inner_join_multi_match") {
    Database db("T");
    db.create_table("a");
    db.create_table("b");
    Table* a = db.get_table("a");
    Table* b = db.get_table("b");
    a->add_column("id", ColumnType::Int);
    a->add_column("name", ColumnType::Text);
    b->add_column("id", ColumnType::Int);
    b->add_column("note", ColumnType::Text);
    a->insert_row({ int64_t(1), std::string("x") });
    b->insert_row({ int64_t(1), std::string("p") });
    b->insert_row({ int64_t(1), std::string("q") });
    std::vector<std::string> headers;
    std::vector<std::vector<Value>> rows;
    REQUIRE(db.inner_join("a", "id", "b", "id", headers, rows));
    REQUIRE(rows.size() == 2);
    REQUIRE(std::get<int64_t>(rows[0][0]) == 1);
    REQUIRE(std::get<std::string>(rows[0][1]) == "x");
    REQUIRE(std::get<int64_t>(rows[1][0]) == 1);
    REQUIRE(std::get<std::string>(rows[1][1]) == "x");
}

TEST_CASE("inner_join_type_mismatch_returns_false") {
    Database db("T");
    db.create_table("a");
    db.create_table("b");
    Table* a = db.get_table("a");
    Table* b = db.get_table("b");
    a->add_column("code", ColumnType::Text);
    b->add_column("code", ColumnType::Int);
    a->insert_row({ std::string("1") });
    b->insert_row({ int64_t(1) });
    std::vector<std::string> headers;
    std::vector<std::vector<Value>> rows;
    REQUIRE_FALSE(db.inner_join("a", "code", "b", "code", headers, rows));
}

TEST_CASE("inner_join_missing_table_or_column_returns_false") {
    Database db("T");
    db.create_table("a");
    Table* a = db.get_table("a");
    a->add_column("id", ColumnType::Int);
    a->insert_row({ int64_t(1) });
    std::vector<std::string> headers;
    std::vector<std::vector<Value>> rows;
    REQUIRE_FALSE(db.inner_join("a", "id", "b", "id", headers, rows));
    REQUIRE_FALSE(db.inner_join("a", "nope", "a", "id", headers, rows));
}