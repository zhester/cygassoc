/*****************************************************************************

main.c

Executes a predetermined program in its native environment (full login shell
inside a console window).  The goal is to allow simple Windows file type
association to be handled by a Cygwin program as if it were invoked from the
shell.

Configuration
-------------

To keep this program as minimal as possible, no run-time configuration is
available.  Adjusting the code and re-compiling is the currently preferred
method to adapt this application to various environments.  Most of the
settings are found in the following files:

- config.h
    Defines all file paths and program options.  This is where you'd
    change you shell to bash.

- Makefile
    Includes the target-specific information.  Replace the first real
    line (`include vimassoc.Makefile`) with a different set of values
    or a file defining customized values from the one provided.
    The specific Windows resource script (`vimassoc.rc`) can also be
    copied and customized to help Windows provide end-user details about
    the executable.

If someone or myself thinks up some other clever uses for such a program,
I may automate the configuration procedure to select the console, shell, and
target program.

Compiling
---------

This program requires native Windows compiling.  Under Cygwin, this can be
compiled using the included Makefile with the MinGW compiler for 64-bit
targets:

  Devel/mingw64-i686-gcc

The Makefile should work fairly universally, but was only used with GNU make
under Cygwin.

A Visual Studio 2013 solution/project is also included.

As the Visual Studio project does not use the Makefile to determine the
Windows resource script, the project settings would need to be changed to use
an alternate resource script.

### Wide Character Support ###

This program was my first foray into dual support for multi-byte strings and
unicode in the same program.  Flipping the compiler's setting between either
option should still result in a perfectly functional program.  Of course, in
multi-byte-string mode, you won't get wide character support.

### Architecture Support ###

Most of my development involved building for 64-bit targets.  The only reason
I know it also works for 32-bit targets is that Visual Studio 2013 Express
still defaults the project to 32-bit targets.

Installation
------------

The only file that matters is the executable output.  This is either
`build/vimassoc.exe` or `vs2013\Release\vimassoc.exe`.  Copy this somewhere
you won't delete later.  The examples below assume the file was copied to
the root of the Cygwin installation (C:\cygwin).

Windows file type association is most easily managed via the `assoc` and
`ftype` commands.

Example (> is input):
  >assoc .c
  .c=CSource
  >ftype CSource
  CSource=notepad.exe "%1"
  >ftype CSource=C:\cygwin\vimassoc.exe "%1"

The "gotcha" to using this method is that the current user will occasionally
have an override to the system's association for a given file type.  The
quickest way to fix such a problem is to check the registry for the file
extension as a key (check "whole word" match, too).  Delete any keys that
occur under the current user settings which are a part of Windows Explorer.
This should put the file type associations back to using the system-wide
settings.

Otherwise, you get to do a lot of clicking on each desired file type in
Windows explorer to target the program as its default.

If there is great demand for an automated installer (e.g. someone besides
myself wants to use this gadget), I'll probably throw together a batch script
or WSH script to perform a file copy and set up file type associations with a
little config file.

*****************************************************************************/

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <strsafe.h>

#include "config.h"
#include "error.h"
#include "path.h"

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Module Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Module Prototypes
----------------------------------------------------------------------------*/

static void free_array(             //frees an array of pointed-to things
    void**              array,      //pointer to array of pointers to free
    int                 count       //number of items in the array
);

static LPSTR* wc2mb_array(          //convert array of strings WC -> MB
    LPCWSTR*            strings,    //wide-character array of string pointers
    int                 count       //number of strings in array
);                                  //multi-byte array of string pointers


