/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Process messages in the database by running rules on them
 *
 * Note: This is the top level daemon for spawning off worker threads (Boss/Worker design pattern)
*/

#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include "setupthang.h"
#include "logerror.h"
#include "dbchatter.h"
#include "rulerunner.h"
#include "jsrunner.h"
#include "sandbox.h"
#include "msgqueue.h"


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

	parseCmd(argc, argv);
	parseConfig(NULL);
#ifdef DEBUG
	openLog(_config->logfile);	// No file logging occurs till cmd and config params are setup
#endif
	if(dbConnect() == false)
		return false;

	// Security wise it might make more sense to have this happening first and
	// put the config file along with database socket in the chroot folder
#ifndef DEBUG
	if(putInChroot(SET_JAIL_RULERUNNER, SET_JAIL_RUNASUSER) != true) {
		printf("rulerunner: FAILED TO PUT IN CHROOT AND SETID\n");
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

	// Setup the env rulerunner will run in
	setup(argc, argv);

	// Did setup work correctly?
	if(getErrType() != ERR_NONE) {
		failureExit(getErrType());
	}

	// Process incoming message queue with spawnRuleRunner() doing the work in each child
	runQueueThreads(&spawnRuleRunner, _config->maxnum_rulerunner_threads,
		DBVAL_message_queue_messageType_EMAILIN, DBVAL_message_queue_track_NORMAL,
		MAX_RULERUNNER_SLEEP_SEC, MAX_RULERUNNER_SLEEP_NSEC,
		&failureExit, SANDBOX_RULERUNNER);

	tidy();

	return SUCCESS; 
}
