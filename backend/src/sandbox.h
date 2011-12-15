/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
*/

#ifndef __SANDBOX_H__
#define __SANDBOX_H__

#include "codewide.h"

typedef enum {
	SANDBOX_RULERUNNER,
	SANDBOX_MSGDELIVERY
} SANDBOXTYPE;

bool putInChroot(char *, char *);	// Put a process in a chroot
bool putInSandbox(SANDBOXTYPE);		// Put a process in the sandbox
bool setupLimits(SANDBOXTYPE);		// Setup resources limits of processes running in this sandbox
void setupSigHandlers();		// Setup catchers for signals, i.e. log error and kill process

void handler_SIGXCPU(int);
void handler_SIGSEGV(int);
void handler_SIGXFSZ(int);
void handler_SIGFPE(int);
void handler_SIGPIPE(int);
void handler_SIGILL(int);
void handler_SIGBUS(int);

#endif
