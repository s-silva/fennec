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
* File Name: libac3dec.c							
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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <time.h>

#include "parse.h"
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"
#include "exponent.h"
#include "mantissa.h"
#include "bit_allocate.h"
#include "crc.h"
#include "libac3dec.h"
#include "rematrix.h"
#include "downmix.h"

const struct frmsize_s frmsizecode_tbl[] = {
	{ 32  ,{64   ,69   ,96   } },
	{ 32  ,{64   ,70   ,96   } },
	{ 40  ,{80   ,87   ,120  } },
	{ 40  ,{80   ,88   ,120  } },
	{ 48  ,{96   ,104  ,144  } },
	{ 48  ,{96   ,105  ,144  } },
	{ 56  ,{112  ,121  ,168  } },
	{ 56  ,{112  ,122  ,168  } },
	{ 64  ,{128  ,139  ,192  } },
	{ 64  ,{128  ,140  ,192  } },
	{ 80  ,{160  ,174  ,240  } },
	{ 80  ,{160  ,175  ,240  } },
	{ 96  ,{192  ,208  ,288  } },
	{ 96  ,{192  ,209  ,288  } },
	{ 112 ,{224  ,243  ,336  } },
	{ 112 ,{224  ,244  ,336  } },
	{ 128 ,{256  ,278  ,384  } },
	{ 128 ,{256  ,279  ,384  } },
	{ 160 ,{320  ,348  ,480  } },
	{ 160 ,{320  ,349  ,480  } },
	{ 192 ,{384  ,417  ,576  } },
	{ 192 ,{384  ,418  ,576  } },
	{ 224 ,{448  ,487  ,672  } },
	{ 224 ,{448  ,488  ,672  } },
	{ 256 ,{512  ,557  ,768  } },
	{ 256 ,{512  ,558  ,768  } },
	{ 320 ,{640  ,696  ,960  } },
	{ 320 ,{640  ,697  ,960  } },
	{ 384 ,{768  ,835  ,1152 } },
	{ 384 ,{768  ,836  ,1152 } },
	{ 448 ,{896  ,975  ,1344 } },
	{ 448 ,{896  ,976  ,1344 } },
	{ 512 ,{1024 ,1114 ,1536 } },
	{ 512 ,{1024 ,1115 ,1536 } },
	{ 576 ,{1152 ,1253 ,1728 } },
	{ 576 ,{1152 ,1254 ,1728 } },
	{ 640 ,{1280 ,1393 ,1920 } },
	{ 640 ,{1280 ,1394 ,1920 } }};

#define BUFFER_SIZE1 3072
#define DECODE_MAGIC_NUMBER 0xdeadbeef

//static void decode_find_sync(bitstream_t *bs);
static int decode_resync(bitstream_t *bs);

static stream_coeffs_t stream_coeffs;
static stream_samples_t stream_samples;
static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;

int m_gainlevel = 0;
int m_gain2level = 0;
int m_gaincenter = 0;
int m_gainrear = 0;
int m_gainlfe = 0;

int AC3_GetAudioInfo(uint_32* pnFrameSize, uint_32 *pnSampleRate,uint_8 *pAc3Header)
{
	uint_8 fscod;
	uint_8 frmsizecode;

	if(!(pAc3Header[0]==0x0B && pAc3Header[1]==0x77))
	{
		*pnSampleRate = 0;
		return 0;
	}
	fscod = (pAc3Header[4] & 0xC0) >> 6;
	switch (fscod)
	{
	case 2:
		*pnSampleRate = 32000;
		break;
	case 1:
		*pnSampleRate = 44100;
		break;
	case 0:
		*pnSampleRate = 48000;
		break;
	default:
		*pnSampleRate = 0;
		return 0;
	}
	frmsizecode = pAc3Header[4] & 0x3F;

	*pnFrameSize = frmsizecode_tbl[frmsizecode].frm_size[fscod];
	*pnFrameSize = *pnFrameSize * 2;

	return 1;
}

