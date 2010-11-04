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

//// Macros typical for Compilers:
//
//  __TURBOC__          Turbo-C, Borland-C
//  __BORLANDC__        Borland-C
//  __ZTC__             Zortech-C
//  _MSC_VER            Microsoft-C
//  __EMX__             Eberhard Mattes EMX (GNU based)
//  __GNUC__            GNU C based compiler (also Cygwin)
//  __CYGWIN__          Cygnus Windows C-Compiler (GNU based)
//  __APPLE_CC__        Apple GCC (GNU based)


//// Macros typical for Operating Systems
//
//  __linux__           Linux
//  __bsdi__            BSDi
//  __FreeBSD__         FreeBSD
//  __NetBSD__          NetBSD
//  __OpenBSD__         OpenBSD
//  __unix__            Unix ????????
//  _WIN16              16 bit-Windows
//  _WIN32              32 bit-Windows (WIN32 is wrong, not defined by not MSC) (also __GNUC__ + _WIN32 is possible)
//  _HPUX_SOURCE        HP-UX
//  __BEOS__            BeOS
//  __APPLE__           Apple Mac OS X (only when using Apple GCC)
//  ???????             MS-DOS and relatives


//// Macros typical for special conformances
//                      System 5 Release 4 (SVr4)
//                      System 5 ID     (SVID)
//                      POSIX 1.0
//                      POSIX 1.0b
//                      X/OPEN
//                      BSD 4.3
//                      BSD 4.4
//                      ANSI


// Macros to manipulate Sockets + Files in one, in (+0x4000)
// output times TIME/TIME_T/DTIME

#ifndef MPPDEC_MPPDEC_H
#define MPPDEC_MPPDEC_H

//// optimization/feature defines //////////////////////////////////
#ifndef NOT_INCLUDE_CONFIG_H
# include "config.h"
#endif
#include "./mpp.h"


//// portable system includes //////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>


//// system dependent system includes //////////////////////////////
// low level I/O, where are prototypes and constants?
#if   defined _WIN32  ||  defined __TURBOC__  ||  defined __ZTC__  ||  defined _MSC_VER
# include <io.h>
# include <fcntl.h>
# include <time.h>
# include <sys/types.h>
# include <sys/stat.h>
#elif defined __unix__  ||  defined __linux__  ||  defined __APPLE__
# include <fcntl.h>
# include <unistd.h>
# include <sys/time.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <sys/stat.h>
#else
// .... add Includes for new Operating System here (with prefix: #elif defined)
# include <fcntl.h>
# include <unistd.h>
# include <sys/ioctl.h>
# include <sys/stat.h>
#endif


#if   defined __linux__
#  include <fpu_control.h>
#elif defined __FreeBSD__
# include <machine/floatingpoint.h>
#elif defined _MSC_VER
# include <float.h>
#endif


#if defined _WIN32
# undef USE_OSS_AUDIO
# undef USE_ESD_AUDIO
# undef USE_SUN_AUDIO
#else
# undef USE_WIN_AUDIO
#endif

#if defined __APPLE__
# undef USE_OSS_AUDIO
# undef USE_SUN_AUDIO
# undef USE_WIN_AUDIO
# undef USE_NICE
# undef USE_REALTIME
# undef USE_ASM
# undef USE_ESD_AUDIO
# define NO_DEV_AUDIO
# undef ENDIAN
# define ENDIAN HAVE_BIG_ENDIAN
#endif

#if defined __TURBOC__
# undef USE_OSS_AUDIO
# undef USE_ESD_AUDIO
# undef USE_SUN_AUDIO
# undef USE_NICE
# undef USE_REALTIME
#endif

#if defined USE_DIET  ||  defined MAKE_24BIT  ||  defined MAKE_32BIT
# undef USE_ESD_AUDIO
#endif

#if defined MAKE_16BIT  ||  defined MAKE_24BIT  ||  defined MAKE_32BIT
# undef USE_ASM
#endif

#if INT_MAX < 2147483647L
# undef USE_ASM
#endif

// sound card
#if defined USE_OSS_AUDIO
# include <sys/ioctl.h>
# include <sys/time.h>
# if   defined __linux__        // the standard is that this file is stored somewhere on the hard disk
#  include <linux/soundcard.h>
# elif defined __bsdi_  ||  defined __FreeBSD__
#  include <sys/soundcard.h>
# elif defined __NetBSD__  ||  defined __OpenBSD__
#  include <soundcard.h>
# elif defined __APPLE__  &&  defined __MACH__
#  include <pleasepatchheretherightpathofsoundcard.hforMacOSX/soundcard.h>
# else
#  include <pleasepatchheretherightpathof/soundcard.h>
# endif
#endif /* USE_OSS_AUDIO */

#if defined USE_ESD_AUDIO
# include <esd.h>
#endif

#if defined USE_SUN_AUDIO
# include <sys/audioio.h>
#endif

#ifdef MPP_ENCODER
# undef USE_HTTP
#endif

#ifdef USE_HTTP
# ifdef _WIN32
#  include <winsock2.h>
# else
#  include <sys/socket.h>
# endif
#endif

#if   defined USE_WIN_AUDIO
# include <windows.h>
# define WINAUDIO_FD            ((FILE_T)-128)
#elif defined USE_IRIX_AUDIO
# define IRIXAUDIO_FD           ((FILE_T)-127)
#endif
#define  NULL_FD                ((FILE_T)-126)

#if defined USE_NICE  &&  !defined _WIN32
# include <sys/resource.h>
#endif

// scheduler stuff
#if defined USE_REALTIME  &&  !defined _WIN32
# include <sched.h>
#endif

#ifndef O_BINARY
# ifdef _O_BINARY
#  define O_BINARY              _O_BINARY
# else
#  define O_BINARY              0
# endif
#endif

#if defined _WIN32  ||  defined __TURBOC__
# define strncasecmp(__s1,__s2,__n) strnicmp ((__s1), (__s2), (__n))
# define strcasecmp(__s1,__s2)      stricmp  ((__s1), (__s2))
# define MKDIR(__dir,__attr)        mkdir ((__dir))
#else
# define MKDIR(__dir,__attr)        mkdir ((__dir), (__attr))
#endif

