/*****************************************************************************

config.h

Common configuration items used to customize the association program.

This file can be customized to suit a particular use case.  Here, you can
adjust the following options:

- Cygwin's installation directory
- The console to use and its options
- The shell to use and its options
- The Cygwin program that is started within the shell and its options

*****************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>

/*----------------------------------------------------------
First, we need the root of the Cygin installation.
----------------------------------------------------------*/
#define CONFIG_CYGWIN_ROOT "C:\\cygwin"
                                    //root of the local cygwin install

/*----------------------------------------------------------
Second, we set up the Windows-side paths/options.
----------------------------------------------------------*/
#define CONFIG_CONSOLE         "\\bin\\mintty.exe"
                                    //path to console program
#define CONFIG_CONSOLE_OPTIONS "-e" //options for the console program
#define CONFIG_SHELL           "\\bin\\tcsh.exe"
                                    //the shell to invoke
#define CONFIG_SHELL_OPTIONS   "-c" //options for the shell program

/*----------------------------------------------------------
Last, the target is executed within a proper shell, so we
set up the Cygwin-side paths/options.
----------------------------------------------------------*/
#define CONFIG_TARGET         "/usr/bin/vim"
                                    //the target program to run
#define CONFIG_TARGET_OPTIONS ""    //options for the target program

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------
Paths to programs with configured options.
----------------------------------------------------------*/
extern LPCTSTR          config_cygpath;
                                    //path and options for cygpath
extern LPCTSTR          config_console;
                                    //path and options for console program
extern LPCTSTR          config_shell;
                                    //path and options for shell
extern LPCTSTR          config_target;
                                    //path and options for target program

/*----------------------------------------------------------------------------
Interface Prototypes
----------------------------------------------------------------------------*/

#endif  /* _CONFIG_H */

