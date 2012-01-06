/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Create javascript thwonk objects for calling native
 *  methods via javascript code
*/

#ifndef __JSTHWONK_H__
#define __JSTHWONK_H__

#include<jsapi.h>
#include "msgqueue.h"

/* Function prototypes */
JSObject *createJSObjectThwonk(JSContext *, JSObject *, Queue_Entry *);

/* Thwonk.* */
JSBool jsObjectThwonk_dummy(JSContext *, uintN, jsval *);
JSBool jsObjectThwonk_print(JSContext *, uintN, jsval *);	// Print a string to STDOUT
JSBool jsObjectThwonk_version(JSContext *, uintN, jsval *);	// Return what is the current version of thwonk

/* Thwonk.message.* */
JSBool jsObjectThwonk_message_getCurrent(JSContext *, uintN, jsval *);	// Get current message associated with this javascript run
JSBool jsObjectThwonk_message_sendAll(JSContext *, uintN, jsval *);	// Send a message to all members of the current Thwonk
JSBool jsObjectThwonk_message_sendMember(JSContext *, uintN, jsval *);	// Send a message to a particular member of a Thwonk

/* Thwonk.file.* */
JSBool jsObjectThwonk_file_read(JSContext *, uintN, jsval *);	// Read in file contents
JSBool jsObjectThwonk_file_write(JSContext *, uintN, jsval *);	// Write to a file

#endif
