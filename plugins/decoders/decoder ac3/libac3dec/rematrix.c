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
* File Name: rematrix.c							
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
#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "rematrix.h"

struct rematrix_band_s
{
	uint_32 start;
	uint_32 end;
};

struct rematrix_band_s rematrix_band[] = { {13,24}, {25,36}, {37 ,60}, {61,252}};

static uint_32 min32(uint_32 a,uint_32 b)
{
	return (a < b ? a : b);
}

/* This routine simply does stereo rematixing for the 2 channel 
 * stereo mode */
void rematrix(audblk_t *audblk, stream_coeffs_t *coeffs)
{
	uint_32 num_bands;
	uint_32 start;
	uint_32 end;
	uint_32 i,j;

	if( (!audblk->cplinu) )
		num_bands = 4;	/* coupling is not in use, num_bands = 4 */
	else
	{	/* coupling is in use */
		if ( audblk->cplbegf > 2 )
			num_bands = 4;
		else if (audblk->cplbegf > 0)
			num_bands = 3;
		else
			num_bands = 2;
	} /* endif ( !audblk->cplinu ) */

	for(i=0;i < num_bands; i++)
	{	
		register float *fbw0, *fbw1;
		register float left,right;
		if(!audblk->rematflg[i])
			continue;

		start = rematrix_band[i].start;
		end = (audblk->cplinu ? min32(rematrix_band[i].end ,
			12 * audblk->cplbegf + 36) : rematrix_band[ i ].end);

		j = start;
		fbw0 = &coeffs->fbw[0][j];
		fbw1 = &coeffs->fbw[1][j];
		for(j=start;j < end; j++)
		{
			left  = (float)0.5 * (*fbw0 + *fbw1);
			right  = (float)0.5 * (*fbw0 - *fbw1);
			*fbw0++ = left;
			*fbw1++ = right;
		}
	}
}