#if defined _WIN32
# include <direct.h>
# define snprintf                   _snprintf
# define getcwd(__buff,__len)       _getcwd ((__buff), (__len))
# define sleep(__sec)               Sleep ((__sec) * 1000)
#endif

#if defined _WIN32
# define TIME_T                     long
# define TIME(__x)                  time ( &(__x) )
# define DTIME(__x,__y)             ( (double)(__y) - (__x) )
#else
# define TIME_T                     struct timeval
# define TIME(__x)                  gettimeofday ( &(__x), NULL )
# define DTIME(__x,__y)             ( ((double)(__y).tv_sec - (__x).tv_sec) + 1.e-6 * ((double)(__y).tv_usec - (__x).tv_usec) )
#endif

#if   defined __GNUC__
# define inline                 __inline__
# define restrict
#elif defined _WIN32
# define inline                 __inline
# define restrict
#else
# define inline
# define restrict
#endif


//// Binary/Low-Level-IO ///////////////////////////////////////////
//
// All file I/O is basicly handled via an ANSI file pointer (type: FILE*) in
// FILEIO-Mode 1 and via a POSIX file descriptor (type: int) in
// FILEIO-Mode 2 and 3.
//
// Some operations are only available via the POSIX interface (fcntl, setmode,
// ...) so we need a function to get the file descriptor from a file pointer.
// In FILEIO-Mode 2 and 3 this is a dummy function because we are always working
// with these file descriptors.
//

#if  FILEIO == 1
# if   defined __BORLANDC__  ||  defined _WIN32
#  define FILENO(__fp)          _fileno ((__fp))
# elif defined __CYGWIN__  ||  defined __TURBOC__  ||  defined __unix__  ||  defined __EMX__  ||  defined _MSC_VER
#  define FILENO(__fp)          fileno  ((__fp))
# else
#  define FILENO(__fp)          fileno  ((__fp))
# endif
#else
#  define FILENO(__fd)          (__fd)
#endif


//
// If we have access to a file via file name, we can open the file with an
// additional "b" or a O_BINARY within the (f)open function to get a
// transparent untranslated data stream which is necessary for audio bitstream
// data and also for PCM data. If we are working with
// stdin/stdout/FILENO_STDIN/FILENO_STDOUT we can't open the file with these
// attributes, because the files are already open. So we need a non
// standardized sequence to switch to this mode (not necessary for Unix).
// Mostly the sequence is the same for incoming and outgoing streams, but only
// mostly so we need one for IN and one for OUT.
// Macros are called with the file pointer and you get back the untransalted file
// pointer which can be equal or different from the original.
//

#if   defined __EMX__
# define SETBINARY_IN(__fp)     (_fsetmode ( (__fp), "b" ), (__fp))
# define SETBINARY_OUT(__fp)    (_fsetmode ( (__fp), "b" ), (__fp))
#elif defined __TURBOC__ || defined __BORLANDC__
# define SETBINARY_IN(__fp)     (setmode   ( FILENO ((__fp)),  O_BINARY ), (__fp))
# define SETBINARY_OUT(__fp)    (setmode   ( FILENO ((__fp)),  O_BINARY ), (__fp))
#elif defined __CYGWIN__
# define SETBINARY_IN(__fp)     (setmode   ( FILENO ((__fp)), _O_BINARY ), (__fp))
# define SETBINARY_OUT(__fp)    (setmode   ( FILENO ((__fp)), _O_BINARY ), (__fp))
#elif defined _WIN32
# define SETBINARY_IN(__fp)     (_setmode  ( FILENO ((__fp)), _O_BINARY ), (__fp))
# define SETBINARY_OUT(__fp)    (_setmode  ( FILENO ((__fp)), _O_BINARY ), (__fp))
#elif defined _MSC_VER
# define SETBINARY_IN(__fp)     (setmode   ( FILENO ((__fp)),  O_BINARY ), (__fp))
# define SETBINARY_OUT(__fp)    (setmode   ( FILENO ((__fp)),  O_BINARY ), (__fp))
#elif defined __unix__
# define SETBINARY_IN(__fp)     (__fp)
# define SETBINARY_OUT(__fp)    (__fp)
#elif 0
# define SETBINARY_IN(__fp)     (freopen   ( NULL, "rb", (__fp) ), (__fp))
# define SETBINARY_OUT(__fp)    (freopen   ( NULL, "wb", (__fp) ), (__fp))
#else
# define SETBINARY_IN(__fp)     (__fp)
# define SETBINARY_OUT(__fp)    (__fp)
#endif

// file I/O using ANSI buffered file I/O via file pointer FILE* (fopen, fread, fwrite, fclose)
#if  FILEIO == 1
# define OFF_T                  signed long
# define FILE_T                 FILE*
# define OPEN(name)             fopen  (name, "rb" )
# define OPENRW(name)           fopen  (name, "r+b")
# define CREATE(name)           fopen  (name, "wb" )
# define INVALID_FILEDESC       NULL
# define CLOSE(fp)              fclose (fp)                  // CLOSE   returns -1 on error, otherwise 0
# define READ(fp,ptr,len)       fread  (ptr, 1, len, fp)     // READ    returns -1 or 0 on error/EOF, otherwise > 0
# define READ1(fp,ptr)          fread  (ptr, 1, 1, fp)       // READ    returns -1 or 0 on error/EOF, otherwise > 0
# define WRITE(fp,ptr,len)      fwrite (ptr, 1, len, fp)     // WRITE   returns -1 or 0 on error/EOF, otherwise > 0
# define SEEK(fp,offs,lbl)      fseek  (fp, offs, lbl)       // SEEK    returns -1 on error, otherwise >= 0
# define FILEPOS(fp)            ftell  (fp)                  // FILEPOS returns -1 on error, otherwise >= 0
# define STDIN                  stdin
# define STDOUT                 stdout
# define STDERR                 stderr
# define FDOPEN(fd,mode)        fdopen (fd, mode)
# define UNBUFFER(fp)           setvbuf(fp, NULL, _IONBF, 0 )
# define FLUSH(fp)              fflush (fp)
#endif /* FILEIO==1 */

