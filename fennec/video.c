/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.2
 Copyright (C) 2009 Chase <c-h@users.sf.net>

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


int callback_videotest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/*
 * 
 */

int video_initialize(void)
{
	//HWND hdlg;
	
	//hdlg = CreateDialog(instance_fennec, MAKEINTRESOURCE(dialog_videotest), 0, (DLGPROC)callback_videotest);
	//ShowWindow(hdlg, SW_SHOW);
	//UpdateWindow(hdlg);
	timeBeginPeriod(1);
	return 0;
}


int video_uninitialize(void)
{
	timeEndPeriod(1);
	return 0;
}


int video_refresh(HWND hwnd, HDC dc, RECT *crect)
{
	unsigned long        pid, fid;
	int                  w, h;
	static int           delay = 0;
	struct video_dec    *vdec;
	static void         *buffer = 0;
	double               audiosec = 0, aspect = 0;
	unsigned long        timems;
	static unsigned long lasttimems = 0;
	string               sec_sub;

	if(audio_getplayerstate() != v_audio_playerstate_playing) return -1;

	audio_get_current_ids(&pid, &fid);
	internal_input_plugin_getvideo(pid, fid, &vdec);
	
	if(!vdec)return -1;

	vdec->video_decoder_getsize(fid, &w, &h);
	
	/* aspect ratio */
	
	vdec->video_decoder_getsize(fid, (int*)&aspect, 0); /* this is a crazy patch */

	if(vdec->video_decoder_trans_info)
		vdec->video_decoder_trans_info(video_set_buffersize, /* in ms */ settings.audio_output.buffer_memory - lasttimems, /* in seconds */ ((double)settings.audio_output.buffer_memory - lasttimems) / 1000.0, 0);

	audio_getdata(audio_v_data_position_for_video, &audiosec);

	
	audiosec += ((double)lasttimems) / 1000.0;

	timems = timeGetTime();
	delay = vdec->video_decoder_getframe_sync(fid, &buffer, audiosec);


	//if(!delay) return 0;

	videoout_setinfo(w, h, 0, 24);
	videoout_setinfo_ex(1, &aspect, 0);
	videoout_pushtext(0, 0, vdec->video_decoder_getsubtitle(fid, -1.0f), 0, 0, 0, 0);
	sec_sub = vdec->video_decoder_getsubtitle(fid, 1.0f);

	videoout_pushtext(1, 0, sec_sub, 0, 0, 0, 0);

	videoout_display(buffer, 0, 0);

	lasttimems = timeGetTime() - timems;
	return delay;
}


int callback_videotest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_TIMER:
		{
			HDC dc = GetDC(hwnd);
			//video_refresh(hwnd, dc);
			ReleaseDC(hwnd, dc);
		}
		break;

	case WM_INITDIALOG:
		SetTimer(hwnd, 232, 10000, 0);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case IDCLOSE:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	
	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}




/*-----------------------------------------------------------------------------
 fennec. 2005 - Sept. 2007.
-----------------------------------------------------------------------------*/
