/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.2 - Video Output Plugin Interface
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




/* data ---------------------------------------------------------------------*/

int                                 videoout_initialized = 0;
t_sys_library_handle                current_videoout_handle = 0;
struct general_videoout_data        current_videoout_data;




/* prototypes ---------------------------------------------------------------*/

unsigned long callc videoout_getdata(int id, int cid, void *mdata, void *sdata);





/* code ---------------------------------------------------------------------*/


/*
 * initialize video output plugin.
 */
int videoout_initialize(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_videoout_initialize  hfunc;
	letter                     plgname[v_sys_maxpath];
	letter                     plgtitle[v_sys_maxpath];

	if(videoout_initialized)
	{
		if(current_videoout_data.initialize)
			current_videoout_data.initialize();
		return 0;// videoout_uninitialize();
	}

	fennec_get_plugings_path(plgname);
	str_cat(plgname, fname);
	

	fhandle = sys_library_load(plgname);

	if(fhandle == v_error_sys_library_load)return -1; /* error: invalid skin file "[fname]" */

	hfunc = (fennec_videoout_initialize) sys_library_getaddress(fhandle, videoout_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return -2; /* error: cannot load skin file "[fname]" */
	}

	current_videoout_data.fiversion    = plugin_version;
	plugin_settings_fillstruct(&current_videoout_data.fsettings);
	current_videoout_data.getdata      = videoout_getdata;
	current_videoout_data.shared       = &fennec;
	current_videoout_data.refresh      = 0;
	current_videoout_data.uninitialize = 0;
	current_videoout_data.settings     = 0;
	current_videoout_data.about        = 0;

	hfunc(&current_videoout_data, plgtitle);

	current_videoout_handle = fhandle;

	videoout_initialized = 1;
	return 0;
}


/*
 * uninitialize video output plugin.
 */
int videoout_uninitialize()
{
	if(!videoout_initialized)return 1; /* warning: already uninitialized */
	if(current_videoout_handle)
	{
		if(current_videoout_data.uninitialize)current_videoout_data.uninitialize(0, 0);	
		sys_pass();
		sys_library_free(current_videoout_handle);
	}

	current_videoout_handle = 0;
	videoout_initialized = 0;
	return 0;
}


/*
 * analyze module.
 */
int videoout_checkfile(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_visuals_initialize  hfunc;
	letter                     plgname[v_sys_maxpath];

	fennec_get_plugings_path(plgname);
	str_cat(plgname, fname);
	
	fhandle = sys_library_load(plgname);

	if(fhandle == v_error_sys_library_load)return 0;

	hfunc = (fennec_visuals_initialize) sys_library_getaddress(fhandle, videoout_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return 0;
	}

	sys_library_free(fhandle);
	return 1;
}


/*
 * display video, called by the skin/fennec.
 */
int videoout_display(void *data, int mode, void *modedata)
{
	if(videoout_initialized && current_videoout_data.display)
	{
		current_videoout_data.display(data, mode, modedata);
		return 1;
	}
	return 0;
}


/*
 * set basic information about plugin.
 */
int videoout_setinfo(int frame_w, int frame_h, int frame_rate, int colors)
{
	if(videoout_initialized && current_videoout_data.setinfo)
	{
		current_videoout_data.setinfo(frame_w, frame_h, frame_rate, colors);
		return 1;
	}
	return 0;
}


/*
 * set advanced information about plugin.
 */
int videoout_setinfo_ex(int id, void *data, int (*afunc)())
{
	if(videoout_initialized && current_videoout_data.setinfo_ex)
	{
		current_videoout_data.setinfo_ex(id, data, afunc);
		return 1;
	}
	return 0;
}

/*
 * place text on the video (subtitles, commnets etc.), pass null 'data' 
 * to remove it.
 */
int videoout_pushtext(int textmode, int addmode, void *data, int x, int y, int w, int h)
{
	if(videoout_initialized && current_videoout_data.pushtext)
	{
		current_videoout_data.pushtext(textmode, addmode, data, x, y, w, h);
		return 1;
	}
	return 0;
}


/*
 * show settings for current video output plugin.
 */
int videoout_settings(void* fdata)
{
	if(videoout_initialized && current_videoout_data.settings)
	{
		current_videoout_data.settings(fdata);
		return 1;
	}
	return 0;
}


/*
 * show settings for current video output plugin.
 */
int videoout_refresh(int rlevel)
{
	if(videoout_initialized && current_videoout_data.refresh)
	{
		current_videoout_data.refresh(rlevel);
		return 1;
	}
	return 0;
}


/*
 * show information about current video output plugin.
 */
int videoout_about(void* fdata)
{
	if(videoout_initialized && current_videoout_data.about)
	{
		current_videoout_data.about(fdata);
		return 1;
	}
	return 0;
}


/*
 * get data from the plugin.
 */
unsigned long callc videoout_getdata(int id, int cid, void *mdata, void *sdata)
{
	switch(id)
	{
	case get_playlist:
	case get_visual:
	case get_visual_dc:
	case get_visual_x:
	case get_visual_y:
	case get_visual_w:
	case get_visual_h:
	case set_msg_proc:
	case get_visual_winproc:
	case get_window_video:
	case get_window_video_dc:
	case get_window_video_rect:
	case get_window_video_state:
	case set_video_window:
		return skins_function_getdata(id, mdata, sizeof(void*));

	case get_color:
		return skins_function_getdata(id, mdata, cid);
	
	}
	return 0;
}


/*-----------------------------------------------------------------------------
 fennec, march 2009.
-----------------------------------------------------------------------------*/