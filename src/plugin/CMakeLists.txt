add_library(
	sqlite SHARED

	main.c
	console.c
	functions.c
	lib/sqlite3.c
)

# https://stackoverflow.com/a/19310455/14197165
# Uncomment to force 32-bit compilation.
#set_target_properties(sqlite PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
