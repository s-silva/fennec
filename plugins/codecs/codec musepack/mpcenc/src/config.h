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

/* Determine Endianess of the machine */

#define HAVE_LITTLE_ENDIAN  1234
#define HAVE_BIG_ENDIAN     4321

#define ENDIAN              HAVE_LITTLE_ENDIAN


/* Test the fast float-to-int rounding trick works */

#define HAVE_IEEE754_FLOAT
#define HAVE_IEEE754_DOUBLE


/* Test the presence of a 80-bit floating point type for writing AIFF headers */

#define HAVE_IEEE854_LONGDOUBLE


#ifndef MPPENC_VERSION
# define MPPENC_VERSION   "1.16"
#endif

#define MPPENC_BUILD  "--Stable--"

/* end of config.h */
