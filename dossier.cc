/*
 * Copyright (c) 2012 Karl N. Redgate
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/** \file house.cc
 * \brief A unit test/diagnostic shell for these libraries
 *
 * \todo add "service" status
 * \todo add process status/grep/lookup/etc
 * \todo add dbus
 * \todo add syslog
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <syslog.h>
#include <tcl.h>

#include "uuid.h"
#include "smbios.h"

/**
 * log level message
 */
static int
ErrorResult( Tcl_Interp *interp, const char *message ) {
    char *s = const_cast<char*>(message);
    Tcl_SetResult( interp, s, TCL_STATIC );
    return TCL_ERROR;
}

/**
 * log level message
 */
static int
Syslog_obj( ClientData data, Tcl_Interp *interp,
             int objc, Tcl_Obj * CONST *objv )
{
    if ( objc != 3 ) {
        Tcl_ResetResult( interp );
        Tcl_WrongNumArgs( interp, 1, objv, "level message" );
        return TCL_ERROR; 
    }

    char *level_name = Tcl_GetStringFromObj( objv[1], NULL );
    char *message = Tcl_GetStringFromObj( objv[2], NULL );

    int level = -1;
    if ( Tcl_StringMatch(level_name, "emerg")     )  level = LOG_EMERG;
    if ( Tcl_StringMatch(level_name, "emergency") )  level = LOG_EMERG;
    if ( Tcl_StringMatch(level_name, "alert")     )  level = LOG_ALERT;
    if ( Tcl_StringMatch(level_name, "crit")      )  level = LOG_CRIT;
    if ( Tcl_StringMatch(level_name, "critical")  )  level = LOG_CRIT;
    if ( Tcl_StringMatch(level_name, "err")       )  level = LOG_ERR;
    if ( Tcl_StringMatch(level_name, "error")     )  level = LOG_ERR;
    if ( Tcl_StringMatch(level_name, "warn")      )  level = LOG_WARNING;
    if ( Tcl_StringMatch(level_name, "warning")   )  level = LOG_WARNING;
    if ( Tcl_StringMatch(level_name, "notice")    )  level = LOG_NOTICE;
    if ( Tcl_StringMatch(level_name, "info")      )  level = LOG_INFO;
    if ( Tcl_StringMatch(level_name, "debug")     )  level = LOG_DEBUG;
    if ( level == -1 ) {
        return ErrorResult(interp, "invalid level");
    }
    syslog( level, "%s", message );

    Tcl_ResetResult( interp );
    return TCL_OK;
}

/**
 */
static void
Syslog_delete( ClientData data ) {
    closelog();
}

/**
 * Syslog command_name daemon application_name
 */
static int
Syslog_cmd( ClientData data, Tcl_Interp *interp,
             int objc, Tcl_Obj * CONST *objv )
{
    if ( objc != 4 ) {
        Tcl_ResetResult( interp );
        Tcl_WrongNumArgs( interp, 1, objv, "name facility application" );
        return TCL_ERROR; 
    }

    char *name = Tcl_GetStringFromObj( objv[1], NULL );
    char *facility_name = Tcl_GetStringFromObj( objv[2], NULL );
    char *application = Tcl_GetStringFromObj( objv[3], NULL );

    int facility = -1;
    if ( Tcl_StringMatch(facility_name, "auth")     )  facility = LOG_AUTHPRIV;
    if ( Tcl_StringMatch(facility_name, "authpriv") )  facility = LOG_AUTHPRIV;
    if ( Tcl_StringMatch(facility_name, "daemon")   )  facility = LOG_DAEMON;
    if ( Tcl_StringMatch(facility_name, "kernel")   )  facility = LOG_KERN;
    if ( Tcl_StringMatch(facility_name, "local0")   )  facility = LOG_LOCAL0;
    if ( Tcl_StringMatch(facility_name, "local1")   )  facility = LOG_LOCAL1;
    if ( Tcl_StringMatch(facility_name, "local2")   )  facility = LOG_LOCAL2;
    if ( Tcl_StringMatch(facility_name, "local3")   )  facility = LOG_LOCAL3;
    if ( Tcl_StringMatch(facility_name, "local4")   )  facility = LOG_LOCAL4;
    if ( Tcl_StringMatch(facility_name, "local5")   )  facility = LOG_LOCAL5;
    if ( Tcl_StringMatch(facility_name, "local6")   )  facility = LOG_LOCAL6;
    if ( Tcl_StringMatch(facility_name, "local7")   )  facility = LOG_LOCAL7;
    if ( Tcl_StringMatch(facility_name, "user")     )  facility = LOG_USER;
    if ( facility == -1 ) {
        return ErrorResult( interp, "invalid facility" );
    }
    openlog( strdup(application), 0, facility );

    long handle = 0;
    Tcl_CreateObjCommand( interp, name, Syslog_obj, (ClientData)handle, Syslog_delete );
    Tcl_SetResult( interp, name, TCL_VOLATILE );
    return TCL_OK;
}

