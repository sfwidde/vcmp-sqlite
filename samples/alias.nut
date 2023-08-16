db <- null; // Handle to our database connection.

function onScriptLoad()
{
	// Open a new database connection.
	db = SQLiteOpen("test.db");
	// Create required tables if they don't exist.
	SQLitePrepare(db,
		"CREATE TABLE IF NOT EXISTS player_uids (" +
			"uid  TEXT        NOT NULL COLLATE NOCASE, " +
			"name TEXT UNIQUE NOT NULL" +
		"); " +

		"CREATE TABLE IF NOT EXISTS player_uid2s (" +
			"uid2 TEXT        NOT NULL COLLATE NOCASE, " +
			"name TEXT UNIQUE NOT NULL" +
		"); " +

		"CREATE TABLE IF NOT EXISTS player_ips (" +
			"ip   TEXT        NOT NULL, " +
			"name TEXT UNIQUE NOT NULL" +
		");",
		true); // 'true' so that we can run multiple SQL statements in a single call to
			   // SQLitePrepare().
}

function onScriptUnload()
{
	// Database is closed automatically when its reference count reaches 0 so null'ing this
	// variable should do the job (there is even no need explicitly do it when the server is
	// shutting down but we are doing it anyway just for the sake of this example).
	db = null;
}

function onPlayerJoin(player)
{
	InsertPlayerAlias(player);
}

function onPlayerCommand(player, cmd, text)
{
	switch (cmd = cmd.tolower())
	{
	case "alias":
		if (!text)
		{
			MessagePlayer("/" + cmd + " <player> <UID/UID2/IP>", player);
			return;
		}

		local tokens = split(text, " "/*, true*/);
		local plr = FindPlayer(IsNum(tokens[0]) ? tokens[0].tointeger() : tokens[0]);
		if (!plr)
		{
			MessagePlayer("Unknown player.", player);
			return;
		}

		if (tokens.len() < 2)
		{
			MessagePlayer("Missing alias type.", player);
			return;
		}

		local type = tokens[1].toupper();
		local source;
		switch (type)
		{
		case "UID":
			source = plr.UniqueID;
			break;

		case "UID2":
			source = plr.UniqueID2;
			break;

		case "IP":
			source = plr.IP;
			break;

		default:
			MessagePlayer("Invalid alias type.", player);
			return;
		}

		local plrName = plr.Name;
		// SQLitePrepare()'s third parameter can be omitted too in which case it will default to false!
		local stmt = SQLitePrepare(db, "SELECT name FROM player_" + type + "s WHERE " + type + " = '" + source + "';"/*, false*/);
		// In case our query returned nothing...
		if (!stmt)
		{
			MessagePlayer(plrName + " has no " + type + " aliases.", player);
			return;
		}

		local aliasName;
		local aliasList;
		do
		{
			aliasName = SQLiteColumnData(stmt, 0);
			// Don't include plr name in the alias list because we already know their name...
			if (aliasName == plrName)
			{
				continue;
			}

			aliasList = aliasList ? (aliasList + ", " + aliasName) : aliasName;
		}
		while (SQLiteStep(stmt));
		// stmt = null; // Same effect as if calling FreeSQLQuery() from the official SQLite plugin.

		if (!aliasList)
		{
			MessagePlayer(plrName + " has no " + type + " aliases.", player);
			return;
		}

		MessagePlayer(plrName + "'s " + type + " aliases: " + aliasList + ".", player);
		return;

		// As you can see, we don't call any sort of function (or set 'stmt' to null as explained
		// above) to free the prepared statement just like we would have done if we were using the
		// official SQLite plugin by manually calling FreeSQLQuery() to prevent memory leaks. Just
		// like SQLiteOpen(), once a prepared statement's reference count reaches 0, it gets
		// automatically freed by Squirrel; in this case, the release is done when 'stmt' goes out
		// of scope i.e. when we return from onPlayerCommand().
	}
}

// ------------------------------------------------------------------------------------------------

function InsertPlayerAlias(player)
{
	local name = player.Name;
	SQLitePrepare(db,
		// UID
		"INSERT OR IGNORE INTO player_uids (" +
			"uid, " +
			"name" +
		") VALUES (" +
			"'" + player.UniqueID + "', " +
			"'" + name +"'" +
		"); " +

		// UID2
		"INSERT OR IGNORE INTO player_uid2s (" +
			"uid2, " +
			"name" +
		") VALUES (" +
			"'" + player.UniqueID2 + "', " +
			"'" + name +"'" +
		"); " +

		// IP
		"INSERT OR IGNORE INTO player_ips (" +
			"ip, " +
			"name" +
		") VALUES (" +
			"'" + player.IP + "', " +
			"'" + name +"'" +
		");",
		true);
}
