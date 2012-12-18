
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

/** \file dumpdmi.cc
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
 */
int main( int argc, char **argv ) {
    SMBIOS::Header smbios;
    smbios.probe( SMBIOS::Header::locate() );
    smbios.print_xml( stdout );
    return 0;
}

/*
 * vim:autoindent
 * vim:expandtab
 */
