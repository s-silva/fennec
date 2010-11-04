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
#include "keyboard.h"




/* defines ------------------------------------------------------------------*/

#define settings_filename   uni("settings.fsd")  /* file type : fennec settings data */
#define settings_signature       0x445346        /* file signature 'FSD" */
#define settings_signature_fail  0x445344        /* file signature 'FSF" */

#define settings_shift_end(s, c, z) (memmove(settings_data + (s + c), settings_data + s, z))




/* prototypes ---------------------------------------------------------------*/

void setlkey(unsigned int i, unsigned short k, unsigned short p);
void setgkey(unsigned int i, unsigned short k, unsigned short p);
int  settings_createfile(void);
int trans_rest(transcoder_settings *trans);



/* data ---------------------------------------------------------------------*/

t_sys_file_handle hsettingsfile;
int               settingsfilecreated = 0;
letter            settingsfilepath[v_sys_maxpath];
char*             settings_data;

settings_struct    settings;




/* code ---------------------------------------------------------------------*/
/* start verification */

int settings_verify(void)
{
	int32_t        filesign;
	unsigned long  nbr, sp;

	sp = sys_file_tell(hsettingsfile);

	sys_file_seek(hsettingsfile, 0);
	nbr = sys_file_read(hsettingsfile, &filesign, sizeof(filesign));

	if(nbr == sizeof(filesign))
	{
		if(filesign == settings_signature_fail)
		{
			int rmode;
			rmode = basewindows_show_restore_settings(0);

			/* restoring modes */

			if(rmode & fennec_restore_default) settings_default();
			if(rmode & fennec_restore_r_plugin)
			{
				letter  frpath[v_sys_maxpath];

				fennec_get_plugings_path(frpath);
				str_cat(frpath, uni("settings.txt"));
				
				_wremove(frpath);
			}

			if(rmode & fennec_restore_r_media)
			{
				letter  frpath[v_sys_maxpath];

				str_cpy(frpath, fennec_get_path(0, 0));
				str_cat(frpath, uni("/playlist.txt"));
				
				_wremove(frpath);

				str_cpy(frpath, fennec_get_path(0, 0));
				str_cat(frpath, uni("/media library.fsd"));
				
				_wremove(frpath);
			}

			if(rmode & fennec_restore_r_skin)
			{
				t_sys_fs_find_handle shandle;
				letter  skins_path[v_sys_maxpath];
				letter  cskin_path[v_sys_maxpath];
				letter  fskin_path[v_sys_maxpath];
				int     sfound;

				str_cpy(skins_path, fennec_get_path(0, 0));
				str_cat(skins_path, uni("/skins/*"));
				str_cat(skins_path, uni(".fsd"));

				sfound = sys_fs_find_start(skins_path, cskin_path, sizeof(cskin_path), &shandle);

				while(sfound)
				{
					str_cpy(fskin_path, fennec_get_path(0, 0));
					str_cat(fskin_path, uni("/skins/"));
					str_cat(fskin_path, cskin_path);
					_wremove(fskin_path);

					sfound = sys_fs_find_next(cskin_path, sizeof(cskin_path), shandle);
				}

				sys_fs_find_close(shandle);
			}



			if(rmode & fennec_restore_r_main)
			{
				sys_file_close(hsettingsfile);
				if(settingsfilepath[0])
					_wremove(settingsfilepath);

				settings_load();
			}

		}else{
			if(filesign != settings_signature) return -1;
		}
	}else{
		return -1;
	}


	/* write fail signature, so we can recognize buggy exits */

	sys_file_seek(hsettingsfile, 0);
	filesign = settings_signature_fail;
	nbr = sys_file_write(hsettingsfile, &filesign, sizeof(filesign));

	sys_file_seek(hsettingsfile, sp);
	return 0;
}

int settings_set_as_bad(void)
{
	int32_t        filesign;
	unsigned long  nbr, sp;

	sp = sys_file_tell(hsettingsfile);

	sys_file_seek(hsettingsfile, 0);
	filesign = settings_signature_fail;
	nbr = sys_file_write(hsettingsfile, &filesign, sizeof(filesign));

	sys_file_seek(hsettingsfile, sp);
	return 0;
}


