cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(imdb_tests test_imdb.cpp)
target_link_libraries(imdb_tests PRIVATE imdb_lib Catch2::Catch2WithMain)
target_include_directories(imdb_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME unit_core_library
         COMMAND imdb_tests -s -r compact)
set_tests_properties(unit_core_library PROPERTIES
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DISPLAY_NAME "Unit: Core library (Catch2)"
)

function(imdb_cli_test name display pass_re)
  set(script "${CMAKE_BINARY_DIR}/testscripts/${name}.sql")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/testscripts")
  set(lines "${ARGN}")
  list(JOIN lines "\n" content)
  file(GENERATE OUTPUT "${script}" CONTENT "${content}\n")
  add_test(NAME ${name}
    COMMAND /bin/bash -lc
      "${CMAKE_BINARY_DIR}/bin/inmemory_db < '${script}' | sed -E 's/^> //' | tr -d '\\r'"
  )
  set_tests_properties(${name} PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DISPLAY_NAME "${display}"
    PASS_REGULAR_EXPRESSION "${pass_re}"
  )
endfunction()

imdb_cli_test(cli_import_csv_counts_3 "CLI: Import CSV counts 3" "IMPORTED 3"
  "CREATE TABLE new_house"
  "ADD COLUMN new_house id INT"
  "ADD COLUMN new_house address TEXT"
  "ADD COLUMN new_house city TEXT"
  "ADD COLUMN new_house price INT"
  "ADD COLUMN new_house bedrooms INT"
  "IMPORT CSV new_house sample/new_house.csv HEADER"
  "SELECT ALL new_house"
  "PRINT TABLE new_house"
  "EXIT"
)

imdb_cli_test(cli_rows_after_import_is_3 "CLI: Rows after import is 3" "Rows: 3"
  "CREATE TABLE new_house"
  "ADD COLUMN new_house id INT"
  "ADD COLUMN new_house address TEXT"
  "ADD COLUMN new_house city TEXT"
  "ADD COLUMN new_house price INT"
  "ADD COLUMN new_house bedrooms INT"
  "IMPORT CSV new_house sample/new_house.csv HEADER"
  "SELECT ALL new_house"
  "EXIT"
)

imdb_cli_test(cli_update_single_row "CLI: Update single row" "UPDATED 1"
  "CREATE TABLE new_house"
  "ADD COLUMN new_house id INT"
  "ADD COLUMN new_house address TEXT"
  "ADD COLUMN new_house city TEXT"
  "ADD COLUMN new_house price INT"
  "ADD COLUMN new_house bedrooms INT"
  "IMPORT CSV new_house sample/new_house.csv HEADER"
  "UPDATE new_house city Sunnyvale price 1000000"
  "EXIT"
)

imdb_cli_test(cli_delete_single_row "CLI: Delete single row" "DELETED 1"
  "CREATE TABLE new_house"
  "ADD COLUMN new_house id INT"
  "ADD COLUMN new_house address TEXT"
  "ADD COLUMN new_house city TEXT"
  "ADD COLUMN new_house price INT"
  "ADD COLUMN new_house bedrooms INT"
  "IMPORT CSV new_house sample/new_house.csv HEADER"
  "DELETE FROM new_house id 2"
  "EXIT"
)

imdb_cli_test(cli_create_insert_select_valid "CLI: Create, Insert, Select (valid)" "Rows: 2"
  "CREATE TABLE t"
  "ADD COLUMN t id INT"
  "ADD COLUMN t name TEXT"
  "INSERT t 1 \"A\""
  "INSERT t 2 \"B\""
  "SELECT ALL t"
  "EXIT"
)

imdb_cli_test(cli_delete_then_select_valid "CLI: Delete then Select (valid)" "Rows: 1"
  "CREATE TABLE t"
  "ADD COLUMN t id INT"
  "ADD COLUMN t name TEXT"
  "INSERT t 1 \"A\""
  "INSERT t 2 \"B\""
  "DELETE FROM t id 1"
  "SELECT ALL t"
  "EXIT"
)

imdb_cli_test(cli_create_duplicate_table_error "CLI: Create duplicate table -> error" "ERR: table exists\\?"
  "CREATE TABLE t"
  "CREATE TABLE t"
  "EXIT"
)

imdb_cli_test(cli_insert_wrong_arity_error "CLI: Insert wrong arity -> error" "need 2 values"
  "CREATE TABLE t"
  "ADD COLUMN t id INT"
  "ADD COLUMN t name TEXT"
  "INSERT t 1"
  "EXIT"
)

imdb_cli_test(cli_select_unknown_column_error "CLI: Select unknown column -> error" "ERR: no such column"
  "CREATE TABLE t"
  "ADD COLUMN t id INT"
  "SELECT WHERE t nope = 1"
  "EXIT"
)

imdb_cli_test(cli_delete_unknown_table_error "CLI: Delete unknown table -> error" "ERR: no such table"
  "DELETE FROM nope id 1"
  "EXIT"
)

imdb_cli_test(cli_join_basic "CLI: JOIN basic" "Rows: 1"
  "CREATE TABLE a"
  "ADD COLUMN a id INT"
  "ADD COLUMN a name TEXT"
  "INSERT a 1 \"x\""
  "INSERT a 2 \"y\""
  "CREATE TABLE b"
  "ADD COLUMN b id INT"
  "ADD COLUMN b note TEXT"
  "INSERT b 1 \"p\""
  "INSERT b 3 \"q\""
  "JOIN a id b id"
  "EXIT"
)

imdb_cli_test(cli_join_type_mismatch_error "CLI: JOIN type mismatch -> error" "ERR"
  "CREATE TABLE a"
  "ADD COLUMN a code TEXT"
  "INSERT a \"x\""
  "CREATE TABLE b"
  "ADD COLUMN b code INT"
  "INSERT b 1"
  "JOIN a code b code"
  "EXIT"
)

imdb_cli_test(cli_join_missing_column_error "CLI: JOIN missing column -> error" "ERR"
  "CREATE TABLE a"
  "ADD COLUMN a id INT"
  "INSERT a 1"
  "CREATE TABLE b"
  "ADD COLUMN b id INT"
  "INSERT b 1"
  "JOIN a nope b id"
  "EXIT"
)