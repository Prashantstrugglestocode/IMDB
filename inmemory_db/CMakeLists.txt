cmake_minimum_required(VERSION 3.16)

project(inmemory_db VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIR})

set(SOURCES
  src/types.cpp
  src/table.cpp
  src/database.cpp
)

add_library(imdb_lib STATIC ${SOURCES})
target_include_directories(imdb_lib PUBLIC ${INCLUDE_DIR})

add_executable(inmemory_db app/main.cpp)
set_target_properties(inmemory_db PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_link_libraries(inmemory_db PRIVATE imdb_lib)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/sample)
file(COPY ${CMAKE_SOURCE_DIR}/sample/ DESTINATION ${CMAKE_BINARY_DIR}/sample)

enable_testing()
add_subdirectory(tests)