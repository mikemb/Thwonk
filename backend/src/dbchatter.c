/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Communicate with the database
 *
 * NOTE: Most of the functions below expect _myconn to have
 * 	been setup with a database connection before they
 * 	get called (via dbConnect())
*/

#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include "setupthang.h"
#include "logerror.h"
#include "dbchatter.h"


/*
 * Purpose: Opens a database connection
 *
 * Entry:
 * 	1st - Username to connect with
 * 	2nd - Password
 * 	3rd - Database name
 * 	4th - Database host
 *
 * Exit:
 * 	True  - Active database connection stored internally
 * 	False - NULL if couldn't connect and _errno set to failure
 * 		  type
 *
 * NOTE: This method expects setupGlobals and parseConfig() to
 *  have populated the global config structure
*/
bool dbConnect() {

	/* Already connected? */
	if(_myconn != NULL)
		return true;

	/* Setup mysql structs and connect */
	_myconn = mysql_init(NULL);

	if(!mysql_real_connect(_myconn, _config->dbserver, _config->dbuser,
		_config->dbpword, _config->dbname, 0, _config->dbsocket, 0)) {

		_myconn = NULL;
		setErrType(ERR_DB_OPEN);
#ifdef DEBUG
		write2Log(getErrTypeMsg());
#endif
		return false;
	}

	setErrType(ERR_NONE);

#ifdef DEBUG
	write2Log("- Connected to DB");
#endif

	return true;
}


/*
 * Purpose: Closes a database connection
 *
 * Entry:
 * 	1st - Database connection to close
 *
 * Exit:
 * 	NONE
*/
void dbDisconnect() {

	if(_myconn != NULL) {
		mysql_close(_myconn);
	}
}


/*
 * Purpose: Escape a string so it can be inserted into a database
 *
 * Entry:
 * 	1st - String to escape
 * 	2nd - Length of string
 *
 * Exit:
 * 	Pointer to a newly malloc'ed block holding the escaped string
 * 	NULL on error and err type set
 *
 * Note: Unsafe assumption (TODO) because the size_t length is 
 *  converted into an unsigned long in mysql_real_escape_string
*/
char *dbEscapeString(char *input, size_t length) {
	char *out;

	if((out = (char *)malloc((length * 2) + 1)) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	mysql_real_escape_string(_myconn, out, input, length);

	return out;
}


/*
 * Purpose: Carry out a database query & store results if available
 *
 * Entry:
 * 	1st - printf like sql query
 * 	X -  any number of args (see man printf)
 *
 * Exit:
 * 	SUCCESS = query result set is available for access
 * 		via dbQueryGetRow(), or if no rows returned but
 * 		query worked then NULL returned and error type set
 * 		to ERR_NONE
 * 	FAILURE = NULL, if query failed err type is set to value
 * 		that is not ERR_NONE
*/
DBRESULT *dbQuery(const char *fmt, ...) {
	DBRESULT *result;
	char *query = NULL;
	size_t length = 0, safe_length = 0;
	va_list ap;

#ifdef DEBUG
    // MIKE: Temp removed
//	write2Log("- Perform SQL Query");
//	write2Log("  * format: %s", fmt);
#endif

	// If dbQuery() is used to update without an err NULL is returned and error type is set
	//  to ERR_NONE (which should be checked by calling code)
	setErrType(ERR_NONE);

	// Figure out length of end string, relies on vsnprint conforming to C99 standard
	va_start(ap, fmt);

	length = vsnprintf(NULL, 0, fmt, ap);
	length += 1;				// Count in trailing \0

	va_end(ap);

	// Apparently on some platform vsnprintf destroys ap so it has to be created again
	va_start(ap, fmt);

	if(length > MAX_LENGTH_DB_QUERY || length < 0) {
		setErrType(ERR_DB_QUERY);
		return NULL;
	}

	if((query = (char *)malloc(length)) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	// Now create the query and lets be ubber parnoid about errors
	safe_length = vsnprintf(query, length, fmt, ap);

	va_end(ap);

#ifdef DEBUG
//	write2Log("  * query: %s", query);
#endif

	if(safe_length < 0 || safe_length > length || safe_length > MAX_LENGTH_DB_QUERY) {
		setErrType(ERR_DB_QUERY);
		free(query);
		return NULL;
	}

	if(mysql_real_query(_myconn, query, safe_length) != 0) {
		setErrType(ERR_DB_QUERY);
		free(query);
		return NULL;
	}

	free(query);

	// Query done, return a result set if available (depends on query type)
	result = mysql_store_result(_myconn);

	return result;
}


/*
 * Purpose: Return a count of number of rows returned or affected by
 * 	query
 *
 * On Entry:
 * 	1st - Point to results structure
 *
 * On Exit:
 * 	SUCCESS >= 0 (count of returned or affected rows)
 *	FAILURE = FAILURE (< 0)
*/
int dbQueryCountRows(DBRESULT *result) {

	if(result != NULL) {
		return mysql_num_rows(result);
	} else if(mysql_field_count(_myconn) == 0) {
		return mysql_affected_rows(_myconn);
	}

	return FAILURE;
}


/*
 * Purpose: Returns a row of the result set generated by a successful
 * 	dbQuery() such as SELECT, etc
 *
 * Entry:
 * 	1st - Result set generated by query
 *
 * Exit:
 * 	NULL if now more rows available or an array containing the next
 * 	row from result set
*/
DBROW dbQueryGetRow(DBRESULT *result) {
	int i;

	i = dbQueryCountRows(result);

	if(i <= 0 || i == FAILURE) {
		return NULL;
	}

	return mysql_fetch_row(result);
}


/*
 * Purpose: Free the memory that may be allocated by a call to dbQuery()
 *
 * On Entry:
 * 	1st - Pointer to results structure
 *
 * On Exit:
 * 	NONE (result = NULL)
*/
void dbQueryFreeResult(DBRESULT *result) {

	if(result != NULL)
		mysql_free_result(result);

	result = NULL;
}


/*
 * Purpose: Return the last auto_increment id generated during a DB INSERT
 *
 * On Entry:
 * 	NONE
 *
 * On Exit:
 * 	NONE (result = NULL)
*/
long dbQueryLastInsertId() {

	return mysql_insert_id(_myconn);
}