int settings_set_as_fine(void)
{
	int32_t        filesign;
	unsigned long  nbr, sp;

	sp = sys_file_tell(hsettingsfile);

	sys_file_seek(hsettingsfile, 0);
	filesign = settings_signature;
	nbr = sys_file_write(hsettingsfile, &filesign, sizeof(filesign));

	sys_file_seek(hsettingsfile, sp);
	return 0;
}

/*
 * load settings file (if not found, create new file with data).
 * return: (-1) - error.
 *           1  - new file created.
 */
int settings_load(void)
{
	int32_t        filesign;
	unsigned long  nbr;

	/* already opened? */

	report("settings: loading", rt_stepping);

	if(!settingsfilecreated)
	{
		if(!settings_createfile())
		{
			settings_default();
			settings_save();

			report("settings: restored", rt_info);
			return 1; /* info: restored to default */
		}

		settings.appending_values.input_plugins_size = 0;
	}

	/* start verification */

	sys_file_seek(hsettingsfile, 0);
	nbr = sys_file_read(hsettingsfile, &filesign, sizeof(filesign));

	if((nbr != sizeof(filesign)) || ((filesign != settings_signature) && (filesign != settings_signature_fail))) return -1;

	/* load other stuff */
	
	nbr = sys_file_read(hsettingsfile, &settings, sizeof(settings));

	if(nbr != sizeof(settings)) return -1;

	/* allocate memory for variable sized settings */

	if(settings_data)sys_mem_free(settings_data);

	settings_data = (char*) sys_mem_alloc(settings.appending_values.appending_size + 10 /* can be zero */);

	/* read settings */

	sys_file_seek(hsettingsfile, settings.appending_values.appending_start);

	nbr = sys_file_read(hsettingsfile, settings_data, settings.appending_values.appending_size);

	if(nbr != settings.appending_values.appending_size) return -1;

	/* alright */

	report("settings: loading, fine", rt_stepping);
	return 0;
}


/*
 * save settings to the file.
 * return: (-1) - error.
 */
int settings_save(void)
{
	uint32_t       filesign;
	unsigned long  nbr;

	report("settings: saving", rt_stepping);

	if(!settingsfilecreated)
	{
		if(!settings_createfile()) return -1;
	}

	/* write signature */

	sys_file_seek(hsettingsfile, 0);

	filesign = settings_signature;
	sys_file_write(hsettingsfile, &filesign, sizeof(filesign));


	/* set header values */

	settings.appending_values.appending_start        = sizeof(unsigned long) + sizeof(settings);
	settings.appending_values.appending_end          = settings.appending_values.appending_start
											         + settings.appending_values.appending_size;

	settings.appending_values.equalizer_presets_start = settings.appending_values.input_plugins_size;
	
	settings.appending_values.input_plugins_start     = 0;

	/* variable values */

	/* <dep> win */

	if(IsIconic(window_main))
	{
		settings.environment.main_window_state = setting_window_minimized;
	}else{
		settings.environment.main_window_state = setting_window_normal;
	}

	/* <dep> win */

	
	/* write settings header */

	nbr = sys_file_write(hsettingsfile, &settings, sizeof(settings));

	if(nbr != sizeof(settings)) return -1;

	/* write settings header */

	nbr = sys_file_write(hsettingsfile, settings_data, settings.appending_values.appending_size);

	if(nbr != settings.appending_values.appending_size) return -1;

	/* no more data */

	sys_file_seteof(hsettingsfile);

	/* alright :) */

	report("settings: saving, fine", rt_stepping);
	return 0;
}


/*
 * restore default settings (header only).
 */
