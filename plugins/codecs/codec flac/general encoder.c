/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0
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

#include "plugin.h"





/* variables ----------------------------------------------------------------*/



struct plugin_encoder_stream  *pestreams;
unsigned long                  pestreams_count      = 0;
int                            encoder_plugin_busy  = 0;

string encoder_info_base = uni("FLAC Encoder");





/* functions ----------------------------------------------------------------*/


/*
 * callback function for initialization.
 */
unsigned long callc fennec_initialize_encoder(struct general_encoder_data* gen)
{
	if(gen->ptype != fennec_plugintype_audioencoder)
		return fennec_input_invalidtype;
	
	if(gen->fiversion >= plugin_version)
	{
		gen->initialize    = encoder_fennec_plugin_initialize;
		gen->uninitialize  = encoder_fennec_plugin_uninitialize;

		gen->file_create   = encoder_file_create;
		gen->file_write    = encoder_file_write;
		gen->file_close    = encoder_file_close;
		gen->file_flush    = encoder_file_flush;
		gen->file_seek     = encoder_file_seek;
		gen->file_tell     = encoder_file_tell;

		gen->encode_initialize   = encode_initialize;
		gen->encode_uninitialize = encode_uninitialize;
		gen->encode_encode       = encode_encode;
		
		gen->about                   = encoder_about;
		gen->settings_fileencoder    = settings_fileencoder;
		gen->settings_encoder        = settings_encoder;

		gen->get_initialization_info = encoder_get_initialization_info;

		if(gen->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gen->pversion = plugin_version;
			return fennec_input_unknownversion;
		}

	}else{
		return fennec_input_invalidversion;
	}
}


/*
 * encoder initialization (real).
 */
int callc encoder_fennec_plugin_initialize(void)
{
	unsigned long i;

	if(pestreams_count)
		return 0; /* already initialized */

	encoder_plugin_busy = 1;

	pestreams_count = plugin_player_base;
	pestreams       = (struct plugin_encoder_stream*) sys_mem_alloc(pestreams_count * sizeof(struct plugin_encoder_stream));
	
	if(pestreams == v_error_sys_mem_alloc)
	{
		pestreams_count = 0;
		encoder_plugin_busy = 0;
		return 0;
	}

	for(i=0; i<pestreams_count; i++) /* free everything */
	{
		pestreams[i].initialized = 0;
	}

	encoder_plugin_busy = 0;
	return 1;
}


/*
 * uninitialize plugin and unload all initialized streams.
 */
int callc encoder_fennec_plugin_uninitialize(void)
{
	unsigned long i;

	if(!pestreams_count)return 0;
	
	for(i=0; i<pestreams_count; i++)
	{
		if(pestreams[i].initialized)
		{
			encoder_file_close(i);
		}
	}

	pestreams_count = 0;
	sys_mem_free(pestreams);
	return 1;
}


/*
 * reserve space for a new data index in 'pestreams'.
 * return: 'fennec_invalid_index' - error
 */
unsigned long encoder_local_locateindex(void)
{
	int           found = 0;
	unsigned long i;
	unsigned long last_count;
	unsigned long j;

	/* try to locate an empty index */

	for(i=0; i<pestreams_count; i++)
	{
		if(!pestreams[i].initialized)
		{
			found = 1;
			break;
		}
	}

	if(found)
	{
		/* there's an empty index, id = 'i' */

		pestreams[i].initialized = 1;
		j = i;

	}else{
		/* should reallocate some memory */

		last_count = pestreams_count;

		if(pestreams_count >= plugin_player_max)
		{
			/* we've reached the maximum limit, sorry can't allocate more */
			encoder_plugin_busy = 0;
			return fennec_invalid_index;
		}

		pestreams_count += plugin_player_add;
		if(pestreams_count > plugin_player_max)pestreams_count = plugin_player_max;

		pestreams = (struct plugin_encoder_stream*) sys_mem_realloc(pestreams, pestreams_count * sizeof(struct plugin_encoder_stream));
		
		for(i=last_count; i<pestreams_count; i++)
		{
			pestreams[i].initialized = 0;
		}

		j = last_count + 1;
		pestreams[j].initialized = 1;
	}
		
	return j;
}