int AC3_SampleConvert(sint_16 *pPcmData, uint_32 *pnPcmDataLen, uint_8* pAc3Buf, uint_32 nAc3DataLen)
{
	int i;
	bitstream_t *bs;	
	long SampleRate = 0; /* bitstream sample-rate */
	uint_32 cbErrors = 0; /* bitstream error count returned by 							decode_sanity_check() */
	uint_32 cbMantErrors, cbExpErrors;		/* error counters for mantissa & exponent unpacking errors */
	bsi_t bsi_blank = {0};
    static audblk_t audblk_blank = {0};

	bs = bitstream_open(pAc3Buf, pAc3Buf+nAc3DataLen);

	imdct_init();
	/* initialize decoder and counter variables */

	(bsi) = bsi_blank;
	(audblk) = audblk_blank;
	decode_sanity_check_init();

	bsi_blank = bsi;
	audblk_blank = audblk;

	cbErrors = 0; 

	audblk = audblk_blank; /* clear out audioblock */

  	if(!(decode_resync(bs)))
	{
		return  0;
	}

	bsi = bsi_blank; /* v0.04 wipe bsi clear, not really necessary */

	parse_syncinfo(&syncinfo,bs);

	parse_bsi(&bsi,bs);

	switch (syncinfo.fscod)
	{
		case 2:
			SampleRate = 32000;
			break;
		case 1:
			SampleRate = 44100;
			break;
		case 0:
			SampleRate = 48000;
			break;
		default:
			return 0;
	}

		/* reset bitstream error counters */
	cbErrors =
	cbExpErrors =
	cbMantErrors = 0;
	
	memset(pPcmData, 0, BUFFER_SIZE1*2);
	*pnPcmDataLen = 0;
	for(i=0; i < 6; i++)
	{
		long buf_offset;
		parse_audblk(&bsi,&audblk,bs,i);					// CPU time 10%

		cbExpErrors = exponent_unpack(&bsi,&audblk);
		if(cbExpErrors > 0)
		{
			return 0;
			cbExpErrors =0;
		}

		bit_allocate(syncinfo.fscod,&bsi,&audblk);			// CPU TIME 1.2%

		if ( bsi.nfchans > 6 )
		{ 
			bsi.nfchans = 0; 
			return 0;//(LPBYTE) out_buf; 
		}

		cbMantErrors = mantissa_unpack(&bsi,&audblk,bs);	// CPU TIME 62.0%
		if( cbMantErrors > 0)
		{
			return 0;
            cbMantErrors = 0;
		}

		uncouple(&bsi,&audblk,&stream_coeffs);				// CPU TIME 1.7%

		if(bsi.acmod == 0x2)
			rematrix(&audblk,&stream_coeffs);				// CPU TIME 0.1%

		imdct(&bsi,&audblk,&stream_coeffs,&stream_samples);	// CPU TIME 11.2%

		buf_offset = i * 512;

		downmix( &stream_samples, pPcmData+buf_offset, &bsi, 0 );
	} /* endfor ( i = 0 ... ) */
	*pnPcmDataLen = 6*512*2;

	cbErrors = decode_sanity_check();
	if(cbErrors > 0)
	{
		return 0;
	}

	parse_auxdata(&syncinfo,bs);			// CPU TIME 2.0%
	if(!crc_validate())
	{
		return 0;
	}
	return 1;
}

static  int decode_resync(bitstream_t *bs) 
{
	uint_16 sync_word;
	//int i = 0;

	/* Make sure we sync'ed */
	sync_word = bitstream_get(bs,16);
	if(sync_word == 0x0b77 )
	{
		crc_init();
		return 1;
	}
	return 0;
}


void decode_sanity_check_init(void)
{
	syncinfo.magic = DECODE_MAGIC_NUMBER;
	bsi.magic = DECODE_MAGIC_NUMBER;
	audblk.magic1 = DECODE_MAGIC_NUMBER;
	audblk.magic2 = DECODE_MAGIC_NUMBER;
	audblk.magic3 = DECODE_MAGIC_NUMBER;
}


/* decode_sanity_check() now returns # errors detected */

long decode_sanity_check(void)
{
	int i;
	long cbError = 0; /* error count */

	if(syncinfo.magic != DECODE_MAGIC_NUMBER)
	{
		++cbError;
	}
	
	if(bsi.magic != DECODE_MAGIC_NUMBER)
	{
		++cbError;
	}

	if(audblk.magic1 != DECODE_MAGIC_NUMBER)
	{
		++cbError;
	}

	if(audblk.magic2 != DECODE_MAGIC_NUMBER)
	{
		++cbError;
	}

	if(audblk.magic3 != DECODE_MAGIC_NUMBER)
	{
		++cbError;
	}

	for(i = 0;i < 5 ; i++)
	{
		if (audblk.fbw_exp[i][255] !=0 || audblk.fbw_exp[i][254] !=0 || 
				audblk.fbw_exp[i][253] !=0)
		{
			audblk.fbw_exp[i][255] = 0;
			audblk.fbw_exp[i][254] = 0;
			audblk.fbw_exp[i][253] = 0;
			++cbError;
		}

		if (audblk.fbw_bap[i][255] !=0 || audblk.fbw_bap[i][254] !=0 || 
				audblk.fbw_bap[i][253] !=0)
		{	
			audblk.fbw_bap[i][255] = 0;
			audblk.fbw_bap[i][254] = 0;
			audblk.fbw_bap[i][253] = 0;
			++cbError;
		}

		if (audblk.chmant[i][255] !=0 || audblk.chmant[i][254] !=0 || 
				audblk.chmant[i][253] !=0)
		{
			audblk.chmant[i][255] = 0;
			audblk.chmant[i][254] = 0;
			audblk.chmant[i][253] = 0;
			++cbError;
		}
	} /* endfor ( i=0 ... ) */

	if (audblk.cpl_exp[255] !=0 || audblk.cpl_exp[254] !=0 || 
			audblk.cpl_exp[253] !=0)
	{
		audblk.cpl_exp[255] = 0;
		audblk.cpl_exp[254] = 0;
		audblk.cpl_exp[253] = 0;
		++cbError;
	}

	if (audblk.cpl_bap[255] !=0 || audblk.cpl_bap[254] !=0 || 
			audblk.cpl_bap[253] !=0)
	{
		audblk.cpl_bap[255] = 0;
		audblk.cpl_bap[254] = 0;
		audblk.cpl_bap[253] = 0;
		++cbError;
	}

	if (audblk.cplmant[255] !=0 || audblk.cplmant[254] !=0 || 
			audblk.cplmant[253] !=0)
	{
		audblk.cplmant[255] = 0;
		audblk.cplmant[254] = 0;
		audblk.cplmant[253] = 0;
		++cbError;
	}

	if ((audblk.cplinu == 1) && (audblk.cplbegf > (audblk.cplendf+2)))
	{
		++cbError;
	}

	for(i=0; i < bsi.nfchans; i++)
	{
		if((audblk.chincpl[i] == 0) && (audblk.chbwcod[i] > 60))
		{
			audblk.chbwcod[i] = 60;
			++cbError;
		}
	} /* endfor ( i = 0 ... ) */

	return cbError;
}	



