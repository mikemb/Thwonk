/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Abstract away low level functions for
 * 	checking whether to accept mail from certain
 * 	email address, etc. Store outgoing mail, etc.
*/

#ifndef __MNGMAIL_H__
#define __MNGMAIL_H__

#include "codewide.h"
#include "parsemail.h"
#include "msgqueue.h"
#include "user.h"


// Store info specific to email headers
typedef struct {
	long id;		// Id of message

	long messageId;		// Id of user who sent the message

	long toFilterUserId;
	long toFilterVoidId;

	long fromFilterUserId;
	long fromFilterVoidId;

	long replytoFilterUserId;
	long replytoFilterVoidId;

} MProtocol_Mail;


/* Function prototypes */
long addMailToInQueue(Address_Mail *, char *, char *, size_t, long);
long addMailToOutQueue(int, Queue_Entry *, char *, char *, char *);
long addMailToOutQueueForUser(int, Queue_Entry *, char *, char *, char *);
char *constructMailPreOut(char *, char *);
void rejectMail(int, char *);			// Reject an email

long insertMessageOutMail(int, char *, User_Filter *, Queue_Entry *);

MProtocol_Mail *createMProtocolMail();
void freeMProtocolMail(MProtocol_Mail *);
MProtocol_Mail *getMProtocolMailByMsgId(long);
long insertMProtocolMail(MProtocol_Mail *);
bool deleteMProtocolMail(long);


/* Used by rejectMail() to define reason for rejecting the mail */
#define RM_UNKNOWN		1		// Mail rejected for an unknown reason
#define RM_INVALID_HEADER	2		// Email header was invalid

#define AM_MAIL_NOTINDB		FAILURE

/* Used by addMailToOutQueue() to decide what kind of mail are we been asked to deliver */
#define AM_MAIL_OUTALL_SUB	1		// Send mail to all members of a void, where subject is provided
						//  as part of the mail
#define AM_MAIL_OUTMEMBER_SUB_FROM_MEMBER	2	// Send mail to a single member of a void, where subject is
							//  provided for the mail and mail comes from the original
							//  sender
#define AM_MAIL_OUTMEMBER_SUB_FROM_THWONK	3	// Send mail to a single member of a void, where subject
							//  is provided and mail comes from the thwonk address
#define AM_MAIL_OUTANYONE_SUB_FROM_THWONK	4	// Send mail to a single email address (membership of
							//  thwonk not checked or required), where subject is
							//  provided and mail comes from the thwonk address
#define AM_MAIL_OUTANYONE_FROM_THWONK_TO_FROM	5   // Send mail to user from thwonk from field

#define AM_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER 6
#define AM_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK 7

#endif
