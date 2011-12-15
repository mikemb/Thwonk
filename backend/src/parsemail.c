/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Parse an email
*/

#include<stdlib.h>
#include<string.h>
#include<mailutils/message.h>
#include<mailutils/stream.h>
#include<mailutils/header.h>
#include<mailutils/address.h>
#include<mailutils/nls.h>
#include "parsemail.h"
#include "logerror.h"
#include "dbchatter.h"


/*
 * Purpose: Reads a mail into memory for easy parsing
 *
 * Entry:
 * 	1st - text of email
 *
 * Exit:
 * 	SUCCESS = pointer to struct containing mail 
 * 	FAILURE = NULL (and errno set to err type - see logerror.c/h)
*/
MSG_MAIL *createMailParse(char *mail) {
	MSG_MAIL *msg;
	mu_stream_t strm;
	int fd;

	mu_init_nls();

	if((msg = (MSG_MAIL *)malloc(sizeof(MSG_MAIL))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	if(mu_message_create(msg, NULL) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		return NULL;
	}

	if(mu_message_get_stream(*msg, &strm) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		return NULL;
	}

	if(mu_stream_write(strm, mail, strlen(mail), 0, NULL) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		return NULL;
	}

	return msg;
}


/*
 * Purpose: Tidies up a mail parse - should get rid of tmp
 * 	files, and free up associated memory
 *
 * Entry:
 * 	1st - Existing mail parse
 *
 * Exit:
 * 	SUCCESS = pointer to struct containing header 
*/
void freeMailParse(MSG_MAIL *msg) {
	mu_message_destroy(msg, NULL);
	free(msg);
}


/*
 * Purpose: Gets header from a mail parse
 *
 * Entry:
 * 	1st - Existing mail parse
 *
 * Exit:
 * 	SUCCESS = pointer to struct containing header 
 * 	FAILURE = NULL
*/
MSG_MAIL_HEADER *getMailHeader(MSG_MAIL *msg) {
	MSG_MAIL_HEADER *hdr;

	if((hdr = (MSG_MAIL_HEADER *)malloc(sizeof(MSG_MAIL_HEADER))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	if(mu_message_get_header(*msg, hdr) != 0) {
		setErrType(ERR_MSG_MAIL_HDR_MISSING);
		return NULL;
	}

	return hdr;
}


/*
 * Purpose: Gets the contents of a header field from email
 *
 * Entry:
 * 	1st - Existing header extracted from a mail
 * 	2nd - Field name to find, e.g. From, Bcc, Message-ID
 * 	      (see mailutils/header.h for possible MU_HEADER* parms)
 *
 * Exit:
 * 	SUCCESS = char * containing contents of header field
 * 	FAILURE = NULL (and errno set to err type)
*/
char *getMailHeaderField(MSG_MAIL_HEADER *hdr, char *field) {
	char *val;

	if(mu_header_aget_value(*hdr, field, &val) != 0) {
		setErrType(ERR_MSG_MAIL_HDR_FIELD_MISSING);
		return NULL;
	}

	return val;
}


/*
 * Purpose: Counts the number of email addresses in a string
 *
 * Entry:
 * 	1st - String containing list To:, CC: or Bcc: formated
 * 		list of email addresses
 *
 * Exit:
 * 	0 = No address available or a FAILURE (size_t is unsigned)
 *	1 >= Number of available address
*/
size_t getMailAddressCount(char *adds) { 
	mu_address_t muadd = NULL;
	size_t count = 0;

	if(mu_address_create(&muadd, adds) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		return 0;
	}

	mu_address_get_count(muadd, &count);

	mu_address_destroy(&muadd);

	return count;
}


/*
 * Purpose: Get an email address at a position in a list of
 * 	possible addresses
 *
 * On Entry:
 * 	1st - string containing list of addresses
 * 	2nd - position of address to get
 *
 * On Exit:
 * 	FAILURE = NULL
 * 	SUCCESS = Address_Mail * containing the address
*/
Address_Mail *getMailAddressPos(char *adds, size_t pos) {
	mu_address_t muadd = NULL;
	Address_Mail *address;

	// Internally mailutils has index positions starting at 1, I don't
	//  like that. It doesn't conform to normal C usage so I'm enforcing
	//  0 index start in my function APIs
	pos += 1;

	if(mu_address_create(&muadd, adds) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		return NULL;
	}

	if((address = (Address_Mail *)malloc(sizeof(Address_Mail))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	// Get full email address (without personal info)
	if(mu_address_aget_email(muadd, pos, &address->unsafe_full) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		mu_address_destroy(&muadd);
		return NULL;
	}

	// Get the local (pre-@) part of address
	if(mu_address_aget_local_part(muadd, pos, &address->unsafe_local) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		mu_address_destroy(&muadd);
		return NULL;
	}

	// Get the domain for the email address
	if(mu_address_aget_domain(muadd, pos, &address->unsafe_domain) != 0) {
		setErrType(ERR_MSG_MAIL_PARSER);
		mu_address_destroy(&muadd);
		return NULL;
	}

	// Get the personal part of the address (optional)
	address->unsafe_personal = NULL;
	mu_address_aget_personal(muadd, pos, &address->unsafe_personal);

	// Tidy up memory usage, really should do the same with Address_Mail if
	//  there is an error
	mu_address_destroy(&muadd);

	// Now make the variables safe for database insertion
	address->full = makeMailTxtDBSafe(address->unsafe_full);
	address->local = makeMailTxtDBSafe(address->unsafe_local);
	address->domain = makeMailTxtDBSafe(address->unsafe_domain);
	address->personal = makeMailTxtDBSafe(address->unsafe_personal);

	return address;
}


/*
 * Purpose: Convert a string of text for safe insertion into the database
 *
 * On Entry:
 * 	1st - Text to convert
 *
 * On Exit:
 * 	SUCCESS = Pointer to allocated string containing the text
 * 	FAILURE = NULL
*/
char *makeMailTxtDBSafe(char *txt) {

	if(txt == NULL) {
		return NULL;
	}

	return dbEscapeString(txt, strlen(txt));
}


/*
 * Purpose: Free the memory used by an email address
 *
 * On Entry:
 * 	1st - Address_Mail structure containing an address
 *
 * On Exit:
 * 	Address_Mail memory free (nothing returned)
*/
void freeMailAddress(Address_Mail *address) {

	if(address == NULL)
		return;

	if(address->full != NULL)
		free(address->full);

	if(address->local != NULL)
		free(address->local);

	if(address->domain != NULL)
		free(address->domain);

	if(address->personal != NULL)
		free(address->personal);

	free(address);

	return;
}
