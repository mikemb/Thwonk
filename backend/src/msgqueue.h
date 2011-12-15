/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle adding entries for queued items
*/

#ifndef __MSGQUEUE_H__
#define __MSGQUEUE_H__

#include "codewide.h"
#include "message.h"
#include "logerror.h"
#include "sandbox.h"

#define QUEUE_THREAD_SLOT_EMPTY		-1

/* Structure for holding details on mail address access rights to a void */
typedef struct {
    long id;                // Id of queue item

	long messageId;         // Id in message table
	int messageType;        // Type of message, i.e. DBVAL_message_messageType_EMAIL

	int queueState;         // State of queue item, whether getting processed, etc

	long userId;            // Id of user who sent message (user table)
	long voidId;            // Id of void this message is for (void table)

	int track;              // Can be used for maintaining different priority queues

	// Note: There is a date field in the database table but not going to use for now
} Queue_Entry;


// Store information about each thread that is getting run for processing messages
typedef struct {
	pid_t id;		// Id of thread
	Queue_Entry *qentry;	// Queue_Entry in database associated with this thread
} Queuerunner_Thread;


// Function prototypes
Queue_Entry *createQueueEntry();	// Allocate mem and setup a Queue_Entry
void freeQueueEntry(Queue_Entry *);	// Release mem associated with a Queue_Entry
bool insertQueueEntry(Queue_Entry *);	// Add a queue entry in the database
Queue_Entry *getQueueEntryOldest(int, int, int);	// Get oldest queue entry
Queue_Entry *getQueueEntryJustinNotRunning(int, int);	// Get oldest queue entry to each void
bool setQueueEntryState(Queue_Entry *, int);		// In the database set the queue state of a Queue Entry

bool runQueueThreads(ERRTYPE (*)(Queue_Entry *), int, int, int, long int, long int, void (*)(ERRTYPE), SANDBOXTYPE);
					// Run worker threads for processing message queues

#endif
