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

int GlobalFunction(unsigned long fid, ...)
{
	int      rvi = 0;

	va_list  amk;
	va_start(amk, fid);

#	define Arg(x)(va_arg(amk, x))

	switch(fid)
	{
	case Function_OpenFileDialog: /* void */
		fennec_show_file_dialog(file_dialog_openfiles, 0, 0, 0);
		break;

	case Function_AddFileDialog:
		fennec_show_file_dialog(file_dialog_addfiles, 0, 0, 0);
		break;

	case Function_Play:
		rvi = audio_play();
		break;

	case Function_Pause:
		rvi = audio_pause();
		break;

	case Function_Stop:
		rvi = audio_stop();
		break;

	case Function_Previous:
		rvi = audio_playlist_previous();
		break;

	case Function_Next:
		rvi = audio_playlist_next();
		break;

	case Function_Rewind:
		{
			double tpos;
			audio_getposition(&tpos);
			rvi = audio_setposition(tpos - 0.01f);
		}
		break;

	case Function_Forward:
		{
			double tpos;
			audio_getposition(&tpos);
			rvi = audio_setposition(tpos + 0.01f);
		}
		break;
	}

	va_end(amk);
	return  rvi;
}


/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/
