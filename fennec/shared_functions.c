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

/* declarations -------------------------------------------------------------*/

int localsf_show_settings(void *indata, void *outdata, unsigned int osize);
int localsf_show_tageditor(void *indata, void *outdata, unsigned int osize);
int localsf_show_about(void *indata, void *outdata, unsigned int osize);
int localsf_show_credits(void *indata, void *outdata, unsigned int osize);
int localsf_show_license(void *indata, void *outdata, unsigned int osize);
int localsf_show_conversion(void *indata, void *outdata, unsigned int osize);
int localsf_show_ripping(void *indata, void *outdata, unsigned int osize);
int localsf_show_joining(void *indata, void *outdata, unsigned int osize);
int localsf_show_error(int etype, const string text);
int localsf_getfennecpath(string fpath, unsigned int psize);
int localsf_getpluginspath(string fpath, unsigned int psize);
int localsf_getskinspath(string fpath, unsigned int psize);
int localsf_sendcommand(const string tcmd);
int localsf_show_openfile(void);
int localsf_show_addfile(void);
int localsf_show_save_playlist(void);
int localsf_show_addfolder(void);
int localsf_show_equalizer_presets(void* wmodal);
int localsf_list_fastsort(void);
int localsf_list_unsort(void);


/* functions ----------------------------------------------------------------*/

