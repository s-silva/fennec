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


void lowpass_init(double cutoff, double frequency, int c, int sp);
void highpass_init(double cutoff, double frequency, int c, int sp);
void bandpass_init(double cutoff, double frequency, int c, int sp);
void notch_init(double cutoff, double frequency, int c, int sp);
void allpass_init(double cutoff, double frequency, int c, int sp);

/* types --------------------------------------------------------------------*/


struct channel_data
{
	struct
	{
		double a[3], b[3];
		double x[3], y[3];
	}lowpass, highpass, notch, bandpass, allpass;
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
		lowpass_init (setting_lowpass  * (double)frequency / 2, (double)frequency, i, sp);
		highpass_init(setting_highpass * (double)frequency / 2, (double)frequency, i, sp);
		bandpass_init(setting_bandpass * (double)frequency / 2, (double)frequency, i, sp);
		notch_init   (setting_notch    * (double)frequency / 2, (double)frequency, i, sp);
		allpass_init (setting_allpass / 10  * (double)frequency / 2, (double)frequency, i, sp);
	}

	sp = 0;
}





void notch_init(double cutoff, double frequency, int c, int sp)
{
	double w0    = two_pi * cutoff / frequency;
	double Q     = 0.4;
	double vcos  = cos(w0);
	double alpha = sin(w0) / (2*Q);

	data[c].notch.b[0] =  1;
	data[c].notch.b[1] = -2 * cos(w0);
	data[c].notch.b[2] =  1;
	data[c].notch.a[0] =  1 + alpha;
	data[c].notch.a[1] = -2 * cos(w0);
	data[c].notch.a[2] =  1 - alpha;



	if(sp)
	{
		data[c].notch.x[0] = data[c].notch.x[1] = 0;
		data[c].notch.y[0] = data[c].notch.y[1] = 0;
	}
}

double notch_process(double x, int c)
{
	double y;

	y =   (data[c].notch.b[0]/data[c].notch.a[0]) * x
		+ (data[c].notch.b[1]/data[c].notch.a[0]) * data[c].notch.x[0]
		+ (data[c].notch.b[2]/data[c].notch.a[0]) * data[c].notch.x[1]
		- (data[c].notch.a[1]/data[c].notch.a[0]) * data[c].notch.y[0]
		- (data[c].notch.a[2]/data[c].notch.a[0]) * data[c].notch.y[1];

	data[c].notch.x[1] = data[c].notch.x[0];
	data[c].notch.x[0] = x;

	data[c].notch.y[1] = data[c].notch.y[0];
	data[c].notch.y[0] = y;

	return y;
}




void bandpass_init(double cutoff, double frequency, int c, int sp)
{
	double w0    = two_pi * cutoff / frequency;
	double Q     = 1;
	double vcos  = cos(w0);
	double alpha = sin(w0) / (2*Q);


	data[c].bandpass.b[0] =  alpha;
	data[c].bandpass.b[1] =  0;
	data[c].bandpass.b[2] = -alpha;
	data[c].bandpass.a[0] =  1 + alpha;
	data[c].bandpass.a[1] = -2 * cos(w0);
	data[c].bandpass.a[2] =  1 - alpha;


	if(sp)
	{
		data[c].bandpass.x[0] = data[c].bandpass.x[1] = 0;
		data[c].bandpass.y[0] = data[c].bandpass.y[1] = 0;
	}
}

double bandpass_process(double x, int c)
{
	double y;

	y =   (data[c].bandpass.b[0]/data[c].bandpass.a[0]) * x
		+ (data[c].bandpass.b[1]/data[c].bandpass.a[0]) * data[c].bandpass.x[0]
		+ (data[c].bandpass.b[2]/data[c].bandpass.a[0]) * data[c].bandpass.x[1]
		- (data[c].bandpass.a[1]/data[c].bandpass.a[0]) * data[c].bandpass.y[0]
		- (data[c].bandpass.a[2]/data[c].bandpass.a[0]) * data[c].bandpass.y[1];

	data[c].bandpass.x[1] = data[c].bandpass.x[0];
	data[c].bandpass.x[0] = x;

	data[c].bandpass.y[1] = data[c].bandpass.y[0];
	data[c].bandpass.y[0] = y;

	return y;
}





void lowpass_init(double cutoff, double frequency, int c, int sp)
{
	double w0    = two_pi * cutoff / frequency;
	double Q     = 0.7;
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
	double Q     = 0.7;
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



void allpass_init(double cutoff, double frequency, int c, int sp)
{
	double w0    = two_pi * cutoff / frequency;
	double Q     = 0.7;
	double vcos  = cos(w0);
	double alpha = sin(w0) / (2*Q);
 
	
	data[c].allpass.b[0] = (1 - vcos) / 2;
	data[c].allpass.b[1] =  1 - vcos;
	data[c].allpass.b[2] = (1 - vcos) / 2;
	data[c].allpass.a[0] =  1 + alpha;
	data[c].allpass.a[1] = -2 * vcos;
	data[c].allpass.a[2] =  1 - alpha;

	if(sp)
	{
		data[c].allpass.x[0] = data[c].allpass.x[1] = 0;
		data[c].allpass.y[0] = data[c].allpass.y[1] = 0;
	}
}

double allpass_process(double x, int c)
{
	double y;

	y =   (data[c].allpass.b[0]/data[c].allpass.a[0]) * x
		+ (data[c].allpass.b[1]/data[c].allpass.a[0]) * data[c].allpass.x[0]
		+ (data[c].allpass.b[2]/data[c].allpass.a[0]) * data[c].allpass.x[1]
		- (data[c].allpass.a[1]/data[c].allpass.a[0]) * data[c].allpass.y[0]
		- (data[c].allpass.a[2]/data[c].allpass.a[0]) * data[c].allpass.y[1];

	data[c].allpass.x[1] = data[c].allpass.x[0];
	data[c].allpass.x[0] = x;

	data[c].allpass.y[1] = data[c].allpass.y[0];
	data[c].allpass.y[0] = y;

	return y;
}




/*
 * sample processing.
 */
fennec_sample effect_process(fennec_sample x, unsigned long frequency, unsigned long channel)
{
	fennec_sample y = x;

	if(setting_highpass > 0.0)
		y = highpass_process(y, channel);

	if(setting_lowpass > 0.0)
		y = lowpass_process(y, channel);

	if(setting_bandpass > 0.0)
		y = bandpass_process(y, channel);

	if(setting_notch > 0.0)
		y = notch_process(y, channel);

	if(setting_allpass > 0)
	{
		double n = allpass_process(x, channel);
		y = (y + n) / 2.0;
	}

	clip(y, 1);
	return y;
}








