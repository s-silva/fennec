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
#include "plugins.h"
#include "fennec_audio.h"
#include "../include/ids.h"





/* structs ------------------------------------------------------------------*/

struct dsp_plugindata /* paths and text data are redirected to settings.h */
{
	t_sys_library_handle     handle;
	struct general_dsp_data  data;
	void                    *lastbuffer;
};




/* prototypes ---------------------------------------------------------------*/

void local_dsp_refreshcache(const string plgpath);




/* data ---------------------------------------------------------------------*/

struct dsp_plugindata loaded_dsp_plugins[16];
unsigned char dsp_init = 0;




/* code ---------------------------------------------------------------------*/


/*
 * initialize dsp engine.
 */
int dsp_initialize(void)
{
	fennec_effects_initialize  initfunc;
	unsigned int               i;
	letter                     spath[v_sys_maxpath];

	if(dsp_init)return 0;


	if(!settings.dsp_plugins.plugins_count)local_dsp_refreshcache(0);
	if(!settings.dsp_plugins.plugins_count) /* still null? */ return 0;

	for(i=0; i<16; i++)
	{
		if(i < settings.dsp_plugins.plugins_count)
		{
			fennec_get_abs_path(spath, settings.dsp_plugins.plugins[i].path);
			loaded_dsp_plugins[i].handle = sys_library_load(spath);
			
			if(loaded_dsp_plugins[i].handle == v_error_sys_library_load)
				loaded_dsp_plugins[i].handle = 0;

			initfunc = (fennec_effects_initialize) sys_library_getaddress(loaded_dsp_plugins[i].handle, "fennec_initialize_dsp");
			
			if(initfunc != v_error_sys_library_getaddress)
			{
				loaded_dsp_plugins[i].data.fiversion     = plugin_version;
				loaded_dsp_plugins[i].data.ptype         = fennec_plugintype_audiodsp;
				loaded_dsp_plugins[i].data.language_text = strings_list;
				loaded_dsp_plugins[i].data.callengine    = dsp_getmessage;


				plugin_settings_fillstruct(&loaded_dsp_plugins[i].data.fsettings);

				initfunc(&loaded_dsp_plugins[i].data, settings.dsp_plugins.plugins[i].name);
				
				if(loaded_dsp_plugins[i].data.initialize)loaded_dsp_plugins[i].data.initialize();
			}
		}else{
			loaded_dsp_plugins[i].handle = 0;
		}
	}

	dsp_init = 1;
	return 1;
}


/*
 * initialize recently added plugins only.
 */
int dsp_reinitialize(void)
{
	letter                     abs_path[v_sys_maxpath];
	fennec_effects_initialize  initfunc;
	register unsigned int      i;

	if(!dsp_init)return 0;

	for(i=0; i<16; i++)
	{
		if((!loaded_dsp_plugins[i].handle) && (settings.dsp_plugins.plugins[i].index < 16))
		{
			if(i >= settings.dsp_plugins.plugins_count)return 1;

			fennec_get_abs_path(abs_path, settings.dsp_plugins.plugins[i].path);
			loaded_dsp_plugins[i].handle = sys_library_load(abs_path);
			
			if(loaded_dsp_plugins[i].handle == v_error_sys_library_load)
				loaded_dsp_plugins[i].handle = 0;

			initfunc = (fennec_effects_initialize) sys_library_getaddress(loaded_dsp_plugins[i].handle, "fennec_initialize_dsp");
			if(initfunc != v_error_sys_library_getaddress)
			{
				loaded_dsp_plugins[i].data.fiversion     = plugin_version;
				loaded_dsp_plugins[i].data.ptype         = fennec_plugintype_audiodsp;
				loaded_dsp_plugins[i].data.language_text = strings_list;
				loaded_dsp_plugins[i].data.callengine    = dsp_getmessage;

				plugin_settings_fillstruct(&loaded_dsp_plugins[i].data.fsettings);

				initfunc(&loaded_dsp_plugins[i].data, settings.dsp_plugins.plugins[i].name);
				
				if(loaded_dsp_plugins[i].data.initialize)loaded_dsp_plugins[i].data.initialize();
			}
		}
	}
	return 1;
}