int shared_functions_fill(unsigned long fmask, struct fennec *o)
{
	o->language_text = strings_list;
	o->call_function = call_function;

	if(fmask & v_shared_function_mask_audio_input)
	{
		o->audio.input.initialize                 = audio_input_initialize;
		o->audio.input.uninitialize               = audio_input_uninitialize;
		o->audio.input.selectinput                = audio_input_selectinput;
		o->audio.input.getextensionsinfo          = audio_input_getextensionsinfo;
		o->audio.input.tagread_known              = audio_input_tagread_known;
		o->audio.input.tagwrite                   = audio_input_tagwrite;
		o->audio.input.tagread                    = audio_input_tagread;
		o->audio.input.gethandle                  = audio_input_gethandle;

		o->audio.input.plugins.initialize         = audio_input_plugin_initialize;
		o->audio.input.plugins.uninitialize       = audio_input_plugin_uninitialize;
		o->audio.input.plugins.mark_usage         = audio_input_plugin_mark_usage;
		o->audio.input.plugins.loadfile           = audio_input_plugin_loadfile;
		o->audio.input.plugins.getformat          = audio_input_plugin_getformat;
		o->audio.input.plugins.readdata           = audio_input_plugin_readdata;
		o->audio.input.plugins.getduration_ms     = audio_input_plugin_getduration_ms;
		o->audio.input.plugins.unloadfile         = audio_input_plugin_unloadfile;
		o->audio.input.plugins.setposition        = audio_input_plugin_setposition;
	}

	if(fmask & v_shared_function_mask_audio_output)
	{
		o->audio.output.initialize                = audio_initialize;
		o->audio.output.uninitialize              = audio_uninitialize;
		o->audio.output.load                      = audio_load;
		o->audio.output.add                       = audio_add;
		o->audio.output.play                      = audio_play;
		o->audio.output.pause                     = audio_pause;
		o->audio.output.stop                      = audio_stop;
		o->audio.output.setposition               = audio_setposition;
		o->audio.output.setposition_ms            = audio_setposition_ms;
		o->audio.output.getplayerstate            = audio_getplayerstate;
		o->audio.output.getposition               = audio_getposition;
		o->audio.output.getposition_ms            = audio_getposition_ms;
		o->audio.output.getduration_ms            = audio_getduration_ms;
		o->audio.output.getpeakpower              = audio_getpeakpower;
		o->audio.output.setvolume                 = audio_setvolume;
		o->audio.output.getvolume                 = audio_getvolume;
		o->audio.output.getcurrentbuffer          = audio_getcurrentbuffer;
		o->audio.output.getfloatbuffer            = audio_getfloatbuffer;
		o->audio.output.getdata                   = audio_getdata;

		o->audio.output.playlist.switch_list      = audio_playlist_switch_list;
		o->audio.output.playlist.clear			  = audio_playlist_clear;
		o->audio.output.playlist.add			  = audio_playlist_add;
		o->audio.output.playlist.move			  = audio_playlist_move;
		o->audio.output.playlist.setshuffle		  = audio_playlist_setshuffle;
		o->audio.output.playlist.shuffle		  = audio_playlist_shuffle;
		o->audio.output.playlist.unshuffle		  = audio_playlist_unshuffle;
		o->audio.output.playlist.remove			  = audio_playlist_remove;
		o->audio.output.playlist.mark			  = audio_playlist_mark;
		o->audio.output.playlist.unmark			  = audio_playlist_unmark;
		o->audio.output.playlist.ismarked		  = audio_playlist_ismarked;
		o->audio.output.playlist.getinfo		  = audio_playlist_getinfo;
		o->audio.output.playlist.next			  = audio_playlist_next;
		o->audio.output.playlist.previous		  = audio_playlist_previous;
		o->audio.output.playlist.switchindex	  = audio_playlist_switch;
		o->audio.output.playlist.getcurrentindex  = audio_playlist_getcurrentindex;
		o->audio.output.playlist.getrealindex	  = audio_playlist_getrealindex;
		o->audio.output.playlist.getlistindex	  = audio_playlist_getlistindex;
		o->audio.output.playlist.getcount         = audio_playlist_getcount;
		o->audio.output.playlist.getsource        = audio_playlist_getsource;

		/* equalizer */

		o->audio.equalizer.set_bands              = equalizer_set_bands;
		o->audio.equalizer.get_bands              = equalizer_get_bands;
		o->audio.equalizer.set_preamp             = equalizer_set_preamp;
		o->audio.equalizer.get_preamp			  = equalizer_get_preamp;
		o->audio.equalizer.reset                  = equalizer_reset;
		o->audio.equalizer.switch_current		  = equalizer_switch_current;
		o->audio.equalizer.equalize               = equalizer_equalize;
		
		/* dsp */

		o->audio.dsp.initialize                   = dsp_initialize;
		o->audio.dsp.reinitialize				  = dsp_reinitialize;
		o->audio.dsp.uninitialize                 = dsp_uninitialize;
		o->audio.dsp.initialize_index			  = dsp_initialize_index;
		o->audio.dsp.uninitialize_index           = dsp_uninitialize_index;
		o->audio.dsp.process					  = dsp_process;
		o->audio.dsp.showabout                    = dsp_showabout;
		o->audio.dsp.showconfig					  = dsp_showconfig;
	}

	if(fmask & v_shared_function_mask_general)
	{
		
		o->general.show_settings                  = localsf_show_settings;
		o->general.show_tageditor				  = localsf_show_tageditor;
		o->general.show_about					  = localsf_show_about;
		o->general.show_credits					  = localsf_show_credits;
		o->general.show_license					  = localsf_show_license;
		o->general.show_conversion				  = localsf_show_conversion;
		o->general.show_ripping					  = localsf_show_ripping;
		o->general.show_joining					  = localsf_show_joining;
		o->general.show_error					  = localsf_show_error;
		o->general.getfennecpath				  = localsf_getfennecpath;
		o->general.getpluginspath				  = localsf_getpluginspath;
		o->general.getskinspath					  = localsf_getskinspath;
		o->general.sendcommand					  = localsf_sendcommand;
		
	}

	if(fmask & v_shared_function_mask_simple)
	{
		
		o->simple.show_openfile                   = localsf_show_openfile;
		o->simple.show_addfile                    = localsf_show_addfile;
		o->simple.show_save_playlist              = localsf_show_save_playlist;
		o->simple.show_addfolder                  = localsf_show_addfolder;
		o->simple.show_equalizer_presets          = localsf_show_equalizer_presets;
		o->simple.list_fastsort                   = localsf_list_fastsort;
		o->simple.list_unsort                     = localsf_list_unsort;

	}

	if(fmask & v_shared_function_mask_settings)
	{                                            
		o->settings.load                          = settings_load;
		o->settings.save                          = settings_save;
		o->settings.reset                         = settings_default;

		o->settings.additional.add                = settings_data_add;
		o->settings.additional.set				  = settings_data_set;
		o->settings.additional.get				  = settings_data_get;
		o->settings.additional.remove			  = settings_data_remove;
		o->settings.additional.clean			  = settings_data_clean;

		o->settings.general                       = &settings;

	}

	if(fmask & v_shared_function_mask_media_library)
	{
		o->mlib.media_library_initialize         = media_library_initialize;
		o->mlib.media_library_uninitialize       = media_library_uninitialize;
		o->mlib.media_library_add_file           = media_library_add_file;
		o->mlib.media_library_save               = media_library_save;
		o->mlib.media_library_load				 = media_library_load;
		o->mlib.media_library_get				 = media_library_get;
		o->mlib.media_library_translate          = media_library_translate;
		o->mlib.media_library_get_dir_names		 = media_library_get_dir_names;
		o->mlib.media_library_get_dir_files		 = media_library_get_dir_files;
		o->mlib.media_library_add_dir            = media_library_add_dir;
		o->mlib.media_library_advanced_function  = media_library_advanced_function;
	}

	if(fmask & v_shared_function_mask_video)
	{
		o->video.easy_draw                       = video_refresh;
	}

	o->setposition_ex        = audio_input_plugin_setposition_ex;

	return 1;
}

