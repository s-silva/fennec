#include "main.h"


/*
 * convert fennec priority settings to system values.
 */
int setting_priority_to_sys(int sp)
{
	switch(sp)
	{
	case setting_priority_idle:
	case setting_priority_lowest:       return THREAD_PRIORITY_IDLE;

	case setting_priority_low:          return THREAD_PRIORITY_LOWEST;        
	case setting_priority_below_normal: return THREAD_PRIORITY_BELOW_NORMAL;
	case setting_priority_normal:       return THREAD_PRIORITY_NORMAL;
	case setting_priority_above_normal: return THREAD_PRIORITY_ABOVE_NORMAL;
	case setting_priority_high:         return THREAD_PRIORITY_HIGHEST;

	case setting_priority_highest:
	case setting_priority_realtime:     return THREAD_PRIORITY_TIME_CRITICAL;
	}
	return THREAD_PRIORITY_NORMAL;
}


/*
 * downsample, uses optimizations.
 */
int local_downsample(void* dout, void *idata, unsigned long dsize)
{
	fennec_sample     *ifdata = (fennec_sample*)idata;
	unsigned long      i, ci, msize = dsize / sizeof(fennec_sample);

	/* noise reduction */

	if(sfennec.settings.general->audio_output.noise_reduction)
	{
		fennec_sample x;

		for(i=0; i<msize; i++)
		{
			ci = i % splayer.channels;

			x = ifdata[i] * 0.5;
			ifdata[i] = x + splayer.nrc[ci];
			splayer.nrc[ci] = x;

			if     (ifdata[i] >  1.0)ifdata[i] =  1.0;
			else if(ifdata[i] < -1.0)ifdata[i] = -1.0;
		}
	}

	/* downsampling */

	if(splayer.output_bps == 32 && splayer.output_float)
	{
		float *sout = (float*)dout;

		for(i=0; i<msize; i++)
		{
			sout[i] = (float)(ifdata[i]);
		}

	}else if(splayer.output_bps == 16){
	
		short *sout = (short*)dout;
		double mval = 32767.0, din;
		short  dout;
		uint16_t cw;

		#ifdef _MSC_VER
			__asm fnstcw cw
			cw |= ~((uint16_t)(3 << 10));
			__asm fldcw cw
		#endif

		for(i=0; i<msize; i++)
		{
			din = (double)ifdata[i];

			#ifdef _MSC_VER
				__asm
				{
					fld   din
					fmul  mval
					fistp dout
				}

			#else
				dout = (short) round(din * mval);
			#endif

			sout[i] = dout;
		}

	}else if(splayer.output_bps == 32){

		int *sout = (int*)dout;

		for(i=0; i<msize; i++)
		{
			sout[i] = (int)(ifdata[i] * 2147483647.0);
		}
	}else if(splayer.output_bps == 24){
		
		uint8_t       *sout = (uint8_t*)dout;
		double         mval = 8388607.0, din;
		int32_t        dout;
		uint16_t       cw;
		unsigned long  j;

		#ifdef _MSC_VER
			__asm fnstcw cw
			cw |= ~((uint16_t)(3 << 10));
			__asm fldcw cw
		#endif

		for(i=0, j=0; i<msize; i++, j+=3)
		{
			din = (double)ifdata[i];

			#ifdef _MSC_VER
				__asm
				{
					fld   din
					fmul  mval
					fistp dout
				}
			#else
				dout = (int32_t) round(din * dout);
			#endif

			sout[j]     = (int8_t)((dout >> 0));
			sout[j + 1] = (int8_t)((dout >> 8));
			sout[j + 2] = (int8_t)((dout >> 16));
		}


	}else if(splayer.output_bps == 64){
		
		if(splayer.output_float)
		{
			double *sout = (double*)dout;

			for(i=0; i<msize; i++)
			{
				sout[i] = (double)ifdata[i];
			}
		}else{
			int64_t *sout = (int64_t*)dout;

			for(i=0; i<msize; i++)
			{
				sout[i] = (int64_t)(ifdata[i] * 9223372036854775808.0);
			}
		}

	}else if(splayer.output_bps == 8){

		if(!splayer.output_signed)
		{
			uint8_t *sout = (uint8_t*)dout;
			double   mval = 127.0, din;
			int32_t  dout;
			uint16_t cw;

			#ifdef _MSC_VER
				__asm fnstcw cw
				cw |= ~((uint16_t)(3 << 10));
				__asm fldcw cw
			#endif

			for(i=0; i<msize; i++)
			{
				din = (double)ifdata[i];

				#ifdef _MSC_VER
					__asm
					{
						fld   din
						fmul  mval
						fistp dout
					}
				#else
					dout = (int32_t) round(din * mval);
				#endif

				sout[i] = (uint8_t)(dout + 127);
			}
		}else{
			int8_t *sout = (int8_t*)dout;

			for(i=0; i<msize; i++)
			{
				sout[i] = (int8_t)(ifdata[i] * 127.0);
			}
		}
	}
	return 1;
}



int local_def_downsample(void* dout, void *idata, unsigned long dsize)
{
	fennec_sample     *ifdata = (fennec_sample*)idata;
	unsigned long      i, msize = dsize / sizeof(fennec_sample);

	short *sout = (short*)dout;
	double mval = 32767.0, din;
	short  ddout;
	uint16_t cw;

	#ifdef _MSC_VER

	__asm fnstcw cw
	cw |= ~((uint16_t)(3 << 10));
	__asm fldcw cw

	#endif


	for(i=0; i<msize; i++)
	{
		din = (double)ifdata[i];
		#ifdef _MSC_VER
			__asm
			{
				fld   din
				fmul  mval
				fistp ddout
			}
		#else
			ddout = (short)round(din * mval);
		#endif

		sout[i] = ddout;
	}

	return 1;
}