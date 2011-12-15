/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Abstract away low level functions that talk
 *  to the database about whether mails should be accepted
 *  based on who sent them, where they're going, etc.
*/

#include<stdlib.h>
#include<strings.h>
#include "setupthang.h"
#include "mngmail.h"
#include "message.h"
#include "msgqueue.h"
#include "logerror.h"
#include "dbchatter.h"
#include "parsemail.h"
#include "void.h"
#include "user.h"
#include "misc.h"


/*
 * Purpose: Checks whether a mail item should be added to the msg queue and if
 * 	so adds it for each destination void (may be more than one)
 *
 * Entry:
 * 	1st - Sender of this email
 * 	2nd - String of email address where this message is getting sent to,
 * 		each email must be separated by a comma
 * 	3rd - Contents of mail (not yet made safe)
 * 	4th - Length of mail
 * 	5th - If AM_MAIL_NOTINDB then mail not yet inserted in database otherwise
 * 		should be positive
 *
 * Exit:
 * 	SUCCESS, > 0 for ID of mail in database
 * 	FAILURE, TODO reject email
 *
 * TODO: Fix FOR loop, was originally going to walk through a text list of email
 * 	addresses
*/
long addMailToInQueue(Address_Mail *sender, char *voids, char *mail, size_t length, long mId) {
	Address_Mail *dest;
	Void_Filter *voidFilter;
	User_Filter *userFilter;
	Queue_Entry *queueEntry;
	User_Entry *userEntry;
	size_t i, n;
	bool didMsg;

	dest = NULL;

	userEntry = NULL;
	userFilter = NULL;

	queueEntry = createQueueEntry();

	// Lets walk through all the target void email addresses sent
	for(i = 0, didMsg = false, n = getMailAddressCount(voids); i < n; i++) {

		dest = getMailAddressPos(voids, i);
		voidFilter = NULL;

		// Make sure destination is valid, i.e. thwonk.com and not to another domain 
		if(strcasecmp(dest->domain, _config->domain) != 0)
			break;

		// Get void filter associated with destination void
		if((voidFilter = getVoidFilterByIdentifier(dest->full, DBVAL_filter_void_filterType_EMAIL)) != NULL) {

			// Can only send to active voids
			if(voidFilter->status == DBVAL_filter_void_status_ACTIVE &&
				checkVoidAllowSubmit(voidFilter, sender) == true) {

				// Try and get user id to associate with this message, if one is available
				if(userEntry == NULL) {

					// Ok, we don't already have a user id to associate with this message
					if((userEntry = getUserEntryByEmail(sender->full)) == NULL) {

						// But we need some kind of id (really only acceptable where void
						//  is set to PUBLICTHWONK (which is checked by checkVoidAllowSubmit()
						if((userEntry = getUserEntryByUsername(DBVAL_user_username_UNKNOWN_USER)) == NULL) {
							break;
						}

						userFilter = getUserFilterById(userEntry->emailId);
					} else {
						userFilter = getUserFilterByIdentifier(DBVAL_filter_user_filterType_EMAIL, sender->full);
					}
				}

				// If this is the first void getting send a message in this function call then
				//  the message contents need to be stored in the database
				if(mId == AM_MAIL_NOTINDB) {
					mId = insertMessage(DBVAL_message_messageType_EMAILIN, userFilter, voidFilter, mail, length);

					queueEntry->messageId = mId;
					queueEntry->messageType = DBVAL_message_queue_messageType_EMAILIN;
				}

				// Make sure mail was inserted in the database, may have happened on previous
				//  iteration of for loop
				if(queueEntry->messageId == AM_MAIL_NOTINDB)
					break;

				queueEntry->userId = userEntry->id;
				queueEntry->voidId = voidFilter->voidId;

				queueEntry->queueState = DBVAL_message_queue_queueState_JUSTIN;

				queueEntry->track = DBVAL_message_queue_track_NORMAL;

				// Add queue item and if that fails remove last inserted message (if message
				//  isn't referred to by another queue entry)
				if(insertQueueEntry(queueEntry) == false && didMsg == false) {
					deleteMessage(queueEntry->messageId);
					mId = AM_MAIL_NOTINDB;
				} else {
					// Queue insertion was success so on next iteration of this loop
					//  we shouldn't try and delete the message (db table constrains should
					//  prohit a deletion anyway - but lets play safe)
					didMsg = true;
				}

			} else {
				// Attempt to send mail to an inactive void, or sender wasn't permitted to send
				//  to that void
				rejectMail(0, mail);
			}

			freeVoidFilter(voidFilter);
			freeUserFilter(userFilter);

		} else {
			// Attempt to send a mail to a void that doesn't exist
			rejectMail(0, mail);
		}
	}

	freeQueueEntry(queueEntry);
	freeUserEntry(userEntry);

	return mId;
}


