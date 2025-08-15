In-Memory Database (C++)

A small, beginner-friendly in-memory database with a simple command-line interface (CLI).
Create tables, add columns, insert/select/update/delete rows, and import data from CSV files.
Written in C++20 with CMake and tested via Catch2 + CTest.

Note: CSV import is supported. CSV export is not included.

⸻

Features
	•	In-memory tables (fast, no external dependencies)
	•	Column types: INT, TEXT
	•	Commands for create/drop table, add column, insert/select/update/delete
	•	Pretty PRINT TABLE and PRINT SCHEMA
	•	CSV import with optional header row
	•	Test suite (Catch2 + CTest)
	•	Works on macOS/Linux/Windows with a C++20 compiler

⸻

Build

Requirements
	•	CMake ≥ 3.16
	•	A C++20 compiler (Apple Clang/Clang/GCC/MSVC)

# from the project root
rm -rf build
cmake -S . -B build
cmake --build build -j

The CLI will be at:

build/bin/inmemory_db

Run it:

./build/bin/inmemory_db

You’ll see:

=============================================
  In-Memory Database CLI
  Type HELP for commands, EXIT to quit
=============================================

Type HELP to see commands.

>


⸻

CLI Cheat Sheet

Use quotes for names/values with spaces, e.g. "new house" or "The Godfather".

HELP
TABLES

CREATE TABLE <name>
DROP TABLE <name>

ADD COLUMN <table> <column> <type>      # type: INT | TEXT

INSERT <table> <val1> <val2> ...        # same order as columns

SELECT ALL <table>
SELECT WHERE <table> <column> = <value>

UPDATE <table> <find_col> <find_val> <set_col> <new_val>
DELETE FROM <table> <column> <value>

PRINT TABLE <table>
PRINT SCHEMA <table>

IMPORT CSV <table> "<path>" [HEADER]
EXIT

Example Session

> CREATE TABLE movies
OK
> ADD COLUMN movies id INT
OK
> ADD COLUMN movies title TEXT
OK
> INSERT movies 1 "The Godfather"
OK
> SELECT ALL movies
> UPDATE movies id 1 title "The Godfather (1972)"
UPDATED 1
> PRINT TABLE movies
> PRINT SCHEMA movies
> EXIT


⸻

CSV Import

Syntax:

IMPORT CSV <table> "<path>" [HEADER]

Rules:
	•	If the CSV has a header row, add HEADER.
	•	CSV columns must appear in the same order as your table’s columns.
	•	INT columns must hold valid integers.
	•	Paths can be relative or absolute. Use quotes if the path has spaces.
	•	The path is resolved from your current working directory when you launch the app.
To avoid confusion, run the CLI from the project root or use an absolute path.

Example

Create table and columns:

> CREATE TABLE new_house
OK
> ADD COLUMN new_house id INT
OK
> ADD COLUMN new_house address TEXT
OK
> ADD COLUMN new_house city TEXT
OK
> ADD COLUMN new_house price INT
OK
> ADD COLUMN new_house bedrooms INT
OK

Import CSV:

> IMPORT CSV new_house "sample/new_house.csv" HEADER
IMPORTED 3
> SELECT ALL new_house

sample/new_house.csv:

id,address,city,price,bedrooms
1,12 Oak St,San Jose,900000,3
2,55 Pine Ave,San Mateo,750000,2
3,88 River Rd,Sunnyvale,1200000,4


⸻

Run Tests

The project includes unit tests and CLI tests.

# after building
cd build
ctest --output-on-failure

All tests should pass. The test runner uses the sample CSV under sample/.

⸻

Project Structure

app/
  main.cpp                # CLI
include/imdb/
  database.hpp            # Database API
  table.hpp               # Table API
  types.hpp               # Types and helpers
src/
  database.cpp
  table.cpp
  types.cpp
sample/
  new_house.csv           # Example CSV used by tests
tests/
  test_imdb.cpp           # Catch2 tests
CMakeLists.txt
README.md


⸻

Troubleshooting
	•	IMPORTED 0 and “cannot open file”: the path is wrong for your current working directory.
Try an absolute path or run the CLI from the project root.
	•	ERR: type/arity mismatch: your INSERT value count/types don’t match the schema.
	•	ERR: no such table/column: verify spelling and that you created the table/columns first.
	•	Paths with spaces must be quoted:
IMPORT CSV table "/Users/me/Data Sets/file.csv" HEADER

⸻

License

For learning/class projects. Use as you like.