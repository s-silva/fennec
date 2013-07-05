/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.2
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
#include "..\include\ids.h"


int call_function(int id, int data, void *a, void *b)
{
	int v;

	switch(id)
	{
	case show_open_experimental:
		fennec_show_file_dialog(file_dialog_openfiles, file_dialog_mode_experimental, 0, 0);
		break;

	case save_setting_int:
		plugin_settings_setnum((char*)a, "", data);
		break;

	case save_setting_text:
		plugin_settings_set((char*)a, "", (char*)b);
		break;

	case load_setting_int:
		v = data;
		plugin_settings_getnum((char*)a, "", &v, 0, 0);
		return v;

	case load_setting_text:
		plugin_settings_get((char*)a, "", (char*)b, data);
		break;

	case call_dsp_getoutput_channels:
		return dsp_getoutput_channels(data);

	case call_fennec_memory_alloc:
		*((void**)b) = sys_mem_alloc(data);
		break;

	case call_fennec_memory_realloc:
		*((void**)b) = sys_mem_realloc(a, data);
		break;

	case call_fennec_memory_free:
		sys_mem_free(a);
		break;

	case call_audio_preview_action:
		audio_preview_action(data, a);
		break;

	case call_visualizations_refresh:
		visualizations_refresh(data);
		break;

	case call_visualizations_select_none:
		visualizations_uninitialize();
		settings.visualizations.selected[0] = 0;
		break;

	case call_videoout_refresh:
		videoout_refresh(data);
		break;

	case call_videoout_initialize:
		videoout_initialize(settings.videooutput.selected);
		break;

	case call_videoout_uninitialize:
		videoout_uninitialize();
		break;

	case call_videoout_test: /* test video compatibility */
		PostMessage(window_main, WM_USER + 124, 0, 0);
		break;

	case call_visualizations_select_next:
		if(settings.visualizations.selected[0])
			visualizations_initialize(settings.visualizations.selected);
		break;

	case call_visualizations_select_prev:
		
		break;

	case call_visualizations_preset_next:
		
		break;

	case call_visualizations_preset_prev:
		
		break;
	
	case call_video_show:
		skins_function_getdata(set_video_window, (void*)1, 0);
		break;

	case call_video_getvdec:
		{
			unsigned long        pid, fid;

			if(!a)break;

			audio_get_current_ids(&pid, &fid);
			internal_input_plugin_getvideo(pid, fid, (struct video_dec**)a);
		}
		break;


	default: return -1; /* invalid function id */
	}
	return 0;
}



/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/