/*
 * Purpose: Checks whether a mail item should be added to the msg queue and if
 * 	so adds it for each destination void (may be more than one)
 *
 * Entry:
 * 	1st - Whether this is sending mail to all members of a Thwonk or
 * 		to a single thwonk member
 * 	2nd - Queue Entry for the inbox version of the message
 * 	3rd - Destination username of the user to send this mail to
 * 	4th - Subject of mail to send
 * 	5th - Contents of mail
 *
 * Exit:
 * 	SUCCESS
 * 	FAILURE
*/
long addMailToOutQueue(int outType, Queue_Entry *qentry, char *dest, char *subject, char *body) {

	if(outType == AM_MAIL_OUTALL_SUB) {

		printf("addMailToOutQueue(): AM_MAIL_OUTALL_SUB\r\n");

	} else if(outType == AM_MAIL_OUTMEMBER_SUB_FROM_MEMBER || outType == AM_MAIL_OUTMEMBER_SUB_FROM_THWONK
		|| outType == AM_MAIL_OUTANYONE_SUB_FROM_THWONK || outType == AM_MAIL_OUTANYONE_FROM_THWONK_TO_FROM) {

//		printf("addMailToOutQueue(): AM_MAIL_OUTMEMBER_SUB_FROM_MEMBER\r\n");
//		printf("id: %ld -- messageId: %ld -- userId: %ld -- voidId: %ld\r\n", qentry->id, qentry->messageId, qentry->userId, qentry->voidId);
		return addMailToOutQueueForUser(outType, qentry, dest, subject, body);

	} else if(outType == AM_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER || outType == AM_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK) {
		return addMailToOutQueueForUser(outType, qentry, dest, subject, body);
	} else {
		return FAILURE;
	}

	return SUCCESS;
}


