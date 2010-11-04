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
* File Name: downmix.h							
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
#ifndef _DOWNMIX_H_
#define _DOWNMIX_H_ 

void downmix( stream_samples_t *samples, sint_16 *out_samples, bsi_t *pbsi, int );
void conv480to441( sint_16 *out_samples, long samp_count, long chan );

#endif

