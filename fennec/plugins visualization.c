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

#include "fennec main.h"
#include "plugins.h"




/* data ---------------------------------------------------------------------*/

int                                 vis_initialized = 0;
t_sys_library_handle                current_vis_handle = 0;
struct general_visualization_data   current_vis_data;




/* prototypes ---------------------------------------------------------------*/

unsigned long callc visualizations_getdata(int id, int cid, void *mdata, void *sdata);





/* code ---------------------------------------------------------------------*/


/*
 * initialize visualization.
 */
int visualizations_initialize(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_visuals_initialize  hfunc;
	letter                     plgname[v_sys_maxpath];
	letter                     plgtitle[v_sys_maxpath];

	if(vis_initialized) return 0;//visualizations_uninitialize();

	fennec_get_abs_path(plgname, fname);

	fhandle = sys_library_load(plgname);

	if(fhandle == v_error_sys_library_load)return -1; /* error: invalid skin file "[fname]" */

	hfunc = (fennec_visuals_initialize) sys_library_getaddress(fhandle, vis_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return -2; /* error: cannot load skin file "[fname]" */
	}

	current_vis_data.fiversion    = plugin_version;
	plugin_settings_fillstruct(&current_vis_data.fsettings);
	current_vis_data.getdata      = visualizations_getdata;
	current_vis_data.shared       = &fennec;
	current_vis_data.refresh      = 0;
	current_vis_data.uninitialize = 0;
	current_vis_data.settings     = 0;
	current_vis_data.about        = 0;

	hfunc(&current_vis_data, plgtitle);

	current_vis_handle = fhandle;

	vis_initialized = 1;
	return 0;
}


/*
 * uninitialize visualization.
 */
int visualizations_uninitialize()
{
	if(!vis_initialized)return 1; /* warning: already uninitialized */
	if(current_vis_handle)
	{
		if(current_vis_data.uninitialize)current_vis_data.uninitialize(0, 0);	
		sys_pass();
		sys_library_free(current_vis_handle);
	}

	current_vis_handle = 0;
	vis_initialized = 0;
	return 0;
}


/*
 * analyze module.
 */
int visualizations_checkfile(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_visuals_initialize  hfunc;
	letter                     plgname[v_sys_maxpath];

	fennec_get_abs_path(plgname, fname);
	
	fhandle = sys_library_load(plgname);

	if(fhandle == v_error_sys_library_load)return 0;

	hfunc = (fennec_visuals_initialize) sys_library_getaddress(fhandle, vis_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return 0;
	}

	sys_library_free(fhandle);
	return 1;
}


/*
 * call visualization's callback function
 * for notifications.
 */
int visualizations_refresh(int rlevel)
{
	if(vis_initialized && current_vis_data.refresh)
	{
		current_vis_data.refresh(0, 0);
		return 1;
	}
	return 0;
}


/*
 * show settings for current visualization.
 */
int visualizations_settings(void* fdata)
{
	if(vis_initialized && current_vis_data.settings)
	{
		current_vis_data.settings(fdata);
		return 1;
	}
	return 0;
}


/*
 * show about for current visualization.
 */
int visualizations_about(void* fdata)
{
	if(vis_initialized && current_vis_data.about)
	{
		current_vis_data.about(fdata);
		return 1;
	}
	return 0;
}


/*
 * get data function.
 */
unsigned long callc visualizations_getdata(int id, int cid, void *mdata, void *sdata)
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
		return skins_function_getdata(id, mdata, sizeof(void*));

	case get_color:
		return skins_function_getdata(id, mdata, cid);
	
	}
	return 0;
}
	

/*-----------------------------------------------------------------------------
 fennec, july 2007.
-----------------------------------------------------------------------------*/