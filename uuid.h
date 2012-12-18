
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

/** \file UUID.h
 * \brief 
 */

#ifndef _UUID_H_
#define _UUID_H_

#include <stdint.h>
#include <unistd.h>
#include <tcl.h>

/**
 * Without an arguement, create a new UUID.
 *
 * With an unsigned char* -- assume 16 bytes of raw uuid data to copy.
 * With a char* -- assume a string to be parsed.
 */
class UUID {
private:
    uint8_t data[16];
    char string[37];
    void parse();
    void format();
    uint8_t nybble( int );
public:
    UUID( UUID& );
    UUID();
    UUID( uint8_t * );
    UUID( char * );
    bool set( uint8_t * );
    bool set( char * );
    inline char *to_s() { return string; }
    inline uint8_t *raw() { return data; }
    bool operator == ( UUID& );
    bool operator != ( UUID& );
};

extern bool UUID_Initialize( Tcl_Interp * );

#endif

/*
 * vim:autoindent
 * vim:expandtab
 */