// file I/O using POSIX unbuffered file I/O via file descriptors (open, read, write, close)
#if  FILEIO == 2
# ifdef WIN32
#  define OFF_T                 _off_t
# else
#  define OFF_T                 off_t
# endif
# define FILE_T                 int
# define OPEN(name)             open  (name, O_RDONLY|O_BINARY)
# define OPENRW(name)           open  (name, O_RDWR  |O_BINARY)
# define CREATE(name)           open  (name, O_WRONLY|O_BINARY|O_TRUNC|O_CREAT, 0644)
# define INVALID_FILEDESC       (-1)
# define CLOSE(fd)              close (fd)                   // CLOSE   returns -1 on error, otherwise 0
# if defined HAVE_INCOMPLETE_READ
#  define READ(fd,ptr,len)      complete_read (fd, ptr, len) // READ    returns -1 or 0 on error/EOF, otherwise > 0
# else
#  define READ(fd,ptr,len)      (size_t)read   (fd, ptr, len)// READ    returns -1 or 0 on error/EOF, otherwise > 0
# endif
# define READ1(fd,ptr)          (size_t)read   (fd, ptr, 1)  // READ    returns -1 or 0 on error/EOF, otherwise > 0
# define WRITE(fd,ptr,len)      (size_t)write  (fd, ptr, len)// WRITE   returns -1 or 0 on error/EOF, otherwise > 0
# define SEEK(fd,offs,lbl)      lseek  (fd, offs, lbl)       // SEEK    returns -1 on error, otherwise >= 0
# define FILEPOS(fd)            lseek  (fd, 0L, SEEK_CUR)    // FILEPOS returns -1 on error, otherwise >= 0
# define STDIN                  0
# define STDOUT                 1
# define STDERR                 2
# define FDOPEN(fd,mode)        (fd)
# define UNBUFFER(fd)           (void)(fd)
# define FLUSH(fd)              (void)(fd)
#endif /* FILEIO==2 */

// file I/O using Turbo-C lowest level unbuffered file I/O via file descriptors (_open, _read, _write, _close)
#if  FILEIO == 3
# define OFF_T                  signed long
# define FILE_T                 int
# define OPEN(name)             _open (name, O_RDONLY)
# define OPENRW(name)           _open (name, O_RDWR  )
# define CREATE(name)           _creat(name, 0)
# define INVALID_FILEDESC       (-1)
# define CLOSE(fd)              _close (fd)                  // CLOSE   returns -1 on error, otherwise 0
# define READ(fd,ptr,len)       (size_t)_read  (fd, ptr, len)// READ    returns -1 or 0 on error/EOF, otherwise > 0
# define READ1(fd,ptr)          (size_t)_read  (fd, ptr, 1)  // READ    returns -1 or 0 on error/EOF, otherwise > 0
# define WRITE(fd,ptr,len)      (size_t)_write (fd, ptr, len)// WRITE   returns -1 or 0 on error/EOF, otherwise > 0
# define SEEK(fd,offs,lbl)      lseek  (fd, offs, lbl)       // SEEK    returns -1 on error, otherwise >= 0
# define FILEPOS(fd)            lseek  (fd, 0L, SEEK_CUR)    // FILEPOS returns -1 on error, otherwise >= 0
# define STDIN                  0
# define STDOUT                 1
# define STDERR                 2
# undef  SETBINARY_IN
# undef  SETBINARY_OUT
# define SETBINARY_IN(fd)       (fd)
# define SETBINARY_OUT(fd)      (fd)
# define FDOPEN(fd,mode)        (fd)
# define UNBUFFER(fd)           (void)(fd)
# define FLUSH(fd)              (void)(fd)
#endif /* FILEIO==3 */

#if FILEIO != 2  &&  defined USE_HTTP
# error HTTP can only be used by FILEIO==2
#endif

#if defined _WIN32  ||  defined __BEOS__
# define WRITE_SOCKET(sock,ptr,len)     send (sock, ptr, len, 0)
# define READ_SOCKET(sock,ptr,len)      recv (sock, ptr, len, 0)
#else
# define WRITE_SOCKET(sock,ptr,len)     write (sock, ptr, len)
# define READ_SOCKET(sock,ptr,len)      read  (sock, ptr, len)
#endif

#ifdef _WIN32
# define POPEN_READ_BINARY_OPEN(cmd)    _popen ((cmd), "rb")
# define POPEN_WRITE_BINARY_OPEN(cmd)   _popen ((cmd), "wb")
# define PCLOSE(fp)                     _pclose(fp)
#else
# define POPEN_READ_BINARY_OPEN(cmd)    popen ((cmd), "r")
# define POPEN_WRITE_BINARY_OPEN(cmd)   popen ((cmd), "w")
# define PCLOSE(fp)                     pclose(fp)
#endif

#if defined _WIN32
# define STRUCT_STAT            struct _stat
# define STAT_CMD(f,s)          _stat (f, s)
#else
# define STRUCT_STAT            struct stat
# define STAT_CMD(f,s)          stat (f, s)
#endif /* WIN32 */

#ifndef S_ISDIR
# if   defined S_IFDIR
#  define S_ISDIR(x)            ((x) &   S_IFDIR)
# elif defined _S_IFDIR
#  define S_ISDIR(x)            ((x) &  _S_IFDIR)
# elif defined __S_IFDIR
#  define S_ISDIR(x)            ((x) & __S_IFDIR)
# else
#  error Cannot find a way to test for a directory
# endif
#endif /* !S_ISDIR */

#if defined __unix__  ||  defined __bsdi__  ||  defined __FreeBSD__  ||  defined __OpenBSD__  ||  defined __NetBSD__  ||  defined __TURBOC__  ||  defined _WIN32  ||  defined __APPLE__
# define ISATTY(fd)             isatty (fd)
#else
# define ISATTY(fd)             0
#endif

// Path separator
#if defined __unix__  ||  defined __bsdi__  ||  defined __FreeBSD__  ||  defined __OpenBSD__  ||  defined __NetBSD__  ||  defined __APPLE__
# define PATH_SEP               '/'
# define DRIVE_SEP              '\0'
# define EXE_EXT                ""
# define DEV_NULL               "/dev/null"
# define ENVPATH_SEP            ':'
#elif defined _WIN32  ||  defined __TURBOC__  ||  defined __ZTC__  ||  defined _MSC_VER
# define PATH_SEP               '\\'
# define DRIVE_SEP              ':'
# define EXE_EXT                ".exe"
# define DEV_NULL               "\\nul"
# define ENVPATH_SEP            ';'
#else
# define PATH_SEP               '/'         // Amiga: C:/
# define DRIVE_SEP              ':'
# define EXE_EXT                ""
# define DEV_NULL               "nul"
# define ENVPATH_SEP            ';'
#endif

