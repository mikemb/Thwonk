/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Code wide settings 
 *
 * TODO: Most of these options should NOT be hardcoded. Move database
 *  connection options to a config file, and everything else into a
 *  database thwonk configuration table.
*/

#ifndef __CODEWIDE_H__
#define __CODEWIDE_H__

#include "config.h"

//#define DEBUG   1
#define RELEASESTRING	PACKAGE_STRING " - http://www.thwonk.com"

/*
 * Configuration specific settings, later on should be in external
 * config file
*/
#define SET_DOMAIN		"thwonk.com"	// Domain to receive thwonk mail on
#define SET_CONTACT_ERRORS	"admin@"SET_DOMAIN
#define SET_MAIL_RETURN_ADDRESS	"\"admin@thwonk.com\""

// Kludge - for accepting mails from mike and jonah @ thwonk.com, all other
//  mails from a thwonk.com email address are rejected. To prevent email
//  loops
#define SET_LOOP_ALLOWED_1      "mike"
#define SET_LOOP_ALLOWED_2      "jonah"

#define SET_BUMPLIST_TO		"thelist@thwonk.com"

#define SET_DBSERVER	"localhost"
//#define SET_DBSERVER	"127.0.0.1"
//#define SET_DBSOCKET	"/var/run/mysqld/mysqld.sock"
#define SET_DBSOCKET	NULL
#define SET_DBUSER	"thwonkuser"
#define SET_DBPWORD	"thwonkpword"
#define SET_DBNAME	"thwonkdb"
#define SET_LOGFILE	"thwonk.log"

#define SET_JAIL_INMAIL		"/home/thwonk/bin/jail"
#define SET_JAIL_OUTMAIL	"/home/thwonk-daemons/jails/msgdelivery"
#define SET_JAIL_RULERUNNER	"/home/thwonk-daemons/jails/rulerunner"
#define SET_JAIL_RUNASUSER	"nobody"

#define SET_PATH_EXEC_MAILOUT	"/usr/sbin/sendmail"

#define MAX_LENGTH_TEXT_STRING	10000
#define MAX_LENGTH_DB_QUERY	100000
#define MAX_LENGTH_MAIL_STDIN	45000
#define MAX_LENGTH_MAIL_OUT	45000
#define MAX_LENGTH_CODE_JAVASCRIPT	100000
#define MAX_LENGTH_FILEPATH    499

#define MAX_NUM_OUTQUEUE_THREADS	10
#define MAX_OUTQUEUE_SLEEP_SEC		1 //0//1 //0
#define MAX_OUTQUEUE_SLEEP_NSEC		0 // 100000000

#define MAX_NUM_RULERUNNER_THREADS	5
#define MAX_RULERUNNER_SLEEP_SEC	0 //0//1 //0
#define MAX_RULERUNNER_SLEEP_NSEC	100 // 100000000

#define SPIDERMONKEY_ALLOC_RAM		16L * 1024L * 1024L	// How much memory to allocated to each SpiderMonkey runtime
								//  see RES_RR_MAX_RAM

/* The max settings for rulerunner */
#define RES_RR_MAX_RAM			67108864	// Max ram in bytes this process can consume before it is killed
							//  !!! NOTE: This is an upper bound to SPIDERMONKEY_ALLOC_RAM !!!
#define RES_RR_MAX_CPU_TIME		1		// Max time a process is allowed to run in CPU secs
#define RES_RR_MAX_FILE_SIZE		0		// Max file size in bytes
#define RES_RR_MAX_FILE_DESC		6		// Max number of file descriptors
#define RES_RR_MAX_PROCESS		0		// Max number of process
#define RES_RR_MAX_CORE_SIZE		0		// Max size of the core

/* Th max settings for msgdelivery */
#define RES_MD_MAX_RAM			67108864	// Max ram in bytes this process can consume before it is killed
#define RES_MD_MAX_CPU_TIME		1		// Max time a process is allowed to run in CPU secs
#define RES_MD_MAX_FILE_SIZE		10000000	// Max file size in bytes
#define RES_MD_MAX_FILE_DESC		50		// Max number of file descriptors
#define RES_MD_MAX_PROCESS		(4 * MAX_NUM_OUTQUEUE_THREADS)		// Max number of process
#define RES_MD_MAX_CORE_SIZE		0		// Max size of the core

/* Internal values, don't changed below here */
#define UNSET	0

#define true	1
#define false	0

#define SUCCESS	0
#define FAILURE	-1

#define bool int

void setupGlobals();	// Make sure all globals are properly initialised

#endif
