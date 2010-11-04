/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (Audio CD).
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

#include "main.h"

/* variables ----------------------------------------------------------------*/

struct plugin_stream* pstreams;
unsigned long pstreams_count = 0;
int           plugin_busy    = 0;

string info_base = uni("Module Decoder (ModPlug)");

/* functions ----------------------------------------------------------------*/

/*-------------------------------------------------------------------
 initialize plugin.
-------------------------------------------------------------------*/

unsigned long callc fennec_initialize_input(struct general_input_data* gin)
{
	OSVERSIONINFO ovi;

	if(gin->ptype != fennec_plugintype_audiodecoder)return fennec_input_invalidtype;

	/* OS version test */

	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	if((ovi.dwPlatformId == VER_PLATFORM_WIN32s || ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (ovi.dwMajorVersion < 5))
		return fennec_input_invalidtype;

	
	if(gin->fiversion >= plugin_version)
	{
		gin->initialize    = fennec_plugin_initialize;
		gin->open          = fennec_plugin_open;
		gin->getformat     = fennec_plugin_getformat;
		gin->getinfo       = 0;
		gin->getduration   = fennec_plugin_getduration;
		gin->getposition   = fennec_plugin_getposition;
		gin->setposition   = fennec_plugin_setposition;
		gin->getstatus     = fennec_plugin_getstatus;
		gin->read          = fennec_plugin_read;
		gin->close         = fennec_plugin_close;
		gin->uninitialize  = fennec_plugin_uninitialize;
		gin->about         = fennec_plugin_about;
		gin->settings      = fennec_plugin_settings;

		gin->get_extension           = fennec_plugin_get_extension;
		gin->get_initialization_info = fennec_plugin_get_initialization_info;
		gin->tagread                 = fennec_plugin_tagread;
		gin->tagwrite                = fennec_plugin_tagwrite;

		gin->score = fennec_plugin_score;

		if(gin->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gin->pversion = plugin_version;
			return fennec_input_unknownversion;
		}

	}else{

		/* version 1 is supported, what? zero? oh no your fault */
		return fennec_input_invalidversion;
	}
}

/*-------------------------------------------------------------------
 real initialization.
-------------------------------------------------------------------*/

int callc fennec_plugin_initialize(void)
{
	unsigned long i;

	if(pstreams_count)return 0;

	plugin_busy = 1;

	pstreams_count = plugin_player_base;
	pstreams       = (struct plugin_stream*) sys_mem_alloc(pstreams_count * sizeof(struct plugin_stream));
	
	if(pstreams == v_error_sys_mem_alloc)
	{
		pstreams_count = 0;
		plugin_busy = 0;
		return 0;
	}

	for(i=0; i<pstreams_count; i++) /* free everything */
	{
		pstreams[i].initialized = 0;
	}

	plugin_busy = 0;
	return 1;
}

/*-------------------------------------------------------------------
 load stream/file.
-------------------------------------------------------------------*/

unsigned long callc fennec_plugin_open(unsigned long otype, unsigned long osize, const string odata)
{
	int           found = 0;
	unsigned long i;
	unsigned long last_count;
	unsigned long j;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	if(otype == fennec_input_open_file || otype == fennec_input_open_internet_stream)
	{
		/* search for an empty player */

		for(i=0; i<pstreams_count; i++)
		{
			if(!pstreams[i].initialized)
			{
				found = 1;
				break;
			}
		}

		if(found)
		{
			/* there's an empty player, id - 'i' */

			pstreams[i].initialized = 1;
			j = i;

		}else{
			/* should reallocate some memory */

			last_count = pstreams_count;

			if(pstreams_count >= plugin_player_max)
			{
				/* we've reached to the maximum, sorry can't load more */
				plugin_busy = 0;
				return 0;
			}

			pstreams_count += plugin_player_add;
			if(pstreams_count > plugin_player_max)pstreams_count = plugin_player_max;

			pstreams = (struct plugin_stream*) sys_mem_realloc(pstreams, pstreams_count * sizeof(struct plugin_stream));
			
			for(i=last_count; i<pstreams_count; i++)
			{
				pstreams[i].initialized = 0;
			}

			j = last_count + 1;
			pstreams[j].initialized = 1;
		}
		
		decoder_load(j, odata);

		plugin_busy = 0;
		return j;
	}

	plugin_busy = 0;
	return 0;
}

/*-------------------------------------------------------------------
 score file.
-------------------------------------------------------------------*/

int callc fennec_plugin_score(const string fname, int foptions)
{
	/* if the file is 'cda', what more? */
	return 10;
}

/*-------------------------------------------------------------------
 get format information (raw).
-------------------------------------------------------------------*/

int callc fennec_plugin_getformat(unsigned long id, unsigned long* ftype, void* fdata)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	switch(*ftype)
	{
	case fennec_input_format_base:
		((struct base_format*)fdata)->bbits = pstreams[id].bitspersample;
		((struct base_format*)fdata)->bfrequency = pstreams[id].frequency;
		((struct base_format*)fdata)->bchannels  = pstreams[id].channels;
		break;

	default:
		plugin_busy = 0;
		return 0;
	}

	plugin_busy = 0;
	return 1;
}