// maximum length of file names
#ifndef PATHLEN_MAX
# if   defined FILENAME_MAX
#  define PATHLEN_MAX           FILENAME_MAX
# elif INT_MAX < 2147483647L
#  define PATHLEN_MAX            128
# else
#  define PATHLEN_MAX           1024
# endif
#endif /* !PATHLEN_MAX */

#ifdef _WIN32
# define TitleBar(text)   SetConsoleTitle (text)
#else
# define TitleBar(text)   (void) (text)
#endif


//// constants /////////////////////////////////////////////////////
#ifdef USE_SV8
# define MAX_SV          "SV8"
#else
# define MAX_SV          "SV7"
#endif

#ifdef USE_ASM
# define BUILD           "3DNOW!/SSE"
#else
# define BUILD           ""
#endif


#define COPYRIGHT        "(C) 1999-2003 Buschmann/Klemm/Piecha/Wolf"

#define DECODER_DELAY    (512 - 32 + 1)
#define BLK_SIZE         (36 * 32)


//// logging defines, for development only /////////////////////////
#if defined _WIN32  ||  defined __TURBOC__
# define LOGPATH         ".\\"
# define MUSICPATH       "D:\\AUDIO\\"
#else
# define LOGPATH         "./"
# define MUSICPATH       "/Archive/Audio/"
#endif
#define _(x)             (void)(fprintf(stderr,"<%d>\n",(x)),fflush(stderr))

#ifdef DEBUG
# define REP(x)          (void)(x)
#else
# define REP(x)
#endif


//// numerical constants ///////////////////////////////////////////
#define C00              (Float) 0.500000000000000000000000L    // Cxx = 0.5 / cos (xx*M_PI/64)
#define C01              (Float) 0.500602998235196301334178L
#define C02              (Float) 0.502419286188155705518560L
#define C03              (Float) 0.505470959897543659956626L
#define C04              (Float) 0.509795579104159168925062L
#define C05              (Float) 0.515447309922624546962323L
#define C06              (Float) 0.522498614939688880640101L
#define C07              (Float) 0.531042591089784174473998L
#define C08              (Float) 0.541196100146196984405269L
#define C09              (Float) 0.553103896034444527838540L
#define C10              (Float) 0.566944034816357703685831L
#define C11              (Float) 0.582934968206133873665654L
#define C12              (Float) 0.601344886935045280535340L
#define C13              (Float) 0.622504123035664816182728L
#define C14              (Float) 0.646821783359990129535794L
#define C15              (Float) 0.674808341455005746033820L
#define C16              (Float) 0.707106781186547524436104L
#define C17              (Float) 0.744536271002298449773679L
#define C18              (Float) 0.788154623451250224773056L
#define C19              (Float) 0.839349645415527038721463L
#define C20              (Float) 0.899976223136415704611808L
#define C21              (Float) 0.972568237861960693780520L
#define C22              (Float) 1.060677685990347471323668L
#define C23              (Float) 1.169439933432884955134476L
#define C24              (Float) 1.306562964876376527851784L
#define C25              (Float) 1.484164616314166277319733L
#define C26              (Float) 1.722447098238333927796261L
#define C27              (Float) 2.057781009953411550808880L
#define C28              (Float) 2.562915447741506178719328L
#define C29              (Float) 3.407608418468718785698107L
#define C30              (Float) 5.101148618689163857960189L
#define C31              (Float)10.190008123548056810994678L

#define SS05             (Float) 0.840896415253714543018917L      // 0.5^0.25


#ifndef M_PI
# define M_PI            3.1415926535897932384626433832795029     // 4*atan(1)
# define M_PIl           3.1415926535897932384626433832795029L
# define M_LN2           0.6931471805599453094172321214581766     // ln(2)
# define M_LN2l          0.6931471805599453094172321214581766L
# define M_LN10          2.3025850929940456840179914546843642     // ln 10 */
# define M_LN10l         2.3025850929940456840179914546843642L
#endif


//// 'Cdecl' forces the use of standard C/C++ calling convention ///////
#if   defined _WIN32
# define Cdecl           __cdecl
#elif defined __ZTC__
# define Cdecl           _cdecl
#elif defined __TURBOC__
# define Cdecl           cdecl
#else
# define Cdecl
#endif

//// expect handling of GCC ////////////////////////////////////////
#ifdef __GNUC__
# if __GNUC__ < 3
#  define __builtin_expect(cond,exp)  (cond)
#  ifndef expect
#    define expect(cond,exp)          __builtin_expect(cond,exp)
#  endif
# else
#  ifndef expect
#   define expect(cond,exp)           __builtin_expect(cond,exp)
#  endif
# endif
#else
# define __builtin_expect(cond,exp)   (cond)
# ifndef expect
#  define expect(cond,exp)            __builtin_expect(cond,exp)
# endif
#endif

#define if0(x)                        if (expect(x,0))
#define if1(x)                        if (expect(x,1))
#define while0(x)                     while (expect(x,0))
#define while1(x)                     while (expect(x,1))

#ifndef __GNUC__
# define __attribute__(x)
#else
# define __attribute__(x)
#endif

//// Remaining macros //////////////////////////////////////////////
// selects input buffer size and some constants needed for input buffer handling
#ifndef IBUFLOG2                 // must be at least 10 (bitrate always <626 kbps) or better 11 ( <1253 kbps)
# if INT_MAX < 2147483647L
#  define IBUFLOG2       11      // 8 KByte buffer, possible 11...13 (32 KByte limit)
# else
#  define IBUFLOG2       21      // 8 MByte buffer, possible 11...29 ( 2 GByte limit)
# endif
#endif
#define IBUFSIZE         ((size_t)(1LU<<(IBUFLOG2)))
#define IBUFSIZE2        ((size_t)((IBUFSIZE)/2))
#define IBUFMASK         ((size_t)((IBUFSIZE)-1))

