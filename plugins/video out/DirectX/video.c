#include "main.h"
#include "../../../include/ids.h"

extern void     *current_frame;
extern int       frame_w, frame_h;
int              video_init_done = 0;

int video_init(HWND hwndp)
{
	if(video_init_done) return 0;

	if(!directdraw_init(hwndp))
	{
		directdraw_uninit(); /* uninitialize any loaded data */
		video_init_done = 0;
	}

	video_init_done = 1;
	return 0;
}

int video_uninit(void)
{
	if(!video_init_done) return 0;

	directdraw_uninit();
	video_init_done = 0;
	return 0;
}

int video_display(void)
{
	static int calledvid_test = 0;


	if(!video_init_done)
	{
		HWND hwndp = 0;
		vdata.getdata(get_window_video, 0, &hwndp, 0);
		if(IsWindow(hwndp))
		{
			video_init(hwndp);
			videoout_refresh(0);
		}
	}

	if(current_frame)
	{
		if(!calledvid_test)
		{
			calledvid_test = 1;
			vdata.shared->call_function(call_videoout_test, 0, 0, 0);
		}

		directdraw_draw(current_frame, frame_w, frame_h);
		directdraw_update(1);
	}else{
		directdraw_draw(0, frame_w, frame_h);
		directdraw_update(0);
		return 0;
	}

	if(vdata.shared->audio.output.getplayerstate() == v_audio_playerstate_stopped)
	{
		directdraw_draw(0, frame_w, frame_h);
		directdraw_update(0);
		current_frame = 0;
	}
	//current_frame = 0;
	return 0;
}