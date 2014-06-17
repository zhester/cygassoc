/*****************************************************************************

main.c

Executes a predetermined program in its native environment (full login shell
inside a console window).  The goal is to allow simple Windows file type
association to be handled by a Cygwin program as if it were invoked from the
shell.

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
    if( arguments == NULL ) {
        return 1;
    }

    //see if a file was specified
    if( argc > 1 ) {

        //check for need to convert to ANSI characters
        #ifndef UNICODE
            argv = wc2mb_array( ( LPCWSTR* ) arguments, argc );
            if( argv == NULL ) {
                LocalFree( arguments );
                return 1;
            }
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

    //console was not started
    else {
        exit_code = 1;
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

    //check pointer
    if( array == NULL ) {
        return;
    }

    //free each item in the array
    for( index = 0; index < count; ++index ) {
        if( array[ index ] != NULL ) {
            free( array[ index ] );
        }
    }

    //free the array of pointers
    free( array );
}


/*=========================================================================*/
static char** wc2mb_array(          //convert array of strings WC -> MB
    LPCWSTR*            strings,    //wide-character array of string pointers
    int                 count       //number of strings in array
) {                                 //multi-byte array of string pointers

    //cleanly stops the string conversion loop
    #define stop_conversion()                           \
        free_array( ( void** ) result, ( index + 1 ) ); \
        result = NULL;                                  \
        break

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

    //check allocation
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

        //check allocation
        if( result[ index ] == NULL ) {

            //free any allocated memory, and stop converting strings
            stop_conversion();
        }

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

            //free any allocated memory, and stop converting strings
            stop_conversion();
        }
    }

    //return list of converted strings (or NULL on failure)
    return result;
}