/*-------------------------------------------------------------------
 get current position.
-------------------------------------------------------------------*/

int callc fennec_plugin_getposition(unsigned long id, unsigned long* ptype, void* pos)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	switch(*ptype)
	{
	case fennec_input_position_32ms:

		*((unsigned long*)pos) = 0;
		break;

	default:
		return 0;

	}

	return 1;
}

/*-------------------------------------------------------------------
 set playing position (some media will not support this, they must
 return zero.
-------------------------------------------------------------------*/

int callc fennec_plugin_setposition(unsigned long id, unsigned long ptype, void* pos)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)return 0; /* just return */
	plugin_busy = 1;

	switch(ptype)
	{
	case fennec_input_position_32ms:
		decoder_seek(id, ((double)*((DWORD*)pos)) / ((double)(pstreams[id].duration * 1000)));
		break;

	case fennec_input_position_fraction:
		decoder_seek(id, *((double*)pos));
		break;

	default:
		plugin_busy = 0;
		return 0;

	}

	plugin_busy = 0;
	return 1;
}

/*-------------------------------------------------------------------
 get duration.
-------------------------------------------------------------------*/

int callc fennec_plugin_getduration(unsigned long id, unsigned long* ptype, void* pos)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	switch(*ptype)
	{
	case fennec_input_position_32ms:

		*((unsigned long*)pos) = pstreams[id].duration;
		break;

	default:
		return 0;

	}

	return 1;
}

/*-------------------------------------------------------------------
 get plugin status.
-------------------------------------------------------------------*/

int callc fennec_plugin_getstatus(unsigned long id, unsigned long* stype, void* stt)
{
	if(*stype != fennec_input_status_base)return 0;

	((struct base_status*)stt)->bstate = fennec_input_status_sleeping;
	return 1;
}

/*-------------------------------------------------------------------
 read (decode) next raw block. (use 'setposition' to seek between blocks.
-------------------------------------------------------------------*/

int callc fennec_plugin_read(unsigned long id, unsigned long dsize, unsigned long* dread, void* rdata)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	*dread = decoder_read(id, rdata, dsize);

	if(*dread < dsize)
	{
		plugin_busy = 0;
		return fennec_input_nodata;
	}

	plugin_busy = 0;
	return 1;
}

/*-------------------------------------------------------------------
 show 'about plugin' dialog etc.
-------------------------------------------------------------------*/

int callc fennec_plugin_about(void* pdata)
{
#ifdef system_microsoft_windows
	
	MessageBox((HWND)pdata, "Module plugin", "About", MB_ICONINFORMATION);

#endif
	return 1;
}

/*-------------------------------------------------------------------
 show 'settings' dialog etc.
-------------------------------------------------------------------*/

int callc fennec_plugin_settings(void* pdata)
{
#ifdef system_microsoft_windows
	MessageBox((HWND)pdata, "Sorry, No settings.", "Settings", MB_ICONINFORMATION);
#endif
	return 1;
}

/*-------------------------------------------------------------------
 get information about plugin.
-------------------------------------------------------------------*/

int callc fennec_plugin_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize)
{
	if(inforreq == fennec_information_basic)
	{
		str_cpy((string)outinfo, info_base);
		*outsize = (unsigned long)str_size(info_base);
		return 0;
	}
	return 0;
}

/*-------------------------------------------------------------------
 get current get an item from the extension list. (return 0 - end of list)
-------------------------------------------------------------------*/

