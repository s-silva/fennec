/*
 * Musepack audio compression
 * Copyright (C) 1999-2004 Buschmann/Klemm/Piecha/Wolf
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "mppdec.h"
#include "profile.h"

/*
 *  For every architecture you want to profile/checkpoint you need the following items:
 *
 *  uintmax_t:
 *      A type which is used for time calculation, mostly 32 bit or 64 bit,
 *      should be large enough so that no overruns occures during the measurement
 *  STD_TIMER_CLK:
 *      Clock frequency of the used timer in MHz
 *  RDTSC():
 *      A macro reading the current time into a local variable timetemp with the type uintmax_t,
 *      Time is in 1.e-6/STD_TIMER_CLK seconds.
 *
 *  Places:
 *      typedef of uintmax_t:     profile.h
 *      RDTSC():                  profile.h
 *      STD_TIMER_CLK:            profile.c
 *      no-inline functions maybe needed by RDTSC():
 *                                profile.c
 */

#ifdef PROFILE

#ifdef __TURBOC__
# define STD_TIMER_CLK  1.193181667 /* MHz */

uintmax_t
readtime ( void )               /* PC onboard timer */
{
    asm  XOR   AX, AX
    asm  MOV   ES, AX
    asm  OUT   67, AL
    asm  MOV   DX, ES:[46Ch]
    asm  IN    AL, 64
    asm  XCHG  AL, AH
    asm  IN    AL, 64
    asm  XCHG  AL, AH
    asm  NEG   AX
}

#elif defined USE_SYSV_TIMER
# define STD_TIMER_CLK    1.0000000 /* MHz */

# include <sys/time.h>
# include <unistd.h>

uintmax_t
readtime ( void )               /* System V timer */
{
    struct timeval  tv;

    gettimeofday ( &tv, NULL );
    return tv.tv_sec * (uintmax_t)1000000LU + tv.tv_usec;
}

#else
# define STD_TIMER_CLK  233.3333333 /* MHz */
#endif


uintmax_t       timecounter    [256];
const char*     timename       [256];
unsigned char   functionstack [1024];
unsigned char*  functionstack_pointer = functionstack;


static void Cdecl
signal_handler ( int signum )
{
    char            name [128];
    char            file [128];
    char            no   [ 32];
    unsigned char*  f;

    (void) stderr_printf ( "\n\nSignal %d detected. Call stack:\n", signum );
    for ( f = functionstack+1; f <= functionstack_pointer; f++ ) {
        (void) sscanf        ( timename[*f], "%128[^|]|%128[^|]|%32[0-9]", name, file, no );
        (void) stderr_printf ( "%-24.24s%12.12s:%s\n", name, file, no );
    }
    _exit ( 128+signum );
}


void
set_signal ( void )
{
    signal ( SIGILL , signal_handler );
    signal ( SIGINT , signal_handler );
    signal ( SIGSEGV, signal_handler );
    signal ( SIGFPE , signal_handler );
}


void
report ( void )
{
    static char  dash [] = "---------------------------------------";
    uintmax_t    sum;
    uintmax_t    max;
    int          i;
    int          j;
    int          k;
    char         name [128];
    char         file [128];
    char         no   [ 32];
    size_t       filelen;
    double       MHz = STD_TIMER_CLK;

#ifdef __linux__
    FILE*        fp;

    // read out CPU frequency if proc-FS is present
    if ( (fp = fopen ("/proc/cpuinfo", "r")) != NULL ) {
        while ( fgets(name, sizeof(name), fp) )
            if ( 1 == sscanf ( name, "cpu MHz : %lf", &MHz ) )
                break;
        (void) fclose (fp);
    }
#endif

    // calculate total time
    for ( sum = 0, i = 1; i < sizeof(timecounter)/sizeof(*timecounter); i++ )
        sum += timecounter [i];

    (void) fprintf ( stderr, "\n%s%s\n", dash, dash );
    (void) fprintf ( stderr, "100.0%%   %13.6f ms   *** TOTAL ***%25s[%.1f MHz]\n", sum/(MHz*1000.), "", MHz );

    // output sorted
    while ( 1 ) {
        for ( max = 0, j = 1; j < sizeof(timecounter)/sizeof(*timecounter); j++ )
            if ( timecounter [j] > max )
                max = timecounter [k = j];
        if (max == 0)
            break;
        sscanf ( timename [k], "%128[^|]|%128[^|]|%32[0-9]", name, file, no );
        filelen = strlen (file);
        (void) fprintf ( stderr, "%6.2f%%  %13.6f ms   %-28.28s%18.18s:%s\n",
                         100. * timecounter[k] / sum, timecounter[k] / (MHz*1000.),
                         name, filelen < 18 ? file : file+filelen-18, no );
        timecounter [k] = 0;
    }

    (void) fprintf ( stderr, "%s%s\n", dash, dash );
    (void) fflush  ( stderr );
}

#endif /* PROFILE */

/* end of profile.c */
