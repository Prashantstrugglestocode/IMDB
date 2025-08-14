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

add_test(NAME unit_tests COMMAND imdb_tests)
set_tests_properties(unit_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_test(NAME cli_import_count
  COMMAND /bin/bash -lc
  "printf '%s\n' \
'CREATE TABLE new_house' \
'ADD COLUMN new_house id INT' \
'ADD COLUMN new_house address TEXT' \
'ADD COLUMN new_house city TEXT' \
'ADD COLUMN new_house price INT' \
'ADD COLUMN new_house bedrooms INT' \
'IMPORT CSV new_house sample/new_house.csv HEADER' \
'SELECT ALL new_house' \
'PRINT TABLE new_house' \
'EXIT' \
| ${CMAKE_BINARY_DIR}/bin/inmemory_db"
)
set_tests_properties(cli_import_count PROPERTIES
  PASS_REGULAR_EXPRESSION "IMPORTED 3"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME cli_rows_after_import
  COMMAND /bin/bash -lc
  "printf '%s\n' \
'CREATE TABLE new_house' \
'ADD COLUMN new_house id INT' \
'ADD COLUMN new_house address TEXT' \
'ADD COLUMN new_house city TEXT' \
'ADD COLUMN new_house price INT' \
'ADD COLUMN new_house bedrooms INT' \
'IMPORT CSV new_house sample/new_house.csv HEADER' \
'SELECT ALL new_house' \
'EXIT' \
| ${CMAKE_BINARY_DIR}/bin/inmemory_db"
)
set_tests_properties(cli_rows_after_import PROPERTIES
  PASS_REGULAR_EXPRESSION "Rows: 3"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME cli_update
  COMMAND /bin/bash -lc
  "printf '%s\n' \
'CREATE TABLE new_house' \
'ADD COLUMN new_house id INT' \
'ADD COLUMN new_house address TEXT' \
'ADD COLUMN new_house city TEXT' \
'ADD COLUMN new_house price INT' \
'ADD COLUMN new_house bedrooms INT' \
'IMPORT CSV new_house sample/new_house.csv HEADER' \
'UPDATE new_house city Sunnyvale price 1000000' \
'EXIT' \
| ${CMAKE_BINARY_DIR}/bin/inmemory_db"
)
set_tests_properties(cli_update PROPERTIES
  PASS_REGULAR_EXPRESSION "UPDATED 1"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME cli_delete
  COMMAND /bin/bash -lc
  "printf '%s\n' \
'CREATE TABLE new_house' \
'ADD COLUMN new_house id INT' \
'ADD COLUMN new_house address TEXT' \
'ADD COLUMN new_house city TEXT' \
'ADD COLUMN new_house price INT' \
'ADD COLUMN new_house bedrooms INT' \
'IMPORT CSV new_house sample/new_house.csv HEADER' \
'DELETE FROM new_house id 2' \
'EXIT' \
| ${CMAKE_BINARY_DIR}/bin/inmemory_db"
)
set_tests_properties(cli_delete PROPERTIES
  PASS_REGULAR_EXPRESSION "DELETED 1"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)