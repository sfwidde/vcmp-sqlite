# SQLite3 module for Vice City: Multiplayer (VC:MP) 0.4 Squirrel servers
Ever felt it awful to script using the official SQLite plugin with no proper
error reporting, or easily forgetting to free your queries because it has to be
done manually? The latter completely defeates the whole purpose of using a
garbage collection scripting language like Squirrel. This plugin fixes those
nuisances, brings a few improvements over the official plugin and, hopefully,
makes it more bearable to script using SQLite in VC:MP.
## Installation steps
1. Head to the
[**Releases** page](https://github.com/sfwidde/vcmp-sqlite/releases/latest),
choose a plugin based on your needs, then download it.
2. Load the plugin by appending its name to the `plugins` line in
**server.cfg**.
## Documentation
All of the functions that we will cover below throw an exception if an error
occurs, unlike the official SQLite plugin where errors are spotted through
return values.
- `SQLiteOpen(string: fileName)`
	- Parameters:
		- `fileName`: database file name.
	- Returns a new database handle used for future manipulation with the newly
	opened database.
- `SQLitePrepare(dbHandle: db, string: sql[, bool: executeOnly])`
	- Parameters:
		- `db`: database handle obtained from a prior successful call to
		`SQLiteOpen()`.
		- `sql`: SQL statement(s) to execute.
		- (optional) `executeOnly`: allows more than one SQL statements,
		separated by semicolons, to be executed in a single function call when
		set to `true`. A value of `false` is mandatory when we want to retrieve
		data from a query i.e. from a `SELECT` statement. Defaults to `false` if
		argument is omitted.
	- Returns:
		- For `SELECT` statements:
			- Nothing if `executeOnly` is `true` regardless.
			- A prepared statement if the query returned any data.
		- `null` otherwise.
- `SQLiteStep(stmt: query)`
	- Parameters:
		- `query`: prepared statement obtained from a prior successful call to
		`SQLitePrepare()`.
	- When this function returns `true` it means it has advanced to the next row
	containing data from the query. When no more rows are available it returns
	`false`. It is a big mistake to call `SQLiteStep()` on the same prepared
	statement again if the function has already returned `false`; if you do
	this, your server could crash!
- `SQLiteColumnCount(stmt: query)`
	- Parameters:
		- `query`: prepared statement obtained from a prior successful call to
		`SQLitePrepare()`.
	- Returns the number of columns for all rows in the query.
- `SQLiteColumnData(stmt: query, int: columnIndex)`
	- Parameters:
		- `query`: prepared statement obtained from a prior successful call to
		`SQLitePrepare()`.
		- `columnIndex`: index of the column we want to retrieve data from.
	- Return type can be of either int, float, string or null, depending on the
	type present at the requested column index in the current row.
- `SQLiteEscapeString(string: string)`
	- Parameters:
		- `string`: string to escape.
	- Returns escaped string as if passed as `%q` format specifier to
	`sqlite3_mprintf()` (see https://www.sqlite.org/printf.html for more
	details).

Check out [**script samples**](samples/) for having a better understanding on
the usage of this plugin.