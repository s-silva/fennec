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
* File Name: mantissa.c							
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
#include "bitstream.h"
#include "mantissa.h"
#include "dither.h"

//
//Lookup tables of 0.15 two's complement quantization values
//
static const uint_16 q_1[3] = 
{
	( -2 << 15)/3, 0,(  2 << 15)/3 
};

static const uint_16 q_2[5] = 
{
	( -4 << 15)/5,( -2 << 15)/5, 0,
	(  2 << 15)/5,(  4 << 15)/5
};

static const uint_16 q_3[7] = 
{
	( -6 << 15)/7,( -4 << 15)/7,( -2 << 15)/7, 0,
	(  2 << 15)/7,(  4 << 15)/7,(  6 << 15)/7
};

static const uint_16 q_4[11] = 
{
	(-10 << 15)/11,(-8 << 15)/11,(-6 << 15)/11, ( -4 << 15)/11,(-2 << 15)/11,  0,
	(  2 << 15)/11,( 4 << 15)/11,( 6 << 15)/11, (  8 << 15)/11,(10 << 15)/11
};

static const uint_16 q_5[15] = 
{
	(-14 << 15)/15,(-12 << 15)/15,(-10 << 15)/15,
	( -8 << 15)/15,( -6 << 15)/15,( -4 << 15)/15,
	( -2 << 15)/15,   0          ,(  2 << 15)/15,
	(  4 << 15)/15,(  6 << 15)/15,(  8 << 15)/15,
	( 10 << 15)/15,( 12 << 15)/15,( 14 << 15)/15
};

//
// Scale factors for convert_to_float
//

static const uint_32 u32_scale_factors[25] = 
{
	0x38000000, //2 ^ -(0 + 15)
	0x37800000, //2 ^ -(1 + 15)
	0x37000000, //2 ^ -(2 + 15)
	0x36800000, //2 ^ -(3 + 15)
	0x36000000, //2 ^ -(4 + 15)
	0x35800000, //2 ^ -(5 + 15)
	0x35000000, //2 ^ -(6 + 15)
	0x34800000, //2 ^ -(7 + 15)
	0x34000000, //2 ^ -(8 + 15)
	0x33800000, //2 ^ -(9 + 15)
	0x33000000, //2 ^ -(10 + 15)
	0x32800000, //2 ^ -(11 + 15)
	0x32000000, //2 ^ -(12 + 15)
	0x31800000, //2 ^ -(13 + 15)
	0x31000000, //2 ^ -(14 + 15)
	0x30800000, //2 ^ -(15 + 15)
	0x30000000, //2 ^ -(16 + 15)
	0x2f800000, //2 ^ -(17 + 15)
	0x2f000000, //2 ^ -(18 + 15)
	0x2e800000, //2 ^ -(19 + 15)
	0x2e000000, //2 ^ -(20 + 15)
	0x2d800000, //2 ^ -(21 + 15)
	0x2d000000, //2 ^ -(22 + 15)
	0x2c800000, //2 ^ -(23 + 15)
	0x2c000000  //2 ^ -(24 + 15)
};

static float *scale_factor = (float*)u32_scale_factors;



//These store the persistent state of the packed mantissas
static uint_16 m_1[3];
static uint_16 m_2[3];
static uint_16 m_4[2];
static uint_16 m_1_pointer;
static uint_16 m_2_pointer;
static uint_16 m_4_pointer;

//uint_16 . fast manitsa lookups instead of calculating it
static uint_16	fastM = 0;
static uint_16	fastM_9[32][3];
static uint_16	fastM_25[128][3];
static uint_16	fastM_11[128][3];


//Conversion from bap to number of bits in the mantissas
//zeros account for cases 0,1,2,4 which are special cased
static uint_16 qnttztab[16] = { 0, 0, 0, 3, 0 , 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16};

static void mantissa_reset(void);
static uint_16 mantissa_get(bitstream_t *bs, uint_16 bap, int do_dither );

static uint_32 cbErrors = 0; /* error counter */

static float convert_to_float(uint_16 exp, sint_16 mantissa)
{
	float x;

	//the scale by 2^-15 is built into the scale factor table
	x = mantissa * scale_factor[exp];

	return x;
}

static void mantissa_reset(void)
{
	m_1[2] = m_1[1] = m_1[0] = 0;
	m_2[2] = m_2[1] = m_2[0] = 0;
	m_4[1] = m_4[0] = 0;
	m_1_pointer = m_2_pointer = m_4_pointer = 3;
}

