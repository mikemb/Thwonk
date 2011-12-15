/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Define javascript objects and methods
*/

#ifndef __JSTHWONKOBJECTS_H__
#define __JSTHWONKOBJECTS_H__

#include<jsapi.h>
#include "jsthwonk.h"


/*
 * Defines used in both Thwonk server and in Javascript
*/
#define TJS_FAILURE	FAILURE
#define TJS_SUCCESS	SUCCESS

#define TJS_MAIL_FROM_MEMBER		1
#define TJS_MAIL_FROM_THWONK_TO_MEMBER	2
#define TJS_MAIL_FROM_THWONK_TO_ANYONE	3
#define TJS_MAIL_FROM_THWONK_TO_FROM    4
#define TJS_MAIL_FROM_THWONK_TO_ANYTHWONK_MEMBER   5
#define TJS_MAIL_FROM_ANYTHWONK_MEMBER_TO_THWONK    6

#define TJS_ERR_NOUSER		-2
#define TJS_ERR_UNSAFE_SUBJECT	-3


/*
 * Javascript: Thwonk object
*/
JSClass jsThwonk_class = {
	"Thwonk",
	0,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSFunctionSpec jsThwonk_methods[] = {
	{"print", jsObjectThwonk_print, 0, 0, 0},
	{"version", jsObjectThwonk_version, 0, 0, 0},
	{NULL},
};


/*
 * Javascript: Thwonk.message object
*/
JSClass jsThwonk_message_class = {
	"message",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSFunctionSpec jsThwonk_message_methods[] = {
	{"getCurrent", jsObjectThwonk_message_getCurrent, 0, 0, 0},
	{"sendAll", jsObjectThwonk_message_sendAll, 3, 0, 0},
	{"sendMember", jsObjectThwonk_message_sendMember, 4, 0, 0},
	{NULL},
};


/*
 * Javascript: Thwonk.file object
*/
JSClass jsThwonk_file_class = {
	"file",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSFunctionSpec jsThwonk_file_methods[] = {
//	{"open", jsObjectThwonk_file_open, 0, 0, 0},
//	{"close", jsObjectThwonk_file_close, 0, 0, 0},
	{"read", jsObjectThwonk_file_read, 1, 0, 0},
	{"write", jsObjectThwonk_file_write, 2, 0, 0},
	{NULL},
};


/*
 * Javascript: Thwonk.member object
*/
JSClass jsThwonk_member_class = {
	"member",
	0,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSFunctionSpec jsThwonk_member_methods[] = {
	{"test", jsObjectThwonk_dummy, 0, 0, 0},
	{NULL},
};

#endif
