# CMake generated Testfile for 
# Source directory: /Users/prashant/Documents/In_memory_database/inmemory_db/tests
# Build directory: /Users/prashant/Documents/In_memory_database/inmemory_db/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unit_tests "/Users/prashant/Documents/In_memory_database/inmemory_db/build/tests/imdb_tests")
set_tests_properties(unit_tests PROPERTIES  WORKING_DIRECTORY "/Users/prashant/Documents/In_memory_database/inmemory_db/build" _BACKTRACE_TRIPLES "/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;14;add_test;/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;0;")
add_test(cli_import_count "/bin/bash" "-lc" "printf '%s
' 'CREATE TABLE new_house' 'ADD COLUMN new_house id INT' 'ADD COLUMN new_house address TEXT' 'ADD COLUMN new_house city TEXT' 'ADD COLUMN new_house price INT' 'ADD COLUMN new_house bedrooms INT' 'IMPORT CSV new_house sample/new_house.csv HEADER' 'SELECT ALL new_house' 'PRINT TABLE new_house' 'EXIT' | /Users/prashant/Documents/In_memory_database/inmemory_db/build/bin/inmemory_db")
set_tests_properties(cli_import_count PROPERTIES  PASS_REGULAR_EXPRESSION "IMPORTED 3" WORKING_DIRECTORY "/Users/prashant/Documents/In_memory_database/inmemory_db/build" _BACKTRACE_TRIPLES "/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;17;add_test;/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;0;")
add_test(cli_rows_after_import "/bin/bash" "-lc" "printf '%s
' 'CREATE TABLE new_house' 'ADD COLUMN new_house id INT' 'ADD COLUMN new_house address TEXT' 'ADD COLUMN new_house city TEXT' 'ADD COLUMN new_house price INT' 'ADD COLUMN new_house bedrooms INT' 'IMPORT CSV new_house sample/new_house.csv HEADER' 'SELECT ALL new_house' 'EXIT' | /Users/prashant/Documents/In_memory_database/inmemory_db/build/bin/inmemory_db")
set_tests_properties(cli_rows_after_import PROPERTIES  PASS_REGULAR_EXPRESSION "Rows: 3" WORKING_DIRECTORY "/Users/prashant/Documents/In_memory_database/inmemory_db/build" _BACKTRACE_TRIPLES "/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;37;add_test;/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;0;")
add_test(cli_update "/bin/bash" "-lc" "printf '%s
' 'CREATE TABLE new_house' 'ADD COLUMN new_house id INT' 'ADD COLUMN new_house address TEXT' 'ADD COLUMN new_house city TEXT' 'ADD COLUMN new_house price INT' 'ADD COLUMN new_house bedrooms INT' 'IMPORT CSV new_house sample/new_house.csv HEADER' 'UPDATE new_house city Sunnyvale price 1000000' 'EXIT' | /Users/prashant/Documents/In_memory_database/inmemory_db/build/bin/inmemory_db")
set_tests_properties(cli_update PROPERTIES  PASS_REGULAR_EXPRESSION "UPDATED 1" WORKING_DIRECTORY "/Users/prashant/Documents/In_memory_database/inmemory_db/build" _BACKTRACE_TRIPLES "/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;56;add_test;/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;0;")
add_test(cli_delete "/bin/bash" "-lc" "printf '%s
' 'CREATE TABLE new_house' 'ADD COLUMN new_house id INT' 'ADD COLUMN new_house address TEXT' 'ADD COLUMN new_house city TEXT' 'ADD COLUMN new_house price INT' 'ADD COLUMN new_house bedrooms INT' 'IMPORT CSV new_house sample/new_house.csv HEADER' 'DELETE FROM new_house id 2' 'EXIT' | /Users/prashant/Documents/In_memory_database/inmemory_db/build/bin/inmemory_db")
set_tests_properties(cli_delete PROPERTIES  PASS_REGULAR_EXPRESSION "DELETED 1" WORKING_DIRECTORY "/Users/prashant/Documents/In_memory_database/inmemory_db/build" _BACKTRACE_TRIPLES "/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;75;add_test;/Users/prashant/Documents/In_memory_database/inmemory_db/tests/CMakeLists.txt;0;")
subdirs("../_deps/catch2-build")