/*=========================================================================*/
int WINAPI WinMain(                 //Windows program entry point
    HINSTANCE           hInstance,  //handle to this application
    HINSTANCE           hPrevInstance,
                                    //handle to previous application
    LPSTR               lpCmdLine,  //executed command
    int                 nCmdShow    //window display options
) {                                 //program exit status

    //local macros
    #define BUFFER_SIZE ( 512 )     //common size of string buffers

    //local variables
    int                 argc;       //number of command line arguments
    LPTSTR*             argv;       //list of command line arguments
    LPWSTR*             arguments;  //list of argument string pointers
    static TCHAR        command[ BUFFER_SIZE ];
                                    //command to execute
    PROCESS_INFORMATION cp_pr_info; //CreateProcess process info
    BOOL                cp_result;  //result of CreateProcess
    STARTUPINFO         cp_su_info; //CreateProcess startup info
    static TCHAR        path[ BUFFER_SIZE ];
                                    //file path argument
    error_t             path_result;//error from path translation
    DWORD               exit_code;  //exit code of spawned process
    HRESULT             str_result; //result of string operations

    //parse the command line
    arguments = CommandLineToArgvW( GetCommandLineW(), &argc );

    //see if a file was specified
    if( argc > 1 ) {

        //check for need to convert to ANSI characters
        #ifndef UNICODE
            argv = wc2mb_array( ( LPCWSTR* ) arguments, argc );
        #else
            argv = arguments;
        #endif

        //translate the file path
        path_result = cygpath( path, BUFFER_SIZE, argv[ 1 ], PATH_OPT_UNIX );

        #ifndef UNICODE
            free_array( ( void** ) argv, argc );
        #endif

        if( path_result < ERROR_NONE ) {
            LocalFree( arguments );
            return 1;
        }

        //format command to execute a program with a file argument
        str_result = StringCchPrintf(
            command,
            BUFFER_SIZE,
            _T( "%s %s '%s %s'" ),
            config_console,
            config_shell,
            config_target,
            path
        );

    }

    //no file specified
    else {

        //format command to execute a program without a file argument
        str_result = StringCchPrintf(
            command,
            BUFFER_SIZE,
            _T( "%s %s '%s'" ),
            config_console,
            config_shell,
            config_target
        );
    }

    //parsed command-line arguments are no longer needed
    LocalFree( arguments );

    //check string formatting results
    if( str_result != S_OK ) {
        return 1;
    }

    //initialize CreateProcess argument data
    memset( &cp_su_info, 0, sizeof( cp_su_info ) );
    memset( &cp_pr_info, 0, sizeof( cp_pr_info ) );

    //create the process for mintty
    cp_result = CreateProcess(
        NULL,
        command,
        NULL,
        NULL,
        FALSE,
        //CREATE_NO_WINDOW,
        0,
        NULL,
        NULL,
        &cp_su_info,
        &cp_pr_info
    );

    //wait for console to finish
    if( cp_result == TRUE ) {
        WaitForSingleObject( cp_pr_info.hProcess, INFINITE );
        cp_result = GetExitCodeProcess( cp_pr_info.hProcess, &exit_code );
        CloseHandle( cp_pr_info.hProcess );
        CloseHandle( cp_pr_info.hThread );
    }

    //return exit code from spawned process
    return exit_code;
}


/*=========================================================================*/
static void free_array(             //frees an array of pointed-to things
    void**              array,      //pointer to array of pointers to free
    int                 count       //number of items in the array
) {

    //local variables
    int                 index;      //array index

    //free each item in the array
    for( index = 0; index < count; ++index ) {
        free( array[ index ] );
    }

    //free the array pointers
    free( array );
}


/*=========================================================================*/
static char** wc2mb_array(          //convert array of strings WC -> MB
    LPCWSTR*            strings,    //wide-character array of string pointers
    int                 count       //number of strings in array
) {                                 //multi-byte array of string pointers

    //local variables
    int                 conv_result;//result of string conversion
    int                 index;      //array index
    int                 length;     //lengths of strings
    char**              result;     //list of converted strings
    int                 size;       //size of new buffer
    BOOL                used_default;
                                    //flag if conversion defaulted

    //allocate an array of string pointers
    result = calloc( count, sizeof( char* ) );

    if( result == NULL ) {
        return NULL;
    }

    //convert each string to multi-byte representation
    for( index = 0; index < count; ++index ) {

        //get length of wide-character input string
        length = wcslen( strings[ index ] );

        //converted string needs enough room for a terminator
        size = length + 1;

        //allocate storage for the same number of bytes
        result[ index ] = calloc( size, sizeof( char ) );

        //perform the conversion to a multi-byte string
        conv_result = WideCharToMultiByte(
            CP_ACP,                 //use context's codepage
            0,                      //default conversion settings
            strings[ index ],       //source string to convert
            length,                 //length of the string to convert
            result[ index ],        //conversion destination memory
            size,                   //size of destination memory
            NULL,                   //use system default character
            &used_default           //flag if conversion defaulted
        );

        //check the conversion
        if( conv_result != length ) {

            //free the memory allocated up to this point
            free_array( ( void** ) result, ( index + 1 ) );

            //indicate a failure
            return NULL;
        }
    }

    //return list of converted strings
    return result;
}