int settings_default(void)
{
	unsigned int i, j;
	char         driveid;

	report("settings: setting default values", rt_stepping);

	/* paths */

	str_cpy(settings.plugins.input_path, uni("\\plugins\\"));
	str_cpy(settings.language_packs.pack_path, uni("\\packs\\english.txt"));

	/* set maximum volume */

	for(i=0; i<(sizeof(settings.player.volume) / sizeof(settings.player.volume[0])); i++)
		settings.player.volume[i] = 1.0;

	/* playlist stuff */

	settings.player.auto_switching         = 1; /* auto switching: on **/
	settings.player.playlist_repeat_list   = 0; /* annoying! */
	settings.player.playlist_repeat_single = 0; /* oh! it sux */
	settings.player.playlist_shuffle_rate  = 2; /* two round shuffle */
	settings.player.playlist_shuffle       = 0; /* speed! */
	settings.player.last_raw_samplerate    = 44100;
	settings.player.last_raw_channels      = 2;
	settings.player.last_raw_bps           = 64;

	settings.player.video_tested             = 0;
	settings.player.video_window_manual_show = 0;


	/* general */
	
	settings.general.allow_multi_instances    = 0;
	settings.general.always_on_top            = 0;
	settings.general.auto_play                = 1;
	settings.general.base_priority            = setting_priority_normal;
	settings.general.scroll_title_taskbar     = 1;
	settings.general.show_splash              = 0;
	settings.general.threads_priority         = setting_priority_high;
	settings.general.use_advanced_open = 0;

	/* formatting */

	str_cpy(settings.formatting.main_title,      uni("Fennec Player 1.0 - [<artist-d> - ][<album-d> - ][<title-r>] ... "));
	str_cpy(settings.formatting.scrolling_title, uni("Fennec Player 1.0 - [<artist-d> - ][<album-d> - ][<title-r>] ... "));
	str_cpy(settings.formatting.playlist_item,   uni("[<artist-d> - ][<title-r>]."));

	/* internal output */

	settings.audio_output.output_device_id = 0;
	settings.audio_output.buffer_memory    = 200;
	settings.audio_output.bit_depth        = 16;
	settings.audio_output.bit_depth_float  = 0;
	settings.audio_output.bit_depth_signed = 1;
	settings.audio_output.noise_reduction  = 0;
	settings.audio_output.perform_effects  = 0;
	
	/* dsp */
	
	settings.dsp_plugins.enable             = 0;
	settings.dsp_plugins.enablenextstartup  = 0;
	settings.dsp_plugins.plugins_count      = 0;

	/* skins */

	settings.skins.selected[0] = 0;

	/* output plug-ins */

	str_cpy(settings.output_plugins.selected, uni("/plugins/directx audio.dll"));

	/* visualizations */

	settings.visualizations.selected[0] = 0;

	/* video output plugins */

	str_cpy(settings.videooutput.selected, uni("video out directx.dll"));

	/* shortcut keys */

	settings.shortcuts.enable_local  = 1;
	settings.shortcuts.enable_global = 1;

	/* local shortcut keys */

	i = 0;

	setlkey(i++, fennec_key_q,           keypurpose_play);
    setlkey(i++, fennec_key_w,           keypurpose_pause);
    setlkey(i++, fennec_key_e,           keypurpose_stop);
    setlkey(i++, fennec_key_r,           keypurpose_load);
    setlkey(i++, fennec_key_left,        keypurpose_rewind);
    setlkey(i++, fennec_key_right,       keypurpose_forward);
    setlkey(i++, fennec_key_y,           keypurpose_previous);
    setlkey(i++, fennec_key_u,           keypurpose_next);
    setlkey(i++, fennec_key_o,           keypurpose_eject);
    setlkey(i++, fennec_key_p,           keypurpose_select);
    setlkey(i++, fennec_key_function5,   keypurpose_panelsw_main);
    setlkey(i++, fennec_key_function6,   keypurpose_panelsw_color);
    setlkey(i++, fennec_key_function7,   keypurpose_panelsw_visualization);
    setlkey(i++, fennec_key_function8,   keypurpose_panelsw_equalizer);
    setlkey(i++, fennec_key_graveaccent, keypurpose_minimize);  
    setlkey(i++, fennec_key_function4,   keypurpose_refresh);  
    setlkey(i++, fennec_key_function9,   keypurpose_conversion);  
    setlkey(i++, fennec_key_function10,  keypurpose_ripping);  
    setlkey(i++, fennec_key_function11,  keypurpose_joining);  
    setlkey(i++, fennec_key_function12,  keypurpose_visualization);
    setlkey(i++, fennec_key_function3,   keypurpose_playlist);  
    setlkey(i++, fennec_key_equals,      keypurpose_volumeup);  
    setlkey(i++, fennec_key_minus,       keypurpose_volumedown);  
    setlkey(i++, fennec_key_a,           keypurpose_addfile);
    setlkey(i++, fennec_key_function2,   keypurpose_preferences);
    setlkey(i++, fennec_key_i,           keypurpose_currenttagging);
    setlkey(i++, fennec_key_tab,         keypurpose_switch_playlist);

    setlkey(i++, fennec_key_minus     | fennec_key_control, keypurpose_volumemin);
    setlkey(i++, fennec_key_equals    | fennec_key_control, keypurpose_volumemax);
    setlkey(i++, fennec_key_q         | fennec_key_shift,   keypurpose_fast_load);
    setlkey(i++, fennec_key_a         | fennec_key_shift,   keypurpose_fast_addfile);
    setlkey(i++, fennec_key_function1 | fennec_key_shift,   keypurpose_keyboardviewer);
    setlkey(i++, fennec_key_tab       | fennec_key_shift,   keypurpose_switch_main);
	setlkey(i++, fennec_key_equals    | fennec_key_shift,   keypurpose_panelnext);
    setlkey(i++, fennec_key_minus     | fennec_key_shift,   keypurpose_panelprevious);	
	
	setlkey(i++, 0, keypurpose_null); /* end */

	/* global shortcut keys */

	i = 0;

	setgkey(i++, fennec_key_home     | fennec_key_control | fennec_key_alternative, keypurpose_play);
    setgkey(i++, fennec_key_pause    | fennec_key_control | fennec_key_alternative, keypurpose_pause);
    setgkey(i++, fennec_key_end      | fennec_key_control | fennec_key_alternative, keypurpose_stop);
    setgkey(i++, fennec_key_insert   | fennec_key_control | fennec_key_alternative, keypurpose_load);
	setgkey(i++, fennec_key_left     | fennec_key_control | fennec_key_alternative, keypurpose_rewind);
    setgkey(i++, fennec_key_right    | fennec_key_control | fennec_key_alternative, keypurpose_forward);
    setgkey(i++, fennec_key_pageup   | fennec_key_control | fennec_key_alternative, keypurpose_previous);
    setgkey(i++, fennec_key_pagedown | fennec_key_control | fennec_key_alternative, keypurpose_next);
	setgkey(i++, fennec_key_pageup   | fennec_key_control | fennec_key_shift      , keypurpose_volumeup);
    setgkey(i++, fennec_key_pagedown | fennec_key_control | fennec_key_shift      , keypurpose_volumedown);
	
	setgkey(i++, 0, keypurpose_null); /* end */

	
	/* conversion */

	trans_rest(&settings.conversion.trans);

	for(i=0; i<16; i++);
		for(j=0; j<32; j++)
		{
			settings.conversion.equalizer_bands.boost[i][j] = 0.0;
			settings.conversion.equalizer_bands.preamp[i]   = 0.0;
		}

	settings.conversion.winplace.showCmd = (unsigned int)-1;

	settings.conversion.last_encoder             = 0;
	settings.conversion.last_equalizer_preset_id = 0;
	settings.conversion.stop_playback            = 0;
	settings.conversion.volume                   = 1.0f;
	settings.conversion.volume_gain              = 0.0f;
	settings.conversion.volume_normalization     = 0;
	settings.conversion.use_equalizer            = 0;

	settings.conversion.last_buffer_size         = 1024;
	settings.conversion.last_path[0]             = 0;

	str_cpy(settings.conversion.last_formatting, uni("[<artist-d> - ][<title-r>]"));
	str_cpy(settings.conversion.last_path,       uni("C:\\"));

	/* ripping */

	trans_rest(&settings.ripping.trans);

	for(i=0; i<16; i++);
		for(j=0; j<32; j++)
		{
			settings.ripping.equalizer_bands.boost[i][j] = 0.0;
			settings.ripping.equalizer_bands.preamp[i]   = 0.0;
		}

	settings.ripping.winplace.showCmd = (unsigned int)-1;

	settings.ripping.last_encoder             = 0;
	settings.ripping.last_equalizer_preset_id = 0;
	settings.ripping.stop_playback            = 1;
	settings.ripping.volume                   = 1.0f;
	settings.ripping.volume_gain              = 0.0f;
	settings.ripping.volume_normalization     = 0;
	settings.ripping.use_equalizer            = 0;

	settings.ripping.last_buffer_size         = 1024;
	settings.ripping.last_path[0]             = 0;

	str_cpy(settings.ripping.last_formatting, uni("[<artist-d> - ][<title-r>]"));
	str_cpy(settings.ripping.last_path,       uni("C:\\"));

	/* joining */

	for(i=0; i<16; i++);
		for(j=0; j<32; j++)
		{
			settings.joining.equalizer_bands.boost[i][j] = 0.0;
			settings.joining.equalizer_bands.preamp[i]   = 0.0;
		}

	settings.joining.last_encoder             = 0;
	settings.joining.last_equalizer_preset_id = 0;
	settings.joining.stop_playback            = 1;
	settings.joining.volume                   = 1.0f;
	settings.joining.volume_gain              = 0.0f;
	settings.joining.volume_normalization     = 0;
	settings.joining.use_equalizer            = 0;

	settings.joining.last_buffer_size         = 64;
	settings.joining.last_load[0]             = 0;
	str_cpy(settings.joining.last_path, uni("C:\\Audio"));

	/* tag editor */
	
	settings.tag_editing.checked_tags[0 ] = 0;
	settings.tag_editing.checked_tags[1 ] = 0;
	settings.tag_editing.checked_tags[2 ] = 0;
	settings.tag_editing.checked_tags[3 ] = 0;
	settings.tag_editing.checked_tags[4 ] = 0;
	settings.tag_editing.checked_tags[5 ] = 0;
	settings.tag_editing.checked_tags[6 ] = 0;
	settings.tag_editing.checked_tags[7 ] = 0;
	settings.tag_editing.checked_tags[8 ] = 0;
	settings.tag_editing.checked_tags[9 ] = 0;
	settings.tag_editing.checked_tags[10] = 0;
	settings.tag_editing.checked_tags[11] = 0;
	settings.tag_editing.checked_tags[12] = 0;
	settings.tag_editing.checked_tags[13] = 0;
	settings.tag_editing.checked_tags[14] = 0;
	settings.tag_editing.checked_tags[15] = 0;
	settings.tag_editing.checked_tags[16] = 0;

	str_cpy(settings.tag_editing.rename_formatting, uni("[<artist-d> - ][<title-r>]"));


	/* find removeable drives, assume that the first removeable 
	   drive is a CD-ROM */

	settings.player.selected_drive = 'C';

	for(driveid = 'C'; driveid <= 'Z'; driveid++)
	{
		letter buf[4] = uni("X:\\");
		buf[0]        = (letter)driveid;

		if(GetDriveType(buf) == DRIVE_CDROM)
		{
			settings.player.selected_drive = driveid;
			break;
		}
	}

	return 0;
}


