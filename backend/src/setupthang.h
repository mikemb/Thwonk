/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle setup of programs, e.g. parse command line, read 
 *          and parse configuration files.
*/

#ifndef __SETUPTHANG_H__
#define __SETUPTHANG_H__

/* Structure for application wide setup & configuration info */
typedef struct {
	char *domain;

	char *dbserver;
	char *dbsocket;
	char *dbuser;
	char *dbpword;
	char *dbname;

	char *logfile;		// Location of log file

	size_t maxlength_db_query;	// Max length of a database query
	size_t maxlength_mail_stdin;	// Max length of emails read in via stdin

	size_t maxnum_rulerunner_threads;	// Max num of rule runner threads
	size_t maxnum_outqueue_threads;		// Max num of outgoing message queue threads
} SCONFIG;

/* Globals */
SCONFIG *_config;	// Globals must start with _, yeah yeah, bad globals - otherwise passing
			//  this around an insane amount

/* Function prototypes */
SCONFIG *initSConfig();			// Intialise SCONFIG structure
SCONFIG *parseCmd(int, char **);	// Parse the cmd line
SCONFIG *parseConfig(char *);		// Parse the config file

#endif
