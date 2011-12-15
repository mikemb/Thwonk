/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle interacting with logic
*/

#include<stdlib.h>
#include "setupthang.h"
#include "logerror.h"
#include "dbchatter.h"
#include "mnglogic.h"
#include "misc.h"


/*
 * Purpose: Create a logic entry struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated Logic_Entry
 * 	FAILURE = NULL, and err type set
 */
Logic_Entry *createLogicEntry() {
	Logic_Entry *lentry;

	if((lentry = (Logic_Entry *)malloc(sizeof(Logic_Entry))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	lentry->id = UNSET;
	lentry->name = NULL;
	lentry->blurb = NULL;
	lentry->version = UNSET;
	lentry->editDate = NULL;
	lentry->language = UNSET;
	lentry->logic = NULL;
	lentry->logicCache = NULL;

	return lentry;
}


/*
 * Purpose: Frees mem allocated to a Logic_Entry
 *
 * Entry:
 * 	1st - Pointer to a Logic_Entry
 *
 * Exit:
 * 	NONE (lentry set to NULL)
 */
void freeLogicEntry(Logic_Entry *lentry) {

	if(lentry == NULL)
		return;

	free(lentry);

	lentry = NULL;
}


/*
 * Purpose: Get logic associated with a void
 *
 * Entry:
 * 	1st - Id of void to get logic for
 *
 * Exit:
 * 	SUCCESS = Pointer to Logic_Entry filled with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
Logic_Entry *getLogicEntryForVoid(long voidId) {
	Logic_Entry *lentry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT logic.id, logic.logic FROM logic, logic_rights WHERE logic_rights.voidId = %ld and logic_rights.rightType = %d AND logic_rights.useRight = %d AND logic.language = %d AND logic.id = logic_rights.logicId", voidId, DBVAL_logic_rights_rightType_VOID, DBVAL_logic_rights_ANYRIGHT_ALLOWED, DBVAL_logic_language_JAVASCRIPT);

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

	if((lentry = createLogicEntry()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	lentry->id = atol(row[0]);
	lentry->logic = mStrndup(row[1], MAX_LENGTH_CODE_JAVASCRIPT);

	dbQueryFreeResult(result);

	return lentry;
}

