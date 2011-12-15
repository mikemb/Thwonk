/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Manage Voids
*/

#ifndef __VOID_H__
#define __VOID_H__

#include "codewide.h"
#include "parsemail.h"
#include "dbchatter.h"

/* Structure for holding details about a void filter */
typedef struct {
	long id;
	char *identifier;
	long filterType;
	long status;
	long accessRights;
	long voidId;
} Void_Filter;


/* Function prototypes */
Void_Filter *createVoidFilter();
void freeVoidFilter(Void_Filter *);
Void_Filter *fillInVoidFilter(DBRESULT *);
Void_Filter *getVoidFilterByIdentifier(char *, long);
Void_Filter *getVoidFilterById(long);
bool checkVoidAllowSubmit(Void_Filter *, Address_Mail *);

long getVoidCreatorUserId(long);
char *getVoidNameById(long);            // Launch kludge, TODO - should be returning a VOID structure

#endif
