/*
* This source code is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*       
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* File Name: decode.h							
*
* Reference:
*
* Author:                                              
*
* Description:
*
* 	
* 
* History:
* 02/23/2005 
*  
*
*CodeReview Log:
* 
*/
#ifndef _DECODE_H_
#define _DECODE_H_ 

typedef struct stream_coeffs_s
{
	float fbw[5][256];
	float lfe[256];
} stream_coeffs_t;

typedef struct stream_samples_s
{
	float channel[6][512];
} stream_samples_t;

#define DECODE_MAGIC_NUMBER 0xdeadbeef
#define ERR_RESYNC_THRESHOLD 2
#define ERR_MANT_THRESHOLD 3
#define ERR_EXP_THRESHOLD 3

void decode_sanity_check_init(void);
long decode_sanity_check(void);

#endif

