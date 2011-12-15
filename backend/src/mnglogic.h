/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Handle interacting with logic
*/

#ifndef __MNGLOGIC_H__
#define __MNGLOGIC_H__

#include "codewide.h"

/* Structure for holding details on about logic script */
typedef struct {
	long id;                // Id of logic item

	char *name;             // Name of logic
	char *blurb;            // Blurb about the logic

	int version;            // Version number of this script

	char *editDate;         // Date this script was last edited

	int language;           // Language this script was written in (1 = Javascript)

	char *logic;            // Logic in scripting language

	char *logicCache;       // Chache containing compiled version of script (not yet used)

	// Note: There is a date field in the database table but not going to use for now
} Logic_Entry;

// Function prototypes
Logic_Entry *createLogicEntry();	// Allocate mem and setup a Logic_Entry
void freeLogicEntry(Logic_Entry *);	// Release mem associated with a Logic_Entry
Logic_Entry *getLogicEntryForVoid(long);	// Get the logic associated with a void

#endif