/*
 * uninitialize dsp engine.
 */
int dsp_uninitialize(void)
{
	register unsigned int i;

	if(!dsp_init)return 0;

	for(i=0; i<16; i++)
	{
		if(loaded_dsp_plugins[i].handle)
		{
			/* first tell the plugin that it's being terminated */
			if(loaded_dsp_plugins[i].data.uninitialize)
				loaded_dsp_plugins[i].data.uninitialize();

			/* free the library (dsp module) */
			sys_library_free(loaded_dsp_plugins[i].handle);

			/* set an empty mark (clarity) */
			loaded_dsp_plugins[i].handle = 0;
		}
	}
	dsp_init = 0;
	return 1;
}


/*
 * initialize plug-in by index.
 */
int dsp_initialize_index(unsigned long pid, unsigned long id)
{
	if(!dsp_init)return 0;
	if(pid > 16)return 0; /* kids only :-D */
	if(!loaded_dsp_plugins[pid].handle)return 0;

	if(!loaded_dsp_plugins[pid].data.open)return 0;

	loaded_dsp_plugins[pid].data.open(id);
	return 1;
}


/*
 * uninitialize plug-in by index.
 */
int dsp_uninitialize_index(unsigned long pid, unsigned long id)
{
	if(!dsp_init)return 0;
	if(pid > 16)return 0;
	if(!loaded_dsp_plugins[pid].handle)return 0;

	if(!loaded_dsp_plugins[pid].data.close)return 0;

	loaded_dsp_plugins[pid].data.close(id);
	return 1;
}


/*
 * show settings.
 */
int dsp_showconfig(unsigned int pid, void* pdata)
{
	if(!dsp_init)return 0;
	if(pid > 16)return 0;
	if(!loaded_dsp_plugins[pid].handle)return 0;

	if(!loaded_dsp_plugins[pid].data.settings)return 0;

	loaded_dsp_plugins[pid].data.settings(pdata);
	return 1;
}


/*
 * show about.
 */
int dsp_showabout(unsigned int pid, void* pdata)
{
	if(!dsp_init)return 0;
	if(pid > 16)return 0;
	if(!loaded_dsp_plugins[pid].handle)return 0;

	if(!loaded_dsp_plugins[pid].data.about)return 0;

	loaded_dsp_plugins[pid].data.about(pdata);
	return 1;
}


/*
 * send message to a plug-in.
 */
int dsp_sendmessage(unsigned int pid, int id, int a, void* b, double d)
{
	if(!dsp_init)return 0;
	if(pid > 16)return 0;
	if(!loaded_dsp_plugins[pid].handle)return 0;

	if(!loaded_dsp_plugins[pid].data.messageproc)return 0;

	return loaded_dsp_plugins[pid].data.messageproc(id, a, b, d);
}

int dsp_getoutput_channels(int inchannels)
{
	unsigned int i, j, oc = inchannels, ti;

	if(!dsp_init)return oc;

	for(i=0; i<min(16, settings.dsp_plugins.plugins_count); i++)
	{
		for(j=0; j<min(16, settings.dsp_plugins.plugins_count); j++)
		{
			if((unsigned int)settings.dsp_plugins.plugins[j].index == i)
			{
				if(loaded_dsp_plugins[j].handle && loaded_dsp_plugins[j].data.messageproc)
				{
					ti = loaded_dsp_plugins[j].data.messageproc(plugin_message_getchannels, oc, 0, 0);
					if(ti) oc = ti;
				}
			}
		}
	}
	return oc;
}

/*
 * receive message.
 */
int callc dsp_getmessage(int id, int a, void* b, double d)
{
	/*
	switch(id)
	{

	}
	*/

	return 0;
}



/*
 * memory reallocation for extended sampling.
 */
