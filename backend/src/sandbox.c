/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Place a resource in a sandbox, i.e. limit it to certain
 *  parts of the file system, limit its mem and CPU usage, etc.
*/

#include<stdlib.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#include "sandbox.h"
#include "logerror.h"


/*
 * Purpose: Put a process in a chroot
 *
 * Entry:
 * 	1st - chroot path
 * 	2nd - user to switch to (switches to same group as user)
 *
 * Exit:
 * 	SUCCESS = true
 * 	FAILURE = false
*/
bool putInChroot(char *path, char *uname) {
	struct passwd *user;

    printf("putInChroot\n");

	if((user = getpwnam(uname)) == NULL)
		return false;

	if(chroot(path) != 0)
		return false;

	if(setregid(user->pw_gid, user->pw_gid) != 0)
		return false;

	if(setreuid(user->pw_uid, user->pw_uid) != 0)
		return false;

	return true;
}


/*
 * Purpose: Put a process in the sandbox
 *
 * Entry:
 * 	1st - Type of sandbox to setup
 *
 * Exit:
 * 	SUCCESS = true
 * 	FAILURE = false
*/
bool putInSandbox(SANDBOXTYPE stype) {

	setupSigHandlers();

	return setupLimits(stype);
}


/*
 * Purpose: Setup resource limits, e.g. max amount of ram allowed, CPU time, etc
 *
 * Entry:
 * 	1st - Type of sandbox to setup, i.e. what limits to use
 *
 * Exit:
 * 	SUCCESS = true
 * 	FAILURE = false
*/
bool setupLimits(SANDBOXTYPE stype) {
	struct rlimit limit;

	// Limit CPU usage
	if(stype == SANDBOX_MSGDELIVERY) {
		limit.rlim_cur = RES_MD_MAX_CPU_TIME;
		limit.rlim_max = RES_MD_MAX_CPU_TIME + 1;	// Add 1 because we want SIGXCPU rather than SIGKILL
	} else {
		limit.rlim_cur = RES_RR_MAX_CPU_TIME;
		limit.rlim_max = RES_RR_MAX_CPU_TIME + 1;	// Add 1 because we want SIGXCPU rather than SIGKILL
	}

	if(setrlimit(RLIMIT_CPU, &limit) == -1)
		return false;

	// Limit memory usage
	if(stype == SANDBOX_MSGDELIVERY) {
		limit.rlim_cur = RES_MD_MAX_RAM;
		limit.rlim_max = RES_MD_MAX_RAM;
	} else {
		limit.rlim_cur = RES_RR_MAX_RAM;
		limit.rlim_max = RES_RR_MAX_RAM;
	}

	if(setrlimit(RLIMIT_AS, &limit) == -1)
		return false;

	// Limit file size
	if(stype == SANDBOX_MSGDELIVERY) {
		limit.rlim_cur = RES_MD_MAX_FILE_SIZE;
		limit.rlim_max = RES_MD_MAX_FILE_SIZE;
	} else {
		limit.rlim_cur = RES_RR_MAX_FILE_SIZE;
		limit.rlim_max = RES_RR_MAX_FILE_SIZE;
	}

	if(setrlimit(RLIMIT_FSIZE, &limit) == -1)
		return false;

	// Limit file descriptors
	if(stype == SANDBOX_MSGDELIVERY) {
		limit.rlim_cur = RES_MD_MAX_FILE_DESC;
		limit.rlim_max = RES_MD_MAX_FILE_DESC;
	} else {
		limit.rlim_cur = RES_RR_MAX_FILE_DESC;
		limit.rlim_max = RES_RR_MAX_FILE_DESC;
	}

	if(setrlimit(RLIMIT_NOFILE, &limit) == -1)
		return false;

	// Limit number of processes that can be created
	if(stype == SANDBOX_MSGDELIVERY) {
		limit.rlim_cur = RES_MD_MAX_PROCESS;
		limit.rlim_max = RES_MD_MAX_PROCESS;
	} else {
		limit.rlim_cur = RES_RR_MAX_PROCESS;
		limit.rlim_max = RES_RR_MAX_PROCESS;
	}

	if(setrlimit(RLIMIT_NPROC, &limit) == -1)
		return false;

	// Limit core size
	if(stype == SANDBOX_MSGDELIVERY) {
		limit.rlim_cur = RES_MD_MAX_CORE_SIZE;
		limit.rlim_max = RES_MD_MAX_CORE_SIZE;
	} else {
		limit.rlim_cur = RES_RR_MAX_CORE_SIZE;
		limit.rlim_max = RES_RR_MAX_CORE_SIZE;
	}

	if(setrlimit(RLIMIT_CORE, &limit) == -1)
		return false;

	return true;
}


/*
 * Purpose: Setup handlers for signals hich are sent when resource
 * 	    limits are reached
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void setupSigHandlers() {
	signal(SIGXCPU, handler_SIGXCPU);	// Too much CPU usage
	signal(SIGSEGV, handler_SIGSEGV);	// Too much memory usage
	signal(SIGXFSZ, handler_SIGXFSZ);	// Tries to write to a file

	signal(SIGFPE, handler_SIGFPE);		// Floating point error
	signal(SIGPIPE, handler_SIGPIPE);	// Send to unread pipe
	signal(SIGILL, handler_SIGILL);		// Illegal instruction
	signal(SIGBUS, handler_SIGBUS);		// Bus error
}


/*
 * Purpose: Signal handler called when script attempts to use too
 * 	    much CPU time. Return error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGXCPU(int ignore) {
	exit(ERR_PROC_CPU_OVERUSE);
}


/*
 * Purpose: Signal handler called when script attempts to use too
 * 	    much memory. Return error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGSEGV(int ignore) {
	exit(ERR_PROC_MEM_EXCEED);
}


/*
 * Purpose: Signal handler called when script attempts to write to
 * 	    a file. Return error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGXFSZ(int ignore) {
	exit(ERR_PROC_FILESIZE);
}



/*
 * Purpose: Signal handler called when script has a floating point
 * 	    error. Return error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGFPE(int ignore) {
	exit(ERR_PROC_FLOATINGPOINT);
}



/*
 * Purpose: Signal handler called when there is an attempt to write
 * 	    to a pipe without a reader. Return error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGPIPE(int ignore) {
	exit(ERR_PROC_PIPE_WRITE);
}


/*
 * Purpose: Signal handler called when script carries out an
 * 	    illegal instruction. Return error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGILL(int ignore) {
	exit(ERR_PROC_ILLEGAL);
}



/*
 * Purpose: Signal handler called when script had a bus error such
 * 	    as trying to access an undefined part of memory. Return
 * 	    error to manager thread.
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	NONE
*/
void handler_SIGBUS(int ignore) {
	exit(ERR_PROC_BUS);
}
