/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Sample i).
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
#include <windows.h>
#include "../../../include/system.h"
#include "../../../include/fennec.h"


int           callc plugin_dsp_initialize(void);
int           callc plugin_dsp_uninitialize(void);
unsigned long callc plugin_dsp_open(unsigned long id);
int           callc plugin_dsp_close(unsigned long id);
void*         callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr);
int           callc plugin_dsp_about(void *pdata);
int           callc plugin_dsp_settings(void *pdata);


unsigned long callc fennec_initialize_dsp(struct general_dsp_data *gdn, string pname)
{
	if(gdn->ptype != fennec_plugintype_audiodsp)return fennec_input_invalidtype;

	pname[0] = 0;
	str_cpy(pname, uni("Sample: Half Rate"));

	if(gdn->fiversion >= plugin_version)
	{
		gdn->initialize    = plugin_dsp_initialize;
		gdn->uninitialize  = plugin_dsp_uninitialize;

		gdn->open          = plugin_dsp_open;
		gdn->process       = plugin_dsp_process;
		gdn->close         = plugin_dsp_close;

		gdn->settings      = plugin_dsp_settings;
		gdn->about         = plugin_dsp_about;

		gdn->messageproc   = 0;

		if(gdn->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gdn->pversion = plugin_version;
			return fennec_input_unknownversion;
		}
	}else{
		/* version 1 is supported, what? zero? oh no your fault */
		return fennec_input_invalidversion;
	}
}

int callc plugin_dsp_initialize(void)
{
	return 1;
}

int callc plugin_dsp_uninitialize(void)
{
	return 1;
}

unsigned long callc plugin_dsp_open(unsigned long id)
{
	return 1;
}

int callc plugin_dsp_close(unsigned long id)
{
	return 1;
}


void* callc plugin_dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize, func_realloc fr)
{
	fennec_sample  *sout;
	unsigned int    i, c = (*bsize) / (bitspersample / 8);
	static double   lv = 0.0f;
	
	int j;
	
	fennec_sample *bkp = malloc((*bsize) + 1);

	memcpy(bkp, sbuffer, *bsize);

	if(avbsize < (*bsize) * 2)
		sbuffer = fr(sbuffer, (*bsize) * 2);

	sout = (fennec_sample*) (((char*)sbuffer) + apointer);
	j = 0;

	for(i=0; i<c; i++)
	{
		sout[j]   = bkp[i];
		sout[j+1] = bkp[i];
		j += 2;
	}

	(*bsize) *= 2;

	free(bkp);

	return sbuffer;
}

int callc plugin_dsp_about(void *pdata)
{
	MessageBox((HWND)pdata, "Sample: reduce playback speed by half", "About", MB_ICONINFORMATION);
	return 1;
}

int callc plugin_dsp_settings(void *pdata)
{
	MessageBox((HWND)pdata, "Sample: reduce playback speed by half, No settings", "Settings", MB_ICONINFORMATION);
	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
