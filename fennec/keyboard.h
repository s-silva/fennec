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

#define keypurpose_null                     0x0
#define keypurpose_play                     0x1
#define keypurpose_pause                    0x2
#define keypurpose_stop                     0x3
#define keypurpose_load                     0x4
#define keypurpose_rewind                   0x5
#define keypurpose_forward                  0x6
#define keypurpose_previous					0x7
#define keypurpose_next						0x8
#define keypurpose_eject					0x9
#define keypurpose_select                   0xa
#define keypurpose_panelsw_main				0xb
#define keypurpose_panelsw_color			0xc
#define keypurpose_panelsw_visualization	0xd
#define keypurpose_panelsw_equalizer		0xe
#define keypurpose_panelsw_mediainfo		0xf
#define keypurpose_panelsw_playlist			0x10
#define keypurpose_panelnext				0x11
#define keypurpose_panelprevious			0x12
#define keypurpose_exit						0x13
#define keypurpose_sleep					0x14
#define keypurpose_minimize					0x15
#define keypurpose_refresh					0x16
#define keypurpose_conversion				0x17
#define keypurpose_ripping					0x18
#define keypurpose_joining					0x19
#define keypurpose_visualization            0x1a     /* external */
#define keypurpose_playlist					0x1b
#define keypurpose_volumeup					0x1c
#define keypurpose_volumedown				0x1d
#define keypurpose_volumeup_auto			0x1e
#define keypurpose_volumedown_auto			0x1f
#define keypurpose_volumemin				0x20
#define keypurpose_volumemax				0x21
#define keypurpose_addfile					0x22
#define keypurpose_fast_load				0x23
#define keypurpose_fast_addfile				0x24
#define keypurpose_preferences				0x25
#define keypurpose_keyboardviewer			0x26
#define keypurpose_currenttagging			0x27
#define keypurpose_switch_playlist			0x28
#define keypurpose_playlist_autoswitching	0x29
#define keypurpose_playlist_shuffle			0x2a
#define keypurpose_playlist_information		0x2b
#define keypurpose_playlist_repeatall		0x2c
#define keypurpose_playlist_repeatsingle	0x2d
#define keypurpose_playlist_insert			0x2e
#define keypurpose_playlist_insertdir		0x2f
#define keypurpose_playlist_remove			0x30
#define keypurpose_switch_main              0x31

#define fennec_key_control                  0x8000
#define fennec_key_alternative              0x4000
#define fennec_key_shift                    0x2000

#define fennec_key_function1                0xf1
#define fennec_key_function2                0xf2
#define fennec_key_function3                0xf3
#define fennec_key_function4                0xf4
#define fennec_key_function5                0xf5
#define fennec_key_function6                0xf6
#define fennec_key_function7                0xf7
#define fennec_key_function8                0xf8
#define fennec_key_function9                0xf9
#define fennec_key_function10               0xfa
#define fennec_key_function11               0xfb
#define fennec_key_function12               0xfc
#define fennec_key_function13               0xfd
#define fennec_key_function14               0xfe
#define fennec_key_function15               0xff

#define fennec_key_a                        0xa1
#define fennec_key_b                        0xa2
#define fennec_key_c                        0xa3
#define fennec_key_d                        0xa4
#define fennec_key_e                        0xa5
#define fennec_key_f                        0xa6
#define fennec_key_g                        0xa7
#define fennec_key_h                        0xa8
#define fennec_key_i                        0xa9
#define fennec_key_j                        0xaa
#define fennec_key_k                        0xab
#define fennec_key_l                        0xac
#define fennec_key_m                        0xad
#define fennec_key_n                        0xae
#define fennec_key_o                        0xaf
#define fennec_key_p                        0xb0
#define fennec_key_q                        0xb1
#define fennec_key_r                        0xb2
#define fennec_key_s                        0xb3
#define fennec_key_t                        0xb4
#define fennec_key_u                        0xb5
#define fennec_key_v                        0xb6
#define fennec_key_w                        0xb7
#define fennec_key_x                        0xb8
#define fennec_key_y                        0xb9
#define fennec_key_z                        0xba

#define fennec_key_0                        0xd0
#define fennec_key_1                        0xd1
#define fennec_key_2                        0xd2
#define fennec_key_3                        0xd3
#define fennec_key_4                        0xd4
#define fennec_key_5                        0xd5
#define fennec_key_6                        0xd6
#define fennec_key_7                        0xd7
#define fennec_key_8                        0xd8
#define fennec_key_9                        0xd9

#define fennec_key_return                   0xef
#define fennec_key_escape                   0xee
#define fennec_key_tab                      0xed
#define fennec_key_graveaccent              0xc0 /* ` */
#define fennec_key_minus                    0xc1 /* - */
#define fennec_key_equals                   0xc2 /* = */
#define fennec_key_forwardslash             0xc3 /* / */
#define fennec_key_backslash                0xc4 /* \ */
#define fennec_key_squarebracket_open       0xc5 /* [ */
#define fennec_key_squarebracket_close      0xc6 /* ] */
#define fennec_key_semicolon                0xc7 /* ; */
#define fennec_key_accent                   0xc8 /* ' */
#define fennec_key_comma                    0xc9 /* , */
#define fennec_key_period                   0xca /* . */
#define fennec_key_space                    0xcb

#define fennec_key_printscreen              0x10
#define fennec_key_scrolllock               0x11
#define fennec_key_pause                    0x12
#define fennec_key_insert                   0x13
#define fennec_key_home                     0x14
#define fennec_key_end                      0x15
#define fennec_key_pageup                   0x16
#define fennec_key_pagedown                 0x17
#define fennec_key_delete                   0x18
#define fennec_key_backspace                0x19
#define fennec_key_up                       0x1a
#define fennec_key_down                     0x1b
#define fennec_key_left                     0x1c
#define fennec_key_right                    0x1d

#define fennec_key_num_0                    0x90
#define fennec_key_num_1                    0x91
#define fennec_key_num_2                    0x92
#define fennec_key_num_3                    0x93
#define fennec_key_num_4                    0x94
#define fennec_key_num_5                    0x95
#define fennec_key_num_6                    0x96
#define fennec_key_num_7                    0x97
#define fennec_key_num_8                    0x98
#define fennec_key_num_9                    0x99
#define fennec_key_num_numlock              0x9a
#define fennec_key_num_divide               0x9b
#define fennec_key_num_multiply             0x9c
#define fennec_key_num_minus                0x9d
#define fennec_key_num_plus                 0x9e
#define fennec_key_num_return               0x9f

unsigned long kb_getpurpose(unsigned short fkey, struct setting_key *kcol, int kcount);
unsigned long kb_action(unsigned short fkey, struct setting_key *kcol, int kcount);

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/