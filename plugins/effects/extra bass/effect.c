/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Stereo Enhancer).
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

#include "effect.h"




/* types --------------------------------------------------------------------*/

struct channel_data
{
	double  frequency, boostdb;
	double  xn[2], yn[2];
	double  omega;
	double  sn, cs, A, shape, beta;
	double  b[3], a[3];
};




/* data ---------------------------------------------------------------------*/

struct channel_data  chd[max_channels];




/* code ---------------------------------------------------------------------*/


/*
 * initialize: called only once.
 */
void effect_initialize(void)
{

}


/*
 * settings changing callback.
 */
void effect_set(unsigned long frequency, unsigned long channels, int mode)
{
	unsigned int i;


	for(i=0; i<channels; i++)
	{
		if(mode) /* higher than 'low' level */
		{
			chd[i].xn[0] = 0.0f;
			chd[i].xn[0] = 0.0f;
			chd[i].yn[0] = 0.0f;
			chd[i].yn[1] = 0.0f;
		}

		chd[i].frequency = (double) setting_band;
		chd[i].boostdb   = (double) setting_boost;

		chd[i].omega  = 2.0f * 3.141592653589f * chd[i].frequency / (double)frequency;
		chd[i].sn     = sin((double)chd[i].omega);
		chd[i].cs     = cos((double)chd[i].omega);
		chd[i].A      = exp(log(10.0) * chd[i].boostdb / 40.0);
		chd[i].shape  = setting_shape;

		chd[i].beta  = sqrt(( (chd[i].A * chd[i].A + 1) / chd[i].shape - (pow((chd[i].A - 1), 2)) ));

		chd[i].b[0]  = chd[i].A * ((chd[i].A + 1) - (chd[i].A - 1) * chd[i].cs + chd[i].beta * chd[i].sn);
		chd[i].b[1]  = 2 * chd[i].A * ((chd[i].A - 1) - (chd[i].A + 1) * chd[i].cs);
		chd[i].b[2]  = chd[i].A * ((chd[i].A + 1) - (chd[i].A - 1) * chd[i].cs - chd[i].beta * chd[i].sn);
		chd[i].a[0]  = ((chd[i].A + 1) + (chd[i].A - 1) * chd[i].cs + chd[i].beta * chd[i].sn);
		chd[i].a[1]  = -2 * ((chd[i].A - 1) + (chd[i].A + 1) * chd[i].cs);
		chd[i].a[2]  = (chd[i].A + 1) + (chd[i].A - 1) * chd[i].cs - chd[i].beta * chd[i].sn;
	}
}


/*
 * sample processing.
 */
fennec_sample effect_process(fennec_sample v, unsigned long frequency, unsigned long channel)
{
	static unsigned long sample_index = 0, clippings = 0;
	fennec_sample	in, out;
	int             i = channel;

	in  = v / (fennec_sample)setting_preamp;
    out = (chd[i].b[0] * in + chd[i].b[1] * chd[i].xn[0] + chd[i].b[2] * chd[i].xn[1] - chd[i].a[1] * chd[i].yn[0] - chd[i].a[2] * chd[i].yn[1]) / chd[i].a[0];

    chd[i].xn[1] = chd[i].xn[0];
    chd[i].xn[0] = in;
    chd[i].yn[1] = chd[i].yn[0];
    chd[i].yn[0] = out;

    if(out < -1.0)
	{
		out = -1.0;
		if(setting_normal)
		{
			if(setting_preamp < 8.0 /* 8dB */)
				setting_preamp += 0.1;
			clippings++;
		}

	}else if(out > 1.0){

		out = 1.0;
		if(setting_normal)
		{
			if(setting_preamp < 8.0 /* 8dB */)
				setting_preamp  += 0.1;
			clippings++;
		}
	}

	if(setting_normal && setting_preamp > 1.0)
	{
		sample_index++;

		if(sample_index > 65536)
		{
			if(clippings < 1)
			{
				setting_preamp -= 0.04;
				if(setting_preamp < 1.0)setting_preamp = 1.0;
			}
			clippings = 0;
			sample_index = 0;
		}
	}


	return out;
}