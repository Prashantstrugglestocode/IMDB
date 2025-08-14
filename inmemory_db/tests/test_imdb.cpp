#include <catch2/catch_test_macros.hpp>
#include "imdb/database.hpp"
#include "imdb/table.hpp"
#include "imdb/types.hpp"

using namespace imdb;

TEST_CASE("csv import and basic ops") {
    Database db("T");
    db.create_table("new_house");
    Table* t = db.get_table("new_house");
    REQUIRE(t != nullptr);

    t->add_column("id", ColumnType::Int);
    t->add_column("address", ColumnType::Text);
    t->add_column("city", ColumnType::Text);
    t->add_column("price", ColumnType::Int);
    t->add_column("bedrooms", ColumnType::Int);

    size_t n = t->import_csv("sample/new_house.csv", true);
    REQUIRE(n == 3);

    auto all = t->select_all();
    REQUIRE(all.size() == 3);
}