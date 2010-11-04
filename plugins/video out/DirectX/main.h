#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"
#include <windows.h>
#include <math.h>

extern int default_sw;
extern int default_sh;
extern HWND                              video_window;
extern HINSTANCE                         hinstance;

extern struct    fennec                  sfennec;
extern struct    general_videoout_data   vdata;

extern double    aspect_ratio;

extern double    video_zoom_x, video_zoom_y;
extern int       video_dx, video_dy;
extern int       show_subtitles;

int  video_init(HWND hwndp);
int  video_uninit(void);
int  video_display(void);

int  directdraw_init(HWND hwndp);
void directdraw_update(int mode);
int  directdraw_draw(void *pic, int w, int h);
int  directdraw_uninit(void);

void osd_display(HDC dc, int w, int h);
void osd_initialize(void);
void osd_initialize_in(HDC dc);
int osd_hide_all(void);

int  callc videoout_refresh(int rlevel);

int osd_mouse_message(int action, int x, int y);

int __stdcall DrawHTML(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);