int callc fennec_plugin_get_extension(int id, string out_ext, string out_desc, int* newdsc)
{
	if(id >= 33)return 0;
	if(id == 0)*newdsc = 1;

	switch(id)
	{
	case 0:
		str_cpy(out_ext,  uni("mod"));
		str_cpy(out_desc, uni("ProTracker Modules"));
		break;

	case 1:
		str_cpy(out_ext,  uni("nst"));
		str_cpy(out_desc, uni("ProTracker Modules"));
		break;

	case 2:
		str_cpy(out_ext,  uni("mdz"));
		str_cpy(out_desc, uni("ProTracker Modules"));
		break;

	case 3:
		str_cpy(out_ext,  uni("mdr"));
		str_cpy(out_desc, uni("ProTracker Modules"));
		break;

	case 4:
		str_cpy(out_ext,  uni("m15"));
		str_cpy(out_desc, uni("ProTracker Modules"));
		break;

	case 5:
		str_cpy(out_ext,  uni("s3m"));
		str_cpy(out_desc, uni("ScreamTracker Modules"));
		*newdsc = 1;
		break;

	case 6:
		str_cpy(out_ext,  uni("stm"));
		str_cpy(out_desc, uni("ScreamTracker Modules"));
		break;

	case 7:
		str_cpy(out_ext,  uni("s3z"));
		str_cpy(out_desc, uni("ScreamTracker Modules"));
		break;

	case 8:
		str_cpy(out_ext,  uni("xm"));
		str_cpy(out_desc, uni("FastTracker Modules"));
		*newdsc = 1;
		break;

	case 9:
		str_cpy(out_ext,  uni("xmz"));
		str_cpy(out_desc, uni("FastTracker Modules"));
		break;

	case 10:
		str_cpy(out_ext,  uni("it"));
		str_cpy(out_desc, uni("Impulse Tracker Modules"));
		*newdsc = 1;
		break;

	case 11:
		str_cpy(out_ext,  uni("itz"));
		str_cpy(out_desc, uni("Impulse Tracker Modules"));
		break;
	
	case 12:
		str_cpy(out_ext,  uni("mtm"));
		str_cpy(out_desc, uni("General Modules"));
		*newdsc = 1;
		break;

	case 13:
		str_cpy(out_ext,  uni("669"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 14:
		str_cpy(out_ext,  uni("ult"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 15:
		str_cpy(out_ext,  uni("wow"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 16:
		str_cpy(out_ext,  uni("far"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 17:
		str_cpy(out_ext,  uni("mdl"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 18:
		str_cpy(out_ext,  uni("okt"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 19:
		str_cpy(out_ext,  uni("dmf"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 20:
		str_cpy(out_ext,  uni("ptm"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 21:
		str_cpy(out_ext,  uni("med"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 22:
		str_cpy(out_ext,  uni("ams"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 23:
		str_cpy(out_ext,  uni("dbm"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 24:
		str_cpy(out_ext,  uni("dsm"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 25:
		str_cpy(out_ext,  uni("umx"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 26:
		str_cpy(out_ext,  uni("amf"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 27:
		str_cpy(out_ext,  uni("psm"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 28:
		str_cpy(out_ext,  uni("mt2"));
		str_cpy(out_desc, uni("General Modules"));
		break;

	case 29:
		str_cpy(out_ext,  uni("mid"));
		str_cpy(out_desc, uni("Midi Files"));
		*newdsc = 1;
		break;

	case 30:
		str_cpy(out_ext,  uni("midi"));
		str_cpy(out_desc, uni("Midi Files"));
		break;

	case 31:
		str_cpy(out_ext,  uni("rmi"));
		str_cpy(out_desc, uni("Midi Files"));
		break;
		
	case 32:
		str_cpy(out_ext,  uni("smf"));
		str_cpy(out_desc, uni("Midi Files"));
		break;
	}
	return 1;
}


/*-------------------------------------------------------------------
 extended version of 'preload_tagread'.
-------------------------------------------------------------------*/

int callc fennec_plugin_tagread(const string fname, struct fennec_audiotag* rtag)
{
	return tagread(fname, rtag);
}
/*-------------------------------------------------------------------
 extended version of 'preload_tagread'.
-------------------------------------------------------------------*/

int callc fennec_plugin_tagwrite(const string fname, struct fennec_audiotag* rtag)
{
	return tagwrite(fname, rtag);
}

/*-------------------------------------------------------------------
 unload stream.
-------------------------------------------------------------------*/

int callc fennec_plugin_close(unsigned long id)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	decoder_close(id);

	pstreams[id].initialized = 0;
	plugin_busy = 0;
	return 1;
}

/*-------------------------------------------------------------------
 uninitialize plugin and unload all streams.
-------------------------------------------------------------------*/

int callc fennec_plugin_uninitialize(void)
{
	unsigned long i;

	if(!pstreams_count)return 0;
	
	for(i=0; i<pstreams_count; i++)
	{
		if(pstreams[i].initialized)
		{
			fennec_plugin_close(i);
		}
	}

	pstreams_count = 0;
	sys_mem_free(pstreams);
	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
