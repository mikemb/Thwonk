/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Code wide settings 
*/

#include "codewide.h"
#include "logerror.h"
#include "setupthang.h"

/*
 * Purpose: Set globals to default values
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	All globals setup with defaults values
*/
void setupGlobals() {
	setErrType(ERR_NONE);	// Setup so no err type currently set

	_logFile = NULL;	// No log file yet open
	_config = NULL;		// No system wide configuration set
}
