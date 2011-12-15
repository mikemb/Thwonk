/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Worker thread to run javascript trigger by a message
*/

#ifndef __JSRUNNER_H__
#define __JSRUNNER_H__

#include<jsapi.h>
#include "logerror.h"
#include "msgqueue.h"

ERRTYPE spawnRuleRunner(Queue_Entry *);		// Spin off a thread to run a rule
void jsErrorHandler(JSContext *, const char *, JSErrorReport *);

#endif
