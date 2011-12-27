/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle setup of programs, e.g. parse command line, read 
 *          and parse configuration files.
*/

#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "setupthang.h"
#include "logerror.h"
#include "codewide.h"


/*
 * Purpose: Initialise global config structure
 *
 * Entry:
 * 	NONE
 *
 * Exit:
 * 	SUCCESS = returns pointer to initialised SCONFIG (systemwide
 * 		   configuration) and global _config set
 *	FAILURE = NULL pointer and _errno set (see getErrType())
*/
SCONFIG *initSConfig() {

	_config = (SCONFIG *)malloc(sizeof(SCONFIG));

	if(_config == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	return _config;
}


/*
 * Purpose: Parse command line options and read them into
 * 	params structure
 *
 * Entry:
 * 	1st - Number of cmd lines arguments
 * 	2nd - Cmd line arguments
 *
 * Exit:
 * 	SUCCESS = pointer to initialised SCONFIG (systemwide configuration)
 *	FAILURE = NULL pointer and _errno set (see getErrType())
*/
SCONFIG *parseCmd(int argc, char **argv) {

	if(_config == NULL) {
		if(initSConfig() == NULL) {
			return NULL;
		}
	}

	return _config;
}


/*
 * Purpose: Parse config file options and store in config structure
 *
 * Entry:
 * 	1st - Location of config file to read
 *
 * Exit:
 * 	SUCCESS = pointer to populated SCONFIG structure
 * 	FAILURE = NULL & use getErrType() to find out about error
*/
SCONFIG *parseConfig(char *configFile) {

	if(_config == NULL) {
		if(initSConfig() == NULL) {
			return NULL;
		}
	}

	_config->domain = (char *)strdup(SET_DOMAIN);

	if(SET_DBSERVER == NULL)
		_config->dbserver = NULL;
	else
		_config->dbserver = (char *)strdup(SET_DBSERVER);

	if(SET_DBSOCKET == NULL)
		_config->dbsocket = NULL;
	else
		_config->dbsocket = (char *)strdup(SET_DBSOCKET);

	_config->dbuser = (char *)strdup(SET_DBUSER);
	_config->dbpword = (char *)strdup(SET_DBPWORD);
	_config->dbname = (char *)strdup(SET_DBNAME);
	_config->logfile = (char *)strdup(SET_LOGFILE);

	_config->maxlength_db_query = MAX_LENGTH_DB_QUERY;
	_config->maxlength_mail_stdin = MAX_LENGTH_MAIL_STDIN;

	_config->maxnum_rulerunner_threads = MAX_NUM_RULERUNNER_THREADS;
	_config->maxnum_outqueue_threads = MAX_NUM_OUTQUEUE_THREADS;

	return _config;
}