int localsf_show_settings(void *indata, void *outdata, unsigned int osize)
{
	settings_ui_show_ex(osize);
	return 1;
}

int localsf_show_tageditor(void *indata, void *outdata, unsigned int osize)
{
	if(indata)
	{
		basewindows_show_tagging(0, (string)indata);
	}else{
		basewindows_show_tagging(0, audio_playlist_getsource(audio_playlist_getcurrentindex()));
	}
	return 1;
}

int localsf_show_about(void *indata, void *outdata, unsigned int osize)
{
	basewindows_show_about(1);
	return 1;
}

int localsf_show_credits(void *indata, void *outdata, unsigned int osize)
{

	return 1;
}

int localsf_show_license(void *indata, void *outdata, unsigned int osize)
{
	return 1;
}

int localsf_show_conversion(void *indata, void *outdata, unsigned int osize)
{
	basewindows_show_conversion(0);
	return 1;
}

int localsf_show_ripping(void *indata, void *outdata, unsigned int osize)
{
	basewindows_show_ripping(0);
	return 1;
}

int localsf_show_joining(void *indata, void *outdata, unsigned int osize)
{
	BaseWindows_ShowJoining(0);
	return 1;
}

int localsf_show_error(int etype, const string text)
{
	return 1;
}

int localsf_getfennecpath(string fpath, unsigned int psize)
{
	unsigned int i  = 0;

	sys_library_getfilename(sys_library_getcurrenthandle(), fpath, psize);
	
	i = (unsigned int)str_len(fpath); if(i)i--;
	while(fpath[i] && fpath[i] != uni('/') && fpath[i] != uni('\\'))i--;
	
	fpath[i] = 0;
	return 1;
}

int localsf_getpluginspath(string fpath, unsigned int psize)
{
	if(settings.plugins.input_path[0] == uni('/') || settings.plugins.input_path[0] == uni('\\'))
	{
		localsf_getfennecpath(fpath, psize);
		str_ncat(fpath, settings.plugins.input_path, psize);
	}else{
		str_ncpy(fpath, settings.plugins.input_path, psize);
	}
	return 1;
}

int localsf_getskinspath(string fpath, unsigned int psize)
{
	localsf_getfennecpath(fpath, psize);
	str_ncat(fpath, uni("/skins"), psize);
	return 1;
}