/*
 * Purpose: Checks whether a mail item should be added to the msg queue and if
 * 	so adds it for each destination void (may be more than one)
 *
 * Entry:
 * 	1st - Whether this is sending mail from the Thwonk address or from
 * 		the user who sent the original mail
 * 	2nd - Queue Entry for the inbox version of the message
 * 	3rd - Destination username of the user to send this mail to
 * 	4th - Subject of mail to send (not yet made safe)
 * 	5th - Contents of mail (not yet made safe)
 *
 * Exit:
 * 	SUCCESS
 * 	FAILURE
*/
long addMailToOutQueueForUser(int outType, Queue_Entry *qentry, char *destUser, char *subject, char *body) {
	char *outMsg;
	User_Filter *destFilter;

	outMsg = NULL;

	if(outType == AM_MAIL_OUTMEMBER_SUB_FROM_MEMBER) {

		// Generate mail content for sending
		if((outMsg = constructMailPreOut(subject, body)) == NULL) {
			return FAILURE;
		}

		// Only deliver mail to a user if the user is a member of the void
		if((destFilter = getUserFilterForVoidMember(qentry->voidId, destUser, DBVAL_filter_user_filterType_EMAIL, DBVAL_filter_user_status_ACTIVE)) == NULL) {
			free(outMsg);
			return FAILURE;
		}

		if(insertMessageOutMail(outType, outMsg, destFilter, qentry) == FAILURE) {
			free(outMsg);
			return FAILURE;
		}

		if(outMsg != NULL)
			free(outMsg);

		return SUCCESS;

	} else if(outType == AM_MAIL_OUTMEMBER_SUB_FROM_THWONK) {

		printf("addMailToOutQueueForUser(): Todo support AM_MAIL_OUTMEMBER_SUB_FROM_THWONK\r\n");
		return FAILURE;

	} else if(outType == AM_MAIL_OUTANYONE_SUB_FROM_THWONK || outType == AM_MAIL_OUTANYONE_FROM_THWONK_TO_FROM) {

		// Generate mail content for sending
		if((outMsg = constructMailPreOut(subject, body)) == NULL) {
			return FAILURE;
		}

		// Since destination is unknown either create a new user with no account
		// but email address entry, or retrive previous record of unknown user
		if((destFilter = insertUserFilterMemberUnknown(destUser, DBVAL_filter_user_filterType_EMAIL, DBVAL_filter_user_status_ACTIVE)) == NULL) {
			free(outMsg);
			return FAILURE;
		}

		if(insertMessageOutMail(outType, outMsg, destFilter, qentry) == FAILURE) {
			free(outMsg);
			return FAILURE;
		}

		if(outMsg != NULL)
			free(outMsg);

		return SUCCESS;
	} else if(outType == AM_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER) {
		return FAILURE;
	} else if(outType == AM_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK) {
		// Generate mail content for sending
		if((outMsg = constructMailPreOut(subject, body)) == NULL) {
			return FAILURE;
		}

		// Only deliver mail to a user if their email address is signed up to THWONK 
		if((destFilter = getUserFilterByIdentifier(DBVAL_filter_user_filterType_EMAIL, destUser)) == NULL) {
			free(outMsg);
			return FAILURE;
		}

		if(insertMessageOutMail(outType, outMsg, destFilter, qentry) == FAILURE) {
			free(outMsg);
			return FAILURE;
		}

		if(outMsg != NULL)
			free(outMsg);

		return SUCCESS;
	} else {
		printf("addMailToOutQueueForUser(): Todo for unkown");
		return FAILURE;
	}

	return SUCCESS;
}


/*
 * Purpose: Creates the subject and body of an outgoing mail
 *
 * Entry:
 * 	1st - Mail subject
 * 	2nd - Mail body
 *
 * Exit:
 * 	SUCCESS = pointer to char * with subject + body in
 * 		outgoing email format
 * 	FAILURE = NULL
*/
char *constructMailPreOut(char *subject, char *body) {
	char *out, *rfcSubject;

	if((out = mStrnjoin("Subject: ", subject, MAX_LENGTH_MAIL_OUT)) == NULL) {
		// setErrType() should have been called in mStrnjoin()
		return NULL;
	}

	if((rfcSubject = mStrnjoin(out, "\r\n\r\n", MAX_LENGTH_MAIL_OUT)) == NULL) {
		// setErrType() should have been called in mStrnjoin()
		free(out);
		return NULL;
	}

	free(out);

	if((out = mStrnjoin(rfcSubject, body, MAX_LENGTH_MAIL_OUT)) == NULL) {
		// setErrType() should have been called in mStrnjoin()
		free(rfcSubject);
		return NULL;
	}

	free(rfcSubject);

	return out;
}


/*
 * Purpose: Reject an email
 *
 * Entry:
 * 	1st - Reason for rejection 
 * 	2nd - Email text including headers
 *
 * Exit:
 * 	NONE
*/
void rejectMail(int reason, char *mail) {
	printf("TODO: \r\n\r\n** Rejecting Email **\r\n\r\n%s\r\n", mail);
//	printf("\r\n\r\n** Rejecting Email **\r\n\r\n");
}


/*
 * Purpose: Create an MProtocol Mail struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated MProtocol_Mail
 * 	FAILURE = NULL, and err type set
 */
