/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
 Copyright (C) 2007 Chase <c-h@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/

#include <math.h>
#include "fennec main.h"
#include "fennec audio.h"


#define equalizer_max_bands 32
#define max_channels        16

struct equalizer_state
{
	unsigned int  nbands;
	double        preamp[max_channels];
	double        rpreamp[max_channels];
	int           frequency;


	struct equalizer_band_state
	{
		int     center_frequency;
		double  Q;

		struct equalizer_channel
		{
			double  a0, a1, a2, b0, b1, b2;	
			double  boost;
			double  x[3];
			double  y[3];

		}c[max_channels];

	}b[equalizer_max_bands];
};


struct equalizer_state   eq_state;




/* interface ----------------------------------------------------------------*/


int equalizer_calculate(int i, int c, int regen, struct equalizer_state *eqs)
{
	/*
	double  A, w0, alpha, pi = 4.0 * atan(1.0), cs;

	A     = exp(log(10.0) * eqs->b[i].c[c].boost / 40.0);

	w0    = 2.0 * pi * (double)eqs->b[i].center_frequency / (double)eqs->frequency;
	
	alpha = sin(w0) / (2.0 * eqs->b[i].Q);
	cs    = cos(w0);


	eqs->b[i].c[c].b0 =   1.0 + alpha * A;
    eqs->b[i].c[c].b1 =  -2.0 * cs;
    eqs->b[i].c[c].b2 =   1.0 - alpha * A;
    eqs->b[i].c[c].a0 =   1.0 + alpha / A;
    eqs->b[i].c[c].a1 =  -2.0 * cs;
    eqs->b[i].c[c].a2 =   1.0 - alpha / A;
	*/

	if(regen)
	{
		double pi    = 4.0 * atan(1.0);
		double theta = 2 * (double)eqs->b[i].center_frequency * pi / eqs->frequency;
		double Q     = eqs->b[i].Q;


		eqs->b[i].c[c].a1 = (Q - theta * 0.5) / (2.0 * Q + theta);
		eqs->b[i].c[c].a0 = (0.5 - eqs->b[i].c[c].a1) / 2.0;
		eqs->b[i].c[c].a2 = (0.5 + eqs->b[i].c[c].a1) * cos (theta);

		eqs->b[i].c[c].a1 *= 2;
		eqs->b[i].c[c].a0 *= 2;
		eqs->b[i].c[c].a2 *= 2;
	}

	eqs->b[i].c[c].b0 = exp(log(10.0) * eqs->b[i].c[c].boost / 20.0);

	eqs->rpreamp[i] = exp(log(10.0) * eqs->preamp[i] / 20.0);

	return 0;
}


int equalizer_initialize(int frequency)
{

	unsigned long i, j, c;
	int           cfs[] = {31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};

	if(frequency == -1)
	{
		for(i=0; i<max_channels; i++)
			eq_state.preamp[i] = settings.player.equalizer_last.preamp[i];
		

		for(i=0; i<max_channels; i++)
			for(j=0; j<equalizer_max_bands; j++)
			{
				eq_state.b[j].c[i].boost = settings.player.equalizer_last.boost[i][j];
			}
	
		return 0;
	}


	if(frequency)
		eq_state.frequency = frequency;


	/* set defualt values */


	eq_state.nbands = 10;

	for(i=0; i<eq_state.nbands; i++)
	{
		eq_state.b[i].center_frequency = cfs[i];
		eq_state.b[i].Q                = 1.2;
	}

	/* remove invalid bands */

	j = eq_state.nbands;

	for(i=0; i<eq_state.nbands; i++)
	{
		if(eq_state.b[i].center_frequency >= (eq_state.frequency / 2) )
			j--;
	}

	eq_state.nbands = j;

	/* calculate coefficients */


	for(c=0; c<max_channels; c++)
	{
		for(i=0; i<eq_state.nbands; i++)
		{
			equalizer_calculate(i, c, 1, &eq_state);
		}
	}

	/* clear history */

	if(frequency)
	{
		for(i=0; i<equalizer_max_bands; i++)
		{
			for(j=0; j<max_channels; j++)
			{
				eq_state.b[i].c[j].x[0] = 0;
				eq_state.b[i].c[j].x[1] = 0;
				eq_state.b[i].c[j].x[2] = 0;

				eq_state.b[i].c[j].y[0] = 0;
				eq_state.b[i].c[j].y[1] = 0;
				eq_state.b[i].c[j].y[2] = 0;
			}
		}
	}
	return 0;
}

