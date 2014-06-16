/*****************************************************************************

path.c

Cygwin path translation using the installed cygpath program.

*****************************************************************************/

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "config.h"
#include "error.h"
#include "path.h"

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

#define MODE_MASK ( 0x000000003 )   //mode option bit mask

#define select_mode( _o ) path_options[ ( _o ) & MODE_MASK ]
                                    //selects the proper mode option

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

static const TCHAR*     path_options[] = {
    _T( "-u" ),                     //0x0: convert to Unix-style path
    _T( "-w" ),                     //0x1: convert to Windows-style path
    _T( "-m" ),                     //0x2: convert to mixed-style path
    _T( "-d" )                      //0x3: convert to DOS-style path
};                                  //path translation mode options
                                    //NOTE: There must be 4 and only 4.

/*----------------------------------------------------------------------------
Module Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Module Prototypes
----------------------------------------------------------------------------*/

static error_t run_cygpath(         //runs cygpath with a given output handle
    HANDLE              output,     //the target output handle
    LPCTSTR             path,       //the path to translate
    path_options_t      options     //translation options
);                                  //error code (0 = no error)


/*==========================================================================*/
error_t cygpath(                    //translates path strings
    LPTSTR              tr_path,    //translated path output
    size_t              tr_size,    //size of output string
    LPCTSTR             path,       //source path to translate
    path_options_t      options     //translation options
) {                                 //length of output or error

    //local macros

    //test for unicode interface
    #ifdef UNICODE
        #define free_buffer() free( buffer )
    #else
        #define free_buffer()
    #endif

    //local variables
    char*               buffer;     //pipe reading buffer (bytes only)
    #ifdef UNICODE
    int                 conv_result;//result of string conversion
    #endif
    DWORD               length;     //length of child process' output string
    error_t             result;     //resulting string length/error
    HANDLE              read_pipe;  //child stdout read-end of pipe
    error_t             run_result; //result of creating cygpath process
    SECURITY_ATTRIBUTES sec_attrs;  //pipe security attributes
    BOOL                win_result; //result of Win32 calls
    HANDLE              write_pipe; //child stdout write-end of pipe

    //initialize a generic return value
    result = ERROR_UNKNOWN;

    //check input
    if( ( tr_path == NULL ) || ( path == NULL ) ) {
        return ERROR_USAGE;
    }

    //initialize security attributes
    memset( &sec_attrs, 0, sizeof( sec_attrs ) );
    sec_attrs.nLength        = sizeof( SECURITY_ATTRIBUTES );
    sec_attrs.bInheritHandle = TRUE;

    //create a pipe for child process' stdout
    win_result = CreatePipe( &read_pipe, &write_pipe, &sec_attrs, 0 );

    if( win_result == FALSE ) {
        return ERROR_API_RESULT;
    }

    //read-end of pipe must not be inherited
    win_result = SetHandleInformation(
        read_pipe,
        HANDLE_FLAG_INHERIT,
        0
    );

    if( win_result == FALSE ) {
        CloseHandle( read_pipe );
        CloseHandle( write_pipe );
        return ERROR_API_RESULT;
    }

    //create the child process to run cygpath
    run_result = run_cygpath( write_pipe, path, options );

    if( run_result != ERROR_NONE ) {
        CloseHandle( read_pipe );
        CloseHandle( write_pipe );
        return run_result;
    }

    //initialize read buffer
    #ifdef UNICODE
        buffer = calloc( tr_size, sizeof( char ) );
    #else
        buffer = tr_path;
    #endif

    //read output from child process' stdout pipe
    win_result = ReadFile( read_pipe, buffer, tr_size, &length, NULL );

    if( win_result == FALSE ) {
        free_buffer();
        CloseHandle( read_pipe );
        CloseHandle( write_pipe );
        return ERROR_API_RESULT;
    }

    //set the output string length (minus the trailing newline)
    result = length - 1;

    //trim the trailing newline
    buffer[ result ] = 0;

    //see if unicode output conversion is necessary
    #ifdef UNICODE

        //convert the path to wide format
        conv_result = MultiByteToWideChar(
            CP_ACP,                     //use context's codepage
            0,                          //default conversion settings
            buffer,
            result,
            tr_path,
            tr_size
        );

        //check result of string conversion
        if( conv_result != result ) {

            //indicate a failure to convert the string
            result = ERROR_API_RESULT;
        }

    #endif

    //release allocated buffer
    free_buffer();

    //release pipe handles
    CloseHandle( read_pipe );
    CloseHandle( write_pipe );

    //return the result of translation
    return result;
}


/*=========================================================================*/
static error_t run_cygpath(         //runs cygpath with a given output handle
    HANDLE              output,     //the target output handle
    LPCTSTR             path,       //the path to translate
    path_options_t      options     //translation options
) {                                 //error code (0 = no error)

    //local macros
    #define BUFFER_SIZE ( 512 )     //string buffer size

    //local variables
    static TCHAR        command[ BUFFER_SIZE ];
                                    //cygpath command string
    PROCESS_INFORMATION proc_info;  //child process information
    STARTUPINFO         start_info; //child process startup information
    BOOL                win_result; //result of Win32 calls
    HRESULT             str_result; //result of string calls

    //initialize local variables
    memset( &proc_info,  0, sizeof( proc_info )  );
    memset( &start_info, 0, sizeof( start_info ) );

    //initialize the startup information to deal with piped outout
    start_info.cb         = sizeof( start_info );
    start_info.hStdError  = output;
    start_info.hStdOutput = output;
    start_info.dwFlags    = STARTF_USESTDHANDLES;

    //create the cygpath command
    str_result = StringCchPrintf(
        command,
        BUFFER_SIZE,
        _T( "%s %s \"%s\"" ),
        config_cygpath,
        select_mode( options ),
        path
    );

    if( str_result != S_OK ) {
        return ERROR_API_RESULT;
    }

    //create the process for cygpath
    win_result = CreateProcess(
        NULL,
        command,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &start_info,
        &proc_info
    );

    if( win_result == FALSE ) {
        return ERROR_API_RESULT;
    }

    //close the process handles
    CloseHandle( proc_info.hProcess );
    CloseHandle( proc_info.hThread );

    //return success
    return ERROR_NONE;
}

