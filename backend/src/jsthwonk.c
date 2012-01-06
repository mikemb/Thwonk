/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Create javascript thwonk objects for calling native
 *  methods via javascript code
*/

#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include "jsthwonk.h"
#include "jsthwonkobjects.h"
#include "message.h"
#include "mngmail.h"
#include "mngvfile.h"
#include "dbchatter.h"
#include "misc.h"


/*
 * Purpose: Create hierarchy of thwonk objects
 *
 * Entry:
 * 	1st - Global context
 * 	2nd - Top level object
 * 	3rd - Queue Entry associated with this instance
 *
 * Exit:
 * 	SUCCESS - Pointer to top level Thwonk Object
 * 	FAILURE - NULL
*/
JSObject *createJSObjectThwonk(JSContext *cx, JSObject *obj, Queue_Entry *qentry) {
	JSObject *jsThwonk, *jsObject;

	// Create Javascript Thwonk object 
	if((jsThwonk = JS_DefineObject(cx, obj, jsThwonk_class.name, &jsThwonk_class, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)) == NULL) {
		printf("Couldn't create Thwonk object\n");
		return NULL;
	}

	JS_DefineFunctions(cx, jsThwonk, jsThwonk_methods);

	// Create Javascript Thwonk.message object
	if((jsObject = JS_DefineObject(cx, jsThwonk, "message", &jsThwonk_message_class, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)) == NULL) {
		printf("Couldn't create Thwonk.message object\n");
		return NULL;
	}

	// Setup Queue_Entry shared among all message functions
	JS_SetPrivate(cx, jsObject, qentry);
	JS_DefineFunctions(cx, jsObject, jsThwonk_message_methods);

	// Create Javascript Thwonk.file object
	if((jsObject = JS_DefineObject(cx, jsThwonk, "file", &jsThwonk_file_class, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)) == NULL) {
		printf("Couldn't create Thwonk.file object\n");
		return NULL;
	}

	// Setup Queue_Entry shared among all file management functions
	JS_SetPrivate(cx, jsObject, qentry);
	JS_DefineFunctions(cx, jsObject, jsThwonk_file_methods);

	// Create Javascript Thwonk.member object 
	if((jsObject = JS_DefineObject(cx, jsThwonk, "member", &jsThwonk_member_class, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)) == NULL) {
		printf("Couldn't create Thwonk.member object\n");
		return NULL;
	}

	JS_DefineFunctions(cx, jsObject, jsThwonk_member_methods);

	return jsThwonk;
}


/*
 * Purpose: Native code for Thwonk.print() that prints a text string to STDOUT
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- Variable
 * 	3rd - Array of arguments
 * 		-- Strings containing text to print
 *
 * Exit:
 * 	Number of bytes written out
 *
 * TODO: Convert so it prints out to a debug field
*/
JSBool jsObjectThwonk_print(JSContext *cx, uintN argc, jsval *vp) {
	int i;
	char *str;
	size_t amount = 0;
	jsval *argv;

	if(argc < 1) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(0));
		return JS_TRUE;
	}

	argv = JS_ARGV(cx, vp);

	for(i = 0; i < argc; i++) {
		JSString *val = JS_ValueToString(cx, argv[i]);
		str = JS_EncodeString(cx, val);

		if(str != NULL) {
			amount += fwrite(str, sizeof(*str), JS_GetStringEncodingLength(cx, val), stdout);
			free(str);
		}
	}

	JS_SET_RVAL(cx, vp, INT_TO_JSVAL(amount));

	printf("\n");

	return JS_TRUE;
}


/*
 * Purpose: Native code for Thwonk.version(), it returns the version of Thwonk
 * 	the scripts are running on
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- 0
 * 	3rd - Array of arguments
 * 		-- None
 *
 * Exit:
 * 	SUCCESS - rval = current version of thwonk
 * 	FAILURE - rval = TJS_FAILURE
*/
JSBool jsObjectThwonk_version(JSContext *cx, uintN argc, jsval *vp) {
	JSString *jstr;

	if(argc != 0) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	jstr = JS_NewStringCopyZ(cx, PACKAGE_VERSION);

	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));

	return JS_TRUE;
}