// save memory space for 16 bit compiler (data + stack < 64 KByte)
#if INT_MAX < 2147483647L
# if VIRT_SHIFT     >  6
#  undef  VIRT_SHIFT
#  define VIRT_SHIFT   6
# endif
# if      IBUFLOG2  > 11
#  undef  IBUFLOG2
#  define IBUFLOG2    11
# endif
# define USE_HUFF_PACK
# define USE_ARRAY_PACK
#endif

// generate a macro which contains information about compile time settings
#define STR(x)   _STR(x)
#define _STR(x)  #x
#ifdef NDEBUG
# define T1  ""
#else
# define T1  "DEBUG "
#endif
#if  defined USE_OSS_AUDIO  ||  defined USE_ESD_AUDIO  ||  defined USE_SUN_AUDIO  ||  defined USE_WIN_AUDIO
# define T2  "SND "
#else
# define T2  ""
#endif
#ifdef USE_NICE
# define T3  "NICE "
#else
# define T3  ""
#endif
#if defined USE_REALTIME
# define T4  "RT "
#else
# define T4  ""
#endif
#ifdef HAVE_IEEE754_FLOAT
# define T5  "IEEE "
#else
# define T5  ""
#endif
#define T6  "IO=" STR(FILEIO) " "
#ifdef USE_HUFF_PACK
# define T7  "H-PCK "
#else
# define T7  ""
#endif
#ifdef USE_ARRAY_PACK
# define T8  "A-PCK "
#else
# define T8  ""
#endif
#define T9  "SHFT=" STR(VIRT_SHIFT) " "
#define T10 "IBUF=" STR(IBUFLOG2) " "

#define COMPILER_FLAGS  T1 T2 T3 T4 T5 T6 T7 T8 T9 T10

// align a pointer by maybe incrementing it
#define ALIGN(ptr,alignment) \
                    (void*)((((ptrdiff_t)(ptr)) & (-(ptrdiff_t)(alignment))) + (alignment))   // aligns a pointer with alignment, the source array should be at least alignment-1 Bytes longer than the needed length


//// Simple types //////////////////////////////////////////////////

#if   CHAR_BIT == 8  &&  SCHAR_MAX == 127L
typedef unsigned char       Uint8_t;    // guaranteed  8 bit unsigned integer type with range 0...255
typedef signed   char       Int8_t;     // guaranteed  8 bit signed   integer type with range -128...127
#else
# error No  8 bit int type found. Tested: char
#endif

#if   SHRT_MAX == 32767L
typedef unsigned short int  Uint16_t;   // guaranteed 16 bit unsigned integer type with range 0...65535
typedef signed   short int  Int16_t;    // guaranteed 16 bit signed   integer type with range -32768...32767
#else
# error No 16 bit int type found. Tested: short
#endif

#if   INT_MAX == 2147483647L
typedef unsigned int        Uint32_t;   // guaranteed 32 bit unsigned integer type with range 0...4294967295
typedef signed   int        Int32_t;    // guaranteed 32 bit signed   integer type with range -2147483648...2147483647
#elif LONG_MAX == 2147483647L
typedef unsigned long int   Uint32_t;   // guaranteed 32 bit unsigned integer type with range 0...4294967295
typedef signed   long int   Int32_t;    // guaranteed 32 bit signed   integer type with range -2147483648...2147483647
#else
# error No 32 bit int type found. Tested: int, long
#endif

#if    defined __C99__                 // C9x has a type which is exact 64 bit
typedef int64_t             Int64_t;
typedef uint64_t            Uint64_t;
typedef intmax_t            IntMax_t;
typedef uintmax_t           UintMax_t;
# define IntMax_MIN        -9223372036854775808
# define IntMax_MAX         9223372036854775807
# define UintMax_MAX       18446744073709551615
# define UintMAX_FP(x)      (long double)(x)
#elif  defined __GNUC__                // GCC uses long long as 64 bit
typedef signed   long long  Int64_t;
typedef unsigned long long  Uint64_t;
typedef signed   long long  IntMax_t;
typedef unsigned long long  UintMax_t;
# define IntMax_MIN        -9223372036854775808LL
# define IntMax_MAX         9223372036854775807LL
# define UintMax_MAX       18446744073709551615LLU
# define UintMAX_FP(x)      (long double)(x)
#elif defined _MSC_VER
typedef signed   __int64    Int64_t;
typedef unsigned __int64    Uint64_t;
typedef signed   __int64    IntMax_t;
typedef unsigned __int64    UintMax_t;
# define IntMax_MIN        -9223372036854775808I64
# define IntMax_MAX         9223372036854775807I64
# define UintMax_MAX       18446744073709551615UI64
# define UintMAX_FP(x)      (long double)(IntMax_t)(x)
#elif defined LLONG_MAX               // long long (when existing) is normally 64 bit
typedef signed   long long  Int64_t;
typedef unsigned long long  Uint64_t;
typedef signed   long long  IntMax_t;
typedef unsigned long long  UintMax_t;
# define IntMax_MIN        -9223372036854775808LL
# define IntMax_MAX         9223372036854775807LL
# define UintMax_MAX       18446744073709551615LLU
# define UintMAX_FP(x)      (long double)(x)
#elif  LONG_MAX > 0xFFFFFFFFLU         // long is longer than 33 bit, assume 64 bit
typedef signed   long       Int64_t;
typedef unsigned long       Uint64_t;
typedef signed   long       IntMax_t;
typedef unsigned long       UintMax_t;
# define IntMax_MIN        -9223372036854775808L
# define IntMax_MAX         9223372036854775807L
# define UintMax_MAX       18446744073709551615LU
# define UintMAX_FP(x)      (long double)(x)
#elif  defined _WIN32                  // Microsoft and Intel call it __int64
typedef signed   __int64    Int64_t;
typedef unsigned __int64    Uint64_t;
typedef signed   __int64    IntMax_t;
typedef unsigned __int64    UintMax_t;
# define IntMax_MIN        -9223372036854775808I64
# define IntMax_MAX         9223372036854775807I64
# define UintMax_MAX       18446744073709551615UI64
# define UintMAX_FP(x)      (long double)(IntMax_t)(x)
#else
# define NO_INT64_T                    // no type mapped to 64 bit integer
typedef signed   long       IntMax_t;
typedef unsigned long       UintMax_t;
# define IntMax_MIN        -2147483648L
# define IntMax_MAX         2147483647L
# define UintMax_MAX        4294967295LU
# define UintMAX_FP(x)      (long double)(x)
#endif


