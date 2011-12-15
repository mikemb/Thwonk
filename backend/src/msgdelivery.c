/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Process messages in the database by running rules on them
 *
 * Note: This is the top level daemon for spawning off worker threads (Boss/Worker design pattern)
 *  for handling delivery of outgoing messages (for now just email)
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
#include "msgdelivery.h"
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
//	if(putInChroot(SET_JAIL_OUTMAIL, SET_JAIL_RUNASUSER) != true) {
//        printf("msgdelivery: FAILED TO PUT IN CHROOT AND SETID\n");
//		exit(FAILURE);
//	}
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
 * Purpose: Entry point for delivering outgoing mail
 *
 * Entry:
 * 	1st - count of arguments
 * 	2nd - string array of arguments
 *
 * Exit:
 * 	Returns SUCCESS
*/
int main(int argc, char **argv) {

	// Setup the env rulerunner will run in
	setup(argc, argv);

	// Did setup work correctly?
	if(getErrType() != ERR_NONE) {
		failureExit(getErrType());
	}

	// Process outgoing message queue with spawnQueue() doing the work in each child
	runQueueThreads(&spawnProcessOutQueue, _config->maxnum_outqueue_threads,
		DBVAL_message_queue_messageType_EMAILOUT, DBVAL_message_queue_track_NORMAL, 
		MAX_OUTQUEUE_SLEEP_SEC, MAX_OUTQUEUE_SLEEP_NSEC,
		&failureExit, SANDBOX_MSGDELIVERY);

	tidy();

	return SUCCESS;
}


/*
 * Purpose: Function for delivering an outgoing mail
 *
 * Entry:
 * 	1st - Queue_Entry for the outgoing mail
 *
 * Exit:
 * 	SUCCESS = ERR_NONE (internally used by runQueueThreads())
 * 	FAILURE = ERR_* that took place
*/
ERRTYPE spawnProcessOutQueue(Queue_Entry *qentry) {
	int pipeNode[2];
	pid_t pid;
	int status;
	char *outMsg;

	// Setup pipe for communication between parent and child
	if(pipe(pipeNode) == -1) {
		return ERR_PROC_PIPE_CREATE;
	}

	// Parent sends outgoing message to child which is delivering
	// message via sendmail cmdline
	if((pid = fork()) == -1) {
		close(pipeNode[0]);
		close(pipeNode[1]);
		return ERR_PROC_FORK;
	}

	// Are we the child process?
	if(pid == 0) {

		// Connect the pipe to STDIN
		status = dup2(pipeNode[0], STDIN_FILENO);

		close(pipeNode[0]);
		close(pipeNode[1]);

		if(status == -1) {
			exit(ERR_PROC_PIPE_CREATE);
		}

		execl(SET_PATH_EXEC_MAILOUT, SET_PATH_EXEC_MAILOUT, "-t", "-i", "-bm", "-r", SET_MAIL_RETURN_ADDRESS, (char *)0);

		exit(ERR_EXEC_MAILOUT);
	}

	// We're the parent process
//	close(STDIN_FILENO);
//	close(STDOUT_FILENO);
//	close(pipeNode[0]);

	// Get the completed outgoing message for delivery (including all protocol headers)
	if((outMsg = buildMessageOut(qentry->messageId)) == NULL) {
		close(pipeNode[1]);
		return ERR_PROC_PIPE_WRITE;
	}

//	printf("Message\r\n================\r\n%s\r\n", outMsg);

	// Write out to pipe which is connected to child's STDIN
	if(write(pipeNode[1], outMsg, strlen(outMsg)) == -1) {
		free(outMsg);
		close(pipeNode[1]);
		return ERR_PROC_PIPE_WRITE;
	}

	free(outMsg);
	close(pipeNode[1]);

	// Tidy up zombies
	if(waitpid(pid, NULL, 0) < 0) 
		return ERR_PROC_FORK;

	return ERR_NONE;
}
