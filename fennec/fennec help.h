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

#ifndef header_fennec_help
#define header_fennec_help

#define tip_text_exit                    uni("Exit (default mode).")
#define tip_text_minimize                uni("Minimize.")
#define tip_text_refresh				 uni("Refresh.")
#define tip_text_poweroffwithoutsaving	 uni("Power off without saving settings.")
#define tip_text_poweroff				 uni("Power off (after saving settings).")
#define tip_text_sleep					 uni("Sleep.")
#define tip_text_panelback				 uni("Goto previous display panel.")
#define tip_text_panelcolor				 uni("Show color/theme selector.")
#define tip_text_panelnext				 uni("Goto next display panel.")
#define tip_text_panelmain				 uni("Show main panel (general information, controls).")
#define tip_text_scratch				 uni("Toggle scratch window (not working).")
#define tip_text_video					 uni("Toggle video/visualizations window (not working).")
#define tip_text_amp					 uni("Toggle equalizer/amplifier.")
#define tip_text_mixlist				 uni("Toggle mixlist window (not working).")
#define tip_text_playlist				 uni("Toggle playlist window.")
#define tip_text_library				 uni("Toggle media library window (not working).")
#define tip_text_recoder				 uni("Toggle recoder (live mix) window (not working).")
#define tip_text_previous				 uni("Goto previous media.")
#define tip_text_rewind					 uni("Rewind.")
#define tip_text_play					 uni("Play/Pause.")
#define tip_text_stop					 uni("Stop.")
#define tip_text_forward				 uni("Forward.")
#define tip_text_next					 uni("Goto next media.")
#define tip_text_load					 uni("Load file(s) (right click to get the experimental version).")
#define tip_text_eject					 uni("Eject selected removeable media drive.")
#define tip_text_select					 uni("Fennec Player: (Left) Load tracks, (Right) Select drive.")
#define tip_text_piano  				 uni("Toggle live audio effects (not working).")

#define tip_text_playlist_autoswitching_on  uni("Toggle auto switching (Turned On).")
#define tip_text_playlist_autoswitching_off uni("Toggle auto switching (Turned Off).")
#define tip_text_playlist_shuffle_on        uni("Toggle shuffle on/off (Turned On).")
#define tip_text_playlist_shuffle_off       uni("Toggle shuffle on/off (Turned Off).")
#define tip_text_playlist_information       uni("Show tag editor for the selected track.")
#define tip_text_playlist_repeat_both       uni("Toggle (track - left, list - right) repeating on/off (Track: On, List: On).")
#define tip_text_playlist_repeat_none       uni("Toggle (track - left, list - right) repeating on/off (Track: Off, List: Off).")
#define tip_text_playlist_repeat_single     uni("Toggle (track - left, list - right) repeating on/off (Track: On, List: Off).")
#define tip_text_playlist_repeat_list       uni("Toggle (track - left, list - right) repeating on/off (Track: Off, List: On).")
#define tip_text_playlist_insert		    uni("Add media file(s), right click to add directories.")
#define tip_text_playlist_remove            uni("Remove selected.")
#define tip_text_playlist_sort			    uni("Left click to sort alphabetically, Right click to unsort the list.")
#define tip_text_playlist_storage           uni("Save/Load playlist file.")
#define tip_text_playlist_settings          uni("Show settings window.")

#define tip_text_conversion              uni("Show conversion window.")
#define tip_text_ripping                 uni("Show ripping window.")
#define tip_text_joining                 uni("Show joining window.")
#define tip_text_visualization           uni("Show visualizations.")
#define tip_text_volume                  uni("Adjust volume.")
#define tip_text_seek                    uni("Set current playing position.")
#define tip_text_peakleft                uni("Left peak meter.")
#define tip_text_peakright               uni("Right peak meter.")
#define tip_text_tageditor               uni("Show tag editor for current media.")

#define tip_text_color_hue               uni("Adjust hue.")
#define tip_text_color_saturation        uni("Adjust saturation.")
#define tip_text_color_lightness         uni("Adjust lightness.")

#define tip_text_equalizer_presets       uni("Select presets (right click to manage presets).")
#define tip_text_equalizer_preampleft    uni("Preamp (left).")
#define tip_text_equalizer_preampright   uni("Preamp (right).")

#define formattinginfo uni(\
\
"Formatting Syntax:\n\
\n\
Title: \t\t [<title-x>]\n\
Album: \t\t [<album-x>]\n\
Artist: \t\t [<artist-x>]\n\
Original artist: \t [<artist.o-x>]\n\
Composer: \t [<composer-x>]\n\
Lyricist: \t\t [<lyricist-x>]\n\
Band/Orchestra: \t [<band-x>]\n\
Copyright: \t [<copyright-x>]\n\
Publisher: \t [<publish-x>]\n\
Encoded by: \t [<encodedby-x>]\n\
Content type: \t [<genre-x>]\n\
Year: \t\t [<year-x>]\n\
URL: \t\t [<url-x>]\n\
Official artist URL: \t [<url.artist.o-x>]\n\
File/Source name: \t [<file.name-x>]\n\
Beats per minute: \t [<bpm-x>]\n\
Track number: \t [<tracknum-x>]\n\
\n\
Replace \'x\' with one of the following letters:\n\
\n\
Example text - \"THe aUdio Track NAmE\".\n\
\n\
        d - Default (\"THe aUdio Track NAmE\")\n\
        r - Required (empty data is replaced by the text \"Unknown\")\n\
        p - Proper title formatting (\"The Audio Track Name\")\n\
        u - Uppercase (\"THE AUDIO TRACK NAME\")\n\
        l - Lowercase (\"the audio track name\")\n\
\n\
Place any optional data within \'[\' and \']\'\n\
\n\
Example: [<title-r>][ - <album-d>][ - <artist-d>]\n\
\n\
Default formatting: [<artist-d> - ][<title-r>].")

#endif

/*-----------------------------------------------------------------------------
   eof.
-----------------------------------------------------------------------------*/
