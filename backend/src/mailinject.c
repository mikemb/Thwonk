/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Inject mail into the Thwonk database.
 *
 * Usage: Can be used in a .forward, etc.
*/

#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include "setupthang.h"
#include "logerror.h"
#include "dbchatter.h"
#include "sandbox.h"
#include "mailinject.h"
#include "parsemail.h"
#include "mngmail.h"
#include "void.h"


/*
 * Purpose: Setup mail inject, parse cmd, setup logs, etc.
 *
 * Entry:
 * 	1st - count of program args
 * 	2nd - array of arg text
 *
 * Exit:
 * 	TRUE = Everything properly setup
 * 	FALSE = There was a failure, have a look at getErrType()
*/
bool setup(int argc, char **argv) {

	setupGlobals();
	parseCmd(argc, argv);
	parseConfig(NULL);
#ifdef DEBUG
	openLog(_config->logfile);	// No file logging occurs till cmd and config params are setup
#endif
	dbConnect();

	// Security wise it might make more sense to have this happening first and
	// put the config file along with database socket in the chroot folder
#ifndef DEBUG
	if(putInChroot(SET_JAIL_INMAIL, SET_JAIL_RUNASUSER) != true) {
		exit(FAILURE);
	}
#endif

	return true;
}


/*
 * Purpose: Tidy up the program, close db conn, close log files, etc
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	TRUE = successfully closed everything
 * 	FALSE = there was a problem, have a look at getErrType()
*/
bool tidy() {

	dbDisconnect();

#ifdef DEBUG
	closeLog();
#endif

	return true;
}


/*
 * Purpose: If there is a failure tidy up and exit
 *
 * Entry:
 * 	1st - Type errror that lead to the failure
 *
 * Exit:
 * 	NONE
*/
void failureExit(ERRTYPE err) {

	setErrType(err);
#ifdef DEBUG
	write2Log(getErrTypeMsg());
#endif
	tidy();
	exit(FAILURE);
}


/*
 * Purpose: Entry point for injecting mailing into the system
 *
 * Entry:
 * 	1st - count of arguments
 * 	2nd - string array of arguments
 *
 * Exit:
 * 	Returns 0
*/
 int main(int argc, char **argv) {
	char *input, *tmp;
	size_t length;
	long mId;
	MSG_MAIL *msg;
	MSG_MAIL_HEADER *msg_hdr;
	Address_Mail *sender;

#ifdef DEBUG
	FILE *testInput;
#endif

	// Setup the env mailinject will run in
	setup(argc, argv);

	if(getErrType() != ERR_NONE) {
		failureExit(getErrType());
	};

	// Read incoming mail from stdin
	if((input = (char *)malloc(_config->maxlength_mail_stdin + 1)) == NULL) {
		failureExit(ERR_MEM_ALLOC);
	}

// Use DEBUG mode to read in a email file via cmdline for testing
#ifdef DEBUG
	if(argc < 2)
		failureExit(ERR_UNKNOWN);
	
	if((testInput = fopen(argv[1], "r")) == NULL) {
		printf("Couldn't open file\r\n");
		failureExit(ERR_UNKNOWN);
	}

	if((length = fread(input, 1, _config->maxlength_mail_stdin, testInput)) == 0) {
#else
	if((length = fread(input, 1, _config->maxlength_mail_stdin, stdin)) == 0) {
#endif
		failureExit(ERR_FILE_STDIN);
	}

	input[length] = '\0';		// Make sure input is nicely terminated

	// Figure out the destination and from fields for this email
	if((msg = createMailParse(input)) == NULL) {
		failureExit(getErrType());
	}

	if((msg_hdr = getMailHeader(msg)) == NULL) {
		freeMailParse(msg);
		failureExit(getErrType());
	}

	// From field should only contain 1 email address
	if((tmp = getMailHeaderField(msg_hdr, MU_HEADER_FROM)) == NULL) {
		freeMailParse(msg);
		failureExit(getErrType());
	}

	if(getMailAddressCount(tmp) != 1) {
		freeMailParse(msg);
		failureExit(getErrType());
	}

	if((sender = getMailAddressPos(tmp, 0)) == NULL) {
		freeMailParse(msg);
		failureExit(getErrType());
	}

    // Check whether mail is from the thwonk.com domain, for now going to
    //  block all mails like that, stop email loops
    if(strcasecmp(sender->domain, SET_DOMAIN) == 0) {
        if(strcasecmp(sender->local, SET_LOOP_ALLOWED_1) != 0 && strcasecmp(sender->local, SET_LOOP_ALLOWED_2) != 0) {
            freeMailParse(msg);
            failureExit(getErrType());
        }
    }

	// Get valid destination and if present queue
	if((tmp = getMailHeaderField(msg_hdr, "X-Original-To")) != NULL) {
		// For now going to enforce destination having 1 address
		if(getMailAddressCount(tmp) != 1) {
			freeMailParse(msg);
			failureExit(getErrType());
		}

		mId = addMailToInQueue(sender, tmp, input, length, AM_MAIL_NOTINDB);
		free(tmp);
//		failureExit(getErrType());
	}

	freeMailParse(msg);
	tidy();

	exit(SUCCESS);
}