void* callc local_func_realloc(void *imem, unsigned int rsize)
{
	return sys_mem_realloc(imem, rsize);
}


/*
 * process samples.
 */
void* dsp_process(unsigned long id, unsigned long *bsize, unsigned long freqency, unsigned long bitspersample, unsigned long channels, void *sbuffer, unsigned int apointer, unsigned int avbsize)
{
	register unsigned int i, j;
	void *pdata = sbuffer;

	if(!dsp_init)return sbuffer;

	/*
	for(i=0; i<min(16, settings.dsp_plugins.plugins_count); i++)
	{
		loaded_dsp_plugins[i].lastbuffer = 0;
	}
	*/

	for(i=0; i<min(16, settings.dsp_plugins.plugins_count); i++)
	{
		for(j=0; j<min(16, settings.dsp_plugins.plugins_count); j++)
		{
			if((unsigned int)settings.dsp_plugins.plugins[j].index == i)
			{
				if(loaded_dsp_plugins[j].handle && loaded_dsp_plugins[j].data.process)
				{
					pdata = loaded_dsp_plugins[j].data.process(id, bsize, freqency, bitspersample, channels, pdata, apointer, avbsize, local_func_realloc);
					loaded_dsp_plugins[j].lastbuffer = pdata;
				}
			}
		}
	}
	return pdata; /* reallocated pointer */ 
}


/*
 * refresh cache (search new plug-ins).
 */
void local_dsp_refreshcache(const string plgpath)
{
	register unsigned int      i = 0, j = 0;
	letter                     spath[v_sys_maxpath];
	letter                     rname[v_sys_maxpath];
	letter                     fname[v_sys_maxpath];
	t_sys_fs_find_handle       fhandle;
	int                        foundfile;
	t_sys_library_handle       libhandle;
	fennec_effects_initialize  initfunc;
	

	/* clear plugins memory */
	settings.dsp_plugins.plugins_count = 0;

	/* prepare search path */

	if(plgpath)
		fennec_get_abs_path(spath, plgpath);
	else
		fennec_get_plugings_path(spath);

	str_cat(spath, uni("*"));
	str_cat(spath, v_sys_lbrary_extension);

	/* start finding process */

	foundfile = sys_fs_find_start(spath, rname, sizeof(rname), &fhandle);

	while(i < 16)
	{
		if(!foundfile)break;

		if(plgpath)
			fennec_get_abs_path(fname, plgpath);
		else
			fennec_get_plugings_path(fname);
		str_cat(fname, rname);

		libhandle = sys_library_load(fname);
		if(libhandle == v_error_sys_library_load)goto point_next;

		initfunc = (fennec_effects_initialize) sys_library_getaddress(libhandle, "fennec_initialize_dsp");
		if(initfunc == v_error_sys_library_getaddress)
		{
			sys_library_free(libhandle);
			goto point_next;
		}

		loaded_dsp_plugins[j].data.fiversion     = plugin_version;
		loaded_dsp_plugins[j].data.ptype         = fennec_plugintype_audiodsp;
		loaded_dsp_plugins[i].data.language_text = strings_list;
		loaded_dsp_plugins[i].data.callengine    = dsp_getmessage; 

		plugin_settings_fillstruct(&loaded_dsp_plugins[j].data.fsettings);


		initfunc(&loaded_dsp_plugins[j].data, settings.dsp_plugins.plugins[j].name);
		
		if(!loaded_dsp_plugins[j].data.initialize)
		{
			sys_library_free(libhandle);
			goto point_next;
		}

		settings.dsp_plugins.plugins[j].name[0] = 0;

		fennec_get_rel_path(settings.dsp_plugins.plugins[j].path, fname);
		settings.dsp_plugins.plugins[j].index = (char)(j + 20); /* not used but the index is 'j' */
		settings.dsp_plugins.plugins_count++;

		j++;
		i++;

point_next:	
		foundfile = sys_fs_find_next(rname, sizeof(rname), fhandle);
	}

	sys_fs_find_close(fhandle);
	return;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