int equalize_buffer(fennec_sample *inout, const fennec_sample *in, int channels, int frequency, int bps, unsigned long datalength, struct equalizer_state *eqs)
{
	unsigned long  i, j, c = datalength / sizeof(fennec_sample), ci;
	double         x, y = 0.0, v;


	if(!in) in = inout;

	if(eqs->frequency != frequency)
		equalizer_initialize(frequency);


	for(i=0; i<c; i++)
	{
		ci = i % channels;

		x  = in[i];

		x *= eqs->rpreamp[ci];

		v = 0;

		

		for(j=0; j<eqs->nbands; j++)
		{
			/*
			y =   (eqs->b[j].c[ci].b0 / eqs->b[j].c[ci].a0) * x
				+ (eqs->b[j].c[ci].b1 / eqs->b[j].c[ci].a0) * eqs->b[j].c[ci].x[0]
				+ (eqs->b[j].c[ci].b2 / eqs->b[j].c[ci].a0) * eqs->b[j].c[ci].x[1]
				- (eqs->b[j].c[ci].a1 / eqs->b[j].c[ci].a0) * eqs->b[j].c[ci].y[0]
				- (eqs->b[j].c[ci].a2 / eqs->b[j].c[ci].a0) * eqs->b[j].c[ci].y[1];
		
			eqs->b[j].c[ci].x[1] = eqs->b[j].c[ci].x[0];
			eqs->b[j].c[ci].x[0] = x;

			eqs->b[j].c[ci].y[1] = eqs->b[j].c[ci].y[0];
			eqs->b[j].c[ci].y[0] = y;
			*/

			/* per-band recursion:
			 * 	y = 2 * (a * (x - x[-2]) + c * y[-1] - b * y[-2]) 
			 */

			y =     (eqs->b[j].c[ci].a0 * (x - eqs->b[j].c[ci].x[1])
				   + eqs->b[j].c[ci].a2 * eqs->b[j].c[ci].y[0]
				   - eqs->b[j].c[ci].a1 * eqs->b[j].c[ci].y[1]);

			v += eqs->b[j].c[ci].b0 * y;


			eqs->b[j].c[ci].x[1] = eqs->b[j].c[ci].x[0];
			eqs->b[j].c[ci].x[0] = x;

			eqs->b[j].c[ci].y[1] = eqs->b[j].c[ci].y[0];
			eqs->b[j].c[ci].y[0] = y;
		}

		y = v;

		inout[i] = y * 0.5;


		if     (inout[i] >  1.0) inout[i] =  1.0;
		else if(inout[i] < -1.0) inout[i] = -1.0;	
	}

	return 0;
}

/*
 * bands[channels][bands]
 * preamps[channels]
 */
void* equalize_buffer_variable_init(equalizer_preset *eqp, int channels, int frequency)
{
	int                     cfs[] = {31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
	unsigned int            i, j;
	struct equalizer_state *eqs;

	eqs = (struct equalizer_state *) sys_mem_alloc(sizeof(struct equalizer_state));

	if(!eqs)return 0;
	
	memset(eqs, 0, sizeof(struct equalizer_state));

	eqs->frequency = frequency;
	eqs->nbands    = 10;
	
	for(i=0; i<(unsigned int)channels; i++)
		eqs->preamp[i] = (double)eqp->preamp[i];

	for(i=0; i<eqs->nbands; i++)
	{
		eqs->b[i].center_frequency = cfs[i];
		eqs->b[i].Q                = 1.2;
	}	


	for(i=0; i<(unsigned int)channels; i++)
		for(j=0; j<eqs->nbands; j++)
		{
			eqs->b[j].c[i].boost = (double)eqp->boost[i][j];
			equalizer_calculate(j, i, 1, eqs);
		}

	return eqs;
}

int equalize_buffer_variable_uninit(void *eqd)
{
	sys_mem_free(eqd);
	return 1;
}


int equalize_buffer_variable(fennec_sample *inout, const fennec_sample *in, int channels, int frequency, int bps, unsigned long datalength, void *eqd)
{
	return equalize_buffer(inout, in, channels, frequency, bps, datalength, (struct equalizer_state*)eqd);
}



/*
 * set bands, values higher than 10,000f will be ignored.
 */
int   equalizer_set_bands(int channel, int bands, float *values)
{
	register int i, j;

	if(channel == -1)
	{
		for(j=0; j<16; j++)
		{
			for(i=0; i<bands; i++)
			{
				if(values[i] < 10000.0f)
				{
					eq_state.b[i].c[0].boost = values[i];
					equalizer_calculate(i, 0, 0, &eq_state);

					if(bands <= equalizer_max_bands)
						settings.player.equalizer_last.boost[0][i] = values[i];
				}
			}
		}

	}else{
		for(i=0; i<bands; i++)
		{
			if(values[i] < 10000.0f)
			{
				eq_state.b[i].c[channel].boost = values[i];
				equalizer_calculate(i, channel, 0, &eq_state);

				if(bands <= equalizer_max_bands)
					settings.player.equalizer_last.boost[channel][i] = values[i];
			}
		}
	}

	return 0;
}


/*
 * get bands.
 */
int   equalizer_get_bands(int channel, int bands, float *values)
{
	register int i;

	for(i=0; i<bands; i++)
	{
		if(channel == -1) channel = 0;
		values[i] = (float)settings.player.equalizer_last.boost[channel][i];
	}
	
	return 0;
}


/*
 * set preamp values.
 */
int   equalizer_set_preamp(int channel, float value)
{
	eq_state.preamp[channel] = (double)value;

	eq_state.rpreamp[channel] = exp(log(10.0) * eq_state.preamp[channel] / 20.0);

	if(channel < max_channels)
		settings.player.equalizer_last.preamp[channel] = value;

	return 0;
}


/*
 * get preamp values.
 */
float equalizer_get_preamp(int channel)
{
	return (float)eq_state.preamp[channel];
}


/*
 * reset equalizer (new stream).
 */
int   equalizer_reset(void)
{
	/* automatically resets */
	return 0;
}


/*
 * turn/on off current equalizer.
 * set state = 3 to retrieve current state.
 */
int   equalizer_switch_current(int state)
{
	if(state == 3) return settings.player.equalizer_enable;

	settings.player.equalizer_enable = state ? 1 : 0;
	return 0;
}


/*
 * equalize sample buffer.
 * (zero inbuf to equalize outbuf itself).
 */
int   equalizer_equalize(void *outbuf, const void *inbuf, int nchan, int freq, int bpsample, unsigned int dlength)
{
	if(!outbuf && !inbuf)return -1;
	if(!settings.player.equalizer_enable) return -2;

	equalize_buffer(outbuf, inbuf, nchan, freq, bpsample, dlength, &eq_state);
	return 0;
}







