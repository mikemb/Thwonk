/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle interacting virtual files
*/

#ifndef __MNGVFILE_H__
#define __MNGVFILE_H__

#include "codewide.h"
#include "msgqueue.h"

/* Structure for holding details on about vfile */
typedef struct {
	long id;                // Id of vfile item

	int fileType;		// File type (reserved)

	char *editDate;		// Date this file was last edited

	char *name;             // Name of vfile
	char *content;		// File content
} VFile_Entry;


/* Structure to hold details about VFile rights */
typedef struct {
	long id;		// Id of vfile right

	long vfileId;		// Vfile associated with this right

	long userId;		// User (if present) this right applies to
	long voidId;		// Void (if present) this right applies to

	int useRight;		// Allow execute/use
	int editRight;		// Allow edit
	int delRight;		// Allow delete
	int viewRight;		// Allow view/read
	int rightRight;		// Allow change rights
} VFile_Rights;


/* Function prototypes */
VFile_Entry *createVFileEntry();	// Allocate mem and setup a VFile_Entry
void freeVFileEntry(VFile_Entry *);	// Release mem associated with a VFile_Entry
VFile_Entry *getVFileEntryById(long);	// Get a VFile_Entry by id
VFile_Entry *getVFileEntryByName(char *);	// Get a VFile_Entry by name
bool insertVFileEntryByName(char *, char *, Queue_Entry *);	// Insert or update the contents of a virtual file

VFile_Rights *createVFileRights();	// Allocate mem and setup a VFile_Rights
void freeVFileRights(VFile_Rights *);	// Release mem associated with a VFile_Rights
VFile_Rights *getVFileRightsById(long);	// Get a VFile_Rights by id
VFile_Rights *getVFileRightsByFileId(long);	// Get a VFile_Rights by file id
bool insertVFileRights(VFile_Rights *);    // Saved VFile_Rights to a database

#endif
