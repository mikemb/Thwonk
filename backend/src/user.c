/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Manage users in database
*/

#include<stdlib.h>
#include "setupthang.h"
#include "logerror.h"
#include "user.h"
#include "dbchatter.h"
#include "misc.h"


/*
 * Purpose: Create a User Entry struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated User_Entry
 * 	FAILURE = NULL, and err type set
 */
User_Entry *createUserEntry() {
	User_Entry *uentry;

	if((uentry = (User_Entry *)malloc(sizeof(User_Entry))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	uentry->id = UNSET;
	uentry->username = NULL;
	uentry->accType = UNSET;
	uentry->emailId = UNSET;

	return uentry;
}


/*
 * Purpose: Free memory associated with User_Entry structure
 *
 * Entry:
 * 	1st - Pointer to a User_Entry
 *
 * Exit:
 * 	NONE (uentry set to NULL)
 */
void freeUserEntry(User_Entry *uentry) {

	if(uentry == NULL)
		return;

	if(uentry->username != NULL)
		free(uentry->username);

	free(uentry);

	uentry = NULL;
}


/*
 * Purpose: Get the details about a user associated with an email address
 *
 * Entry:
 * 	1st - Email address of user to fetch details of
 *
 * Exit:
 * 	SUCCESS = User_Entry found
 * 	FAILURE = NULL, user not found or an err
 */
User_Entry *getUserEntryByEmail(char *email) {
	User_Entry *uentry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT user.id, user.username, user.accType, user.emailId FROM user, filter_user WHERE filter_user.identifier = '%s' AND filter_user.id = user.emailId LIMIT 1", email);

	if(getErrType() != ERR_NONE) {
		return NULL;
	}

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	uentry = createUserEntry();

	uentry->id = atol(row[0]);
	uentry->username = mStrndup(row[1], MAX_LENGTH_TEXT_STRING);
	uentry->accType = atoi(row[2]);
	uentry->emailId = atol(row[3]);

	dbQueryFreeResult(result);

	if(uentry->username == NULL) {
		freeUserEntry(uentry);
		return NULL;
	}

	return uentry;
}


/*
 * Purpose: Get the details about a user associated with a username
 *
 * Entry:
 * 	1st - Username of user to fetch details of
 *
 * Exit:
 * 	SUCCESS = User_Entry found
 * 	FAILURE = NULL, user not found or an err
 *
 * TODO: getUserEntryByUsername() + getUserEntryByEmail() can share most code
 */
User_Entry *getUserEntryByUsername(char *username) {
	User_Entry *uentry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, username, accType, emailId FROM user WHERE username = '%s'", username);

	if(getErrType() != ERR_NONE) {
		return NULL;
	}

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	uentry = createUserEntry();

	uentry->id = atol(row[0]);
	uentry->username = mStrndup(row[1], MAX_LENGTH_TEXT_STRING);
	uentry->accType = atoi(row[2]);
	uentry->emailId = atol(row[3]);

	dbQueryFreeResult(result);

	if(uentry->username == NULL) {
		freeUserEntry(uentry);
		return NULL;
	}

	return uentry;
}


/*
 * Purpose: Create a User Filter Struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated User_Filter
 * 	FAILURE = NULL, and err type set
 */
User_Filter *createUserFilter() {
	User_Filter *ufilter;

	if((ufilter = (User_Filter *)malloc(sizeof(User_Filter))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	ufilter->id = UNSET;
	ufilter->identifier = NULL;
	ufilter->filterType = UNSET;
	ufilter->status = UNSET;
	ufilter->userId = UNSET;

	return ufilter;
}


/*
 * Purpose: Free memory associated with User_Filter structure
 *
 * Entry:
 * 	1st - Pointer to a User_Filter
 *
 * Exit:
 * 	NONE (ufilter set to NULL)
 */
void freeUserFilter(User_Filter *ufilter) {

	if(ufilter == NULL)
		return;

	if(ufilter->identifier != NULL)
		free(ufilter->identifier);

	free(ufilter);

	ufilter = NULL;
}


/*
 * Purpose: Get the details about a user filter based on the
 * 	identifier of the filter
 *
 * Entry:
 * 	1st - Type of filter to find (e.g. email)
 * 	2nd - Name of user identifier to fetch details of
 *
 * Exit:
 * 	SUCCESS = User_Filter with user filter details
 * 	FAILURE = NULL, if void not found or an error
 */
User_Filter *getUserFilterByIdentifier(long filterType, char *name) {
	User_Filter *ufilter;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, status, userId FROM filter_user WHERE identifier = '%s' AND filterType = '%ld' LIMIT 1", name, filterType);

	if(getErrType() != ERR_NONE) {
		return NULL;
	}

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((ufilter = createUserFilter()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	ufilter->id = atol(row[0]);
	ufilter->identifier = mStrndup(name, MAX_LENGTH_TEXT_STRING);
	ufilter->filterType = filterType;
	ufilter->status = atol(row[1]);
	ufilter->userId = atol(row[2]);

	dbQueryFreeResult(result);

	if(ufilter->identifier == NULL) {
		freeUserFilter(ufilter);
		return NULL;
	}

	return ufilter;
}


/*
 * Purpose: Get the details about a user filter based on the
 * 	id of the filter
 *
 * Entry:
 * 	1st - Unique id of user filter to get
 *
 * Exit:
 * 	SUCCESS = User_Filter with user filter details
 * 	FAILURE = NULL, if void not found or an error
 */
User_Filter *getUserFilterById(long id) {
	User_Filter *ufilter;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, identifier, filterType, status, userId FROM filter_user WHERE id = '%ld' LIMIT 1", id);

	if(getErrType() != ERR_NONE) {
		return NULL;
	}

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((ufilter = createUserFilter()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	ufilter->id = atol(row[0]);
	ufilter->identifier = mStrndup(row[1], MAX_LENGTH_TEXT_STRING); //mStrndup(name, MAX_LENGTH_TEXT_STRING);
	ufilter->filterType = atol(row[2]);
	ufilter->status = atol(row[3]);
	ufilter->userId = atol(row[4]);

	dbQueryFreeResult(result);

	if(ufilter->identifier == NULL) {
		freeUserFilter(ufilter);
		return NULL;
	}

	return ufilter;
}


/*
 * Purpose: Get the details about a user filter based on the
 * 	user id of the filter
 *
 * Entry:
 * 	1st - Unique id of user filter to get
 *
 * Exit:
 * 	SUCCESS = User_Filter with user filter details
 * 	FAILURE = NULL, if void not found or an error
 *
 * Note: This maybe not be a 1 to 1 mapping, depending on
 *  how code goes a userId maybe be associated with multiple
 *  user_filters
 */
User_Filter *getUserFilterByUserId(long userId) {
	User_Filter *ufilter;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, identifier, filterType, status, userId FROM filter_user WHERE userId = '%ld' LIMIT 1", userId);

	if(getErrType() != ERR_NONE) {
		return NULL;
	}

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((ufilter = createUserFilter()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	ufilter->id = atol(row[0]);
	ufilter->identifier = mStrndup(row[1], MAX_LENGTH_TEXT_STRING); //mStrndup(name, MAX_LENGTH_TEXT_STRING);
	ufilter->filterType = atol(row[2]);
	ufilter->status = atol(row[3]);
	ufilter->userId = atol(row[4]);

	dbQueryFreeResult(result);

	if(ufilter->identifier == NULL) {
		freeUserFilter(ufilter);
		return NULL;
	}

	return ufilter;
}


/*
 * Purpose: Creates (or gets) a User Filter for users that don't have a
 * 	full Thwonk account, that is we have a destination address but
 * 	no agreed signup to Thwonk
 *
 * Entry:
 * 	1st - Void id to create associated user_filter for
 * 	2nd - Email address to add user_filter for
 *
 * Exit:
 * 	SUCCESS = User_Filter *  of the member
 * 	FAILURE = NULL, no user identified for this void/username combination
 *
 * Note: Bumplist kludge, remove later
*/
User_Filter *insertUserFilterMemberUnknown(char *member, long filterType, long status) {
	User_Filter *ufilter;
	User_Entry *uentry;
	DBRESULT *result;
	long uid, eid;

	if((uentry = getUserEntryByUsername(member)) == NULL) {

		// User doesn't exist, create a new one
		result = dbQuery("INSERT INTO user (username, pword, lastLogin, accType, emailId) VALUES ('%s', PASSWORD('nologin'), now(), 1000, 11)", member);

		dbQueryFreeResult(result);

		if(getErrType() != ERR_NONE) {
			return NULL;
		}

		// Get table id for user that was inserted
		if((uid = dbQueryLastInsertId()) == 0)
			return NULL;

		// Create new filter_user for new user
		result = dbQuery("INSERT INTO filter_user (identifier, filterType, status, userId) VALUES ('%s', %ld, %ld, %ld)", member, filterType, status, uid);

		dbQueryFreeResult(result);

		if(getErrType() != ERR_NONE) {
			return NULL;
		}

		// Get table id for email that was inserted
		if((eid = dbQueryLastInsertId()) == 0)
			return NULL;

		// Link user to filter_user entry
		result = dbQuery("UPDATE user SET emailId = %ld WHERE id = %ld", eid, uid);

		dbQueryFreeResult(result);

		if(getErrType() != ERR_NONE) {
			return NULL;
		}

	} else {
		eid = uentry->emailId;

		freeUserEntry(uentry);
	}

	// Get newly created filter user for returning
	ufilter = getUserFilterById(eid);

	return ufilter;
}


/*
 * Purpose: Gets the User Filter associated with a user if the user
 * 	is a member of a specific void
 *
 * Entry:
 * 	1st - Void id to get associated user_filter for
 * 	2nd - Username to get user_filter for
 * 	3rd - Filter Type
 * 	4th - Whether looking for only active accounts, etc
 *
 * Exit:
 * 	SUCCESS = User_Filter *  of the member
 * 	FAILURE = NULL, no user identified for this void/username combination
*/
User_Filter *getUserFilterForVoidMember(long voidId, char *member, long filterType, long status) {
	User_Filter *ufilter;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT filter_user.id, filter_user.identifier, filter_user.userId FROM void_membership, user, filter_user WHERE void_membership.voidId = '%ld' AND void_membership.userId = user.id AND filter_user.userId = user.Id AND filter_user.status = '%ld' AND filter_user.filterType = '%ld' AND user.username = '%s'", voidId, status, filterType, member);

	if(getErrType() != ERR_NONE)
		return NULL;

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	if((ufilter = createUserFilter()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	ufilter->id = atol(row[0]);
	ufilter->identifier = mStrndup(row[1], MAX_LENGTH_TEXT_STRING);
	ufilter->filterType = filterType;
	ufilter->status = status;
	ufilter->userId = atol(row[2]);

	dbQueryFreeResult(result);

	if(ufilter->identifier == NULL) {
		freeUserFilter(ufilter);
		return NULL;
	}

	return ufilter;
}
