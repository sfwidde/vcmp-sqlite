#include "../squirrel/SQModule.h"
#include "lib/sqlite3.h"
#include "console.h"
#include <stddef.h>
#ifdef _SQ64
#include <limits.h>
#endif

extern HSQAPI sq;
extern HSQUIRRELVM v;

static SQInteger DatabaseHandleReleaseHook(SQUserPointer, SQInteger);
static SQInteger PreparedStatementReleaseHook(SQUserPointer, SQInteger);

static SQInteger SQLiteOpen(HSQUIRRELVM v)
{
	const SQChar* fileName;
	if (SQ_FAILED(sq->getstring(v, 2, &fileName)))
	{
		return sq->throwerror(v, _SC("unable to retrieve database file name"));
	}

	sqlite3* newDb;
	if (sqlite3_open(fileName, &newDb) != SQLITE_OK)
	{
		const char* errorMessage = sqlite3_errmsg(newDb);
		// https://www.sqlite.org/c3ref/open.html
		// "Whether or not an error occurs when it is opened, resources associated with the
		// database connection handle should be released by passing it to sqlite3_close() when it
		// is no longer required."
		sqlite3_close(newDb);
		return sq->throwerror(v, errorMessage);
	}

	*(sqlite3**)sq->newuserdata(v, sizeof(sqlite3**)) = newDb;
	sq->setreleasehook(v, -1, DatabaseHandleReleaseHook);
	return 1;
}

static SQInteger SQLitePrepare(HSQUIRRELVM v)
{
	SQUserPointer p;
	if (SQ_FAILED(sq->getuserdata(v, 2, &p, NULL)))
	{
		return sq->throwerror(v, _SC("unable to retrieve database handle"));
	}

	const SQChar* sql;
	if (SQ_FAILED(sq->getstring(v, 3, &sql)))
	{
		return sq->throwerror(v, _SC("unable to retrieve SQL statement"));
	}

	SQInteger argumentCount = sq->gettop(v);
	SQBool executeOnly;
	if (argumentCount < 4)
	{
		executeOnly = SQFalse;
	}
	else if (argumentCount > 4)
	{
		return sq->throwerror(v, _SC("wrong number of parameters"));
	}
	else if (sq->gettype(v, 4) != OT_BOOL)
	{
		return sq->throwerror(v, _SC("'execute only' conditional must be of type bool"));
	}
	else if (SQ_FAILED(sq->getbool(v, 4, &executeOnly)))
	{
		return sq->throwerror(v, _SC("unable to retrieve 'execute only' conditional"));
	}

	sqlite3* db = *(sqlite3**)p;
	if (executeOnly)
	{
		return (sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) ? 0 :
			sq->throwerror(v, sqlite3_errmsg(db));
	}
	else
	{
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
		{
			return sq->throwerror(v, sqlite3_errmsg(db));
		}

		if (sqlite3_step(stmt) != SQLITE_ROW)
		{
			sqlite3_finalize(stmt);
			sq->pushnull(v);
			return 1;
		}

		*(sqlite3_stmt**)sq->newuserdata(v, sizeof(sqlite3_stmt**)) = stmt;
		sq->setreleasehook(v, -1, PreparedStatementReleaseHook);
		return 1;
	}
}

static SQInteger SQLiteStep(HSQUIRRELVM v)
{
	SQUserPointer p;
	if (SQ_FAILED(sq->getuserdata(v, 2, &p, NULL)))
	{
		return sq->throwerror(v, _SC("unable to retrieve prepared statement"));
	}

	sq->pushbool(v, sqlite3_step(*(sqlite3_stmt**)p) == SQLITE_ROW);
	return 1;
}

static SQInteger SQLiteColumnCount(HSQUIRRELVM v)
{
	SQUserPointer p;
	if (SQ_FAILED(sq->getuserdata(v, 2, &p, NULL)))
	{
		return sq->throwerror(v, _SC("unable to retrieve prepared statement"));
	}

	sq->pushinteger(v, sqlite3_column_count(*(sqlite3_stmt**)p));
	return 1;
}

