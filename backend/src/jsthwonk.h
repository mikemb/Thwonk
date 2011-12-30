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
JSBool jsObjectThwonk_message_getCurrent(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Get current message associated with this javascript run
JSBool jsObjectThwonk_message_sendAll(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Send a message to all members of the current Thwonk
JSBool jsObjectThwonk_message_sendMember(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Send a message to a particular member of a Thwonk

/* Thwonk.file.* */
JSBool jsObjectThwonk_file_open(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Open a file
JSBool jsObjectThwonk_file_close(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Close a file
JSBool jsObjectThwonk_file_read(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Read in file contents
JSBool jsObjectThwonk_file_write(JSContext *, JSObject *, uintN, jsval *, jsval *);	// Write to a file

#endif