int localsf_sendcommand(const string tcmd)
{
	if(tcmd)
	{
		if(tcmd[0] == uni('>'))
		{
			if(str_icmp(tcmd, uni(">eject")) == 0)
				if(settings.player.selected_drive)
				{
					if(audio_playlist_getsource(audio_playlist_getcurrentindex())[0] == settings.player.selected_drive)audio_stop();
					audio_input_drive_eject((char)settings.player.selected_drive);
				}

			if(str_icmp(tcmd, uni(">selectdrive")) == 0)
				Menu_MainProc(menu_select_drive);

			if(str_icmp(tcmd, uni(">loadtracks")) == 0)
				playback_loadtracks();

		}else{
			text_command_base(tcmd);
		}
	}
	return 1;
}

int localsf_show_openfile(void)
{
	GlobalFunction(Function_OpenFileDialog);
	return 1;
}

int localsf_show_addfile(void)
{
	GlobalFunction(Function_AddFileDialog);
	return 1;
}


int localsf_show_save_playlist(void)
{
	unsigned int  i, c;
	letter        fpath[v_sys_maxpath];
	OPENFILENAME  lofn;

	memset(&lofn, 0, sizeof(lofn));

	fpath[0] = 0;

	lofn.lStructSize     = sizeof(lofn);
	lofn.lpstrTitle      = uni("Save Playlist File");
	lofn.hwndOwner       = window_main;
	lofn.lpstrFile       = fpath;
	lofn.nMaxFile        = sizeof(fpath);
	lofn.lpstrFilter     = uni("Text file (*.txt)\0*.txt\0M3U file\0*.m3u");
	lofn.nFilterIndex    = 0;
	lofn.lpstrFileTitle  = 0;
	lofn.nMaxFileTitle   = 0;
	lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	lofn.hInstance       = instance_fennec;

	GetSaveFileName(&lofn);

	c = (unsigned int) str_len(fpath);
	
	if(c)
	{
		if(c >= 4)
		{
			i = c - 4;

			if((str_icmp(fpath + i, uni(".txt")) != 0) && (str_icmp(fpath + i, uni(".m3u")) != 0))
			{
				if(lofn.nFilterIndex == 1)
				{
					str_cat(fpath, uni(".txt"));
				}else{
					str_cat(fpath, uni(".m3u"));
				}
			}
		}else{
			if(lofn.nFilterIndex == 1)
			{
				str_cat(fpath, uni(".txt"));
			}else{
				str_cat(fpath, uni(".m3u"));
			}
		}

		playlist_t_save_current(fpath);
	}
	return 1;
}


int localsf_show_addfolder(void)
{
	return basewindows_show_addfolder();
}


int localsf_show_equalizer_presets(void* wmodal)
{
	return basewindows_show_presets(wmodal);
}

/*
 * local function: sort by file name.
 */
int sort_by_filename(const string f1, const string f2)
{
	size_t c1 = str_len(f1), c2 = str_len(f2);

	while(c1 && (f1[c1] != uni('/') && f1[c1] != uni('\\')))c1--;
	while(c2 && (f2[c2] != uni('/') && f2[c2] != uni('\\')))c2--;

	return str_icmp(f1 + c1 + 1, f2 + c2 + 1);
}


int localsf_list_fastsort(void)
{
	unsigned int i, k, c, b;

	c = audio_playlist_getcount();

	for(k=0; k<c; k++)
	{
		for(i=0; i<c; i++)
		{
			b = i + 1;

			if(b >= c)continue;

			if(sort_by_filename(audio_playlist_getsource(b), audio_playlist_getsource(i)) == -1)
				audio_playlist_exchange(b, i);
		}
	}

	fennec_refresh(fennec_v_refresh_force_high);

	return 1;
}


int localsf_list_unsort(void)
{
#	define getrand(max) (rand() % (int)((max) + 1))

	unsigned int i, c, b, p;

	srand(timeGetTime());
	c = audio_playlist_getcount();
	p = audio_playlist_getcurrentindex();

	for(i=0; i<c; i++)
	{
		b = getrand(c);

		if(b >= c)continue;

		if((i == p) || (b == p))continue;

		audio_playlist_exchange(b, i);
	}

	fennec_refresh(fennec_v_refresh_force_high);
	return 1;
}


/*-----------------------------------------------------------------------------
 fennec, july 2007.
-----------------------------------------------------------------------------*/