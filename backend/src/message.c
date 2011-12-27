/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle messages in Thwonk, whether they have just
 *  arrived on are getting used for further processing
*/

#include<stdlib.h>
#include<strings.h>
#include "setupthang.h"
#include "message.h"
#include "mngmail.h"
#include "logerror.h"
#include "dbchatter.h"
#include "user.h"
#include "misc.h"


/*
 * Purpose: Create a message entry struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated Message_Entry
 * 	FAILURE = NULL, and err type set
 */
Message_Entry *createMessageEntry() {
	Message_Entry *mentry;

	if((mentry = (Message_Entry *)malloc(sizeof(Message_Entry))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	mentry->id = UNSET;
	mentry->userId = UNSET;
	mentry->filterVoidId = UNSET;
	mentry->filterUserId = UNSET;
	mentry->messageType = UNSET;
	mentry->messageState = UNSET;
	mentry->processDate = NULL;
	mentry->rawContent = NULL;

	return mentry;
}


/*
 * Purpose: Frees mem allocated to a Message Entry
 *
 * Entry:
 * 	1st - Pointer to a Message_Entry
 *
 * Exit:
 * 	NONE (mentry set to NULL)
 */
void freeMessageEntry(Message_Entry *mentry) {

	if(mentry == NULL)
		return;

	if(mentry->processDate != NULL)
		free(mentry->processDate);

	if(mentry->rawContent != NULL)
		free(mentry->rawContent);

	free(mentry);

	mentry = NULL;
}


/*
 * Purpose: Get message entry associated with an id
 *
 * Entry:
 * 	1st - Id message to fill Message_Entry with
 *
 * Exit:
 * 	SUCCESS = Pointer to Message_Entry with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
Message_Entry *getMessageEntryById(long messageId) {
	Message_Entry *mentry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT userId, filterVoidId, filterUserId, messageType, messageState, processDate, rawContent FROM message WHERE id = %ld", messageId);

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

	mentry = createMessageEntry();

	mentry->id = messageId;
	mentry->userId = atol(row[0]);
	mentry->filterVoidId = atol(row[1]);
	mentry->filterUserId = atol(row[2]);
	mentry->messageType = atoi(row[3]);
	mentry->messageState = atoi(row[4]);
	mentry->rawContent = mStrdup(row[6]);

	dbQueryFreeResult(result);

	return mentry;
}


/*
 * Purpose: Insert a message into the database and get unique ID for inserted message
 *
 * Entry:
 * 	1st - Type of message (see dbchatter.h - DBVAL_message_messageType_*)
 * 	2nd - User filter of sender who sent this message (if available)
 * 	3rd - Void filter of destination for this message
 * 	4th - Text of message
 * 	5th - Length of message
 *
 * Exit:
 * 	SUCCESS, return value contains unique id of message in database
 * 	FAILURE, return FAILURE (= AM_MAIL_NOTINDB) and err type set
 *
 * Note: Having FAILURE and AM_MAIL_NOTINDB as equivalent is confusing
*/
long insertMessage(int messageType, User_Filter *userFilter, Void_Filter *voidFilter, char *input, size_t length) {
	DBRESULT *result;
	char *safe_db_input;
	long id;

	// Escape data for inserting into the database
	if((safe_db_input = dbEscapeString(input, length)) == NULL) {
		setErrType(ERR_SAFE_DB_STRING);
		return FAILURE;
	}

	// Everything was setup properly so now get on with db insertion
	result = dbQuery("INSERT INTO message (userId, filterVoidId, filterUserId, messageType, messageState, rawContent, processDate) VALUES (%ld, %ld, %ld, %d, %d, '%s', now())", userFilter->userId, voidFilter->id, userFilter->id, messageType, DBVAL_message_messageState_JUSTIN, safe_db_input);

	free(safe_db_input);
	dbQueryFreeResult(result);

	if(getErrType() != ERR_NONE) {
		return FAILURE;
	}

	// Get table id for email that was inserted
	if((id = dbQueryLastInsertId()) == 0)
		return FAILURE;

	return id;
}


/*
 * Purpose: Delete a message from the database
 *
 * Entry:
 * 	1st - Id of message to delete
 *
 * Exit:
 * 	NONE
*/
bool deleteMessage(long id) {
	DBRESULT *result;

	result = dbQuery("DELETE FROM message WHERE id = %ld", id);

	dbQueryFreeResult(result);

	if(getErrType() != ERR_NONE) {
		return false;
	}

	return true;
}


/*
 * Purpose: Create a string containing the complete outgoing message
 *
 * Entry:
 * 	1st - Id out outgoing message
 *
 * Exit:
 * 	SUCCES = pointer to string with the message
 * 	FAILURE = NULL and err type set
*/
char *buildMessageOut(long id) {
	char *outMsg;
	Message_Entry *mentry;
	MProtocol_Mail *mpMail;
	User_Filter *fromUserFilter, *toUserFilter;
	Void_Filter *replytoVoidFilter;

	outMsg = (char *)malloc(sizeof(char) * 10000);

	if((mentry = getMessageEntryById(id)) == NULL) {
		return NULL;
	}

	switch (mentry->messageType) {

		case DBVAL_message_messageType_EMAILOUT_TYPE2:

			if((mpMail = getMProtocolMailByMsgId(mentry->id)) == NULL) {
				setErrType(ERR_UNKNOWN);
				freeMessageEntry(mentry);
				return NULL;
			}

			fromUserFilter = getUserFilterById(mpMail->fromFilterUserId);
			toUserFilter = getUserFilterById(mpMail->toFilterUserId);
			replytoVoidFilter = getVoidFilterById(mpMail->replytoFilterVoidId);

// MIKE			snprintf(outMsg, 10001, "From: %s\r\nTo: %s\r\nBcc: %s\r\nReply-To: %s\r\nErrors-To: %s\r\nX-Mailer: %s\r\n%s\r\n", fromUserFilter->identifier, replytoVoidFilter->identifier, toUserFilter->identifier, replytoVoidFilter->identifier, SET_CONTACT_ERRORS, RELEASESTRING, mentry->rawContent);
			snprintf(outMsg, 10001, "From: %s\r\nTo: %s\r\nBcc: %s\r\nReply-To: %s\r\nErrors-To: %s\r\nX-Mailer: %s\r\n%s\r\n", replytoVoidFilter->identifier, replytoVoidFilter->identifier, toUserFilter->identifier, replytoVoidFilter->identifier, SET_CONTACT_ERRORS, RELEASESTRING, mentry->rawContent);

			freeMProtocolMail(mpMail);
		break;

		case DBVAL_message_messageType_EMAILOUT_TYPE4:

			if((mpMail = getMProtocolMailByMsgId(mentry->id)) == NULL) {
				setErrType(ERR_UNKNOWN);
				freeMessageEntry(mentry);
				return NULL;
			}

			fromUserFilter = getUserFilterById(mpMail->fromFilterUserId);
			toUserFilter = getUserFilterById(mpMail->toFilterUserId);
			replytoVoidFilter = getVoidFilterById(mpMail->replytoFilterVoidId);

			snprintf(outMsg, 10001, "From: %s\r\nTo: %s\r\nReply-To: %s\r\nErrors-To: %s\r\nX-Mailer: %s\r\n%s\r\n", replytoVoidFilter->identifier, toUserFilter->identifier, replytoVoidFilter->identifier, SET_CONTACT_ERRORS, RELEASESTRING, mentry->rawContent);

			freeMProtocolMail(mpMail);
		break;

		default:
			setErrType(ERR_DB_UNKNOWN_DEFINE);
			outMsg = NULL;
	}

	freeMessageEntry(mentry);

	return outMsg;
}
