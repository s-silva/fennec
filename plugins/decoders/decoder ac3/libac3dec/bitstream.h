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
* File Name: bitstream.h							
*
* Reference:
*
* Author: Li Feng,  fli_linux@yahoo.com.cn                                                 
*
* Description:
*
* 	
* 
* History:
* 02/23/2005  Li Feng    Created
*  
*
*CodeReview Log:
* 
*/
#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "ac3.h"

typedef struct bitstream_s
{
	uint_8* lpCurr;
	uint_32 current_word;
	uint_32 bits_left;
	uint_8* lpEnd;
} bitstream_t;

bitstream_t* bitstream_open(uint_8* lpStart, uint_8* lpEnd);
uint_32 bitstream_get(bitstream_t *bs,uint_32 num_bits);

#endif 

