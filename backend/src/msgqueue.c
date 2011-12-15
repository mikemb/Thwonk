/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle adding entries for queued items
*/

#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include "setupthang.h"
#include "msgqueue.h"
#include "logerror.h"
#include "dbchatter.h"
#include "mngmail.h"
#include "sandbox.h"


/*
 * Purpose: Creates a queue entry struct
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = pointer to allocated Queue_Entry
 * 	FAILURE = NULL, and err type set
 */
Queue_Entry *createQueueEntry() {
	Queue_Entry *qentry;

	if((qentry = (Queue_Entry *)malloc(sizeof(Queue_Entry))) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	qentry->id = UNSET;
	qentry->messageId = UNSET;
	qentry->messageType = UNSET;
	qentry->queueState = UNSET;
	qentry->userId = UNSET;
	qentry->voidId = UNSET;
	qentry->track = UNSET;

	return qentry;
}


/*
 * Purpose: Frees mem used by a queue entry
 *
 * Entry:
 * 	1st - Pointer to a Queue_Entry
 *
 * Exit:
 * 	NONE (qentry set to NULL)
 */
void freeQueueEntry(Queue_Entry *qentry) {

	if(qentry == NULL)
		return;

	free(qentry);

	qentry = NULL;
}


/*
 * Purpose: Add an item to the message queue
 *
 * Entry:
 * 	1st - Queue_Entry with struct values filled in
 *
 * Exit:
 * 	SUCCESS = true
 * 	FAILURE = false
*/
bool insertQueueEntry(Queue_Entry *qentry) {
	DBRESULT *result;

	result = dbQuery("INSERT INTO message_queue (messageId, messageType, queueState, userId, voidId, track, processDate) VALUES (%ld, %d, %d, %ld, %ld, %d, now())", qentry->messageId, qentry->messageType, qentry->queueState, qentry->userId, qentry->voidId, qentry->track);

	dbQueryFreeResult(result);

	if(getErrType() != ERR_NONE) {
		return false;
	}

	return true;
}


/*
 * Purpose: Gets the oldest item in the message queue in a specific
 * 	state, of a specific message type and on a specific queue track
 *
 * Entry:
 * 	1st - Queue item state to get
 * 	2nd - Queue track to get item on
 * 	3rd - Type of message queue entry to get, e.g. email
 *
 * Exit:
 * 	SUCCESS = Pointer to Queue_Entry struck with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
*/
Queue_Entry *getQueueEntryOldest(int queueState, int track, int messageType) {
	Queue_Entry *qentry;
	DBRESULT *result;
	DBROW row;

	result = dbQuery("SELECT id, messageId, userId, voidId FROM message_queue WHERE queueState = %d AND track = %d AND messageType = %d ORDER BY processDate ASC LIMIT 1", queueState, track, messageType);

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

	if((qentry = createQueueEntry()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	qentry->id = atol(row[0]);
	qentry->messageId = atol(row[1]);
	qentry->messageType = messageType;

	qentry->queueState = queueState;

	qentry->userId = atol(row[2]);
	qentry->voidId = atol(row[3]);
	qentry->track = track;

	dbQueryFreeResult(result);

	return qentry;
}


/*
 * Purpose: Gets the oldest item in the message queue in a specific
 * 	state and on a specific queue track BUT only get items where
 * 	a rule isn't currently getting run for a void
 *
 * Entry:
 * 	1st - Queue track to get item on
 * 	2nd - Type of message queue entry to get, e.g. email
 *
 * Exit:
 * 	SUCCESS = Pointer to Queue_Entry struck with details
 * 		or NULL if not found
 * 	FAILURE = NULL and err type set
 *
 * Todo: This function could probably be speeded up by
 * 	returning an array of items in the message queue
*/
Queue_Entry *getQueueEntryJustinNotRunning(int track, int messageType) {
	Queue_Entry *qentry;
	DBRESULT *result = NULL;
	DBROW row;

//	result = dbQuery("SELECT id, messageId, userId, voidId FROM message_queue WHERE queueState = %d AND messageType = %d AND track = %d AND voidId NOT IN (SELECT voidId FROM message_queue WHERE queueState = %d AND messageType = %d AND track = %d) ORDER BY processDate ASC LIMIT 1", DBVAL_message_queue_queueState_JUSTIN, messageType, track, DBVAL_message_queue_queueState_PROCESSING, messageType, track);

//	result = dbQuery("select id, messageId, userId, voidId from message_queue where processDate = (select min(processDate) from message_queue where voidId not in (select voidId FROM message_queue WHERE queueState = 2 AND messageType = 3 AND track = 1000) and queueState = 1) and queueState = 1 limit 1");

//	result = dbQuery("select t1.id, t1.messageId, t1.userId, t1.voidId from (select id, messageId, userId, voidId, processDate from message_queue where queueState = 1) as t1 LEFT JOIN (select voidId from message_queue where queueState = 2) as t2 ON t1.voidId = t2.voidId WHERE t2.voidId IS NULL order by processDate limit 1");

	if(messageType == DBVAL_message_queue_messageType_EMAILIN) {
		result = dbQuery("select t1.id, t1.messageId, t1.userId, t1.voidId from (select id, messageId, userId, voidId, processDate from message_queue where queueState = %d AND messageType = %d limit 8) as t1 LEFT JOIN (select voidId from message_queue where queueState = %d AND messageType = %d) as t2 USING (voidId) WHERE t2.voidId IS NULL limit 1", DBVAL_message_queue_queueState_JUSTIN, messageType, DBVAL_message_queue_queueState_PROCESSING, messageType);
	} else if(messageType == DBVAL_message_queue_messageType_EMAILOUT) {
		result = dbQuery("select id, messageId, userId, voidId from message_queue where queueState = %d AND messageType = %d order by id asc limit 1", DBVAL_message_queue_queueState_JUSTIN, messageType);
	}

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

	if((qentry = createQueueEntry()) == NULL) {
		dbQueryFreeResult(result);
		return NULL;
	}

	qentry->id = atol(row[0]);
	qentry->messageId = atol(row[1]);
	qentry->messageType = messageType;

	qentry->queueState = DBVAL_message_queue_queueState_JUSTIN;

	qentry->userId = atol(row[2]);
	qentry->voidId = atol(row[3]);
	qentry->track = track;

	dbQueryFreeResult(result);

	return qentry;
}


/*
 * Purpose: In the database set the queueState of queue entry
 *
 * Entry:
 * 	1st - Queue_Entry to set state of
 * 	2nd - Queue state to set it to
 *
 * Exit:
 * 	SUCCESS = true, and qentry queueState updated with the new state
 * 	FAILURE = false
*/
bool setQueueEntryState(Queue_Entry *qentry, int state) {
	DBRESULT *result;

	result = dbQuery("UPDATE message_queue SET queueState = %d, processDate = now() WHERE id = %ld AND queueState = %d", state, qentry->id, qentry->queueState);

	if(getErrType() != ERR_NONE) {
		dbQueryFreeResult(result);
		return false;
	}

	// Make sure a single row was updated, otherwise update definitely didn't work
	if(dbQueryCountRows(result) != 1) {
		dbQueryFreeResult(result);
		return false;
	}

	dbQueryFreeResult(result);

	qentry->queueState = state;

	return true;
}


/*
 * Purpose: Thread worker spawner (Boss/Worker design pattern)
 *
 * Entry:
 * 	1st - Function pointer to function to do the work in child threads
 * 	2nd - Max number of threads to run at once
 * 	3rd - What message queue to process
 * 	4th - What priority on the message queue to process
 * 	5th - Number of secs to sleep between spawning worker threads
 * 	6th - Number of nanosecs to sleep (+ secs) between spawning worker
 * 		threads
 * 	7th - Function to call when a serious error occurs that should
 * 		stop the thread runner
 * 	8th - What kind of sandbox should child processes be put into
 *
 * Exit:
 * 	SUCCESS = No return from this method UNLESS there is a FAILURE
 * 	FAILURE = Err set to error type
 *
 * Note: This function normally calls the failureExit function pointer upon
 * 	a serious error. When this function is called it should exit the program
 * 	and NOT return back to continue running this function where it left off.
*/
bool runQueueThreads(ERRTYPE (*worker)(Queue_Entry *), int numThreads, int queueType, int track, long int sleepSec, long int sleepNsec, void (*failureExit)(ERRTYPE), SANDBOXTYPE stype) {

	Queuerunner_Thread **threads;
	int i, n = 0, status /*, msgId */;
	struct timespec delay;

	// Everything was setup properly so now get on with main code
	if((threads = (Queuerunner_Thread **)malloc(sizeof(Queuerunner_Thread *) * (numThreads + 1))) == NULL) {
		failureExit(ERR_MEM_ALLOC);
	}

	// Time to sleep between checking whether there's work to do
	delay.tv_sec = sleepSec;
	delay.tv_nsec = sleepNsec; 

	// Initialise with no threads running
	for(i = 0; i < numThreads; i++) {

		if((threads[i] = (Queuerunner_Thread *)malloc(sizeof(Queuerunner_Thread))) == NULL) {
			failureExit(ERR_MEM_ALLOC);
		}

		threads[i]->id = QUEUE_THREAD_SLOT_EMPTY;
		threads[i]->qentry = NULL;
	}

	while(true) {

		// Do we need to get rid of a thread that has finished?
		for(i = 0; i < numThreads; i++) {

			// Check whether a thread is dead
			if(threads[i]->id != QUEUE_THREAD_SLOT_EMPTY && waitpid(threads[i]->id, &status, WNOHANG) != 0) {
				setErrType(ERR_NONE);

				// How did the thread die? Via a kill signal (an unknown kill)?
				if(WIFEXITED(status) != true && WIFSIGNALED(status) == true) {
					setErrType(ERR_PROC_KILLED);
				} else {
					setErrType(WEXITSTATUS(status));
				}

				// Check whether there was a threading error, and if need be
				//  record this to the database
				// TODO: Log error if one occurred in the database
//				printf("%d  --  %s\r\n", _errno, getErrTypeMsg());

				setQueueEntryState(threads[i]->qentry, DBVAL_message_queue_queueState_DONE);
				threads[i]->id = QUEUE_THREAD_SLOT_EMPTY;

				freeQueueEntry(threads[i]->qentry);
				threads[i]->qentry = NULL;
			}
		}

		// Do we have a slot to run a new thread?
		for(i = 0; i < numThreads; i++) {

			// Don't want to replace existing threads
			if(threads[i]->id == QUEUE_THREAD_SLOT_EMPTY
				&& (threads[i]->qentry = getQueueEntryJustinNotRunning(track, queueType)) != NULL) {

//				printf("********** Got item: %ld\r\n", threads[i]->qentry->id);

				if(setQueueEntryState(threads[i]->qentry, DBVAL_message_queue_queueState_PROCESSING) == true) {
					// TODO: Add check in case fork() fails
					if((threads[i]->id = fork()) == 0) {

						_myconn = NULL;

						if(dbConnect() == false) {
							printf("ERROR connecting to database\r\n");
							status = ERR_SANDBOX_SETUP;
						} else if(putInSandbox(stype) == true) {
							status = worker(threads[i]->qentry);
							fflush(stdout);
						} else {
							printf("ERROR setting up sandbox\r\n");
							status = ERR_SANDBOX_SETUP;
						}

						// Exit returning status so manager thread can check whether all ended well
						dbDisconnect();

						exit(status);
					} else {
						n++;
						printf("Created New Thread * Thread Slot: %d  --  Thread Id: %d  --  Total Threads: %d  --  DB id: %ld\n", i, threads[i]->id, n, threads[i]->qentry->id);
					}
				}

//				freeQueueEntry(threads[i]->qentry);
			}
		}

		// Sleep and let child threads run
		nanosleep(&delay, NULL);
	}

	return SUCCESS;
}