/*
 * get variable-size data.
 * sid - setting id.
 * iid - index.
 * val - (out) value.
 * vsz - (out) value size.
 */
int settings_data_get(unsigned long sid, unsigned long iid, void *val, unsigned long *vsz)
{
	unsigned short fnsize = 0;

	switch(sid)
	{
	case setting_id_input_plugin:

		if(iid > settings.plugins.input_plugins_count)return -1;

		if(iid > settings.plugins.input_plugins_count)return -1;

		fnsize = sizeof(struct internal_input_plugin);
		*vsz   = sizeof(struct internal_input_plugin);

		memcpy(val, settings_data + (settings.appending_values.input_plugins_start + (iid * sizeof(struct internal_input_plugin))), sizeof(struct internal_input_plugin));
		break;


	case setting_id_equalizer_preset:

		if(iid > settings.player.equalizer_presets)return -1;

		fnsize = sizeof(equalizer_preset);
		*vsz   = (unsigned long)fnsize;

		memcpy(val, settings_data + (settings.appending_values.equalizer_presets_start + (iid * sizeof(equalizer_preset))), sizeof(equalizer_preset));
		break;
	}

	return 0;
}


/*
 * get variable-size data.
 * sid - setting id.
 * val - value.
 * vsz - value size.
 */
int settings_data_add(unsigned long sid, void *val, unsigned long vsz)
{
	unsigned long lsz = 0; /* last size */

	/* this function won't change the values of Settings excluding AppendingValues */

	if(vsz >= 0xFFFF)return 0;

	switch(sid)
	{
	case setting_id_input_plugin:

		lsz = sizeof(struct internal_input_plugin);

		settings_data = (char*) sys_mem_realloc(settings_data, settings.appending_values.appending_size + lsz);

		memmove(settings_data + (settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size + lsz)
			   , settings_data + (settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size)
			   , settings.appending_values.appending_size - (settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size));

		memcpy(settings_data + (settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size)
			  , val, lsz);

		settings.appending_values.appending_size         += lsz;
		settings.appending_values.input_plugins_size      += lsz; 

		settings.appending_values.equalizer_presets_start += lsz;
		break;


	case setting_id_equalizer_preset:
		
		lsz = vsz;

		settings_data = (char*) sys_mem_realloc(settings_data, settings.appending_values.appending_size + lsz);

		settings_shift_end(settings.appending_values.equalizer_presets_start, lsz,
			               settings.appending_values.appending_size - settings.appending_values.equalizer_presets_start);

		memcpy(settings_data + settings.appending_values.equalizer_presets_start,
			val, vsz);

		settings.appending_values.appending_size        += lsz;
		settings.appending_values.equalizer_presets_size += lsz;
		break;


	default:
		return -1;
	}

	return 0;
}


