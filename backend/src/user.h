/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Manage users in the database
*/

#ifndef __USER_H__
#define __USER_H__

#include "codewide.h"

/* Structure for holding details about a user entry */
typedef struct {
	long id;		// User id
	char *username;		// Username (not the same as email address)
	int accType;		// Type of account (see dbchatter.h for defines)
	long emailId;		// Email id in filter_user table
} User_Entry;


/* Structure for holding details about a user filter */
typedef struct {
	long id;
	char *identifier;
	long filterType;
	long status;
	long userId;
} User_Filter;


/* Function prototypes */
User_Entry *createUserEntry();
void freeUserEntry(User_Entry *);
User_Entry *getUserEntryByEmail(char *);
User_Entry *getUserEntryByUsername(char *);

User_Filter *createUserFilter();
void freeUserFilter(User_Filter *);
User_Filter *getUserFilterByIdentifier(long, char *);
User_Filter *getUserFilterById(long);
User_Filter *getUserFilterByUserId(long);

User_Filter *getUserFilterForVoidMember(long, char *, long, long);
User_Filter *insertUserFilterMemberUnknown(char *, long, long);

#endif