#if defined _WIN32  &&  !defined __GNUC__  &&  !defined __C99__
typedef signed long         ssize_t;
#endif

#ifdef USE_ARRAY_PACK
typedef signed char         Bool_t;     // ==0: false, !=0: true
#else
typedef signed int          Bool_t;     // ==0: false, !=0: true
#endif
typedef Uint32_t            Ibuf_t;     // type for input buffer, currently this type must be 32 bit
typedef signed   char       Schar;      // at least -127...+127
typedef unsigned char       Uchar;      // at least 0...255
typedef signed   short int  Short;      // at least -32767...+32767, memory economic type
typedef unsigned short int  Ushort;     // at least 0...65535, memory economic type
typedef signed   int        Int;        // at least -32767...+32767, fast type
typedef unsigned int        Uint;       // at least 0...65535, fast type
typedef signed   long int   Long;       // at least -2147483647...+2147483647, but more is better
typedef unsigned long int   Ulong;      // at least 0...4294967295, but more is better
//                          size_t;     // size of memory objects
//                          ptrdiff_t;  // pointer differences, may be larger than size_t
typedef float               Float32_t;  // guaranteed 32 bit floating point type
typedef double              Float64_t;  // guaranteed 64 bit floating point type
typedef float               Float;      // fastest floating point type, memory economic (used for all PCM calculations)
#define SIZEOF_Float  4                 // size of the type 'Float' in sizeof units
typedef double              Double;     // floating point with extended precision (more than 32 bit mantissa)
typedef long double         Ldouble;    // most exact floating point format
typedef Int16_t             Int2x16_t [2];
typedef Int32_t             Int2x32_t [2];

#if   defined MAKE_16BIT  ||  defined MAKE_24BIT  ||  defined MAKE_32BIT
# ifdef NO_INT64_T
#   error No 64 bit int type found, needed for HQ 16...32 bit output
# endif
typedef Int2x32_t           Int2xSample_t;
# if defined MAKE_32BIT
#  define SAMPLE_SIZE        32
#  define PROG_NAME          "mppdec32"
#  define SAMPLE_SIZE_STRING " (32 bit HQ)"
#  define Write_PCM(fd,p,b)  Write_PCM_HQ_2x32bit ( fd, p, b )
#  define Synthese_Filter(Stream,offset,Vi,Yi,ch) \
                            Synthese_Filter_32_C ( Stream, offset, Vi, Yi, ch )
#  undef  USE_ESD_AUDIO
# elif defined MAKE_24BIT
#  define SAMPLE_SIZE        24
#  define PROG_NAME          "mppdec24"
#  define SAMPLE_SIZE_STRING " (24 bit HQ)"
#  define Write_PCM(fd,p,b)  Write_PCM_HQ_2x24bit ( fd, p, b )
#  define Synthese_Filter(Stream,offset,Vi,Yi,ch) \
                            Synthese_Filter_32_C ( Stream, offset, Vi, Yi, ch )
#  undef  USE_ESD_AUDIO
# elif defined MAKE_16BIT
#  define SAMPLE_SIZE        16
#  define PROG_NAME          "mppdec16"
#  define SAMPLE_SIZE_STRING " (16 bit HQ)"
#  define Write_PCM(fd,p,b)  Write_PCM_HQ_2x16bit ( fd, p, b )
#  define Synthese_Filter(Stream,offset,Vi,Yi,ch) \
                            Synthese_Filter_32_C ( Stream, offset, Vi, Yi, ch )
# endif
#else
typedef Int2x16_t           Int2xSample_t;
# define SAMPLE_SIZE        16
# define PROG_NAME          "mppdec"
# define SAMPLE_SIZE_STRING ""
# define Write_PCM(fd,p,b)  Write_PCM_2x16bit ( fd, p, b )
# ifdef USE_ASM
#  define Synthese_Filter(Stream,offset,Vi,Yi,ch) \
                            Synthese_Filter_16 ( Stream, offset, Vi, Yi )
# else
#  define Synthese_Filter(Stream,offset,Vi,Yi,ch) \
                            Synthese_Filter_16_C ( Stream, offset, Vi, Yi )
# endif /* USE_ASM */
#endif


//// More complex types ////////////////////////////////////////////
typedef struct {
    Int    L [36];
    Int    R [36];
} Quant_t ;

typedef struct {
    Uint   L;
    Uint   R;
} UPair_t ;

typedef struct {
    Int    L;
    Int    R;
} Pair_t ;

typedef struct {
#ifdef USE_ARRAY_PACK
    Schar  L;
    Schar  R;
#else
    Int    L;
    Int    R;
#endif
} CPair_t ;

typedef Float     FloatArray [32];
typedef UPair_t   UPairArray [32];
typedef Pair_t    PairArray  [32];
typedef CPair_t   CPairArray [32];
typedef Float     SCFTriple   [3];
typedef struct {
#ifndef MPP_ENCODER
    Uint32_t      Code;         // >=32 bit
# ifdef USE_HUFF_PACK
    Schar         Value;        // >= 7 bit
    Uchar         Length;       // >= 4 bit
# else
    Int           Value;
    Uint          Length;
# endif
#else
# ifdef USE_HUFF_PACK
    Uint8_t       Length;      // >=  4 bit
    Uint8_t       ___;
    Uint16_t      Code;        // >= 14 bit
# else
    Uint          Code;
    Uint          Length;
# endif
#endif
} Huffman_t ;

typedef struct {
    Uint          Code   : 16;  // >= 14 bit
    Uint          Length :  8;  // >=  4 bit
} HuffSrc_t ;

typedef struct {
    OFF_T         FileSize;
    Int           GenreNo;
    Int           TrackNo;
    char          Genre   [128];
    char          Year    [ 20];
    char          Track   [  8];
    char          Title   [256];
    char          Artist  [256];
    char          Album   [256];
    char          Comment [512];
} TagInfo_t ;

typedef void  (*SyntheseFilter16_t) ( Int2x16_t* Stream, Int* const offset, Float* Vi, const FloatArray* Yi );
typedef void  (*SyntheseFilter32_t) ( Int2x32_t* Stream, Int* const offset, Float* Vi, const FloatArray* Yi, int ch );
typedef Int   (*HeaderWriter_t)     ( FILE_T outputFile, Ldouble  SampleFreq, Uint BitsPerSample, Uint Channels, Ulong SamplesPerChannel );

