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
* File Name: downmix.c							
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
#include <stdio.h>
#include <math.h>

#include "ac3.h"
#include "decode.h"
#include "downmix.h"

const static float SQRT2_2 = 0.707106781; /* (SQRT(2))/2 */

/* these mixing gains are for the downmixing routines.  They do not directly
   correspond to elements in the AC3 bitstream */
static float lrmixgain;  /* front L/R channel mix gain */
static float lfemixgain; /* lfe channel mix gain */
static float cmixgain; /* center channel mix gain */
static float smixgain; /* surround channel mix gain */
static float slev,clev;

extern int m_gainlevel;
extern int m_gain2level;
extern int m_gaincenter;
extern int m_gainrear;
extern int m_gainlfe;

#define	GAINSCALE	32767.0


void conv480to441( sint_16 *out_samples, long samp_count, long chan )
{
	sint_16 *source, *dest, s1,s2,s3, lp;
	long	i, cl, tmp;
	double	ind=0, inc=48000/44100.0, h1, h2, f1, f2;

	source = dest = out_samples;
	for(i=0; i<samp_count ; i++){
		lp = ind;
		if ( lp>255 ) lp = 255;
		lp = lp*chan;

		h2 = modf( ind, (double*)&tmp );
		h1 = 1.0 - h2;

		for( cl=0; cl < chan; cl++){
			s1 = source[ lp+cl ];			// read samples
			s2 = source[ lp+cl+2 ];
			f1 = s1*h1;						// scale samples
			f2 = s2*h2;
			s3 = (f1+f2);					// average the samples
			if ( lp>=(255*2) )
				s3=s1;
			*out_samples++ = s3;			// store result
		}

		ind += inc;
	}
}


void PerformGlobalGain( sint_16 *out_samples )
{
	int i;
	short left,right, *samples = out_samples;
	double gainlevel, gain;


	if ( m_gainlevel<1 ) return;
	gainlevel = 1+ (m_gainlevel/100.0);

	for ( i=0; i < 256; ++i )
	{
			left = *samples++;
			right = *samples++;

			gain = (((32000-abs(left))/32000.0) * gainlevel);
			*out_samples++ = (sint_16) (left * gain);

			gain = (((32000-abs(right))/32000.0) * gainlevel);
			*out_samples++ = (sint_16) (right * gain);
	}
}

void PerformCurvedGlobalGain( sint_16 *out_samples )
{
	int i;
	short left,right, *samples = out_samples;
	double gainlevel, gain;


	if ( m_gain2level<1 ) return;
	gainlevel = 1+ (m_gain2level/100.0);

	for ( i=0; i < 256; ++i )
	{
			left = *samples++;
			right = *samples++;

			gain = (((32000-abs(left))/32000.0));
			gain = gain * gain * gainlevel;
			*out_samples++ = (sint_16) (left * gain);

			gain = (((32000-abs(right))/32000.0));
			gain = gain * gain * gainlevel;
			*out_samples++ = (sint_16) (right * gain);
	}
}



void PerformPreSampleGain( stream_samples_t *samples )
{
	int i;
	float gainlevel, gain;
	register float *sam, *sam3, *sam4, sample;

	if ( m_gaincenter>1 ){
		gainlevel = 1+ (m_gaincenter/100.0);
		if( m_gaincenter > 999 ) gainlevel = 0;

		sam = samples->channel[1];

		for ( i=0; i < 256; ++i )
		{
				sample = *sam;

				gain = (((1-fabs(sample))/1.0));
				*sam++ = sample * gain * gain * gainlevel;
		}
	}
	if ( m_gainrear>1 ){
		gainlevel = 1+ (m_gainrear/100.0);
		if( m_gainrear > 999 ) gainlevel = 0;

		sam3 = samples->channel[3];
		sam4 = samples->channel[4];

		for ( i=0; i < 256; ++i )
		{
				sample = *sam3;
				gain = (((1-fabs(sample))/1.0));
				*sam3++ = sample * gain * gain * gainlevel;
				sample = *sam4;
				gain = (((1-fabs(sample))/1.0));
				*sam4++ = sample * gain * gain * gainlevel;
		}
	}
	if ( m_gainlfe>1 ){
		gainlevel = 1+ (m_gainlfe/100.0);
		if( m_gainlfe > 999 ) gainlevel = 0;

		sam = samples->channel[5];

		for ( i=0; i < 256; ++i )
		{
				sample = *sam;

				gain = (((1-fabs(sample))/1.0));
				*sam++ = sample * gain * gain * gainlevel;
		}
	}
}


