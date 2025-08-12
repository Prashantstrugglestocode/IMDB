cmake_minimum_required(VERSION 3.16)
project(inmemory_db VERSION 0.1.0 LANGUAGES CXX)

# Configure C++ standards
set(CMAKE_CXX_STANDARD 20)          # Use C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Make C++20 mandatory
set(CMAKE_CXX_EXTENSIONS OFF)      # Disable compiler extensions


add_library(inmemory_db_lib
    src/types.cpp
    src/table.cpp
    src/database.cpp
)


target_include_directories(inmemory_db_lib
    PUBLIC include
)

# Create executable target
add_executable(${PROJECT_NAME}
    app/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE inmemory_db_lib  # changed the name from previous due to name conflict
)