#if defined MAKE_16BIT  ||  defined MAKE_24BIT  ||  defined MAKE_32BIT
typedef struct {
    const Float*  FilterCoeff;
    Uint64_t      Mask;
    Float64_t     Add;
    Float         Dither;
    Uint32_t      Overdrives;
    Int64_t       MaxLevel;
    Bool_t        NoShaping;
    Float         ErrorHistory     [2] [16];       // max. 2 channels, 16th order Noise shaping
    Float         DitherHistory    [2] [16];
    Int32_t       LastRandomNumber [2];
} dither_t;
#else
typedef struct {
    Uint32_t      Overdrives;
    Int32_t       MaxLevel;
} dither_t;
#endif


//// Variables /////////////////////////////////////////////////////

// decode.c
extern Ibuf_t             InputBuff [IBUFSIZE]; // read buffer for the MP+ data stream
extern size_t             InputCnt;             // current offset in this buffer

// huffsv7.c
extern Huffman_t          HuffHdr    [10];
extern Huffman_t          HuffSCFI   [ 4];
extern Huffman_t          HuffDSCF   [16];
extern Huffman_t          HuffQ1 [2] [ 3*3*3];
extern Huffman_t          HuffQ2 [2] [ 5*5];
extern Huffman_t          HuffQ3 [2] [ 7];
extern Huffman_t          HuffN3 [2] [ 7*7];
extern Huffman_t          HuffQ4 [2] [ 9];
extern Huffman_t          HuffQ5 [2] [15];
extern Huffman_t          HuffQ6 [2] [31];
extern Huffman_t          HuffQ7 [2] [63];
extern Huffman_t          HuffN8 [2][127];
extern const Huffman_t*   HuffQ  [2] [ 8];
extern const Huffman_t*   HuffN  [2] [ 9];
extern Uint8_t            LUT1_0  [1<< 6];
extern Uint8_t            LUT1_1  [1<< 9];
extern Uint8_t            LUT2_0  [1<< 7];
extern Uint8_t            LUT2_1  [1<<10];
extern Uint8_t            LUT3_0  [1<< 4];
extern Uint8_t            LUT3_1  [1<< 5];
extern Uint8_t            LUT4_0  [1<< 4];
extern Uint8_t            LUT4_1  [1<< 5];
extern Uint8_t            LUT5_0  [1<< 6];
extern Uint8_t            LUT5_1  [1<< 8];
extern Uint8_t            LUT6_0  [1<< 7];
extern Uint8_t            LUT6_1  [1<< 7];
extern Uint8_t            LUT7_0  [1<< 8];
extern Uint8_t            LUT7_1  [1<< 8];
extern Uint8_t            LUTDSCF [1<< 6];

// huffsv46.c
extern const Huffman_t*   Entropie      [18];
extern const Huffman_t*   Region        [32];
extern Huffman_t          SCFI_Bundle   [ 8];
extern Huffman_t          DSCF_Entropie [13];

// mppdec.c
extern Float              Y_L      [36] [32];
extern Float              Y_R      [36] [32];
extern CPair_t            SCF_Index [3] [32];      // Scalefactor
extern CPair_t            Res           [32];      // resolution steps of the subbands
extern Quant_t            Q             [32];      // quantized samples
extern CPair_t            SCFI          [32];      // transfer order of the SCF
extern Bool_t             MS_Band       [32];      // subband-wise flag for M/S-signal guidance
extern Bool_t             MS_used;                 // global flag for M/S-signal guidance
extern Bool_t             IS_used;

#define LITTLE                  0
#define BIG                     1
extern Bool_t                   output_endianess;
#if   ENDIAN == HAVE_LITTLE_ENDIAN
# define machine_endianess      LITTLE
#elif ENDIAN == HAVE_BIG_ENDIAN
# define machine_endianess      BIG
#endif

// requant.c
extern Float              __SCF    [6 + 128];       // tabulated Scalefactors from -6 to +127
#define SCF             ( __SCF + 6 )
extern Int8_t             Q_bit         [32];       // number of bits to save the resolution (SV6)
extern Int8_t             Q_res         [32] [16];  // Index -> resolution (SV6)
extern Uint               Bitrate;
extern Int                Min_Band;
extern Int                Max_Band;
extern Float              __Cc          [1 + 18];
extern const Uint         __Dc          [1 + 18];
#define Cc              ( __Cc + 1 )
#define Dc              ( __Dc + 1 )

// synthtab.c
extern const Float        Cos64         [32];
extern const Float        Di_opt        [32] [16];

// stderr.c


//// procedures/functions //////////////////////////////////////////
// cpu_feat.c
Bool_t Cdecl  Has_MMX                 ( void );
Bool_t Cdecl  Has_SIMD                ( void );
Bool_t Cdecl  Has_SIMD2               ( void );
Bool_t Cdecl  Has_3DNow               ( void );

// decode.c
void       Bitstream_init             ( void );
Ulong      BitsRead                   ( void );
Uint32_t   Bitstream_read             ( Int  bits );
Uint32_t   Bitstream_peek             ( Uint pos, Int bits );
void       Bitstream_skip             ( Uint bits );
Uint32_t   Bitstream_preview          ( Int  bits );  // same as above, but data doesn't get receipted yet
void       Read_Bitstream_SV6         ( void );
void       Read_Bitstream_SV7         ( void );
void       Read_Bitstream_SV8         ( void );

// http.c
int        http_open                  ( const char* URL );

// huffsv7.c
void       Init_Huffman_Decoder_SV7   ( void );
void       Init_Huffman_Encoder_SV7   ( void );

// huffsv46.c
void       Init_Huffman_Decoder_SV4_6 ( void );

// id3tag.c
Int        Read_ID3V1_Tags            ( FILE_T fp, TagInfo_t* tip );
Int        Read_APE_Tags              ( FILE_T fp, TagInfo_t* tip );

// requant.c
void       Init_QuantTab              ( Int maximum_Band, Bool_t used_IS, Double amplification, Uint StreamVersion );

// synth.c
Uint32_t   random_int                 ( void );

