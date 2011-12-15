/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle logging and error reporting
*/

#include<stdarg.h>
#include "logerror.h"

/*
 * Link errtypes with error strings
*/
struct ErrorMsg {
	ERRTYPE err;
	char *msg;
} ErrorMsgs[] = {
	{ERR_LOG_OPEN,		"* ERROR: Couldn't open log file"},
	{ERR_LOG_CLOSE,		"* ERROR: Couldn't close log file"},
	{ERR_LOG_WRITE,		"* ERROR: Couldn't write to log file"},
	{ERR_DB_OPEN,		"* ERROR: Couldn't connect to database"},
	{ERR_DB_CLOSE,		"* ERROR: Couldn't disconnect from database"},
	{ERR_DB_QUERY,		"* ERROR: Couldn't perform database query"},
	{ERR_DB_UNKNOWN_DEFINE,	"* ERROR: Tried to use an unknown define when talking to the database"},
	{ERR_PROC_FORK,		"* ERROR: Couldn't fork process"},
	{ERR_PROC_PIPE_CREATE,	"* ERROR: Couldn't create a pipe"},
	{ERR_PROC_PIPE_WRITE,	"* ERROR: Couldn't write to a pipe"},
	{ERR_PROC_CPU_OVERUSE,	"* ERROR: Child process attempted to use more CPU time than allowed"},
	{ERR_PROC_MEM_EXCEED,	"* ERROR: Child process attempted to use more memory than allowed"},
	{ERR_PROC_FILESIZE,	"* ERROR: Child process attempted to have a large file than allowed"},
	{ERR_PROC_FLOATINGPOINT,"* ERROR: Child process had a floating point error"},
	{ERR_PROC_ILLEGAL,	"* ERROR: Child process had an illegal instruction"},
	{ERR_PROC_BUS,		"* ERROR: Child process tried to access memory it wasn't allowed or able to"},
	{ERR_PROC_KILLED,	"* ERROR: Child process was killed"},
	{ERR_MSG_MAIL_PARSER,	"* ERROR: Couldn't create parse structure for mail message"},
	{ERR_MSG_MAIL_HDR_MISSING,	 "* ERROR: Email header is missing or cannot be parsed correctly"},
	{ERR_MSG_MAIL_HDR_FIELD_MISSING, "* ERROR: Requested email header field not found"},
	{ERR_SANDBOX_SETUP,	"* ERROR: Couldn't setup the sandbox"},
	{ERR_EXEC_MAILOUT,	"* ERROR: Coulnt't execute the outgoing mail delivery program (codewide.h:SET_PATH_MAILOUT)"},
	{ERR_SAFE_DB_STRING,	"* ERROR: Failed to convert string to SQL safe version"},
	{ERR_FILE_STDIN,	"* ERROR: Couldn't open STDIN"},
	{ERR_MEM_ALLOC,		"* ERROR: Problem  allocating memory"},
	{ERR_MISC_STRNDUP,	"* ERROR: mStrndup() input string was bigger than max lenght allowed"},
	{ERR_MISC_STRNJOIN,	"* ERROR: mStrnjoin() input strings were bigger than max lenght allowed"},
	{ERR_UNKNOWN,		"* ERROR: An unknown error has occurred"},
	{ERR_NONE,		"- What ya doin 'ere? No error occurred"},
	{_ERR_END,		NULL}		// Keep ERR_NONE in the last position because used by
						// getErrType() as end of array marker
};


/*
 * Purpose: Set the error num to
 *
 * Entry:
 * 	1st - ERRTYPE to set errno to
 *
 * Exit:
 * 	Internal variable _errno set to error type
*/
void setErrType(ERRTYPE err) {
	_errno = err;
}


/*
 * Purpose: Get the ERRORTYPE of _errno, which should
 * 	contain the last err reported
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	ERRTYPE containing the last err to have occured
*/
ERRTYPE getErrType() {
	return _errno;
}


/*
 * Purpose: Get the ERRORTYPE of _errno, which should
 * 	contain the last err reported
 *
 * Entry:
 * 	Internal variable _errno should be set to a known error
 *
 * Exit:
 * 	Pointer to a string containing a description of the error in text
 * 	NULL if no error
*/
 char *getErrTypeMsg() {
	int i;

	for(i = 0; ErrorMsgs[i].err != _ERR_END
			&& ErrorMsgs[i].err != _errno; i++);

	if(ErrorMsgs[i].err == _ERR_END)
		return NULL;
	
	return ErrorMsgs[i].msg;
}


/*
 * Purpose: Open log file
 *
 * Entry:
 * 	1st - Location of the log
 *
 * Exit:
 * 	SUCCESS - true
 * 	FAILURE - false and errno set to failure type
*/
bool openLog(char *logLocation) {

	_logFile = fopen(logLocation, "a");

	if(_logFile == NULL) {
		setErrType(ERR_LOG_OPEN);
		return false;
	}

	return true;
}


/*
 * Purpose: Write out info to log file
 *
 * Entry:
 * 	1st - printf like string to write
 * 	X - any number of args (see man printf)
 *
 * Exit:
 * 	TRUE = successfully wrote to log
 * 	FALSE = failed (should look at getErrType() for more info)
*/
bool write2Log(const char *fmt, ...) {
	va_list ap;

	if(_logFile == NULL) {
		setErrType(ERR_LOG_WRITE);
		return false;
	}

	va_start(ap, fmt);

	if(vfprintf(_logFile, fmt, ap) < 0) {
		setErrType(ERR_LOG_WRITE);
		return false;
	}

	va_end(ap);

	if(fprintf(_logFile, "\r\n") < 0) {
		setErrType(ERR_LOG_WRITE);
		return false;
	}

	fflush(_logFile);

	return true;
}


/*
 * Purpose: Close log file
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS - true
 * 	FAILURE - false and errno set to failure type
*/
bool closeLog() {

	if(_logFile == NULL) {
		setErrType(ERR_LOG_CLOSE);
		return false;
	}

	if(fclose(_logFile) == 0)
		return true;

	setErrType(ERR_LOG_CLOSE);

	return false;
}
