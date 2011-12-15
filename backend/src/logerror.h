/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle logging and error reporting
*/

#ifndef __LOGERROR_H__
#define __LOGERROR_H__

#include <stdio.h>
#include "codewide.h"

/*
 * List of possible errors that may be caught and reported
 *
 * ********************************************************
 * NOTE: ANYTIME A NEW ERROR IS ADDED MAKE SURE AND UPDATE
 *       ErrorMsgs[] IN logerror.c 
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
typedef enum {
	ERR_LOG_OPEN = 100,	// Couldn't open log file
	ERR_LOG_CLOSE,		// Couldn't close log file
	ERR_LOG_WRITE,		// Couldn't write to log file
	ERR_DB_OPEN,		// Couldn't connect database
	ERR_DB_CLOSE,		// Couldn't disconnect from database
	ERR_DB_QUERY,		// Couldn't perform database query
	ERR_DB_UNKNOWN_DEFINE,	// Tried to use an unknown define when talking to the database (see dbchatter.h for a
				//  list of the available define)
	ERR_PROC_FORK,		// Couldn't fork a process
	ERR_PROC_PIPE_CREATE,	// Couldn't create a pipe
	ERR_PROC_PIPE_WRITE,	// Couldn't write to a pipe
	ERR_PROC_CPU_OVERUSE,	// Child process received a too much CPU usage signal (usually if a process
				//  has run too long)
	ERR_PROC_MEM_EXCEED,	// Child process attempted to use more memory than allowed
	ERR_PROC_FILESIZE,	// Child process attempted to write to a file large than allowed
	ERR_PROC_FLOATINGPOINT,	// Child process had a floating point error
	ERR_PROC_ILLEGAL,	// Child process attempts to run an illegal instruction
	ERR_PROC_BUS,		// Child process tried to access a part of memory it wasn't allowed or able to
	ERR_PROC_KILLED,	// Child process was killed
	ERR_MSG_MAIL_PARSER,	// Couldn't create parser for processing a mail message structure
	ERR_MSG_MAIL_HDR_MISSING,	// Couldn't find header in email
	ERR_MSG_MAIL_HDR_FIELD_MISSING, // Couldn't find the requested field in the header
	ERR_SANDBOX_SETUP,	// Couldn't setup the sandbox for some reason
	ERR_EXEC_MAILOUT,	// Couldn't execute the sendmail command line program for delivering outgoing mail
	ERR_SAFE_DB_STRING,	// Failed to convert string to safe SQL version
	ERR_FILE_STDIN,		// Couldn't open STDIN
	ERR_MEM_ALLOC,		// Couldn't allocate memory
	ERR_MISC_STRNDUP,	// Input string was longer than the max string allowed
	ERR_MISC_STRNJOIN,	// Input strings were longer than the max output string allowed
	ERR_UNKNOWN,		// Unknown error
	ERR_NONE,		// No Error
	_ERR_END		// DONT USE - MARKS END OF ARRAY
} ERRTYPE;

/* Globals */
ERRTYPE _errno;			// global with containing error num, should only be access via methods
				//  setErrno(), getErrno()

FILE *_logFile;			// global containing file handle for log file

/* Function prototypes */
void setErrType(ERRTYPE);
ERRTYPE getErrType();
char *getErrTypeMsg();

bool openLog(char *);
bool write2Log(const char *, ...);
bool closeLog();

#endif
