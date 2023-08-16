cmake_minimum_required(VERSION 3.12)

project(vcmp-sqlite)

# Remove prefix that may be automatically prepended to shared libraries (especially on Linux).
set(CMAKE_SHARED_LIBRARY_PREFIX "")
# Assuming there are 8 bits in a single byte
math(EXPR CPU_BITS "${CMAKE_SIZEOF_VOID_P} * 8")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# Follow VC:MP plugin naming convention (04rel32/64.dll).
	set(CMAKE_SHARED_LIBRARY_SUFFIX 04rel${CPU_BITS}.dll)

	add_compile_definitions(OS_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# Follow VC:MP plugin naming convention (04rel32/64.so).
	set(CMAKE_SHARED_LIBRARY_SUFFIX 04rel${CPU_BITS}.so)

	add_compile_definitions(OS_LINUX)
else()
	message(FATAL_ERROR "project must be compiled either on Windows or Linux operating systems "
		"since VC:MP only supports both of them")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_definitions(COMP_MSVC)
	# Suppress some C4996 warnings.
	# https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4996?view=msvc-170#disable-the-warning-using-preprocessor-macros
	add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_definitions(COMP_GCC)
endif()

include_directories(src/include)

add_subdirectory(src/plugin)
