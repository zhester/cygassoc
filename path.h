/*****************************************************************************

path.h

Cygwin path translation interface declarations.

*****************************************************************************/

#ifndef _PATH_H
#define _PATH_H

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

#include "error.h"

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

typedef unsigned long path_options_t;
                                    //path translation options

enum {
    PATH_OPT_UNIX   = 0x00000000,   //translate to Unix-style path
    PATH_OPT_WIN    = 0x00000001,   //translate to Windows-style path
    PATH_OPT_MIXED  = 0x00000002,   //translate to mixed-style path
    PATH_OPT_DOS    = 0x00000003    //translate to DOS-style path
};

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Interface Prototypes
----------------------------------------------------------------------------*/

error_t cygpath(                    //translates path strings
    LPTSTR              tr_path,    //translated path output
    size_t              tr_size,    //size of output string
    LPCTSTR             path,       //source path to translate
    path_options_t      options     //translation options
);                                  //length of output or error


#endif  /* _PATH_H */