MProtocol_Mail *createMProtocolMail() {
	MProtocol_Mail *mpMail;

	if((mpMail = (MProtocol_Mail *)malloc(sizeof(MProtocol_Mail))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	mpMail->id = UNSET;
	mpMail->messageId = UNSET;
	mpMail->toFilterUserId = UNSET;
	mpMail->toFilterVoidId = UNSET;
	mpMail->fromFilterUserId = UNSET;
	mpMail->fromFilterVoidId = UNSET;
	mpMail->replytoFilterUserId = UNSET;
	mpMail->replytoFilterVoidId = UNSET;

	return mpMail;
}


/*
 * Purpose: Frees mem allocated to a MProtocol Mail 
 *
 * Entry:
 * 	1st - Pointer to a MProtocol Mail
 *
 * Exit:
 * 	NONE (mpMail set to NULL)
 */
void freeMProtocolMail(MProtocol_Mail *mpMail) {

	if(mpMail == NULL)
		return;

	free(mpMail);

	mpMail = NULL;
}


/*
 * Purpose: Get mprotocol smtp associated with a message id
 *
 * Entry:
 * 	1st - Id of message associated with MProtocol_Mail
 *
 * Exit:
 * 	SUCCESS = Pointer to MProtocol_Mail with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
MProtocol_Mail *getMProtocolMailByMsgId(long messageId) {
	MProtocol_Mail *mpMail;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, toFilterUserId, toFilterVoidId, fromFilterUserId, fromFilterVoidId, replytoFilterUserId, replytoFilterVoidId FROM message_protocol_mail WHERE messageId = %ld", messageId);

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

	mpMail = createMProtocolMail();

	mpMail->id = atol(row[0]);;
	mpMail->messageId = messageId;
	mpMail->toFilterUserId = atol(row[1]);
	mpMail->toFilterVoidId = atol(row[2]);
	mpMail->fromFilterUserId = atol(row[3]);
	mpMail->fromFilterVoidId = atol(row[4]);
	mpMail->replytoFilterUserId = atol(row[5]);
	mpMail->replytoFilterVoidId = atol(row[6]);

	dbQueryFreeResult(result);

	return mpMail;
}


/*
 * Purpose: Insert a MProtocol Mail into the database and get unique ID
 * 	for inserted MProtocol Mail
 *
 * Entry:
 * 	1st - MProtcol Mail with fields filled in
 *
 * Exit:
 * 	SUCCESS, return value contains unique id of MProtocol Mail in database
 * 	FAILURE, return FAILURE (= AM_MAIL_NOTINDB) and err type set
 *
 * Note: Having FAILURE and AM_MAIL_NOTINDB as equivalent is confusing
*/
long insertMProtocolMail(MProtocol_Mail *mpMail) {
	DBRESULT *result;
	long id;

	// Everything was setup properly so now get on with db insertion
	result = dbQuery("INSERT INTO message_protocol_mail (messageId, toFilterUserId, toFilterVoidId, fromFilterUserId, fromFilterVoidId, replytoFilterUserId, replytoFilterVoidId) VALUES (%ld, %ld, %ld, %ld, %ld, %ld, %ld)", mpMail->messageId, mpMail->toFilterUserId, mpMail->toFilterVoidId, mpMail->fromFilterUserId, mpMail->fromFilterVoidId, mpMail->replytoFilterUserId, mpMail->replytoFilterVoidId);

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
 * Purpose: Delete a MProtocol Mail from the database
 *
 * Entry:
 * 	1st - Id of MProtocol Mail to delete
 *
 * Exit:
 * 	NONE
*/
bool deleteMProtocolMail(long id) {
	DBRESULT *result;

	result = dbQuery("DELETE FROM message_protocol_mail WHERE id = %ld", id);

	dbQueryFreeResult(result);

	if(getErrType() != ERR_NONE) {
		return false;
	}

	return true;
}


/*
 * Purpose: Add an outgoing mail to the outgoing queue
 *
 * Entry:
 * 	1st - Type of outgoing messaage
 * 	2nd - Message content (may already have some protocol formatting applied)
 * 	3rd - User Filter with details of user to send the message to
 * 	4th - Queue Entry for parent message
 *
 * Exit:
 * 	SUCCESS = Outgoing queue in the database has mail to deliver
 * 	FAILURE = Err set to error type and FAILURE
*/
long insertMessageOutMail(int outType, char *msg, User_Filter *destUser, Queue_Entry *qentryParent) {
	Void_Filter *voidFilter;
	Queue_Entry *qentryMsg;
	MProtocol_Mail *mpMail;
	Message_Entry *mentryParent;

	long msgId, mprotoId;

    if(outType == AM_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER || outType == AM_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK) {
//	if(outType == AM_MAIL_OUTMEMBER_SUB_FROM_MEMBER || outType == AM_MAIL_OUTANYONE_SUB_FROM_THWONK ||
//       outType == AM_MAIL_OUTANYONE_FROM_THWONK_TO_FROM) {

		// TODO: For now only handling TYPE2 outgoing mails 

		if((mentryParent = getMessageEntryById(qentryParent->messageId)) == NULL) {
			return FAILURE;
		}

		// Setup a VoidFilter with the bare min details required for
		//  insertMessage(). Could access the database to populate userFilter
		//  and voidFilter but doing it this way reduces number of SQL queries
		//  This works because insertMessage() only uses the voidFilter id
		if((voidFilter = createVoidFilter()) == NULL) {
			freeMessageEntry(mentryParent);
			return FAILURE;
		}

		voidFilter->id = mentryParent->filterVoidId;

		// Store outgoing message in the database
        if(outType == AM_MAIL_OUTANYONE_FROM_THWONK_TO_FROM) {
		    msgId = insertMessage(DBVAL_message_messageType_EMAILOUT_TYPE4, destUser, voidFilter, msg, strlen(msg));
        } else { 
		    msgId = insertMessage(DBVAL_message_messageType_EMAILOUT_TYPE2, destUser, voidFilter, msg, strlen(msg));
        }

		freeVoidFilter(voidFilter);

		if(msgId == AM_MAIL_NOTINDB) {
			freeMessageEntry(mentryParent);
			return FAILURE;
		}

		// Store destination and sender details for outgoing mail message
		if((mpMail = createMProtocolMail()) == NULL) {
			freeMessageEntry(mentryParent);
			deleteMessage(msgId);
			return FAILURE;
		}

		mpMail->messageId = msgId;
		mpMail->fromFilterUserId = mentryParent->filterUserId;
		mpMail->toFilterUserId = destUser->id;
		mpMail->replytoFilterVoidId = mentryParent->filterVoidId;

		mprotoId = insertMProtocolMail(mpMail);

		freeMessageEntry(mentryParent);
		freeMProtocolMail(mpMail);

		if(mprotoId == FAILURE) {
			deleteMessage(msgId);
			return FAILURE;
		}

		// Add an entry for the outgoing message to the out going queue
		if((qentryMsg = createQueueEntry()) == NULL) {
			deleteMessage(msgId);
			return FAILURE;
		}

		qentryMsg->messageId = msgId;
		qentryMsg->messageType = DBVAL_message_queue_messageType_EMAILOUT;
		qentryMsg->queueState = DBVAL_message_queue_queueState_JUSTIN;
		qentryMsg->track = DBVAL_message_queue_track_NORMAL;

		qentryMsg->userId = destUser->userId;
		qentryMsg->voidId = qentryParent->voidId;

		if(insertQueueEntry(qentryMsg) == false) {
			freeQueueEntry(qentryMsg);
			deleteMessage(msgId);
			return FAILURE;
		}

		freeQueueEntry(qentryMsg);

		return SUCCESS;

	} else {
#ifdef DEBUG
		write2Log("insertMessageOutMail(): Not implemented yet");
#endif
		printf("insertMessageOutMail(): (TODO) Not implemented yet");
		return FAILURE;
	}

	return FAILURE;
}
