/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle messages in Thwonk, whether they have just
 *  arrived on are getting used for further processing
*/

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "codewide.h"
#include "user.h"
#include "void.h"


typedef struct {
	long id;		// Id of message

	long userId;		// Id of user who sent the message

	long filterVoidId;	// Filter void that was used to filter this message
	long filterUserId;	// Filter user that was used to filter this message

	int messageType;	// Type of message
	int messageState;	// State of the message

	char *processDate;	// Date this message will last acted upon

	char *rawContent;	// Textual content of the message
} Message_Entry;


/* Function prototypes */
Message_Entry *createMessageEntry();
void freeMessageEntry(Message_Entry *);
Message_Entry *getMessageEntryById(long);
long insertMessage(int, User_Filter *, Void_Filter *, char *, size_t);
bool deleteMessage(long);
char *buildMessageOut(long);

#endif