void Cdecl Calculate_New_V_i387       ( const Float* Sample, Float* V );
void Cdecl Calculate_New_V_3DNow      ( const Float* Sample, Float* V );
void Cdecl New_V_Helper2              ( Float* A, const Float* Sample );
void Cdecl New_V_Helper3              ( Float* A, const Float* Sample );
void Cdecl New_V_Helper4              ( Float* V );

void Cdecl VectorMult_i387            ( void* buff, const Float* V );
void Cdecl VectorMult_3DNow           ( void* buff, const Float* V );
void Cdecl VectorMult_SIMD            ( void* buff, const Float* V );

void       Synthese_Filter_16_C       ( Int2x16_t* Stream, Int* const offset, Float* Vi, const FloatArray* Yi );
void       Synthese_Filter_32_C       ( Int2x32_t* Stream, Int* const offset, Float* Vi, const FloatArray* Yi, Uint channel );

void Cdecl Reset_FPU                  ( void );
void Cdecl Reset_FPU_3DNow            ( void );
void Cdecl memcpy_dn_MMX              ( void* dst, const void* src, size_t words64byte  );
void Cdecl memcpy_dn_SIMD             ( void* dst, const void* src, size_t words128byte );

void       Init_Dither                ( Int bits, int shapingtype, Double dither );
void       OverdriveReport            ( void );
SyntheseFilter16_t
           Get_Synthese_Filter        ( void );

// tools.c
size_t     Read_LittleEndians         ( FILE_T fp, Uint32_t* dst, size_t words32bit );
void       Requantize_MidSideStereo   ( Int Stop_Band, const Bool_t* used_MS );
void       Requantize_IntensityStereo ( Int Start_Band, Int Stop_Band );
void       Resort_HuffTable           ( Huffman_t* const Table, const size_t elements, Int offset );
void       Make_HuffTable             ( Huffman_t* dst, const HuffSrc_t* src, size_t len );
void       Make_LookupTable           ( Uint8_t* LUT, size_t LUT_len, const Huffman_t* const Table, const size_t elements );
size_t     complete_read              ( int fd, void* dest, size_t bytes );
int        isdir                      ( const char* Name );
void       Init_FPU                   ( void );

// wave_out.c
Int        Write_WAVE_Header          ( FILE_T outputFile, Ldouble SampleFreq, Uint BitsPerSample, Uint Channels, Ulong SamplesPerChannel );
Int        Write_AIFF_Header          ( FILE_T outputFile, Ldouble SampleFreq, Uint BitsPerSample, Uint Channels, Ulong SamplesPerChannel );
Int        Write_Raw_Header           ( FILE_T outputFile, Ldouble SampleFreq, Uint BitsPerSample, Uint Channels, Ulong SamplesPerChannel );
Int        Set_DSP_OSS_Params         ( FILE_T outputFile, Ldouble SampleFreq, Uint BitsPerSample, Uint Channels );
Int        Set_DSP_Sun_Params         ( FILE_T outputFile, Ldouble SampleFreq, Uint BitsPerSample, Uint Channels );
Int        Set_ESD_Params             ( FILE_T dummyFile , Ldouble SampleFreq, Uint BitsPerSample, Uint Channels );
Int        Set_WIN_Params             ( FILE_T dummyFile , Ldouble SampleFreq, Uint BitsPerSample, Uint Channels );
Int        Set_IRIX_Params            ( FILE_T dummyFile , Ldouble SampleFreq, Uint BitsPerSample, Uint Channels );
size_t     Write_PCM_2x16bit          ( FILE_T outputFile, Int2x16_t* data, size_t len );
size_t     Write_PCM_HQ_2x16bit       ( FILE_T outputFile, Int2x32_t* data, size_t len );
size_t     Write_PCM_HQ_2x24bit       ( FILE_T outputFile, Int2x32_t* data, size_t len );
size_t     Write_PCM_HQ_2x32bit       ( FILE_T outputFile, Int2x32_t* data, size_t len );
int        WIN_Play_Samples           ( const void* buff, size_t len );
int        IRIX_Play_Samples          ( const void* buff, size_t len );
int        WIN_Audio_close            ( void );
int        IRIX_Audio_close           ( void );
void       DisableSUID                ( void );
void       EnableSUID                 ( void );

// pipeopen.c
FILE*      pipeopen                   ( const char* command, const char* filename );

// stderr.c
void       SetStderrSilent            ( Bool_t state );
Bool_t     GetStderrSilent            ( void );
int Cdecl  stderr_printf              ( const char* format, ... );

// _setargv.c
long       treewalk                   ( const char* start, const char** mask, int (*fn)(const char* filename, void* aux), void* aux );
void       mysetargv                  ( int* argc, char*** argv, const char** extentions );

#if ENDIAN == HAVE_BIG_ENDIAN

# define ReadLE32(dst,psrc)       dst = *(Uint32_t*)(psrc)
# define ReadBE32(dst,psrc)                           \
       ((Uint8_t*)&(dst))[0] = ((Uint8_t*)(psrc))[3], \
       ((Uint8_t*)&(dst))[1] = ((Uint8_t*)(psrc))[2], \
       ((Uint8_t*)&(dst))[2] = ((Uint8_t*)(psrc))[1], \
       ((Uint8_t*)&(dst))[3] = ((Uint8_t*)(psrc))[0]


#elif defined __i386__           /* 486+ */

#  define ReadBE32(dst,psrc)      __asm__ ( "bswap %0" : "=r" (dst) : "0" (*(Uint32_t*)(psrc)) )
#  define ReadLE32(dst,psrc)       dst = *(Uint32_t*)(psrc)

# else

#  define ReadBE32(dst,psrc)                          \
       ((Uint8_t*)&(dst))[0] = ((Uint8_t*)(psrc))[3], \
       ((Uint8_t*)&(dst))[1] = ((Uint8_t*)(psrc))[2], \
       ((Uint8_t*)&(dst))[2] = ((Uint8_t*)(psrc))[1], \
       ((Uint8_t*)&(dst))[3] = ((Uint8_t*)(psrc))[0]
#  define ReadLE32(dst,psrc)       dst = *(Uint32_t*)(psrc)

# endif

//// Profiler include //////////////////////////////////////////////
#include "profile.h"

#ifdef _MSC_VER
#pragma warning ( disable : 4244 )
#endif

#endif /* MPPDEC_MPPDEC_H */

/* end of mppdec.h */
