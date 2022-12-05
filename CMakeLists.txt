cmake_minimum_required(VERSION 3.5)

project(lox1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Release)

set(REGULAR_FLAGS "-Wall -Wextra -Wpedantic \
-Wconversion -Wsign-conversion -Wfloat-equal \
-Wdisabled-optimization")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "/W4 /O2")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
					OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS ${REGULAR_FLAGS})
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto")
	set(CMAKE_CXX_FLAGS_DEBUG "-g3")
	
	#add_compile_options("-fsanitize=address,undefined")
	
	# breaks clang on MSYS2
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		add_link_options("-static")
		add_compile_options("-static")
	endif()
endif()
#message("${CMAKE_SYSTEM_NAME}")

add_executable(lox1
	main.cpp
	scanner.hpp scanner.cpp
	parser.hpp parser.cpp
	io.hpp io.cpp
	
	expr.hpp expr.cpp
	expr_visitor.hpp expr_visitor_fwd.hpp
	stmt.hpp stmt.cpp
	stmt_visitor.hpp stmt_visitor_fwd.hpp
	
	resolver.hpp resolver.cpp
	
	interp.hpp interp.cpp
	token.hpp token.cpp
	ltype.hpp ltype.cpp
	func.hpp func.cpp
	runent.hpp
	assert.hpp assert.cpp
	functional.hpp functional.cpp
	uncopyable.hpp
)
target_link_libraries(lox1 fmt)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_executable(testdriver ${CMAKE_CURRENT_SOURCE_DIR}/testing/main.cpp)
	target_link_libraries(testdriver fmt)
endif()
