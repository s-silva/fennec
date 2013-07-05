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

#ifdef   system_microsoft_windows
#	include <shlobj.h>
#endif




/* prototypes ---------------------------------------------------------------*/

int tc_local_action(const string tcommand, unsigned int tlen);




/* data ---------------------------------------------------------------------*/

unsigned long last_commandline_time = 0;




/* code ---------------------------------------------------------------------*/


/*
 * global function, text_command_base
 */
int text_command_base(const string tcommand)
{
	unsigned int i  = 0;
	unsigned int lp = 0; /* last pointer (to save last 'i') */
	unsigned int iq = 0; /* inside quotes */
	int          fc = 1; /* first command: exe path */
	letter       mname[v_sys_maxpath];
	int          fadds = 0;

	if(!tcommand)        return 0; /* r u kiddin'? */
	if(!str_len(tcommand))return 0; /* what? don't!.. ok. */


	
	sys_library_getfilename(0, mname, sizeof(mname));
	i = (unsigned int)str_len(mname) + 2 /* for two quotes */;


	/* multiple files? */

	if(last_commandline_time + 200 > sys_timer_getms())
	{
		last_commandline_time = sys_timer_getms();
	}else{
		
		last_commandline_time = sys_timer_getms();
		audio_playlist_clear();		
	}


	/* recognize commands by spaces */

	for(;;)
	{
		if(tcommand[i] == uni('\"'))
		{

			/* 
			 * toggle, one to note; note to one ...
			 * cuz the same quote character ends the selection.
			*/

			fc = 0;

			iq ^= 1;

			if(!iq && !fc)
			{
				tc_local_action(tcommand + lp + 1, i - lp - 1); /* decrease length by one, if quotes found */
				fadds++;
			}

			lp = i;
		}

		if((tcommand[i] == uni('-')) && !iq)
		{
			tc_local_action(tcommand + i, (unsigned int)str_len(tcommand) - i);
			fadds++;
		}
		
		if(tcommand[i] == 0)break;

		i++;
	}
	
	return (fadds ? 1 : 0);
}


/*
 * start parsed action by name.
 */
int tc_local_action(const string tcommand, unsigned int tlen)
{
	letter       ntcommand[1024]; /* null terminated command string */

	/* we'll need a null terminated string */

	if(!tlen)return 0;
	if(tlen > 1024)return 0;
	
	str_mcpy(ntcommand, tcommand, tlen);
	ntcommand[tlen] = 0;

	/* action! */

	if(tcommand[0] != uni('-')) /* not an option, it's a file */
	{
		audio_playlist_add(ntcommand, 0, 0);

		if(audio_playlist_getcount() == 1)
		{
			audio_playlist_switch(0);
			audio_play();
		}
	}else{ /* option */

		switch(tcommand[1])
		{

		case uni('0'): /* zero volume */
			audio_setvolume(0.0f, 0.0f);
			break;
		
		case uni('1'): /* full volume */
			audio_setvolume(1.0f, 1.0f);
			break;

		case uni('i'):
		case uni('I'):
			
			if(MessageBox(0, uni("Set file association settings?"), uni("Fennec Player - Installation"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				letter         icopath[1024];
				letter         fennecpath[1024];
				letter         ext[260];
				letter         dsc[260];
				unsigned int   i = 0;

				memset(fennecpath, 0, sizeof(fennecpath));

				GetModuleFileName(instance_fennec, fennecpath, sizeof(fennecpath));
					
				while(audio_input_getextensionsinfo(i, ext, dsc))
				{
					str_cpy(icopath, fennec_get_path(0, 0));
					str_cat(icopath, uni("\\"));
					str_cat(icopath, icon_library);
				
					fileassociation_set(ext, uni("Play"), uni("&Play in Fennec Player"), icopath, fennecpath, dsc, -1);

					memset(ext, 0, sizeof(ext));
					memset(dsc, 0, sizeof(dsc));

					i++;
				}

				SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, 0, 0);
				
				
			}

			if((ntcommand[2] != uni('S')) && (ntcommand[2] != uni('s')))
			{
				settings_save();
				fennec_power_exit();
			}
			break;

		case uni('u'):
		case uni('U'):
			if(str_incmp(ntcommand, uni("-uninstall"), str_len(uni("-uninstall"))) == 0)
			{
				unsigned long i = 0;
				letter        ext[255], dsc[255];

				while(audio_input_getextensionsinfo(i, ext, dsc))
				{
					fileassociation_restore(ext);
					i++;
				}

#				ifdef system_microsoft_windows
					SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, 0, 0);
#				endif

				fennec_register_contextmenu(0); /* unregister context menu */
				fennec_power_exit();
			}
			break;

		case uni('d'):
			settings_default();
			settings_save();
			break;

		}
	}

	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/
