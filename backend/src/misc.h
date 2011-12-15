/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Misc useful functions, some of which are implemented
 * 	here to maintain portability
*/

#ifndef __MISC_H__
#define __MISC_H__

#include "codewide.h"

// Function prototypes
char *mStrdup(char *);
char *mStrndup(char *, size_t);
char *mStrnjoin(char *, char *, size_t);
bool doesStringHaveNewline(char *);

#endif
