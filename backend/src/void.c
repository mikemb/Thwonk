/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Manage Voids and related actions involving voids
*/

#include<stdlib.h>
#include<string.h>
#include "setupthang.h"
#include "logerror.h"
#include "void.h"
#include "dbchatter.h"
#include "misc.h"


/*
 * Purpose: Create a Void Filter Struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated Void_Filter
 * 	FAILURE = NULL, and err type set
*/
Void_Filter *createVoidFilter() {
	Void_Filter *vfilter;

	if((vfilter = (Void_Filter *)malloc(sizeof(Void_Filter))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	vfilter->id = UNSET;
	vfilter->identifier = NULL;
	vfilter->filterType = UNSET;
	vfilter->status = UNSET;
	vfilter->accessRights = UNSET;
	vfilter->voidId = UNSET;

	return vfilter;
}


/*
 * Purpose: Free memory associated with Void_Filter structure
 *
 * Entry:
 * 	1st - Pointer to a Void_Filter
 *
 * Exit:
 * 	NONE (ventry set to NULL)
*/
void freeVoidFilter(Void_Filter *vfilter) {

	if(vfilter == NULL)
		return;

	if(vfilter->identifier != NULL)
		free(vfilter->identifier);

	free(vfilter);

	vfilter = NULL;
}


/*
 * Purpose: Fill a Void_Filter structure, if successful, with
 * 	row contents from the database for the filter_void table
 *
 * Entry:
 * 	1st - Results of a database query for all columns in
 * 	      filter_void table
 *
 * Exit:
 * 	SUCCESS = Void_Filter with void filter details
 * 	FAILURE = NULL, if void not found or an error
*/
Void_Filter *fillInVoidFilter(DBRESULT *result) {
	Void_Filter *vfilter;
	DBROW row;

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

	if((vfilter = createVoidFilter()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	vfilter->id = atol(row[0]);
	vfilter->identifier = mStrndup(row[1], MAX_LENGTH_TEXT_STRING);
	vfilter->filterType = atol(row[2]);
	vfilter->status = atol(row[3]);
	vfilter->accessRights = atol(row[4]);
	vfilter->voidId = atol(row[5]);

	dbQueryFreeResult(result);

	if(vfilter->identifier == NULL) {
		freeVoidFilter(vfilter);
		return NULL;
	}

	return vfilter;
}


/*
 * Purpose: Get the details about a void filter based on the
 * 	identifier of the filter
 *
 * Entry:
 * 	1st - Name of void to fetch details of
 * 	2nd - Type of filter to find (e.g. email)
 *
 * Exit:
 * 	SUCCESS = Void_Filter with void filter details
 * 	FAILURE = NULL, if void not found or an error
*/
Void_Filter *getVoidFilterByIdentifier(char *name, long filterType) {
	DBRESULT *result;

	result = dbQuery("SELECT id, identifier, filterType, status, accessRights, voidId FROM filter_void WHERE identifier = '%s' AND filterType = '%ld' LIMIT 1", name, filterType);

	return fillInVoidFilter(result);
}


/*
 * Purpose: Get the details about a void filter based on
 * 	the id of the filter
 *
 * Entry:
 * 	1st - Id of void filter to get
 *
 * Exit:
 * 	SUCCESS = Void_Entry with void details
 * 	FAILURE = NULL, if void not found or an error
*/
Void_Filter *getVoidFilterById(long id) {
	DBRESULT *result;

	result = dbQuery("SELECT id, identifier, filterType, status, accessRights, voidId FROM filter_void WHERE id = '%ld' LIMIT 1", id);

	return fillInVoidFilter(result);
}


/*
 * Purpose: Check whether user has permission to send to a void. This
 * 	check does not take into consideration whether the destination
 * 	void is ACTIVE or BLOCKED
 *
 * Entry:
 * 	1st - Void to check user permissions for
 * 	2nd - Identifier of submitter, e.g. email address
 *
 * Exit:
 * 	SUCCESS = true, sender has permission to submit to that void
 * 	FAILURE = false, sender doesn't have enough permissions to submit
*/
bool checkVoidAllowSubmit(Void_Filter *vfilter, Address_Mail *sender) {
	DBRESULT *result;
	int n;

	// At this stage this only checks memberships that exist via email
	if(vfilter->filterType != DBVAL_filter_void_filterType_EMAIL)
		return false;

	/*
	 * Lets figure out membership based on void accessRights
	 *
	 * When a void is:
	 * 	1) PRIVATETHWONK - a sender must be a member of thwonk, belong
	 * 	   to the void they are sending to and not be blocked
	 *
	 * 	2) PUBLICTHWONK - a sender must be a member of thwonk, and
	 * 	   not be blocked
	 *
	 *	3) PUBLICWORLD - a sender doesn't have to be a member of thwonk
	 *	   but they must not be blocked
	*/
	switch(vfilter->accessRights) {

		// Only thwonk members who belong to destination void can send
		case DBVAL_filter_void_accessRights_PRIVATETHWONK:
			result = dbQuery("SELECT void_membership.userId, void_membership.voidId FROM void_membership, filter_user WHERE voidId = %ld AND filter_user.identifier = '%s' AND filter_user.userId = void_membership.userId LIMIT 1", vfilter->voidId, sender->full);
		break;

		// Only members of thwonk can send
		case DBVAL_filter_void_accessRights_PUBLICTHWONK:
			result = dbQuery("SELECT userId FROM filter_user WHERE identifier = '%s' AND status = '%ld' LIMIT 1", sender->full, DBVAL_filter_user_status_ACTIVE);
		break;

		// Anyone can send
		case DBVAL_filter_void_accessRights_PUBLICWORLD:
			result = dbQuery("SELECT userId FROM filter_user WHERE identifier = '%s' AND status = '%ld' LIMIT 1", sender->full, DBVAL_filter_void_status_BLOCKED);
		break;

		// Unknown accessRight asked for
		default:
			return false;
		break;
	}

	if(getErrType() != ERR_NONE)
		return false;

	n = dbQueryCountRows(result);
	dbQueryFreeResult(result);

	// If a row was returned for PUBLICWORLD then sender was blocked
	if(vfilter->accessRights == DBVAL_filter_void_accessRights_PUBLICWORLD) {
		if(n == 0)
			return true;
		else
			return false;
	}

	// Otherwise accessRight was PRIVATETHWONK or PUBLICTHWONK both of which should
	// have returned a single row on successful acceptance
	if(n == 1)
		return true;

	return false;
}


/*
 * Purpose: Get the user id of the person who creator a specific void
 *
 * Entry:
 * 	1st - Id of void to find creator of
 *
 * Exit:
 * 	SUCCESS = User id of void creator
 * 	FAILURE = Returns FAILURE if no matching creator found, or error
*/
long getVoidCreatorUserId(long id) {
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT userId FROM void_membership WHERE voidId = '%ld' AND privilege = '%ld' LIMIT 1", id, DBVAL_void_membership_privilege_CREATOR);

	if(getErrType() != ERR_NONE) {
		return FAILURE;
	}

	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return FAILURE;
	}

	if((row = dbQueryGetRow(result)) == NULL) {
		dbQueryFreeResult(result);
		return FAILURE;
	}

	dbQueryFreeResult(result);

	return atol(row[0]);
}


/*
 * Purpose: Get the name of a void
 *
 * Entry:
 * 	1st - Id of void to get name of
 *
 * Exit:
 * 	SUCCESS = char string with name
 * 	FAILURE = NULL and errtype set
*/
char *getVoidNameById(long id) {
	DBRESULT *result;
	DBROW row;
    char *name;

	result = dbQuery("SELECT name FROM void WHERE id = '%ld' LIMIT 1", id);

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

    name = mStrndup(row[0], MAX_LENGTH_TEXT_STRING);

	dbQueryFreeResult(result);

    if(name == NULL)
        return NULL;

	return name;
}