static void	makeFastMantissaLookups( void )
{
	long i;
	
	for( i=0; i<1<<5; i++){
		fastM_9[i][0] = i / 9;
		fastM_9[i][1] = (i % 9) / 3;
		fastM_9[i][2] = (i % 9) % 3;
	}
	for( i=0; i<1<<7; i++){
		fastM_25[i][0] = i / 25;
		fastM_25[i][1] = (i % 25) / 5;
		fastM_25[i][2] = (i % 25) % 5;
	}
	for( i=0; i<1<<7; i++){
		fastM_11[i][0] = i / 11;
		fastM_11[i][1] = i % 11;
	}
}

uint_32 mantissa_unpack(bsi_t *bsi, audblk_t *audblk,bitstream_t *bs)
{
	uint_16 i,j;
	uint_32 done_cpl = 0;
	register uint_16 *mant_ptr, *bap_ptr;
	int do_dither = 0;

	mantissa_reset();

	if ( !fastM )
	{
		makeFastMantissaLookups();
		fastM = 1;
	}
	//FIXME remove
	//start = bs->total_bits_read;

	for(i=0; i< bsi->nfchans; i++){
		mant_ptr = &audblk->chmant[i][0];
		bap_ptr = &audblk->fbw_bap[i][0];
		do_dither = audblk->dithflag[i];

		for(j=0; j < audblk->endmant[i]; j++){
			*mant_ptr++ = mantissa_get( bs, *bap_ptr++, do_dither );
		}

		if(audblk->cplinu && audblk->chincpl[i] && !(done_cpl))
		{
			j=audblk->cplstrtmant;
			mant_ptr = &audblk->cplmant[j];
			bap_ptr = &audblk->cpl_bap[j];

			/* ncplmant is equal to 12 * ncplsubnd */
			for(j=audblk->cplstrtmant; j < audblk->cplendmant; j++){
				*mant_ptr++ = mantissa_get( bs, *bap_ptr++, do_dither );
			}

			done_cpl = 1;
		}
	}

	if(bsi->lfeon){
		mant_ptr = &audblk->lfemant[0];
		bap_ptr = &audblk->lfe_bap[0];

		/* There are always 7 mantissas for lfe */

		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		*mant_ptr++ = mantissa_get(bs, *bap_ptr++, do_dither );
		
	}

	return cbErrors; /* return # errors */
}



static uint_16 mantissa_get(bitstream_t *bs, uint_16 bap, int do_dither )
{
	uint_16 result, index;
	uint_16 group_code;
	static uint_16 *fast_m_1, *fast_m_2, *fast_m_4;

	//If the bap is 0-5 then we have special cases to take care of
	switch(bap)
	{
		case 0:
			//FIXME change to respect the dither flag
			if ( do_dither )
				result = dither_gen();
			else
				result = 0;
			break;

		case 1:
			if(m_1_pointer > 2)
			{
				group_code = bitstream_get(bs,5);

				fast_m_1 = &fastM_9[group_code][0];
				m_1_pointer = 0;
			}
			index = fast_m_1[m_1_pointer++];
			result = q_1[index];
			break;
		case 2:
			if(m_2_pointer > 2)
			{
				group_code = bitstream_get(bs,7);

				fast_m_2 = &fastM_25[group_code][0];
				m_2_pointer = 0;
			}
			index = fast_m_2[m_2_pointer++];
			result = q_2[index];
			break;

		case 3:
			result = bitstream_get(bs,3);

			result = q_3[result];
			break;

		case 4:
			if(m_4_pointer > 1)
			{
				group_code = bitstream_get(bs,7);

				fast_m_4 = &fastM_11[group_code][0];
				m_4_pointer = 0;
			}
			index = fast_m_4[m_4_pointer++];
			result = q_4[index];
			break;

		case 5:
			result = bitstream_get(bs,4);

			result = q_5[result];
			break;

		default:
			result = bitstream_get(bs,qnttztab[bap]);
			result <<= 16 - qnttztab[bap];
	}

	return result;
}


static float cpl_tmp[256];


