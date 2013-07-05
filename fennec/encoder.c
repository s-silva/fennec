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

#include "fennec_main.h"
#include "fennec_audio.h"
#include "plugins.h"




/* defines ------------------------------------------------------------------*/

#define encoders_startup        0x2
#define encoders_addition       0x2
#define input_plugins_version   1
#define input_invalid_plugin    -1




/* prototypes ---------------------------------------------------------------*/

int local_encoders_refresh(const string spath);




/* structs ------------------------------------------------------------------*/

struct encoder_plugin
{
	letter                       fname[v_sys_maxpath];
	letter                       encname[v_sys_maxpath];
	t_sys_library_handle         hencoder;
	struct general_encoder_data  ged;
};




/* data ---------------------------------------------------------------------*/

struct encoder_plugin*  encoders;
unsigned int            encoders_count = 0;
unsigned int            encoders_size  = 0;




/* code ---------------------------------------------------------------------*/


/*
 * initialize encoder engine.
 */
int encoder_initialize(void)
{
	unsigned int i;
	letter       pluginspath[v_sys_maxpath];
	/* search for plug-ins (do not cache) */
	
	encoders_size  = encoders_startup;
	encoders_count = 0;

	report("initializing encoders engine", rt_stepping);

	encoders = (struct encoder_plugin*) sys_mem_alloc(encoders_size * sizeof(struct encoder_plugin));

	for(i = 0;  i < encoders_size;  i++)
	{
		encoders[i].hencoder = 0;
	}

	fennec_get_plugings_path(pluginspath);
	local_encoders_refresh(pluginspath);
	return 1;
}


/*
 * uninitialize encoder engine.
 */
int encoder_uninitialize(void)
{	
	if(encoders_size)
	{
		unsigned int i;

		report("uninitializing encoders engine", rt_stepping);

		for(i=0; i<encoders_count; i++)
		{
			encoder_plugin_uninitialize(i);
		}

		sys_mem_free(encoders);
		encoders_size = 0;
	}
	return 1;
}


/*
 * get located encoders count.
 */
unsigned int encoder_getencoderscount(void)
{
	return encoders_count;
}


/*
 * get module path from id.
 */
string encoder_getpath(unsigned int id)
{
	if(id >= encoders_count)return 0;
	return encoders[id].fname;
}


/*
 * get description string.
 */
string encoder_getname(unsigned int id)
{
	if(id >= encoders_count)return 0;
	return encoders[id].encname;
}


/*
 * initialize encoder plug-in.
 */
int encoder_plugin_initialize(unsigned int id)
{
	fennec_encoder_initialize fnc_initialize;

	if(id >= encoders_count)return 0;
	if(encoders[id].hencoder)return 0;

	reportx("initializing encoder: %s", rt_stepping, encoders[id].fname);
	
	encoders[id].hencoder = sys_library_load(encoders[id].fname);

	if(encoders[id].hencoder == v_error_sys_library_load)return 0;

	fnc_initialize = (fennec_encoder_initialize) sys_library_getaddress(encoders[id].hencoder, "fennec_initialize_encoder");

	if(fnc_initialize == v_error_sys_library_getaddress)
	{
		sys_library_free(encoders[id].hencoder);
		return 0;
	}

	encoders[id].ged.ptype     = fennec_plugintype_audioencoder;
	encoders[id].ged.fiversion = input_plugins_version;
	encoders[id].ged.language_text = strings_list;

	plugin_settings_fillstruct(&encoders[id].ged.fsettings);

	if(!fnc_initialize(&encoders[id].ged))
	{
		sys_library_free(encoders[id].hencoder);
		return 0;
	}

	if(!encoders[id].ged.initialize)
	{
		sys_library_free(encoders[id].hencoder);
		return 0;
	}

	encoders[id].ged.initialize();
	return 1;
}


/*
 * uninitialize encoder plug-in.
 */
int encoder_plugin_uninitialize(unsigned int id)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	report("uninitializing encoder", rt_stepping);

	encoders[id].ged.uninitialize();
	sys_library_free(encoders[id].hencoder);
	encoders[id].hencoder = 0;
	return 1;
}


/*
 * show encoder settings for files (not used).
 */
int encoder_plugin_file_settings(unsigned int id, unsigned long fid, void* odata)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.settings_fileencoder(fid, odata);
}


/*
 * show encoder settings (not used).
 */
int encoder_plugin_encoder_settings(unsigned int id, unsigned long fid, void* odata)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.settings_encoder(fid, odata);
}


/*
 * show about for an index (not used).
 */
int encoder_plugin_about(unsigned int id, unsigned long fid, void* odata)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.about(fid, odata);
}


/*
 * show global settings for files.
 */
