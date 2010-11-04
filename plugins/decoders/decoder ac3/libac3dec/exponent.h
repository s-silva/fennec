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
* File Name: exponent.h							
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
#ifndef _EXPONENT_H_
#define _EXPONENT_H_

#define UNPACK_FBW  1
#define UNPACK_CPL  2
#define UNPACK_LFE  4

uint_32 exponent_unpack( bsi_t *bsi, audblk_t *audblk);

#endif
