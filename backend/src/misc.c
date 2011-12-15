/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Misc useful functions, some of which are implemented
 * 	here to maintain portability
 *
 * TODO: Change mStrndup() to use memcpy() rather than strncpy()
 * 	because that should be safer and more general in case it
 * 	gets binary data
 *
 * BUG: (Probably) \0 in the functions below is probably overwriting
 * 	the last character where string lengths exactly match the maxes
*/

#include<stdlib.h>
#include<string.h>
#include "setupthang.h"
#include "logerror.h"
#include "misc.h"


/*
 * Purpose: Cross platform version of strdup
 *
 * Entry:
 * 	1st - string to copy
 *
 * Exit:
 * 	SUCCESS = pointer to allocated copy of string
 * 	FAILURE = NULL, and err type set
 */
char *mStrdup(char *src) {
	size_t length;

	length = strlen(src);

	return mStrndup(src, length);
}


/*
 * Purpose: Cross platform version of strndup
 *
 * Entry:
 * 	1st - string to copy
 * 	2nd - max size of string
 *
 * Exit:
 * 	SUCCESS = pointer to allocated copy of string
 * 	FAILURE = NULL, and err type set
 */
char *mStrndup(char *src, size_t n) {
	char *dest;
	size_t size;

	size = strlen(src);

	if(size > n) {
		setErrType(ERR_MISC_STRNDUP);
		return NULL;
	}

	if((dest = (char *)malloc((sizeof(char) * size) + 1)) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	strncpy(dest, src, size);
	dest[size] = '\0';

	return dest;
}


/*
 * Purpose: Another version of strncat but params different
 * 	from normal in important ways
 *
 * Entry:
 * 	1st - First part to join
 * 	2nd - Second part to join
 * 	3rd - max allowable size of end string
 *
 * Exit:
 * 	SUCCESS = pointer to newly allocated joined string
 * 	FAILURE = NULL, and err type set
 *
 * NOTE: I know I could use va_list BUT later on this
 * 	code will be updated to use memcpy() for increased
 * 	security and an ability to handle a wider range of
 * 	character sets. (And yes, multiple calls to mStrnjoin()
 * 	are required to join a string of more than 2 parts.
 * 	Its a tradeoff for security over efficiency.)
 */
char *mStrnjoin(char *p1, char *p2, size_t n) {
	char *string;
	size_t sizeP1, sizeP2;

	sizeP1 = strlen(p1);
	sizeP2 = strlen(p2);

	if((sizeP1 + sizeP2) > n) {
		setErrType(ERR_MISC_STRNJOIN);
		return NULL;
	}

	if((string = (char *)malloc((sizeof(char) * (sizeP1 + sizeP2)) + 1)) == NULL) {
		setErrType(ERR_MEM_ALLOC);
		return NULL;
	}

	strncpy(string, p1, sizeP1);
	string[sizeP1] = '\0';

	strncat(string, p2, sizeP2);
	string[sizeP1 + sizeP2] = '\0';		// strncat should be doing this BUT lets
						//  be overly parnoid

	return string;
}



/*
 * Purpose: Checks whether a string has a newline character
 *
 * Entry:
 * 	1st - string to check
 *
 * Exit:
 * 	TRUE = Has a newline char
 * 	FALSE = Doesn't have a newline char
 */
bool doesStringHaveNewline(char *src) {

	if(strchr(src, '\n') == NULL)
		return false;

	return true;
}