/**
 */
static int
devno_cmd( ClientData data, Tcl_Interp *interp,
             int objc, Tcl_Obj * CONST *objv )
{
    if ( objc != 2 ) {
        Tcl_ResetResult( interp );
        Tcl_WrongNumArgs( interp, 1, objv, "filename" );
        return TCL_ERROR; 
    }

    char *filename = Tcl_GetStringFromObj( objv[1], NULL );
    struct stat s;
    if ( stat(filename, &s) < 0 ) {
        // would be better if these were errno messages
        return ErrorResult(interp, "failed to stat file");
    }
    if ( (S_ISCHR(s.st_mode) || S_ISBLK(s.st_mode)) == false ) {
        return ErrorResult(interp, "file is not a device");
    }

    Tcl_Obj *list = Tcl_NewListObj( 0, 0 );
    Tcl_ListObjAppendElement( interp, list, Tcl_NewIntObj( s.st_rdev >> 8 ) );
    Tcl_ListObjAppendElement( interp, list, Tcl_NewIntObj( s.st_rdev & 0xFF ) );
    Tcl_SetObjResult( interp, list );
    return TCL_OK;
}

/**
 */
int House_Init( Tcl_Interp *interp ) {
    int interactive = 1;
    Tcl_Obj *interactive_obj;
    interactive_obj = Tcl_GetVar2Ex( interp, "tcl_interactive", NULL, TCL_GLOBAL_ONLY );
    if ( interactive_obj != NULL ) {
        Tcl_GetIntFromObj( interp, interactive_obj, &interactive );
    }

    Tcl_Command command;
    command = Tcl_CreateObjCommand(interp, "Syslog", Syslog_cmd, (ClientData)0, NULL);
    if ( command == NULL ) {
        return false;
    }

    if ( interactive ) printf( " ** House debug tool v1.0\n" );
    Tcl_SetVar(interp, "tcl_rcFileName", "~/.houserc", TCL_GLOBAL_ONLY);

    Tcl_EvalEx( interp, "proc clock {command} { namespace eval ::tcl::clock $command}", -1, TCL_EVAL_GLOBAL );
    Tcl_EvalEx( interp, "proc commands {} {namespace eval commands {info procs}}", -1, TCL_EVAL_GLOBAL );

    if ( UUID_Initialize(interp) == false ) {
        return ErrorResult(interp, "UUID_Initialize failed");
    }
    if ( interactive ) printf( "UUID initialized\n" );

    if ( getuid() != 0 ) {
        if ( interactive ) printf( "BIOS not initialized, no access to /dev/mem\n" );
    } else {
        if ( BIOS::Initialize(interp) == false ) {
            return ErrorResult(interp, "BIOS::Initialize failed");
        }
        if ( interactive ) printf( "BIOS initialized\n" );
    }

    command = Tcl_CreateObjCommand(interp, "devno", devno_cmd, (ClientData)0, NULL);
    if ( command == NULL ) {
        return false;
    }

    return TCL_OK;
}

/**
 */
int main( int argc, char **argv ) {
    Tcl_Main( argc, argv, House_Init );
}

/*
 * vim:autoindent
 * vim:expandtab
 */