/*
 * Purpose: Native code for Thwonk.message.getCurrent() which gets the message
 * 	associated with the current run of javascript
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- 0
 * 	3rd - Array of arguments
 * 		-- None
 *
 * Exit:
 * 	SUCCESS - rval = Content of current message
 * 	FAILURE - rval = TJS_FAILURE
*/
JSBool jsObjectThwonk_message_getCurrent(JSContext *cx, uintN argc, jsval *vp) {
	Queue_Entry *qentry;
	Message_Entry *mentry;
	JSString *jstr;
	JSObject *obj;

	if(argc != 0) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	obj = JS_THIS_OBJECT(cx, vp);

	if(obj == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	qentry = (Queue_Entry *)JS_GetPrivate(cx, obj);

	if(qentry == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	if((mentry = getMessageEntryById(qentry->messageId)) == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	jstr = JS_NewStringCopyZ(cx, mentry->rawContent);

	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));

	freeMessageEntry(mentry);

	return JS_TRUE;
}


/*
 * Purpose: Native code for Thwonk.message.sendAll() which sends a message
 * 	to all members of a Thwonk
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- 3
 * 	3rd - Array of arguments
 * 		-- 1st = Reserved, should be set to javascript THWONK_UNSET (0)
 * 		-- 2nd = Text of subject
 * 		-- 3rd = Text of message to send
 *
 * Exit:
 * 	SUCCESS - rval = current version of thwonk
 * 	FAILURE - rval = TJS_FAILURE
*/
JSBool jsObjectThwonk_message_sendAll(JSContext *cx, uintN argc, jsval *vp) {
	Queue_Entry *qentry;
	JSObject *obj;

	obj = JS_THIS_OBJECT(cx, vp);

	if(obj == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	qentry = (Queue_Entry *)JS_GetPrivate(cx, obj);

	if(qentry == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	addMailToOutQueue(AM_MAIL_OUTALL_SUB, qentry, "thwonk", "Subject of mail", "This is the mail");

	return JS_TRUE;
}


/*
 * Purpose: Native code for Thwonk.message.sendMember() which sends a message
 * 	to a particular Thwonk member
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- 4
 * 	3rd - Array of arguments
 * 		-- 1st = May be TJS_MAIL_FROM_MEMBER, TJS_MAIL_FROM_THWONK_TO_MEMBER,
 * 			TJS_MAIL_FROM_THWONK_TO_ANYONE
 * 		-- 2nd = Username of member to send messagge to
 * 		-- 3rd = Text of subject to send
 * 		-- 4th = Text of message to send
 *
 * Exit:
 * 	SUCCESS - rval = TJS_SUCCESS
 * 	FAILURE - rval = TJS_FAILURE
*/
JSBool jsObjectThwonk_message_sendMember(JSContext *cx, uintN argc, jsval *vp) {
	Queue_Entry *qentry;
	char *userUnsafe, *subjectUnsafe, *bodyUnsafe;
	char *user, *subject, *body;
	int outType;
	JSObject *obj;
	jsval *argv;

	if(argc != 4) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	argv = JS_ARGV(cx, vp);

	// Should this message be sent from the orginal sender or from the THWONK?
	// And should it be sendable to members or anyone?
	outType = JSVAL_TO_INT(argv[0]);

	// Convert from external defines into internal defines
	switch(outType) {
/*
		case TJS_MAIL_FROM_MEMBER:
			outType = AM_MAIL_OUTMEMBER_SUB_FROM_MEMBER;
		break;

		case TJS_MAIL_FROM_THWONK_TO_MEMBER:
			outType = AM_MAIL_OUTMEMBER_SUB_FROM_THWONK;
		break;

		case TJS_MAIL_FROM_THWONK_TO_ANYONE:
			outType = AM_MAIL_OUTANYONE_SUB_FROM_THWONK;
		break;

		case TJS_MAIL_FROM_THWONK_TO_FROM:
			outType = AM_MAIL_OUTANYONE_FROM_THWONK_TO_FROM;
		break;

		case TJS_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER:
			outType = AM_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER;
		break;
*/
		case TJS_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK:
			outType = AM_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK;
		break;

		default:
			JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
			return JS_TRUE;
	}

	// Lets get the fields for sending the mail
	userUnsafe = JS_EncodeString(cx, JS_ValueToString(cx, argv[1]));
	subjectUnsafe = JS_EncodeString(cx, JS_ValueToString(cx, argv[2]));
	bodyUnsafe = JS_EncodeString(cx, JS_ValueToString(cx, argv[3]));
//	bodyUnsafe = JS_GetStringBytes(JS_GetStringChars(JS_ValueToString(cx, argv[3])));
//	bodyUnsafe = JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));

	if(userUnsafe == NULL || subjectUnsafe == NULL || bodyUnsafe == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	user = dbEscapeString(userUnsafe, strlen(userUnsafe));
	subject = dbEscapeString(subjectUnsafe, strlen(subjectUnsafe));
	body = dbEscapeString(bodyUnsafe, strlen(bodyUnsafe));

	// JS_EncodeString uses mallocs, so have to free that to prevent memory leaks
	free(userUnsafe);
	free(subjectUnsafe);
	free(bodyUnsafe);

	// Now check was escaping of unsafe strings successful
	if(user == NULL || subject == NULL || bodyUnsafe == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	// Make sure the user ain't trying anything naughty by trying
	// to rewrite the outgoing mail headers. To prevent this NO
	// newlines are allowed in the subject
	if(doesStringHaveNewline(subject) == true) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_ERR_UNSAFE_SUBJECT));
		return JS_TRUE;
	}

	// Right, now that everything is setup try and add the mail to the outgoing message queue
	obj = JS_THIS_OBJECT(cx, vp);

	if(obj == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	qentry = (Queue_Entry *)JS_GetPrivate(cx, obj);

	if(qentry == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	// Bumplist: Using bodyUnsafe because of double escape in insertMessage() and when creating
	// body - need nicer way for thwonk (dbEscapeString())
//	if(addMailToOutQueue(i, qentry, user, subject, body) == SUCCESS)

	if(addMailToOutQueue(outType, qentry, user, subject, bodyUnsafe) == SUCCESS) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_SUCCESS));
	} else {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
    	}

	if(user != NULL)
		free(user);

	if(subject != NULL)
		free(subject);
	
	if(body != NULL)
		free(body);

	return JS_TRUE;
}


/*
 * Purpose: Read a file into memory
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- 1
 * 	3rd - Array of arguments
 * 		-- 1st = Virtual path to the file
 *
 * Exit:
 * 	SUCCESS - rval = Content of current message
 * 	FAILURE - rval = TJS_FAILURE
*/
JSBool jsObjectThwonk_file_read(JSContext *cx, uintN argc, jsval *vp) {
	VFile_Entry *vfile;
	char *nameUnsafe;
	char *name;
	jsval *argv;
	JSString *jstr;

	if(argc != 1) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	argv = JS_ARGV(cx, vp);

	nameUnsafe = JS_EncodeString(cx, JS_ValueToString(cx, argv[0]));

	if(nameUnsafe == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	name = dbEscapeString(nameUnsafe, strlen(nameUnsafe));

	free(nameUnsafe);		// Don't leak memory

	if(name == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	if((vfile = getVFileEntryByName(name)) == NULL) {

		if(name != NULL)
			free(name);

		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	if(name != NULL)
		free(name);

	jstr = JS_NewStringCopyZ(cx, vfile->content);

	freeVFileEntry(vfile);

	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));

	return JS_TRUE;
}


/*
 * Purpose: Write a file into database
 *
 * Entry:
 * 	1st - Context this methods was called from
 * 	2nd - Number of arguments passed to this method call
 * 		-- 2
 * 	3rd - Array of arguments
 * 		-- 1st = Virtual path to the file
 * 		-- 2nd = Contents to store in the file
 *
 * Exit:
 * 	SUCCESS - rval = TJS_SUCCESS
 * 	FAILURE - rval = TJS_FAILURE
*/
JSBool jsObjectThwonk_file_write(JSContext *cx, uintN argc, jsval *vp) {
	Queue_Entry *qentry;
	char *nameUnsafe, *contentUnsafe;
	char *name, *content;
	JSObject *obj;
	jsval *argv;

	if(argc != 2) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	argv = JS_ARGV(cx, vp);

	nameUnsafe = JS_EncodeString(cx, JS_ValueToString(cx, argv[0]));
	contentUnsafe = JS_EncodeString(cx, JS_ValueToString(cx, argv[1]));

	if(nameUnsafe == NULL || contentUnsafe == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	name = dbEscapeString(nameUnsafe, strlen(nameUnsafe));
	content = dbEscapeString(contentUnsafe, strlen(contentUnsafe));

	free(nameUnsafe);
	free(contentUnsafe);

	if(name == NULL || content == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

    // Get Queue_Entry for setting up vfile_rights correctly
	obj = JS_THIS_OBJECT(cx, vp);

	if(obj == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	qentry = (Queue_Entry *)JS_GetPrivate(cx, obj);

	if(qentry == NULL) {
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
		return JS_TRUE;
	}

	if(insertVFileEntryByName(name, content, qentry) == false)
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_FAILURE));
	else
		JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TJS_SUCCESS));

	if(name != NULL)
		free(name);

	if(content != NULL)
		free(content);

	return JS_TRUE;
}


/*
 * Purpose: Dummy function
 *
 * Entry:
 * 	Ignored
 *
 * Exit:
 * 	Message printed to screen
*/
JSBool jsObjectThwonk_dummy(JSContext *cx, uintN argc, jsval *vp) {
	JSString *jstr;

	printf("jsObjectThwonk_dummy() called\r\n");

	jstr = JS_NewStringCopyZ(cx, "moo");

	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));

	return JS_TRUE;
}