static SQInteger SQLiteColumnData(HSQUIRRELVM v)
{
	SQUserPointer p;
	if (SQ_FAILED(sq->getuserdata(v, 2, &p, NULL)))
	{
		return sq->throwerror(v, _SC("unable to retrieve prepared statement"));
	}

	SQInteger columnIndex;
	if (SQ_FAILED(sq->getinteger(v, 3, &columnIndex)))
	{
		return sq->throwerror(v, _SC("unable to retrieve column index"));
	}

	sqlite3_stmt* stmt = *(sqlite3_stmt**)p;
#ifdef _SQ64
	int colIdx = (columnIndex > INT_MAX) ? INT_MAX : (int)columnIndex;
#else
	int colIdx = columnIndex;
#endif
	if (colIdx < 0 || colIdx > (sqlite3_column_count(stmt) - 1))
	{
		return sq->throwerror(v, _SC("column index is out of bounds"));
	}

	switch (sqlite3_column_type(stmt, colIdx))
	{
	case SQLITE_INTEGER:
#ifdef _SQ64
		sq->pushinteger(v, sqlite3_column_int64(stmt, colIdx));
#else
		sq->pushinteger(v, sqlite3_column_int(stmt, colIdx));
#endif
		return 1;

	case SQLITE_FLOAT:
		sq->pushfloat(v, (SQFloat)sqlite3_column_double(stmt, colIdx));
		return 1;

	case SQLITE_TEXT:
	case SQLITE_BLOB:
		sq->pushstring(v, (const SQChar*)sqlite3_column_text(stmt, colIdx), -1);
		return 1;

	case SQLITE_NULL:
		sq->pushnull(v);
		return 1;

	default:
		return sq->throwerror(v, _SC("sqlite3_column_type somehow managed to return something else than "
			"what's covered by the switch statement"));
	}
}

static SQInteger SQLiteEscapeString(HSQUIRRELVM v)
{
	const SQChar* string;
	if (SQ_FAILED(sq->getstring(v, 2, &string)))
	{
		return sq->throwerror(v, _SC("unable to retrieve string to escape"));
	}

	char* escapedString = sqlite3_mprintf("%q", string);
	if (!escapedString)
	{
		return sq->throwerror(v, _SC("unable to escape string"));
	}

	sq->pushstring(v, escapedString, -1);
	sqlite3_free(escapedString);
	return 1;
}

SQInteger DatabaseHandleReleaseHook(SQUserPointer p, SQInteger size)
{
	sqlite3* db = *(sqlite3**)p;
	if (sqlite3_close(db) != SQLITE_OK)
	{
		PrintConsoleMessage(CONSOLE_WARNING_MESSAGE, "Release hook SQLite error: '%s'.", sqlite3_errmsg(db));
	}
	return 1;
}

SQInteger PreparedStatementReleaseHook(SQUserPointer p, SQInteger size)
{
	sqlite3_finalize(*(sqlite3_stmt**)p);
	return 1;
}

// Files in ../squirrel/ and the following function were taken from here:
// https://forum.vc-mp.org/index.php?topic=206.msg3206#msg3206
// This function however is slightly modified for convenience.
static void RegisterSquirrelFunction(SQFUNCTION function, const SQChar* functionName,
	SQInteger parameterCount, const SQChar* parameterMask)
{
	sq->pushroottable(v);
	sq->pushstring(v, functionName, -1);
	sq->newclosure(v, function, 0);
	sq->setparamscheck(v, parameterCount, parameterMask);
	sq->setnativeclosurename(v, -1, functionName);
	sq->newslot(v, -3, SQFalse);
	sq->pop(v, 1);
}

#define REG_SQ_FUNC(function, parameterCount, parameterMask) \
	RegisterSquirrelFunction(function, _SC(#function), parameterCount, _SC(parameterMask))

void RegisterSQLiteFunctions(void)
{
	REG_SQ_FUNC(SQLiteOpen, SQ_MATCHTYPEMASKSTRING, "ts");
	REG_SQ_FUNC(SQLitePrepare, -3, "tus");
	REG_SQ_FUNC(SQLiteStep, SQ_MATCHTYPEMASKSTRING, "tu");
	REG_SQ_FUNC(SQLiteColumnCount, SQ_MATCHTYPEMASKSTRING, "tu");
	REG_SQ_FUNC(SQLiteColumnData, SQ_MATCHTYPEMASKSTRING, "tui");
	REG_SQ_FUNC(SQLiteEscapeString, SQ_MATCHTYPEMASKSTRING, "ts");
}