/*
 * set variable-size settings.
 * sid - setting id.
 * iid - index.
 * val - value.
 */
int settings_data_set(unsigned long sid, unsigned long iid, void *val)
{
	int vsz = 1;

	switch(sid)
	{
	case setting_id_equalizer_preset: /* rename equalizer preset (changing ain't allowed) */
		
		vsz = sizeof(equalizer_preset);
	
		if(settings.appending_values.equalizer_presets_size < (vsz * iid))return -1;

		memcpy(settings_data + settings.appending_values.equalizer_presets_start + (vsz * iid), (char*)val, sizeof(equalizer_preset));
		break;


	default:
		return -1;
	}

	return 0;
}


/*
 * remove variable-size setting by index.
 * sid - setting id.
 * iid - index.
 */
int settings_data_remove(unsigned long sid, unsigned long iid)
{
	int vsz = 1;

	switch(sid)
	{
	case setting_id_equalizer_preset:
		vsz = sizeof(equalizer_preset);

		if(settings.appending_values.equalizer_presets_size < (vsz * iid))return -1;

		memmove(settings_data + settings.appending_values.equalizer_presets_start + (vsz * iid)
			   , settings_data + settings.appending_values.equalizer_presets_start + (vsz * (iid + 1))
			   , (settings.appending_values.appending_size - settings.appending_values.equalizer_presets_start) - (vsz * (iid + 1)));
		

		settings.appending_values.appending_size -= vsz;

		if(settings.appending_values.equalizer_presets_size >= (unsigned long)vsz)
			settings.appending_values.equalizer_presets_size -= vsz;
		break;


	default:
		return -1;
	}

	return 0;
}


