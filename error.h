/*****************************************************************************

error.h

Common error/return codes.

*****************************************************************************/

#ifndef _ERROR_H
#define _ERROR_H

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

typedef long error_t;               //local error type

enum {                              //path translation error codes
    ERROR_NONE    =     0,          //no error encountered
    ERROR_UNKNOWN = -1023,          //non-specific error (lazy developer)
    ERROR_USAGE,                    //interface usage error
    ERROR_API_RESULT,               //API returned unfavorable result
    ERROR_ALLOC                     //memory allocation error
};

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Interface Prototypes
----------------------------------------------------------------------------*/

#endif  /* _ERROR_H */
