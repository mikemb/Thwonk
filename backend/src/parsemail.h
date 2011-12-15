/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Parse an email
 *
 * Note: MU_HEADER_* are defined in "mailutils/header.h"
*/

#ifndef __PARSEMAIL_H__
#define __PARSEMAIL_H__

#include<mailutils/message.h>
#include<mailutils/header.h>
#include "codewide.h"

/* Redefine in case we want to create our own msg struct at a later date */
#define MSG_MAIL	mu_message_t
#define MSG_MAIL_HEADER	mu_header_t
#define MSG_MAIL_BODY	mu_body_t

/* Structure for holding info about an email address */
typedef struct {
	char *full;		// Full email address
	char *local;		// Local part of email address (part in front of @)
	char *domain;		// Domain for this address
	char *personal;		// Optional part of address, e.g. "Mike"

	// These versions aren't checked for security/hacking
	char *unsafe_full;		// Full email address
	char *unsafe_local;		// Local part of email address (part in front of @)
	char *unsafe_domain;		// Domain for this address
	char *unsafe_personal;		// Optional part of address, e.g. "Mike"
} Address_Mail;

/* Function prototypes */
MSG_MAIL *createMailParse(char *);			// Read mail into mem for parsing
void freeMailParse(MSG_MAIL *);				// Fre files and mem created by parse
MSG_MAIL_HEADER *getMailHeader(MSG_MAIL *);		// Get header part of email
char *getMailHeaderField(MSG_MAIL_HEADER *, char *);	// Get specific field in the header
size_t getMailAddressCount(char *);			// Count the number of email addresses in a string
Address_Mail *getMailAddressPos(char *, size_t);	// Get mail address at a position in a string addresses
void freeMailAddress(Address_Mail *);			// Free memory used to hold email address structure
char *makeMailTxtDBSafe(char *);			// Convert text in mail for database insertion

#endif