/*
 * file name should not contain any extension, encoder will append an
 * extension automatically.
 */
unsigned long callc encoder_file_create(string fname, unsigned long fcreatemode)
{
	unsigned long id;

	id = encoder_local_locateindex();

	if(id == fennec_invalid_index)return fennec_invalid_index;

	pestreams[id].bmode = fennec_plugintype_audioencoder;
	pestreams[id].smode = fennec_encoder_file;

	str_cpy(pestreams[id].filepath, fname);

	pestreams[id].fhandle        = v_error_sys_file_create;
	pestreams[id].cfrequency     = 0;
	pestreams[id].cbitspersample = 0;
	pestreams[id].cchannels      = 0;

	/* set defaults */

	encoder_setdefaults(id);
	encoder_appendextension(id, pestreams[id].filepath);
	
	str_cpy(fname, pestreams[id].filepath);
	
	encoder_initialize(id, fcreatemode);
	return id;
}


/*
 * encode and write to a file.
 */
int callc encoder_file_write(unsigned long id, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize)
{
	if(pestreams[id].cbitspersample == 0 || pestreams[id].cfrequency == 0 || pestreams[id].cchannels == 0)
	{
		pestreams[id].cfrequency     = sfreq;
		pestreams[id].cbitspersample = sbps;
		pestreams[id].cchannels      = schannels;

		if(pestreams[id].cbitspersample == 0 || pestreams[id].cfrequency == 0 || pestreams[id].cchannels == 0)return 0;
	
		encoder_set(id);


	}else{ /* variable sampling data */

		if(sfreq != pestreams[id].cfrequency)return 0; /* still cannot change */
		if(schannels != pestreams[id].cchannels)return 0;
		if(sbps != pestreams[id].cbitspersample)return 0;
	}

	return encoder_write(id, pcmin, dsize);
}


/*
 * close encoder index.
 */
int callc encoder_file_close(unsigned long id)
{
	encoder_uninitialize(id);
	return 1;
}


/*
 * not implemented
 */
int callc encoder_file_flush(unsigned long id)
{
	return 1;
}


/*
 * not implemented
 */
int callc encoder_file_seek(unsigned long id, double spos)
{
	return 1;
}


/*
 * not implemented
 */
double callc encoder_file_tell(unsigned long id, unsigned long tellmode)
{
	return 1;
}


/*
 * not implemented
 */
unsigned long callc encode_initialize(void)
{
	return 1;
}


/*
 * stream encoder: not implemented
 */
int callc encode_encode(unsigned long id, void* pcmin, void* eout, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize, unsigned long* osize, encode_realloc reallocfunc)
{
	return 1;
}


/*
 * stream encoder: not implemented
 */
int callc encode_uninitialize(unsigned long id)
{
	return 1;
}


/*
 * show some information.
 * 'fennec_invalid_index' to display general about.
 */
int callc encoder_about(unsigned long id, void* odata)
{
#	if defined(system_microsoft_windows)

		MessageBox((HWND)odata, uni("Free Lossless Audio Codec (FLAC) for Fennec Player."), uni("About FLAC Plug-in"), MB_ICONINFORMATION);

#	endif

	return 1;
}


/*
 * show file settings.
 * 'fennec_invalid_index' to display general settings.
 */
int callc settings_fileencoder(unsigned long id, void* odata)
{
	if(id == fennec_invalid_index)
	{

	}
	return 1;
}


/*
 * show stream settings.
 * 'fennec_invalid_index' to display general settings.
 */
int callc settings_encoder(unsigned long id, void* odata)
{
	return 1;
}


/*
 * get encoder name.
 */
int callc encoder_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize)
{
	if(inforreq == fennec_information_basic)
	{
		str_ncpy(outinfo, encoder_info_base, min(*outsize + 1, str_len(encoder_info_base) + 1));
		*outsize = (unsigned long)str_len(encoder_info_base);
		return 1;
	}
	return 0;
}



/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
