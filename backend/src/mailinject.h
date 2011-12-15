/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle logging and error reporting
*/

#ifndef __MAILINJECT_H__
#define __MAILINJECT_H__

#include "codewide.h"
#include "logerror.h"

bool setup(int, char **);	// Setup mail inject env
bool tidy();			// Disconnect from db, etc
void failureExit(ERRTYPE);	// If there is a failure, tidy up and exit

#endif