/*
 * clean variable-size settings.
 * sid - setting id to be cleaned.
 */
int settings_data_clean(unsigned long sid)
{
	switch(sid)
	{
	case setting_id_input_plugin:
		
		memmove(settings_data + settings.appending_values.input_plugins_start
			   , settings_data + (settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size)
			   , settings.appending_values.appending_size - (settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size));

		settings.appending_values.appending_size   -= settings.appending_values.input_plugins_size;
		settings.appending_values.input_plugins_size = 0;

		/* reset other depending values */

		settings.plugins.input_plugins_count = 0;
		
		settings.appending_values.equalizer_presets_start = settings.appending_values.input_plugins_start + settings.appending_values.input_plugins_size;
		break;


	case setting_id_equalizer_preset:

		memmove(settings_data + settings.appending_values.equalizer_presets_start
			   , settings_data + (settings.appending_values.equalizer_presets_start + settings.appending_values.equalizer_presets_size)
			   , settings.appending_values.appending_size - (settings.appending_values.equalizer_presets_start + settings.appending_values.equalizer_presets_size));

		settings.appending_values.appending_size       -= settings.appending_values.equalizer_presets_size;
		settings.appending_values.equalizer_presets_size = 0;
		break;

	default:
		return -1;
	}

	return 0;
}


