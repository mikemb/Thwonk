/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle interacting with virtual files
*/

#include<stdlib.h>
#include "setupthang.h"
#include "logerror.h"
#include "dbchatter.h"
#include "mngvfile.h"
#include "void.h"
#include "misc.h"


/*
 * Purpose: Create a vfile entry struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated VFile_Entry
 * 	FAILURE = NULL, and err type set
 */
VFile_Entry *createVFileEntry() {
	VFile_Entry *ventry;

	if((ventry = (VFile_Entry *)malloc(sizeof(VFile_Entry))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	ventry->id = UNSET;
	ventry->fileType = UNSET;
	ventry->editDate = NULL;
	ventry->name = NULL;
	ventry->content = NULL;

	return ventry;
}


/*
 * Purpose: Frees mem allocated to a VFile_Entry
 *
 * Entry:
 * 	1st - Pointer to a VFile_Entry
 *
 * Exit:
 * 	NONE (ventry set to NULL)
 */
void freeVFileEntry(VFile_Entry *ventry) {

	if(ventry == NULL)
		return;

	if(ventry->editDate != NULL)
		free(ventry->editDate);

	if(ventry->name != NULL)
		free(ventry->name);

	if(ventry->content != NULL)
		free(ventry->content);

	free(ventry);

	ventry = NULL;
}


/*
 * Purpose: Get vfile entry by id
 *
 * Entry:
 * 	1st - Id of vfile entry
 *
 * Exit:
 * 	SUCCESS = Pointer to VFile_Entry filled with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
VFile_Entry *getVFileEntryById(long id) {
	VFile_Entry *ventry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT fileType, editDate, name, content FROM vfile WHERE id = %ld", id);

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

	if((ventry = createVFileEntry()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	ventry->id = id;
	ventry->fileType = atoi(row[0]);
	ventry->editDate = mStrdup(row[1]);
	ventry->name = mStrdup(row[2]);
	ventry->content = mStrdup(row[3]);

	dbQueryFreeResult(result);

	return ventry;
}


/*
 * Purpose: Get vfile entry by name (path)
 *
 * Entry:
 * 	1st - Path to the file
 *
 * Exit:
 * 	SUCCESS = Pointer to VFile_Entry filled with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
VFile_Entry *getVFileEntryByName(char *name) {
	VFile_Entry *ventry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, fileType, editDate, name, content FROM vfile WHERE name = \"%s\"", name);

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

	if((ventry = createVFileEntry()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	ventry->id = atol(row[0]);
	ventry->fileType = atoi(row[1]);
	ventry->editDate = mStrdup(row[2]);
	ventry->name = mStrdup(row[3]);
	ventry->content = mStrdup(row[4]);

	dbQueryFreeResult(result);

	return ventry;
}


/*
 * Purpose: Create a new or update the contents of an existing
 * 	virtual file, make sure user has rights to write to
 *  this file. If file is created then only user has full rights
 *  to it.
 *
 * Entry:
 * 	1st - Path to the file
 * 	2nd - Contents of the file
 *  3rd - Qentry this function call occurs under
 *
 * Exit:
 * 	SUCCESS = True
 * 	FAILURE = False
 *
 * Notes:
 *  Make sure user has permission to create this file, keep it in their space
 *  If a name begins with /, then it MUST be /THWONK_NAME/path
 *  otherwise if it doesn't have a slash appended /THWONK_NAME/ to it
*/
bool insertVFileEntryByName(char *name, char *content, Queue_Entry *qentry) {
	VFile_Entry *vfile;
    VFile_Rights *vrights;
	DBRESULT *result;
    long vid, uid;
    char *voidname, *tmp, *safe_name;

    // Make sure user has permission to create this file, keep it in their space
    //  If a name begins with /, then it MUST be /THWONK_NAME/path
    //  otherwise if it doesn't have a slash appended /THWONK_NAME/ to it
    if((voidname = getVoidNameById(qentry->voidId)) == NULL)
        return false;

    // Are we creating a file with an absolute or relative path?
    if(name[0] == '/') {
        // Absolute path - make sure they're writing it in the thwonk filesystem
        if((tmp = mStrnjoin("/", voidname, MAX_LENGTH_FILEPATH)) == NULL)
            return false;

        free(voidname);

        if((voidname = mStrnjoin(tmp, "/", MAX_LENGTH_FILEPATH)) == NULL)
            return false;

        // Are they trying to write to a folder they aren't allowed to?
        if(strncmp(voidname, name, strlen(voidname)) != 0) {
            return false;
        }

        if((safe_name = mStrndup(name, MAX_LENGTH_FILEPATH)) == NULL)
            return false;

        free(voidname);

    } else {
        // Relative path, append THWONK name
        if((safe_name = mStrnjoin("/", voidname, MAX_LENGTH_FILEPATH)) == NULL)
            return false;

        if((tmp = mStrnjoin(safe_name, "/", MAX_LENGTH_FILEPATH)) == NULL)
            return false;

        free(safe_name);

        if((safe_name = mStrnjoin(tmp, name, MAX_LENGTH_FILEPATH)) == NULL)
            return false;

        free(tmp);
    }

	if((vfile = getVFileEntryByName(safe_name)) == NULL) {

		// File doesn't exist, create a new one
		result = dbQuery("INSERT INTO vfile (fileType, editDate, name, content) VALUES (%d, now(), '%s', '%s')", DBVAL_vfile_fileType_UNKNOWN, safe_name, content);

        vid = dbQueryLastInsertId();

        free(safe_name);

		dbQueryFreeResult(result);

		if(getErrType() != ERR_NONE) {
			return false;
		}

        // Setup VFile Rights
        if((vrights = createVFileRights()) == NULL) {
            return false;
        }

        vrights->vfileId = vid;
        vrights->userId = getVoidCreatorUserId(qentry->voidId);
        vrights->voidId = qentry->voidId;
        vrights->useRight = DBVAL_logic_rights_ANYRIGHT_ALLOWED;
        vrights->editRight = DBVAL_logic_rights_ANYRIGHT_ALLOWED;
        vrights->delRight = DBVAL_logic_rights_ANYRIGHT_ALLOWED;
        vrights->viewRight = DBVAL_logic_rights_ANYRIGHT_ALLOWED;
        vrights->rightRight = DBVAL_logic_rights_ANYRIGHT_ALLOWED;

        if(insertVFileRights(vrights) == false)
            return false;

        freeVFileRights(vrights);

	} else {

        if((vrights = getVFileRightsByFileId(vfile->id)) == NULL)
            return false;
 
        uid = getVoidCreatorUserId(qentry->voidId);

        // Does this void and userid (creator) have rights to the file?
        // TODO: Check granularity of file rights
        if(qentry->voidId != vrights->voidId || uid != vrights->userId)
            return false;

		freeVFileEntry(vfile);

        // * TODO: Convert to always dealing with absolute paths
		// File DOES exist, so update the contents
		result = dbQuery("UPDATE vfile SET content = '%s', editDate = now() WHERE name = '%s'", content, safe_name);

		if(getErrType() != ERR_NONE) {
			dbQueryFreeResult(result);
			return false;
		}

		if(dbQueryCountRows(result) != 1) {
			dbQueryFreeResult(result);
			return false;
		}

        free(safe_name);
		dbQueryFreeResult(result);
	}

	return true;
}


/*
 * Purpose: Create a vfile rights struct. Default mode is
 *  rights aren't allowed
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated VFile_Rights
 * 	FAILURE = NULL, and err type set
 */
VFile_Rights *createVFileRights() {
	VFile_Rights *vrights;

	if((vrights = (VFile_Rights *)malloc(sizeof(VFile_Rights))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	vrights->id = UNSET;
	vrights->vfileId = UNSET;
	vrights->userId = UNSET;
	vrights->voidId = UNSET;

	vrights->useRight = DBVAL_logic_rights_ANYRIGHT_DENIED;
	vrights->editRight = DBVAL_logic_rights_ANYRIGHT_DENIED;
	vrights->delRight = DBVAL_logic_rights_ANYRIGHT_DENIED;
	vrights->viewRight = DBVAL_logic_rights_ANYRIGHT_DENIED;
	vrights->rightRight = DBVAL_logic_rights_ANYRIGHT_DENIED;

	return vrights;
}


/*
 * Purpose: Frees mem allocated to a VFile_Rights
 *
 * Entry:
 * 	1st - Pointer to a VFile_Rights
 *
 * Exit:
 * 	NONE (vrights set to NULL)
 */
void freeVFileRights(VFile_Rights *vrights) {

	if(vrights == NULL)
		return;

	free(vrights);

	vrights = NULL;
}


/*
 * Purpose: Get vfile rights by id
 *
 * Entry:
 * 	1st - Id of vfile rights
 *
 * Exit:
 * 	SUCCESS = Pointer to VFile_Rights filled with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
VFile_Rights *getVFileRightsById(long id) {
	VFile_Rights *vrights;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT vfileId, userId, voidId, useRight, editRight, delRight, viewRight, rightRight FROM vfile_rights WHERE id = %ld", id);

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

	if((vrights = createVFileRights()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	vrights->id = id;
	vrights->vfileId = atol(row[0]);
	vrights->userId = atol(row[1]);
	vrights->voidId = atol(row[2]);
	vrights->useRight = atoi(row[3]);
	vrights->editRight = atoi(row[4]);
	vrights->delRight = atoi(row[5]);
	vrights->viewRight = atoi(row[6]);
	vrights->rightRight = atoi(row[7]);

	dbQueryFreeResult(result);

	return vrights;
}


/*
 * Purpose: Get vfile rights by file id
 *
 * Entry:
 * 	1st - Id of vfile
 *
 * Exit:
 * 	SUCCESS = Pointer to VFile_Rights filled with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
VFile_Rights *getVFileRightsByFileId(long fileid) {
	VFile_Rights *vrights;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, userId, voidId, useRight, editRight, delRight, viewRight, rightRight FROM vfile_rights WHERE vfileId = %ld", fileid);

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

	if((vrights = createVFileRights()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	vrights->id = atol(row[0]);
	vrights->vfileId = fileid;
	vrights->userId = atol(row[1]);
	vrights->voidId = atol(row[2]);
	vrights->useRight = atoi(row[3]);
	vrights->editRight = atoi(row[4]);
	vrights->delRight = atoi(row[5]);
	vrights->viewRight = atoi(row[6]);
	vrights->rightRight = atoi(row[7]);

	dbQueryFreeResult(result);

	return vrights;
}

/*
 * Purpose: Insert a VFile_Rights into the database
 *
 * Entry:
 * 	1st - Filled in VFile_Rights structure to create
 *
 * Exit:
 * 	SUCCESS = True
 * 	FAILURE = False and err type set
*/
bool insertVFileRights(VFile_Rights *vrights) {
    DBRESULT *result;

    // Everything was setup properly so now get on with db insertion
    result = dbQuery("INSERT INTO vfile_rights (vfileId, userId, voidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld)", vrights->vfileId, vrights->userId, vrights->voidId, vrights->useRight, vrights->editRight, vrights->delRight, vrights->viewRight, vrights->rightRight);

    dbQueryFreeResult(result);

    if(getErrType() != ERR_NONE) {
        return false;
    }

    return true;
}
