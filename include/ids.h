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

enum calls
{
	 show_open_experimental
	,save_setting_int
	,save_setting_text
	,load_setting_int
	,load_setting_text
	,call_dsp_getoutput_channels
	,call_fennec_memory_alloc
	,call_fennec_memory_realloc
	,call_fennec_memory_free
	,call_audio_preview_action
	,call_visualizations_refresh
	,call_visualizations_select_none
	,call_visualizations_select_next
	,call_visualizations_select_prev
	,call_visualizations_preset_next
	,call_visualizations_preset_prev
	,call_videoout_refresh
	,call_videoout_initialize
	,call_videoout_uninitialize
	,call_videoout_test
	,call_video_show
	,call_video_getvdec
};

enum dsp_plugin_messages
{
	 plugin_message_getchannels
	,plugin_message_getsamplerate
	,plugin_message_settings_init
	,plugin_message_settings_close
	,plugin_message_settings_set
	,plugin_message_settings_save
	,plugin_message_settings_get
};

enum dsp_engine_messages
{
	 dsp_engine_settings_show
	,dsp_engine_add_control
};

enum dsp_controls
{
	 dsp_control_slider_h
	,dsp_control_slider_v
	,dsp_control_knob
	,dsp_control_knob_center
	,dsp_control_toggle
	,dsp_control_status
};



/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/