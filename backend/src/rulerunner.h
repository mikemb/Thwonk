/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Process messages in the database by running rules on them
*/

#ifndef __RULERUNNER_H__
#define __RULERUNNER_H__

#include "codewide.h"
#include "logerror.h"
#include "msgqueue.h"


// Function prototypes
bool setup(int, char **);	// Setup rule runner env
bool tidy();			// Disconnect from db, etc
void failureExit(ERRTYPE);	// If there is a failure, tidy up and exit

#endif
