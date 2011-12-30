/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Worker thread to run javascript trigger by a message
*/

#include<stdlib.h>
#include<string.h>
#include "jsrunner.h"
#include "jsthwonk.h"
#include "mnglogic.h"


JSClass js_global_object_class = {
	"System", //"global",			// "global"
	JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_StrictPropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};


/*
 * Purpose: Carry out rule running in this spun off thread
 *
 * Entry:
 * 	1st - Queue entry
 *
 * Exit:
 *	SUCCESS = ERR_NONE
 * 	FAILURE = ERR_* (type of error)
*/
ERRTYPE spawnRuleRunner(Queue_Entry *qentry) {
	JSObject *script = NULL;
	JSRuntime *rt = NULL;
	JSContext *cx = NULL;
	JSBool ret;
	JSObject *global;
	jsval rval;
	Logic_Entry *lentry;

	if((lentry = getLogicEntryForVoid(qentry->voidId)) == NULL) {
		printf("logic 2\r\n");
		return ERR_NONE;
	}

	rt = JS_NewRuntime(SPIDERMONKEY_ALLOC_RAM);

	if(rt == NULL) {
		return ERR_UNKNOWN;
	}

	/*
	 * 8192 = size of each stack chunk (not stack size)
	 *
	 * Apparently this is an internal variable in spidermonkey
	 * that shouldn't be tweaked without knowing a lot about
	 * spidermonkey's garbage collection.
	*/
	cx = JS_NewContext(rt, 8192);

	if(cx == NULL) {
		JS_DestroyRuntime(rt);
		return ERR_UNKNOWN;
	}

	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT | JSOPTION_COMPILE_N_GO);
	JS_SetVersion(cx, JSVERSION_LATEST);

	JS_SetErrorReporter(cx, jsErrorHandler);

	// Create the global object in a new compartment. See http://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide#Native_functions
	global = JS_NewCompartmentAndGlobalObject(cx, &js_global_object_class, NULL);

    	if (global == NULL) {
		JS_DestroyContext(cx);
		JS_DestroyRuntime(rt);
		return ERR_UNKNOWN;
	}

	if(!JS_InitStandardClasses(cx, global)) {
		JS_DestroyContext(cx);
		JS_DestroyRuntime(rt);
		return ERR_UNKNOWN;
	}

	createJSObjectThwonk(cx, global, qentry);

	script = JS_CompileScript(cx, global, lentry->logic, strlen(lentry->logic), "<inline>", 0);

	if(script == NULL) {
		printf("Couldn't compiled the script\n");
		return ERR_UNKNOWN;
	}

	ret = JS_ExecuteScript(cx, global, script, &rval);
 
	if(ret == JS_FALSE) {
		printf("Failed to run compiled script.\n");
		return ERR_UNKNOWN;
	}

//	str = JS_ValueToString(cx, rval);

//	printf("script result: %s\n", JS_GetStringBytes(str));

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return ERR_NONE;
}


/*
 * Purpose: Gets called when an error occurs, print out error message.
 *
 * Entry:
 * 	1st - Context for javascript
 * 	2nd - Error message
 * 	3rd - Struct containing more details about the error
 *
 * Exit:
 * 	NONE
 *
 * TODO: Extend for inserting errors in database	
*/
void jsErrorHandler(JSContext *cx, const char *msg, JSErrorReport *err) {

	printf("JS Error: %s\nFile: %s\nLine num: %u\nBad line: %s\nBad Token: %s\n", msg, err->filename, err->lineno, err->linebuf, err->tokenptr);

}

