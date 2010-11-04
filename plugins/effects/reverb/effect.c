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
#include <math.h>

/* defines ------------------------------------------------------------------*/

#define	clip(v, m) {if ((v) >  (m)) (v) =  (m); else if ((v) < -(m)) (v) = -(m);}
#define pi      3.1415926535897932384626433832795
#define two_pi  6.283185307179586476925286766559
#define max_delay 80200

void lowpass_init(double cutoff, double frequency, int c, int sp);
void highpass_init(double cutoff, double frequency, int c, int sp);

/* types --------------------------------------------------------------------*/


struct channel_data
{
	struct
	{
		double a[3], b[3];
		double x[3], y[3];
	}lowpass;

	struct
	{
		double a[3], b[3];
		double x[3], y[3];
	}highpass;


	double  combd[10][max_delay];
	int     combi[10];
	double  evs[10];
	double  i, j;


};




/* data ---------------------------------------------------------------------*/

struct channel_data  data[2];
int                  accessing = 0;



/* code ---------------------------------------------------------------------*/


/*
 * initialize: called only once.
 */
void effect_initialize(void)
{
	int    i, j;
	double k = 1.0;

	for(j=0; j<10; j++)
	{
		for(i=0; i<2; i++)
		{
			data[i].combi[j] = 0;
			data[i].evs[j]   = k;
			data[i].j = 0.0;
		}
		
		k /= 1.1;
	}
}


/*
 * settings changing callback.
 */
void effect_set(unsigned long frequency, unsigned long channels, int mode)
{
	static int sp = 1;
	int i;

	if(mode == effect_set_mode_high)
	{
		sp = 0;
		effect_initialize();
	}
	

	for(i=0; i<2; i++)
	{
		lowpass_init(setting_lowpass / 100 * (double)frequency / 2, (double)frequency, i, sp);
		highpass_init(setting_highpass / 100 * (double)frequency / 2, (double)frequency, i, sp);
	}

	sp = 0;
}





void lowpass_init(double cutoff, double frequency, int c, int sp)
{
	double w0    = two_pi * cutoff / frequency;
	double Q     = 1;
	double vcos  = cos(w0);
	double alpha = sin(w0) / (2*Q);


	data[c].lowpass.b[0] = (1 - vcos) / 2;
	data[c].lowpass.b[1] =  1 - vcos;
	data[c].lowpass.b[2] = (1 - vcos) / 2;
	data[c].lowpass.a[0] =  1 + alpha;
	data[c].lowpass.a[1] = -2 * vcos;
	data[c].lowpass.a[2] =  1 - alpha;

	if(sp)
	{
		data[c].lowpass.x[0] = data[c].lowpass.x[1] = 0;
		data[c].lowpass.y[0] = data[c].lowpass.y[1] = 0;
	}
}

double lowpass_process(double x, int c)
{
	double y;

	y =   (data[c].lowpass.b[0]/data[c].lowpass.a[0]) * x
		+ (data[c].lowpass.b[1]/data[c].lowpass.a[0]) * data[c].lowpass.x[0]
		+ (data[c].lowpass.b[2]/data[c].lowpass.a[0]) * data[c].lowpass.x[1]
		- (data[c].lowpass.a[1]/data[c].lowpass.a[0]) * data[c].lowpass.y[0]
		- (data[c].lowpass.a[2]/data[c].lowpass.a[0]) * data[c].lowpass.y[1];

	data[c].lowpass.x[1] = data[c].lowpass.x[0];
	data[c].lowpass.x[0] = x;

	data[c].lowpass.y[1] = data[c].lowpass.y[0];
	data[c].lowpass.y[0] = y;

	return y;
}








void highpass_init(double cutoff, double frequency, int c, int sp)
{
	double w0    = two_pi * cutoff / frequency;
	double Q     = 1;
	double vcos  = cos(w0);
	double alpha = sin(w0) / (2*Q);
 
	data[c].highpass.b[0] =  (1 + vcos) / 2;
	data[c].highpass.b[1] = -(1 + vcos);
	data[c].highpass.b[2] =  (1 + vcos) / 2;
	data[c].highpass.a[0] =   1 + alpha;
	data[c].highpass.a[1] =  -2 * vcos;
	data[c].highpass.a[2] =   1 - alpha;

	if(sp)
	{
		data[c].highpass.x[0] = data[c].highpass.x[1] = 0;
		data[c].highpass.y[0] = data[c].highpass.y[1] = 0;
	}
}

double highpass_process(double x, int c)
{
	double y;

	y =   (data[c].highpass.b[0]/data[c].highpass.a[0]) * x
		+ (data[c].highpass.b[1]/data[c].highpass.a[0]) * data[c].highpass.x[0]
		+ (data[c].highpass.b[2]/data[c].highpass.a[0]) * data[c].highpass.x[1]
		- (data[c].highpass.a[1]/data[c].highpass.a[0]) * data[c].highpass.y[0]
		- (data[c].highpass.a[2]/data[c].highpass.a[0]) * data[c].highpass.y[1];

	data[c].highpass.x[1] = data[c].highpass.x[0];
	data[c].highpass.x[0] = x;

	data[c].highpass.y[1] = data[c].highpass.y[0];
	data[c].highpass.y[0] = y;

	return y;
}




double process_comb(double x, int c, int j)
{
	int    i = data[c].combi[j], maxb;
	double rv;
	double wet = setting_wet  / 100;
	double dry = setting_dry / 100;


	rv = (x * dry) + (data[c].combd[j][i] * wet);

	data[c].combd[j][i] = rv + (x * wet);

	if(setting_lowpass < 99.0)
		data[c].combd[j][i] = lowpass_process(data[c].combd[j][i], c);

	if(setting_highpass > 1.0)
		data[c].combd[j][i] = highpass_process(data[c].combd[j][i], c);

	data[c].combd[j][i] *= (setting_phase / 100);


	maxb = (int)(max_delay * ((setting_delay + 1.0) / 102.0) * data[c].evs[j]);
	if(++i > maxb)i = 0;

	data[c].combi[j] = i;
	return rv;
}



















/*
 * sample processing.
 */
fennec_sample effect_process(fennec_sample x, unsigned long frequency, unsigned long channel)
{
	int           i, m;
	double        y;
	static double ly[2][80000];
	static int    ly_i[2] = {-1, -1};

	if(channel < 0 || channel > 2)return 0.0;

	if(ly_i[channel] < 0)
	{
		memset(ly[channel], 0, sizeof(double) * 80000);
		ly_i[channel] = 0;
	}

	if     (x >  1) x =  1;
	else if(x < -1) x = -1;



	m = (int)(setting_diffusion / 10);
	if(m > 10) m = 10;
	else if (m < 1) m = 1;

	y = 0;

	for(i=0; i<m; i++)
		y += process_comb(x, channel, i);

	y *= 0.1;
	
	y = (y*0.6 + ly[!channel][i]*0.4);

	


	if      (y >  1) y =  1;
	else if (y < -1) y = -1;


	ly[channel][i] = y;

	if(++ly_i[channel] >= 8000) ly_i[channel] = 0;

	return y;
}


