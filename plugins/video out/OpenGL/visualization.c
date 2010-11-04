/*-----------------------------------------------------------------------------
  sample visualization for fennec (cube).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include <math.h>
#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"
#include "main.h"



/* prototypes ---------------------------------------------------------------*/

int callc videoout_initialize(void);
int callc videoout_uninitialize(int idata, void* vdata);
int callc videoout_display(void *data, int mode, void *modedata);
int callc videoout_refresh(int rlevel);
int callc videoout_settings(void* vdata);
int callc videoout_about(void* vdata);
int callc videoout_pushtext(int textmode, int addmode, void *data, int x, int y, int w, int h);
int callc videoout_setinfo_ex(int id, void *data, int (*afunc)());
int callc videoout_setinfo(int frame_w, int frame_h, int frame_rate, int colors);
int callc videoout_setstate(int state, int *extradata);






/* data ---------------------------------------------------------------------*/

struct    fennec                  sfennec;
struct    general_videoout_data   vdata;
int       w = 0, h = 0;
void     *current_frame;
double    aspect_ratio = 0;
extern    HWND		hWnd;
string    subtitle_text = 0;

extern int         screen_w, screen_h;

/* functions ----------------------------------------------------------------*/


int callc fennec_videoout_initialize(struct general_videoout_data *sdata, string plgtitle)
{
	memcpy(&sfennec, sdata->shared, sizeof(struct fennec));
	memcpy(&vdata, sdata, sizeof(struct general_videoout_data));

	sdata->initialize    = videoout_initialize;
	sdata->refresh       = videoout_refresh;
	sdata->pushtext      = videoout_pushtext;
	sdata->setinfo_ex    = videoout_setinfo_ex;
	sdata->setinfo       = videoout_setinfo;
	sdata->setstate      = videoout_setstate;
	sdata->display       = videoout_display;
	sdata->uninitialize  = videoout_uninitialize;

	glvis_init(0);

	return 1;
}

int callc videoout_initialize(void)
{

	return 1;
}

int callc videoout_uninitialize(int idata, void* vdata)
{

	glvis_uninit();

	return 1;
}

int callc videoout_display(void *data, int mode, void *modedata)
{
	current_frame = data;
	glvis_display();
	return 1;
}


int callc videoout_refresh(int rlevel)
{
	RECT rct;

	vdata.getdata(get_window_video_rect, 0, &rct, 0);

	SetWindowPos(hWnd, 0, rct.left, rct.top, rct.right - rct.left, rct.bottom - rct.top, SWP_NOZORDER);
	
	screen_w = rct.right - rct.left;
	screen_h = rct.bottom - rct.top;

	return 1;
}

int callc videoout_pushtext(int textmode, int addmode, void *data, int x, int y, int w, int h)
{
	subtitle_text = (string)data;
	return 1;
}

int callc videoout_setinfo_ex(int id, void *data, int (*afunc)())
{
	if(id == 1) /* aspect ratio */
	{
		aspect_ratio = *((double*)data);
	}
	return 1;
}

int callc videoout_setinfo(int frame_w, int frame_h, int frame_rate, int colors)
{
	w = frame_w;
	h = frame_h;
	return 1;
}

int callc videoout_setstate(int state, int *extradata)
{
	return 1;
}



int callc videoout_settings(void* vdata)
{
	MessageBox((HWND)vdata, uni("No Settings"), uni("Settings"), MB_ICONINFORMATION);
	return 1;
}

int callc videoout_about(void* vdata)
{
	MessageBox((HWND)vdata, uni("OpenGL video output plug-in"), uni("About"), MB_ICONINFORMATION);
	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