/*
 * uninitialize settings interface
 * and close the file.
 */
int settings_uninitialize(void)
{
	if(!settingsfilecreated) return -1;

	sys_file_close(hsettingsfile);
	sys_mem_free(settings_data);
	
	settings_data       = 0;
	settingsfilecreated = 0;
		
	return 0;
}



/* local --------------------------------------------------------------------*/



/*
 * set local shortcut key.
 */
void setlkey(unsigned int i, unsigned short k, unsigned short p)
{
	settings.shortcuts.localkeys[i].kaction = p;
	settings.shortcuts.localkeys[i].kcomb   = k;
}


/*
 * set global shortcut key.
 */
void setgkey(unsigned int i, unsigned short k, unsigned short p)
{
	settings.shortcuts.globalkeys[i].kaction = p;
	settings.shortcuts.globalkeys[i].kcomb   = k;
}


/*
 * create settings file (?).
 */
int settings_createfile(void)
{
	if(settingsfilecreated) return 0;

	str_cpy(settingsfilepath, fennec_get_path(0, 0));
	str_cat(settingsfilepath, uni("\\"));
	str_cat(settingsfilepath, settings_filename);

	reportx("settings: creating/loading settings file: %s", rt_stepping, settingsfilepath);

	hsettingsfile = sys_file_openstream(settingsfilepath, v_sys_file_forread | v_sys_file_forwrite);


	if(hsettingsfile == v_error_sys_file_open)
	{
		/* try read only */

		report("settings: couldn't create settings file, trying read-only", rt_info);

		hsettingsfile = sys_file_openstream(settingsfilepath, v_sys_file_forread);

		if(hsettingsfile == v_error_sys_file_open)
		{
			report("settings: file not found, creating file", rt_warning);
			
			settings_default();

			hsettingsfile = sys_file_openforcestream(settingsfilepath, v_sys_file_forread | v_sys_file_forwrite);
			
			if(hsettingsfile == v_error_sys_file_open) return 0;

			/* yeah we've done */

			settingsfilecreated = 1;
			settings_save();
			
			return 1;
		}
	}

	settingsfilecreated = 1;

	report("settings: loaded fine", rt_info);
	return 1;
}


int trans_rest(transcoder_settings *trans)
{
	int i, j;

	trans->general.buffersize    = 64;

	trans->eq.enable_eq          = 0;
	trans->eq.current_eq_preset  = (unsigned long)-1;

	trans->eq.eq.parametric      = 0;

	for(i=0; i<16; i++);
	{
		trans->eq.eq.preamp[i]   = 0.0;
		for(j=0; j<32; j++)
		{
			trans->eq.eq.boost[i][j] = 0.0;
			
		}
	}

	trans->volume.enable_vol     = 0;
	trans->volume.vol            = 0.0;
	trans->volume.gain           = 0.0;

	trans->tagging.enable_tagging = 1;
	trans->tagging.e_title        = 1;
	trans->tagging.e_artist       = 1;
	trans->tagging.e_album        = 1;
	trans->tagging.e_year         = 1;
	trans->tagging.e_genre        = 1;
	trans->tagging.e_comments     = 1;
	
	memset(trans->tagging.d_title,    0, sizeof(trans->tagging.d_title));
	memset(trans->tagging.d_artist,   0, sizeof(trans->tagging.d_artist));
	memset(trans->tagging.d_album,    0, sizeof(trans->tagging.d_album));
	memset(trans->tagging.d_year,     0, sizeof(trans->tagging.d_year));
	memset(trans->tagging.d_genre,    0, sizeof(trans->tagging.d_genre));
	memset(trans->tagging.d_comments, 0, sizeof(trans->tagging.d_comments));

	trans->misc.date_and_time_mode     = 0; /* original */
	trans->misc.custom_year            = 2008;
	trans->misc.custom_month           = 6;
	trans->misc.custom_date            = 19;
	trans->misc.custom_hour            = 7;
	trans->misc.custom_minute          = 0;

	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
