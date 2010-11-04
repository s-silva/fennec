/*-----------------------------------------------------------------------------
 
 Fennec 7.1
 Copyright (C) 2007 Chase Holloway
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-----------------------------------------------------------------------------*/

#include "main.h"

/* variables ----------------------------------------------------------------*/

struct plugin_stream* pstreams;
unsigned long pstreams_count = 0;
int           plugin_busy    = 0;

struct plugin_settings fsettings;

const string info_base = uni("Musepack Encoder");

/* functions ----------------------------------------------------------------*/

/*-------------------------------------------------------------------
 initialize plugin.
-------------------------------------------------------------------*/

unsigned long callc fennec_initialize_encoder(struct general_encoder_data* gen)
{
	/* are you looking for me? */

	if(gen->ptype != fennec_plugintype_audioencoder)return fennec_input_invalidtype;
	
	if(gen->fiversion >= plugin_version)
	{
		gen->initialize    = fennec_plugin_initialize;
		gen->uninitialize  = fennec_plugin_uninitialize;

		gen->file_create   = file_create;
		gen->file_write    = file_write;
		gen->file_close    = file_close;
		gen->file_flush    = file_flush;
		gen->file_seek     = file_seek;
		gen->file_tell     = file_tell;

		gen->encode_initialize   = encode_initialize;
		gen->encode_uninitialize = encode_uninitialize;
		gen->encode_encode       = encode_encode;
		
		gen->about                   = about;
		gen->settings_fileencoder    = settings_fileencoder;
		gen->settings_encoder        = settings_encoder;

		gen->get_initialization_info = get_initialization_info;

		memcpy(&fsettings, &gen->fsettings, sizeof(struct plugin_settings));

		if(gen->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gen->pversion = plugin_version;
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
			//fennec_plugin_close(i);
		}
	}

	pstreams_count = 0;
	sys_mem_free(pstreams);
	return 1;
}

/*-------------------------------------------------------------------
 reserve space for a new data index in 'pstreams'.
 return: 'fennec_invalid_index' - error
-------------------------------------------------------------------*/

unsigned long local_locateindex(void)
{
	int           found = 0;
	unsigned long i;
	unsigned long last_count;
	unsigned long j;

	/* try to locate an empty index */

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
		/* there's an empty index, id = 'i' */

		pstreams[i].initialized = 1;
		j = i;

	}else{
		/* should reallocate some memory */

		last_count = pstreams_count;

		if(pstreams_count >= plugin_player_max)
		{
			/* we've reached the maximum limit, sorry can't allocate more */
			plugin_busy = 0;
			return fennec_invalid_index;
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
		
	return j;
}


/*-------------------------------------------------------------------
 file name should not contain any extension, encoder will append an
 extension to it automatically.
-------------------------------------------------------------------*/

unsigned long callc file_create(string fname, unsigned long fcreatemode)
{
	unsigned long id;

	id = local_locateindex();

	if(id == fennec_invalid_index)return fennec_invalid_index;

	pstreams[id].bmode = fennec_plugintype_audioencoder;
	pstreams[id].smode = fennec_encoder_file;

	str_cpy(pstreams[id].filepath, fname);

	pstreams[id].fhandle        = v_error_sys_file_create;
	pstreams[id].cfrequency     = 0;
	pstreams[id].cbitspersample = 0;
	pstreams[id].cchannels      = 0;

	/* set defaults */

	encoder_setdefaults(id);
	encoder_appendextension(id, pstreams[id].filepath);

	str_cpy(fname, pstreams[id].filepath);
	
	encoder_initialize(id, fcreatemode);
	return id;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

int callc file_write(unsigned long id, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize)
{
	if(pstreams[id].cbitspersample == 0 || pstreams[id].cfrequency == 0 || pstreams[id].cchannels == 0)
	{
		pstreams[id].cfrequency     = sfreq;
		pstreams[id].cbitspersample = sbps;
		pstreams[id].cchannels      = schannels;

		if(pstreams[id].cbitspersample == 0 || pstreams[id].cfrequency == 0 || pstreams[id].cchannels == 0)return 0;
	
		encoder_set(id);
	}else{ /* variable sampling data */

		if(sfreq != pstreams[id].cfrequency)return 0; /* still cannot change */
		if(schannels != pstreams[id].cchannels)return 0;
		if(sbps != pstreams[id].cbitspersample)return 0;
	}

	return encoder_write(id, pcmin, dsize);
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

int callc file_close(unsigned long id)
{
	encoder_uninitialize(id);
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

int callc file_flush(unsigned long id)
{
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

int callc file_seek(unsigned long id, double spos)
{
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

double callc file_tell(unsigned long id, unsigned long tellmode)
{
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

unsigned long callc encode_initialize(void)
{
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

int callc encode_encode(unsigned long id, void* pcmin, void* eout, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize, unsigned long* osize, encode_realloc reallocfunc)
{
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/

int callc encode_uninitialize(unsigned long id)
{
	return 1;
}

/*-------------------------------------------------------------------
 
 'fennec_invalid_index' to display general about.
-------------------------------------------------------------------*/

int callc about(unsigned long id, void* odata)
{

#if defined(system_microsoft_windows)

	MessageBox((HWND)odata, uni("Musepack Encoder"), uni("About Musepack Encoder"), MB_ICONINFORMATION);

#endif

	return 1;
}

/*-------------------------------------------------------------------

 'fennec_invalid_index' to display general settings.
-------------------------------------------------------------------*/


int callc settings_fileencoder(unsigned long id, void* odata)
{

#if defined(system_microsoft_windows)

	DialogBox(dll_instance, MAKEINTRESOURCE(IDD_SETTINGS), (HWND)odata, (DLGPROC) proc_settings);
	
#endif

	return 1;
}

/*-------------------------------------------------------------------
 
 'fennec_invalid_index' to display general settings.
-------------------------------------------------------------------*/


int callc settings_encoder(unsigned long id, void* odata)
{
	return 1;
}

/*-------------------------------------------------------------------
 
-------------------------------------------------------------------*/


int callc get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize)
{
	if(inforreq == fennec_information_basic)
	{
		str_ncpy(outinfo, info_base, min(*outsize + 1, str_size(info_base) + sizeof(letter)));
		*outsize = (unsigned long)str_size(info_base);
		return 1;
	}
	return 0;
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/