void 
uncouple_channel(stream_coeffs_t *coeffs,bsi_t *pbsi, audblk_t *audblk, uint_32 ch )
{
	uint_32 bnd = 0;
	uint_32 i;
	float cpl_coord;
	uint_32 cpl_exp_tmp;
	uint_32 cpl_mant_tmp;

	for(i=audblk->cplstrtmant;i<audblk->cplendmant;i+=12)
	{
		if(!audblk->cplbndstrc[bnd])
		{
			cpl_exp_tmp = audblk->cplcoexp[ch][bnd] + 3 * audblk->mstrcplco[ch];
			if(audblk->cplcoexp[ch][bnd] == 15)
				cpl_mant_tmp = (audblk->cplcomant[ch][bnd]) << 12;
			else
				cpl_mant_tmp = ((0x10) | audblk->cplcomant[ch][bnd]) << 11;
			
			cpl_coord = convert_to_float(cpl_exp_tmp,cpl_mant_tmp);
		}


		/* If in 2.0 (stereo) mode, check phase-invert flags */
		if ( pbsi->acmod == 0x02 && ch == 1 && audblk->phsflginu && 
			audblk->phsflg[ bnd ] )
		{
			coeffs->fbw[ch][i]   = - cpl_coord * cpl_tmp[i];
			coeffs->fbw[ch][i+1] = - cpl_coord * cpl_tmp[i+1];
			coeffs->fbw[ch][i+2] = - cpl_coord * cpl_tmp[i+2];
			coeffs->fbw[ch][i+3] = - cpl_coord * cpl_tmp[i+3];
			coeffs->fbw[ch][i+4] = - cpl_coord * cpl_tmp[i+4];
			coeffs->fbw[ch][i+5] = - cpl_coord * cpl_tmp[i+5];
			coeffs->fbw[ch][i+6] = - cpl_coord * cpl_tmp[i+6];
			coeffs->fbw[ch][i+7] = - cpl_coord * cpl_tmp[i+7];
			coeffs->fbw[ch][i+8] = - cpl_coord * cpl_tmp[i+8];
			coeffs->fbw[ch][i+9] = - cpl_coord * cpl_tmp[i+9];
			coeffs->fbw[ch][i+10]= - cpl_coord * cpl_tmp[i+10];
			coeffs->fbw[ch][i+11]= - cpl_coord * cpl_tmp[i+11];
		}
		else
		{	/* normal phase, phaseinvert not in use */
			coeffs->fbw[ch][i]   = cpl_coord * cpl_tmp[i];
			coeffs->fbw[ch][i+1] = cpl_coord * cpl_tmp[i+1];
			coeffs->fbw[ch][i+2] = cpl_coord * cpl_tmp[i+2];
			coeffs->fbw[ch][i+3] = cpl_coord * cpl_tmp[i+3];
			coeffs->fbw[ch][i+4] = cpl_coord * cpl_tmp[i+4];
			coeffs->fbw[ch][i+5] = cpl_coord * cpl_tmp[i+5];
			coeffs->fbw[ch][i+6] = cpl_coord * cpl_tmp[i+6];
			coeffs->fbw[ch][i+7] = cpl_coord * cpl_tmp[i+7];
			coeffs->fbw[ch][i+8] = cpl_coord * cpl_tmp[i+8];
			coeffs->fbw[ch][i+9] = cpl_coord * cpl_tmp[i+9];
			coeffs->fbw[ch][i+10]= cpl_coord * cpl_tmp[i+10];
			coeffs->fbw[ch][i+11]= cpl_coord * cpl_tmp[i+11];
		}


		bnd++;
	}
}

void uncouple(bsi_t *bsi,audblk_t *audblk,stream_coeffs_t *coeffs)
{
	int i,j;

	for(i=0; i< bsi->nfchans; i++)
	{
		for(j=0; j < audblk->endmant[i]; j++)
			 coeffs->fbw[i][j] = convert_to_float(audblk->fbw_exp[i][j],audblk->chmant[i][j] );
	}

	if(audblk->cplinu)
	{

		for(j=audblk->cplstrtmant; j < audblk->cplendmant; j++)
			 cpl_tmp[j] = convert_to_float(audblk->cpl_exp[j],audblk->cplmant[j] );

		for(i=0; i< bsi->nfchans; i++)
		{
			if(audblk->chincpl[i])
			{
				uncouple_channel(coeffs, bsi,audblk,i );
			}
		}

	}

	if(bsi->lfeon)
	{
		/* There are always 7 mantissas for lfe */
		for(j=0; j < 7 ; j++)
			 coeffs->lfe[j] = convert_to_float(audblk->lfe_exp[j],audblk->lfemant[j] );
	}
}

