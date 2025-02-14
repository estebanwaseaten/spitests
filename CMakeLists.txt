cmake_minimum_required(VERSION 3.0.0)

project(spitests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall ) # -Wextra)

set(SOURCE_LIST
	"spitests.cpp"
)

set(LIBRARIES_LIST
)

# if libs have to be compiled --> calls CMakeLists.txt in those dirs: ???
foreach(LIBRARY ${LIBRARIES_LIST} )
	add_subdirectory(${LIBRARY} ${LIBRARY}/build)				#  # first ${LIBRARY} for source dir, second for binary dir (i.e. the dir where all the output files are put, but not the library binary)
endforeach()


#defines the target:
add_executable(${PROJECT_NAME} ${SOURCE_LIST})

set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}
)