int encoder_plugin_global_file_settings(unsigned int id, void* odata)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.settings_fileencoder((unsigned long)fennec_invalid_index, odata);
}


/*
 * show global settings (for streams).
 */
int encoder_plugin_global_encoder_settings(unsigned int id, void* odata)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.settings_encoder((unsigned long)fennec_invalid_index, odata);
}


/*
 * show global about (about encoder in general).
 */
int encoder_plugin_global_about(unsigned int id, void* odata)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.about((unsigned long)fennec_invalid_index, odata);
}


/*
 * set/create output files.
 */
unsigned long encoder_plugin_file_create(unsigned int id, string fname)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	return encoders[id].ged.file_create(fname, fennec_encode_openmode_create);
}


/*
 * write samples to a file.
 */
int encoder_plugin_file_write(unsigned int id, unsigned long fid, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	encoders[id].ged.file_write(fid, pcmin, sfreq, sbps, schannels, dsize);
	return 1;
}


/*
 * close file (end encoding).
 */
int encoder_plugin_file_close(unsigned int id, unsigned long fid)
{
	if(id >= encoders_count)return 0;
	if(!encoders[id].hencoder)return 0;

	encoders[id].ged.file_close(fid);
	return 1;
}


/*
 * refresh encoder list.
 */
int local_encoders_refresh(const string spath)
{
	int                          foundfile;
	t_sys_fs_find_handle         find_handle;
	string                       spath_wild;
	string                       plg_path;
	letter                       plg_name[320];
	letter                       plg_init_info[1024];
	struct general_encoder_data  ged;
	unsigned long                rsize = 0;


	t_sys_library_handle         plg_handle;

	fennec_encoder_initialize    plg_func_initialize;

	/* allocate memory for search path + extension + wildcard (dir\*.lib) */

	spath_wild = (string) sys_mem_alloc((str_len(spath) + str_len(v_sys_lbrary_extension) + 16) * sizeof(letter));

	/* make search path */

	str_cpy(spath_wild, spath);
	str_cat(spath_wild, uni("*"));
	str_cat(spath_wild, v_sys_lbrary_extension);

	reportx("searching for encoders: %s", rt_stepping, spath_wild);

	/* allocate memory for the file name buffer */

	plg_path = (string)sys_mem_alloc(1024 * sizeof(letter));

	/* start the search process */

	foundfile = sys_fs_find_start(spath_wild, plg_name, 300, &find_handle);

	while(foundfile)
	{
		memset(plg_path, 0, 1024 * sizeof(letter));
		str_cpy(plg_path, spath);
		str_cat(plg_path, plg_name);

		plg_handle = sys_library_load(plg_path);

		if(plg_handle == v_error_sys_library_load)goto ignore_cache_plugin;

		plg_func_initialize = (fennec_encoder_initialize) sys_library_getaddress(plg_handle, "fennec_initialize_encoder");

		if(plg_func_initialize == v_error_sys_library_getaddress)
		{
			sys_library_free(plg_handle);
			goto ignore_cache_plugin;
		}

		ged.ptype     = fennec_plugintype_audioencoder;
		ged.fiversion = input_plugins_version;

		if(!plg_func_initialize(&ged))goto ignore_cache_plugin;

//		strcpy(plg_info.fname, plg_name); /* we're searching in the 'input plugins' folder, so full path shouldn't be used */

		if(encoders_count >= encoders_size)
		{
			unsigned int add_e_count;

			encoders_size += encoders_addition;
			encoders = (struct encoder_plugin*) sys_mem_realloc(encoders, encoders_size * sizeof(struct encoder_plugin));
		
			for(add_e_count = 1;  add_e_count <= encoders_addition;  add_e_count++)
			{
				encoders[encoders_size - add_e_count].hencoder = 0;
			}
		}

		str_cpy(encoders[encoders_count].fname, plg_path);
		encoders[encoders_count].hencoder = 0;

		/* initialize plugin */
		ged.initialize();

		/* get information */
		plg_init_info[0] = 0;
		rsize = 256;

		ged.get_initialization_info(fennec_information_basic, plg_init_info, &rsize);
		str_ncpy(encoders[encoders_count].encname, plg_init_info, min(rsize + 1, 256));

		if(rsize)
		{
		//	strcpy(plg_info.pluginname, plg_init_info);
		}
		
		encoders_count++;

		ged.uninitialize();
		sys_library_free(plg_handle);

ignore_cache_plugin:
		foundfile = sys_fs_find_next(plg_name, 300, find_handle);
	}

	sys_fs_find_close(find_handle);

	sys_mem_free(spath_wild);
	sys_mem_free(plg_path);
	return 1;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
