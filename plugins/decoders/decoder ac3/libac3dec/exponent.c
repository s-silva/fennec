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
* File Name: exponent.c							
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
#include "exponent.h"


static uint_32 exp_unpack_ch(uint_16 type,uint_16 expstr,uint_16 ngrps,uint_16 initial_exp, 
		uint_16 exps[], uint_16 *dest);

uint_32
exponent_unpack( bsi_t *bsi, audblk_t *audblk)
{
	uint_16 i;
	uint_32 cbErrors = 0;

	for(i=0; i< bsi->nfchans; i++)
		cbErrors += exp_unpack_ch(UNPACK_FBW, audblk->chexpstr[i], 
			audblk->nchgrps[i], audblk->exps[i][0], &audblk->exps[i][1], 
			audblk->fbw_exp[i]);

	if(audblk->cplinu)
		cbErrors += exp_unpack_ch(UNPACK_CPL, audblk->cplexpstr, 
			audblk->ncplgrps, audblk->cplabsexp << 1, audblk->cplexps, 
			&audblk->cpl_exp[audblk->cplstrtmant]);

	if(bsi->lfeon)
		cbErrors += exp_unpack_ch(UNPACK_LFE, audblk->lfeexpstr, 2, 
			audblk->lfeexps[0], &audblk->lfeexps[1], audblk->lfe_exp);

	return cbErrors; /* return # unpacking errors */
}


static uint_32
exp_unpack_ch(uint_16 type,uint_16 expstr,uint_16 ngrps,uint_16 initial_exp, 
		uint_16 exps[], uint_16 *dest)
{
	uint_16 i,j;
	sint_16 exp_acc;
	sint_16 exp_1,exp_2,exp_3;
	uint_32 cbErrors = 0; /* error counter */

	if(expstr == EXP_REUSE)
		return cbErrors;

	/* Handle the initial absolute exponent */
	exp_acc = initial_exp;
	j = 0;

	/* In the case of a fbw channel then the initial absolute values is 
	 * also an exponent */
	if(type != UNPACK_CPL)
		dest[j++] = exp_acc;

	/* Loop through the groups and fill the dest array appropriately */
	for(i=0; i< ngrps; i++)
	{
		if(exps[i] > 124)
		{
			++cbErrors;
			continue; // do not update current exponent 
			//exit(1);
		}

		exp_1 = exps[i] / 25;
		exp_2 = (exps[i] - (exp_1 * 25)) / 5;
		exp_3 = exps[i] - (exp_1 * 25) - (exp_2 * 5) ;

		exp_acc += (exp_1 - 2);

		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}

		exp_acc += (exp_2 - 2);

		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}

		exp_acc += (exp_3 - 2);

		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}
	}	

	return cbErrors; /* return # exponent unpacking errors */
}

