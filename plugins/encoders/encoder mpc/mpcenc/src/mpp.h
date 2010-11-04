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

/******************************************************
 *                                                    *
 *            Source Compile configuration            *
 *                                                    *
 ******************************************************/


#if !defined(__APPLE__)
// use optimized assembler routines for Pentium III/K6-2/Athlon (only 32 bit OS, Intel x86 and no MAKE_xxBITS)
// you need the NASM assembler on your system, the program becomes a little bit larger and decoding
// on AMD K6-2 (x3), AMD K6-III (x3), AMD Duron (x1.7), AMD Athlon (x1.7), Pentium III (x2) and Pentium 4 (x1.8) becomes faster
#define USE_ASM

// Open Sound System support (only Unix with OSS support)
// If your Operating System supports the Open Sound System, you can output to /dev/dsp* and
// instead of writing a file the program plays the file via this sound device.
// on some systems you also must link the libossaudio library, so maybe you also must edit the Makefile
#define USE_OSS_AUDIO

// Enlightenment Sound Daemon support (only Unix with ESD support)
// If your Operating System supports the Enlightenment Sound Daemon you can output to /dev/esd and
// instead of writing a file the program plays the file via this sound device.
// you also must link the libesd library, so maybe you also must edit the Makefile
//#define USE_ESD_AUDIO

#endif

// native Sun Onboard-Audio support (only SunOS)
// If you have a Sun Workstation with Onboard-Audio, you can output to /dev/audio and
// instead of writing a file the program plays the file via this sound device.
// Some machines lacking librt.a so you are unable to link a static executable with realtime-support.
// Although you can still perfectly use the dynamic executable.
//#define USE_SUN_AUDIO

// Sound support for SGI Irix
// If you have a SGI Workstation running IRIX, you can output to /dev/audio and
// instead of writing a file the program plays the file via this sound device.
//#define USE_IRIX_AUDIO

// Audio support for Windows (WAVE OUT) (only Windows)
// If you have a Windows based system and if you also want to play files directly instead of only writing audio files,
// then define the next item
#define USE_WIN_AUDIO

// Buffersize for Windows Audio in 4.5 KByte units
// Only needed for Windows+USE_WIN_AUDIO
// Good values are 8...32 for fast machines and 128...512 for slow machines
// large values decrease average performance a little bit, increase memory
// consumption (1 Block = 4.5 KByte), but increase buffering, so it takes a
// longer time to get a dropout. Note that I don't have a 486/80...133, so
// I don't know anything about their performance.
// (Attention: 512 = additional 2.3 MByte of memory)
#define MAX_WAVEBLOCKS    40

// increase priority if destination is an audio device
// this increases the priority of the decoder when playing the file directly to a sound card to reduce/prevent
// dropouts during the playback due to CPU time shortage
#define USE_NICE

// use realtime scheduling if destination is an audio device
// This sets the program to real time priority when playing the file directly to a sound card.
// Now it should be really difficult to get dropouts (file IO and other realtime programs are the remaining weak points)
#define USE_REALTIME

// use ANSI-Escape sequences to structure output
#define USE_ANSI_ESCAPE

// Use termios for reading values from keyboard without echo and ENTER
#define USE_TERMIOS

// if none of the next three macros MAKE_xxBIT is defined,
// normal non-dithered and non-shaped 16 bit PCM output is generated

// create 16 bit Output
// output is 16 bit wide, you can also dither and noise shape
//#define MAKE_16BIT

// create 24 bit Output
// output is 24 bit wide instead of 16 bit wide, you can also dither and noise shape
//#define MAKE_24BIT

// create 32 bit Output
// output is 32 bit wide instead of 16 bit wide, you can also dither and noise shape
//#define MAKE_32BIT

// Select subset of function used for file I/O:
//   1: ANSI via file pointer (FILE*)
//   2: POSIX via file handle (int or HANDLE)
//   3: POSIX like lowest level function of Turbo/Borland C
//   4: WinAMP 3: running inside WinAMP
// Try to use '2', if this doesn't work, try '1'. '3' is for Borland compilers.
#ifndef FILEIO
# if   defined MPP_ENCODER
#  define FILEIO      1             // mppenc still uses buffered ANSI-I/O
# elif defined MPP_DECODER
#  define FILEIO      2
# else
#   error Neigher MPP_DECODER nor MPP_ENCODER is defined. Abort.
# endif
#endif

// the POSIX function read() can return less bytes than requested not only at the end of the file.
// if this happens, the following macro must be defined:
#define HAVE_INCOMPLETE_READ

// use a shorter Huffman_t  representation, may be faster
// use for performance tuning
#define USE_HUFF_PACK

// use shorter representation for SCF_Index[][] and Res[], may be faster
// use for performance tuning
#define USE_ARRAY_PACK

// use the System 5 timer for profiling
// otherwise a special piece of code for Turbo-C is used or the Timestamp Counter on Intel IA32/gcc systems.
// Both is highly non-portable. This solution is more portable (you only need a SYS 5 compatible system,
// but also much much more inaccurate.
//#define USE_SYSV_TIMER

// do a memory shift every n subband samples, otherwise only increment pointer (6, 12, 18 and 36 are good values)
// use for performance tuning
#define VIRT_SHIFT    18

// selects InputBuff size, size is 4 * 2^IBUFLOG2 bytes (11...14 are good values)
// use for performance tuning
// can also be used to eliminate disk performance issue while tuning the program
// (set to a value, so the test cases are fully read before decoding
#define IBUFLOG2      14

// Dump contents of MPEGplus files (only for development), 0x00 no dump
// Bit 0: maxband, Bit 1: msbits, Bit 2: allocation/resolution, Bit 3: SCF
// Bit 4: Subsamples, Bit 5: Datenrate, Bit 6: Bitusage der Sektionen
//#define DUMPSELECT    0xFF

// 16 bit and 32 bit accesses must be aligned, otherwise a bus error occures.
// try this if you get bus errors
//#define MUST_ALIGNED

// Experimental: use http/ftp streaming
#define USE_HTTP

// _use setargv module
#define USE_ARGV


// Use IPv4 and IPv6
//#define USE_IPv4_6
// Use only IPv6
//#define USE_IPv6

// compile StreamVersion 8 decoding (always disable, no usabiltity)
// do not edit
//#define USE_SV8

// disables assert()
// assert() is for development only and decreases speed and increases the size of the program
#ifndef NDEBUG
# define NDEBUG
#endif

// Some other tracing (only for development)
// do not edit
//#define DEBUG

// Some tracings of popen()
// do not edit
//#define DEBUG2

// activate simple profiler
//#define PROFILE

// make debug output in tags.c stfu
#define STFU

/* end of mpp.h */