/* AC3 5.1 channel downmix, 3 front, 2 rear channels, LFE */
void
mix321to20( stream_samples_t *samples, sint_16 *out_samples, int full51output )
{
	int i;

	sint_32 left,	/* source-sample front left */
		center,	/* source-sample front center */
		right,  /* source-sample front right */
		rear,  /* source-sample (mono) rear left and right */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam3, *sam4, *sam5;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam3 = samples->channel[3];
	sam4 = samples->channel[4];
	sam5 = samples->channel[5];

	/* AC3 5.1 channel downmix, 3 front, 2 rear channels */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		center= *sam1++ * cmixgain;
		right = *sam2++ * lrmixgain;
		rear = (*sam3++ + *sam4++) * smixgain;
		lfe = *sam5++ * lfemixgain;

		Acc1 = ( 2 + left + center + lfe - rear ) >> 2;
		Acc2 = ( 2 + right+ center + lfe + rear ) >> 2;

		if( full51output ){
			*out_samples++ = left;
			*out_samples++ = right;
			*out_samples++ = center;
			*out_samples++ = rear;
			*out_samples++ = lfe;
		} else {
			*out_samples++ = Acc1;
			*out_samples++ = Acc2;
		}
	}
}

/* AC3 5.0 channel downmix, 3 front, 2 rear channels */
void mix320to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;
	sint_32 left,	/* source-sample front left */
		center,	/* source-sample front center */
		right,  /* source-sample front right */
		rear;  /* source-sample (mono) rear left and right */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam3, *sam4;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam3 = samples->channel[3];
	sam4 = samples->channel[4];

	/* AC3 5.0 channel downmix, 3 front, 2 rear channels */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		center= *sam1++ * cmixgain;
		right = *sam2++ * lrmixgain;
		rear = (*sam3++ + *sam4++) * smixgain;

		Acc1 = ( 2 + left + center - rear ) >> 2;
		Acc2 = ( 2 + right+ center + rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 2/2 channel downmix, 2 front (left/right), 2 rear channels */
void
mix220to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		right,  /* source-sample front right */
		rear;  /* source-sample (mono) rear left and right */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam3;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam3 = samples->channel[3];

	/* AC3 2/2 channel downmix, 2 front, 2 rear channels */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		right = *sam1++ * lrmixgain;
		rear = (*sam2++ + *sam3++) * smixgain;

		Acc1 = ( 2 + left - rear ) >> 2;
		Acc2 = ( 2 + right+ rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 2/2.1 channel downmix, 2 front (left/right), 2 rear channels, lfe */
void
mix221to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		right,  /* source-sample front right */
		rear,  /* source-sample (mono) rear left and right */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam3, *sam5;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam3 = samples->channel[3];
	sam5 = samples->channel[5];

	/* AC3 2/2.1 channel downmix, 2 front, 2 rear channels, lfe */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		right = *sam1++ * lrmixgain;
		rear = (*sam2++ + *sam3++) * smixgain;
		lfe = *sam5++ * lfemixgain;

		Acc1 = ( 2 + left + lfe - rear ) >> 2;
		Acc2 = ( 2 + right+ lfe + rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 3/1.1 channel downmix, 3 front, 1 rear channel, LFE */
void
mix311to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		center,	/* source-sample front center */
		right,  /* source-sample front right */
		rear,  /* source-sample (mono) rear */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam3, *sam5;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam3 = samples->channel[3];
	sam5 = samples->channel[5];

	/* AC3 3/1.1 channel downmix, 3 front, 1 rear channel, lfe */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		center= *sam1++ * cmixgain;
		right = *sam2++ * lrmixgain;
		rear = *sam3++ * smixgain * SQRT2_2;
		lfe = *sam5++ * lfemixgain;

		Acc1 = ( 2 + left + center + lfe - rear ) >> 2;
		Acc2 = ( 2 + right+ center + lfe + rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}



/* AC3 3/1.0 channel downmix, 3 front, 1 rear channel */
void
mix310to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		center,	/* source-sample front center */
		right,  /* source-sample front right */
		rear;  /* source-sample (mono) rear */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam3;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam3 = samples->channel[3];

	/* AC3 3/1.1 channel downmix, 3 front, 1 rear channel, lfe */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		center= *sam1++ * cmixgain;
		right = *sam2++ * lrmixgain;
		rear = *sam3++ * smixgain * SQRT2_2;

		Acc1 = ( 2 + left + center - rear ) >> 2;
		Acc2 = ( 2 + right+ center + rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 3/0.1 channel downmix, 3 front, LFE */
void
mix301to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		center,	/* source-sample front center */
		right,  /* source-sample front right */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam5;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam5 = samples->channel[5];

	/* AC3 3/0.1 channel downmix, 3 front, lfe */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		center= *sam1++ * cmixgain;
		right = *sam2++ * lrmixgain;
		lfe = *sam5++ * lfemixgain;

		Acc1 = ( 2 + left + center + lfe ) >> 2;
		Acc2 = ( 2 + right+ center + lfe ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 3/0.0 channel downmix, 3 front */
void
mix300to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		center,	/* source-sample front center */
		right;  /* source-sample front right */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];

	/* AC3 3/0.0 channel downmix, 3 front */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		center= *sam1++ * cmixgain;
		right = *sam2++ * lrmixgain;

		Acc1 = ( 2 + left + center ) >> 2;
		Acc2 = ( 2 + right+ center ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 2/1.1 channel downmix, 2 front, 1 rear channel, LFE */
void
mix211to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		right,  /* source-sample front right */
		rear,  /* source-sample (mono) rear */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2, *sam5;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];
	sam5 = samples->channel[5];

	/* AC3 2/1.1 channel downmix, 2 front, 1 rear channel, lfe */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		right = *sam1++ * lrmixgain;
		rear = *sam2++ * smixgain * SQRT2_2;
		lfe = *sam5++ * lfemixgain;

		Acc1 = ( 2 + left + lfe - rear ) >> 2;
		Acc2 = ( 2 + right+ lfe + rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}



/* AC3 2/1.0 channel downmix, 2 front, 1 rear channel */
void mix210to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 left,	/* source-sample front left */
		right,  /* source-sample front right */
		rear;  /* source-sample (mono) rear */

	register sint_16 Acc1, Acc2; /* accumulators */
	register float *sam0, *sam1, *sam2;


	sam0 = samples->channel[0];
	sam1 = samples->channel[1];
	sam2 = samples->channel[2];

	/* AC3 2/1.1 channel downmix, 2 front, 1 rear channel */
	for ( i=0; i < 256; ++i )
	{
		left = *sam0++ * lrmixgain;
		right = *sam1++ * lrmixgain;
		rear = *sam2++ * smixgain * SQRT2_2;

		Acc1 = ( 2 + left  - rear ) >> 2;
		Acc2 = ( 2 + right + rear ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;
	}
}


/* AC3 2 channel downmix, LFE */
void mix201to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;
	register float *lsam, *rsam, *lfesam;
	register sint_32 left,	/* source-sample front left */
		right,  /* source-sample front right */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1, Acc2; /* accumulators */

	lsam = samples->channel[0];
	rsam = samples->channel[1];
	lfesam = samples->channel[5];

	/* AC3 2/0.1 or 1/1.1 channel downmix, 2 front, lfe */
	for ( i=0; i < 256; ++i )
	{
		left = *lsam++ * lrmixgain;
		right = *rsam++ * lrmixgain;
		lfe = *lfesam++ * lfemixgain;

		lfe += 2;

		Acc1 = ( left + lfe ) >> 2;
		Acc2 = ( right+ lfe ) >> 2;

		*out_samples++ = Acc1;
		*out_samples++ = Acc2;

	}
}

void mix200to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;
	register float *lsam, *rsam;

	lsam = samples->channel[0];
	rsam = samples->channel[1];

	/* AC3 2/0.0 or 1/1.0 channel downmix, 2 front */
	for ( i=0; i < 256; ++i ){
		*out_samples++ = (sint_16) (*lsam++ * lrmixgain);		//left = samples->channel[0][i] * lrmixgain;
		*out_samples++ = (sint_16) (*rsam++ * lrmixgain);		//right = samples->channel[1][i] * lrmixgain;
	}
}



/* AC3 1 channel downmix */
void
mix101to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_32 center,	/* front center */
		lfe;	/* low frequency effects channel */

	sint_16 Acc1; /* accumulators */
	register float *sam0, *sam5;


	sam0 = samples->channel[0];
	sam5 = samples->channel[5];

	/* AC3 1/0.1 channel downmix, 2 front, lfe */
	for ( i=0; i < 256; ++i )
	{
		center = *sam0++ * cmixgain;
		lfe = *sam5++ * lfemixgain;

		Acc1 = ( 2 + center+ lfe ) >> 2;
		*out_samples++ = Acc1;
		*out_samples++ = Acc1;
	}
}


/* AC3 1 channel downmix */
void
mix100to20( stream_samples_t *samples, sint_16 *out_samples )
{
	int i;

	sint_16 center;	/* front center */
	register float *sam0;

	sam0 = samples->channel[0];
	/* AC3 1/0.0 channel downmix, 1 front */
	for ( i=0; i < 256; ++i )
	{
		center = *sam0++ * cmixgain;
		*out_samples++ = center;
		*out_samples++ = center;
	}
}




//Pre-scaled downmix coefficients
static float cmixlev_lut[4] = { 0.2928, 0.2468, 0.2071, 0.2468 };
static float smixlev_lut[4] = { 0.2928, 0.2071, 0.0   , 0.2071 };



/* master downmix function */
void
downmix( stream_samples_t *samples, sint_16 *out_samples, bsi_t *bsi, int keep51output )
{
	/* process the bsi mixlevel directives.  These directives influence
	   the decoder. */

	clev = cmixlev_lut[bsi->cmixlev];
	slev = smixlev_lut[bsi->surmixlev];
	
	
	switch( bsi->cmixlev )
	{
		case 0 : // -3.0dB
			cmixgain = SQRT2_2;
			break;
		case 1 : // -4.5dB
			cmixgain = 0.595662143;
			break;
		case 2 : // -6.0dB
			cmixgain = 0.5;
			break;
		case 3 : // reserved, use -4.5dB
			cmixgain = 0.595662143;
			break;
		default :
			fprintf( stderr, "\ndownmix.c : unknown bsi.cmixlev ( %u )", bsi->cmixlev );
	}

	switch( bsi->surmixlev )
	{
		case 0 : // -3.0dB
			smixgain = SQRT2_2;
			break;
		case 1 : // -6.0dB
			smixgain = 0.5;
			break;
		case 2 : // +0.0 dB
			smixgain = 1;
			break;
		case 3 : // reserved, use +0.0 dB
			smixgain = 1;
			break;
		default :
			fprintf( stderr, "\ndownmix.c : unknown bsi.surmixlev ( %u )", bsi->surmixlev );
	}

	PerformPreSampleGain( samples );

	lfemixgain = 3.16227766; /* +10dB */
	lrmixgain = 1.0; /* +0.0dB */

	lrmixgain *= GAINSCALE;
	cmixgain *= GAINSCALE;
	smixgain *= GAINSCALE;
	lfemixgain *= SQRT2_2 * GAINSCALE;

	if ( bsi->lfeon )
	{	// lfe on, "X.1"
		//printf( "Doing bsi->acmod=%d\n", bsi->acmod );
		switch ( bsi->acmod )
		{
			case 0:  // 1 + 1, dual-mono, lfe
				lrmixgain = cmixgain * GAINSCALE;
				mix201to20( samples, out_samples );
				break;
			case 1:  // 1, mono, lfe
				cmixgain *= SQRT2_2 * GAINSCALE;
				mix101to20( samples, out_samples );
				break;
			case 2:  // 2-channel stereo, lfe
				mix201to20( samples, out_samples );
				break;
			case 3:  // 3-front, lfe
				mix301to20( samples, out_samples );
				break;
			case 4:  // 2-front, 1-rear, lfe
				mix211to20( samples, out_samples );
				break;
			case 5:  // 3-front, 1-rear, lfe
				mix311to20( samples, out_samples );
				break;
			case 6:  // 2-front, 2-rear, lfe
				mix221to20( samples, out_samples );
				break;
			case 7:  // 3-front, 2-rear, lfe
				mix321to20( samples, out_samples, keep51output );
				//downmix_3f_2r_to_2ch( samples, out_samples );
				break;
			default :
			;
		}	// endswitch ( bsi->acmod )
	} // endif ( bsi->lfeon )
	else
	{ // lfe off, "X.0" 
		//printf( "Doing bsi->acmod=%d\n", bsi->acmod );
		switch ( bsi->acmod )
		{
			case 0:  // 1 + 1, dual-mono
				lrmixgain = cmixgain * GAINSCALE;
				mix200to20( samples, out_samples );
				break;
			case 1:  // 1, mono,
				cmixgain *= SQRT2_2 * GAINSCALE;
				mix100to20( samples, out_samples );
				break;
			case 2:  // 2-channel stereo,
				mix200to20( samples, out_samples );
				break;
			case 3:  // 3-front
				mix300to20( samples, out_samples );
				break;
			case 4:  // 2-front, 1-rear
				mix210to20( samples, out_samples );
				break;
			case 5:  // 3-front, 1-rear
				mix310to20( samples, out_samples );
				break;
			case 6:  // 2-front, 2-rear
				mix220to20( samples, out_samples );
				break;
			case 7:  // 3-front, 2-rear
				mix320to20( samples, out_samples );
				break;
			default :
				;
		}	// endswitch ( bsi->acmod )
	} // endif ( bsi->lfeon )

	// only gain on 2.0 output
	if ( !keep51output ){
		PerformGlobalGain( out_samples );
		PerformCurvedGlobalGain( out_samples );
	}
}

