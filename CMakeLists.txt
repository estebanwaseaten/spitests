cmake_minimum_required(VERSION 3.0.0)

project(spitests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall ) # -Wextra)

set(SOURCE_LIST_SPISEND
	"spisend_src/spisend.cpp"
)

set(SOURCE_LIST_LOOPBACK
	"spiloopback_src/spitests.cpp"
)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")

#defines the target:
add_executable(loopback ${SOURCE_LIST_LOOPBACK})
add_executable(spisend ${SOURCE_LIST_SPISEND})
