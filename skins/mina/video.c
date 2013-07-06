#include "skin.h"
#include "skin settings.h"

#define vid_window_class uni("fennec.skin.3.video")
#define user_msg_start_keep_cursor 0x1
#define user_msg_end_keep_cursor   0x2

void draw_imagebox(HDC ddc, HDC sdc, int w, int h, struct coord *lt, struct coord *rt, struct coord *bl, struct coord *br, struct coord *t, struct coord *b, struct coord *l, struct coord *r);

int   vid_init = 0;
HWND  window_vid = 0;
HDC   hdc_vid = 0;
HRGN  rgn_vid = 0;
int   power_timeouts_set = 0;
UINT  power_timeout_screensaver = 0;
UINT  power_timeout_lowpower = 0;
UINT  power_timeout_poweroff = 0;
int   video_last_state = 0xbad;
int   video_thread_terminate = 0;
CRITICAL_SECTION video_cs;
int   video_window_maximized = 0, hide_cursor = 0, hide_cursor_done = 0;
int   keep_cursor = 0;
int   vid_draw_next = 0;

void vid_reinit(double aspect)
{
	RECT     wrect, area_rect;
	int      vheight;

	if(video_window_maximized)return; /* nothing to do if the window is maximized */

	GetWindowRect(wnd, &wrect);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &area_rect, 0);

	vheight = (int)((double)(wrect.right - wrect.left) / aspect);
	
	SetWindowPos(window_vid, 0, wrect.left, wrect.top - vheight - 5, wrect.right - wrect.left, vheight, SWP_NOZORDER);
	return;
}


void vid_create(HWND hwndp, double aspect)
{
	WNDCLASS wndc;
	RECT     wrect, area_rect;
	int      vheight;

	if(vid_init)
	{
		vid_reinit(aspect);
		return;
	}

	power_timeouts_set = 0;

	wndc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_DROPSHADOW;
	wndc.lpfnWndProc   = (WNDPROC)callback_vid_window;
	wndc.cbClsExtra    = 0;
	wndc.cbWndExtra    = 0;
	wndc.hInstance     = instance_skin;;
	wndc.hIcon         = LoadIcon(skin.finstance, (LPCTSTR)0);
	wndc.hCursor       = LoadCursor(0, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndc.lpszMenuName  = 0;
	wndc.lpszClassName = vid_window_class;

	RegisterClass(&wndc);

	/* create window */

	GetWindowRect(hwndp, &wrect);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &area_rect, 0);

	vheight = (int)((double)(wrect.right - wrect.left) / aspect);
	
	window_vid = CreateWindow(vid_window_class, uni("vidualization"), WS_POPUP, wrect.left, wrect.top - vheight - 5, wrect.right - wrect.left, vheight, hwndp, 0, instance_skin, 0);

	
	ShowWindow(window_vid, SW_SHOW);
	UpdateWindow(window_vid);

	vid_init = 1;
	
}

void set_power_timeouts(void)
{
	if(power_timeouts_set) return;

	/* back up */

	SystemParametersInfo(SPI_GETPOWEROFFTIMEOUT,   0, &power_timeout_poweroff, 0);
	SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &power_timeout_screensaver, 0);
	SystemParametersInfo(SPI_GETLOWPOWERTIMEOUT,   0, &power_timeout_lowpower, 0);

	/* stop them */

	if(power_timeout_poweroff)    SystemParametersInfo(SPI_SETPOWEROFFTIMEOUT,   0, 0, 0);
	if(power_timeout_screensaver) SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 0, 0, 0);
	if(power_timeout_lowpower)    SystemParametersInfo(SPI_SETLOWPOWERTIMEOUT,   0, 0, 0);

	power_timeouts_set = 1;
}

void restore_power_timeouts(void)
{
	if(!power_timeouts_set) return;

	if(power_timeout_poweroff)    SystemParametersInfo(SPI_SETPOWEROFFTIMEOUT,   power_timeout_poweroff, 0, 0);
	if(power_timeout_screensaver) SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, power_timeout_screensaver, 0, 0);
	if(power_timeout_lowpower)    SystemParametersInfo(SPI_SETLOWPOWERTIMEOUT,   power_timeout_lowpower, 0, 0);

	power_timeouts_set = 0;
}


void vid_close(void)
{
	if(!vid_init)return;

	vid_init = 0;

	//EnterCriticalSection(&video_cs);
	//video_thread_terminate = 1;
	//LeaveCriticalSection(&video_cs);
	//while(video_thread_terminate)
	//	Sleep(0);
	//

	str_cpy(settings_data.vis.current_vis, skin.shared->settings.general->visualizations.selected);
	skin.shared->call_function(call_visualizations_select_none, 0, 0, 0);



	if(hdc_vid)DeleteDC(hdc_vid);
	DestroyWindow(window_vid);
	if(rgn_vid)DeleteObject(rgn_vid);
}


DWORD th(LPVOID lp)
{

point_start:

	//EnterCriticalSection(&video_cs);
	
	if(video_thread_terminate)
	{
		//LeaveCriticalSection(&video_cs);
		video_thread_terminate = 0;

		//DeleteCriticalSection(&video_cs);
		return 0;
	}

	if(vid_init)
	{
		if(skin.shared->audio.output.getplayerstate() != v_audio_playerstate_playing)
		{
			//Sleep(100);
			//LeaveCriticalSection(&video_cs);
			goto point_start;
		}


		{
			RECT rct;
			vid_get_position(&rct);
			SendMessage(window_vid, WM_USER + 21, (WPARAM)hdc_vid, (LPARAM)&rct);
		}

		if(vid_draw_next > 0 && vid_draw_next < 100)
			Sleep(vid_draw_next);
		else
			Sleep(30);

	}

	//LeaveCriticalSection(&video_cs);
	goto point_start;
}



void vid_draw_background(int maxd)
{
	RECT  rct;

	GetClientRect(window_vid, &rct);	

	/* maximized mode won't need a border */

	if(maxd)
	{
		SetWindowRgn(window_vid, 0, 1);

		drawrect(hdc_vid, 0, 0, rct.right, rct.bottom, 0);

		skin.shared->call_function(call_visualizations_refresh, v_fennec_refresh_force_less, 0, 0);
		return;

	}else{
		if(rgn_vid)DeleteObject(rgn_vid);
		rgn_vid = CreateRoundRectRgn(0, 0, rct.right, rct.bottom, 4, 4);
		SetWindowRgn(window_vid, rgn_vid, 1);
	}


	{
		struct coord tl = {0, 0, 6, 208, 0, 0, 0, 0, 0, 0};
		struct coord tr = {0, 0, 7, 208, 7, 0, 0, 0, 0, 0};
		struct coord bl = {0, 0, 7, 9, 0, 235, 0, 0, 0, 0};
		struct coord br = {0, 0, 7, 7, 7, 236, 0, 0, 0, 0};
		struct coord t  = {0, 0, 15, 6, 0, 244, 0, 0, 0, 0};
		struct coord b  = {0, 0, 15, 6, 0, 252, 0, 0, 0, 0};
		struct coord l  = {0, 0, 5, 28, 0, 208, 0, 0, 0, 0};
		struct coord r  = {0, 0, 5, 28, 9, 208, 0, 0, 0, 0};

		draw_imagebox(hdc_vid, windowsheet_dc, rct.right, rct.bottom, &tl, &tr, &bl, &br, &t, &b, &l, &r);
	}

	drawrect(hdc_vid, 5, 5, rct.right - 10, rct.bottom - 10, 0x0);

	skin.shared->call_function(call_visualizations_refresh, v_fennec_refresh_force_less, 0, 0);
}


int vid_refresh(void)
{
	WINDOWPLACEMENT  wp;

	GetWindowPlacement(window_vid, &wp);

	vid_draw_background((wp.showCmd == SW_MAXIMIZE));
	return 1;
}

/*
 * this function returns the corner vectors,
 * (not like vis_get_position)
 */
int vid_get_position(RECT *retp)
{
	RECT             rct;
	WINDOWPLACEMENT  wp;

	if(!IsWindow(window_vid))
	{
		retp->left   = 0;
		retp->top    = 0;
		retp->right  = 0;
		retp->bottom = 0;
		return 0;
	}

	GetClientRect(window_vid, &rct);

	GetWindowPlacement(window_vid, &wp);

	if(wp.showCmd == SW_MAXIMIZE)
	{
		retp->left   = 0;
		retp->top    = 0;
		retp->right  = rct.right;
		retp->bottom = rct.bottom;
		video_window_maximized = 1;

	}else{

		retp->left   = 5;
		retp->top    = 5;
		retp->right  = rct.right  - 5;
		retp->bottom = rct.bottom - 5;
		video_window_maximized = 0;
	}
	return 1;
}


LRESULT CALLBACK callback_vid_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, dx, dy, lx, ly, maximized = 0;


	switch(msg)
	{
	case WM_USER + 21:
		//vid_draw_next = skin.shared->video.easy_draw(window_vid, hdc_vid, (RECT*)lParam);

		if(vid_draw_next != 0)
		{
			vis_active = 0;
		}
		break;

	case WM_TIMER:
		{
			int  nreturn;
			RECT rct;
			static int  iv = 0;

			if(settings_data.vis.show_vis && !settings_data.vis.video_when_available) break;

			if(settings_data.vis.show_vis && settings_data.vis.video_when_available)
			{
				if(vis_used) break;
			}

			vid_get_position(&rct);
			//SendMessage(window_vid, WM_USER + 21, (WPARAM)hdc_vid, (LPARAM)&rct);

			vid_draw_next = skin.shared->video.easy_draw(window_vid, hdc_vid, (RECT*)&rct);

			iv++;

			if(iv > 6)
			{
				iv = 0;

				nreturn = skin.shared->audio.output.getplayerstate();

				if(nreturn != video_last_state) /* some changes.. start, stopped or such */
				{
					WINDOWPLACEMENT  wp;

					if(nreturn == v_audio_playerstate_playing)
					{
						GetWindowPlacement(window_vid, &wp);

						if(wp.showCmd == SW_MAXIMIZE)
							set_power_timeouts();
						else
							restore_power_timeouts();
					}else{
						restore_power_timeouts();
					}

					video_last_state = nreturn; 
				}

				/* hide mouse cursor */
				if(video_window_maximized)
				{
					if(!keep_cursor)
					{
						if(hide_cursor < 15)
						{
							hide_cursor++;
						}else{
							//if(!hide_cursor_done)
								SetCursor(0);
							hide_cursor_done = 1;
						}
					}

				}else{
					hide_cursor = 0;
					hide_cursor_done = 0;
				}
			}
		}
		break;

	case WM_USER + 22: /* force hide */
		settings_data.video.force_hide = 1;
		//skin.shared->call_function(call_videoout_uninitialize, 0, 0, 0);
		vid_close();
		break;

	case WM_MOUSEMOVE:
		{
			RECT rct;
			int  x = (int)LOWORD(lParam), y = (int)HIWORD(lParam);

			hide_cursor = 0;
			hide_cursor_done = 0;
			SetCursor(LoadCursor(0, IDC_ARROW));

			GetClientRect(hwnd, &rct);

			if(x > rct.right - 20 && y > rct.bottom - 20 &&
			   x <= rct.right && y <= rct.bottom)
			{
				SetCursor(LoadCursor(0, IDC_SIZENWSE));
			}

			if(mdown == 1)
			{
				//POINT pt;

				//GetCursorPos(&pt);

				//move_docking_window(window_id_vid, pt.x - dx, pt.y - dy);

							
				//skin.shared->call_function(call_videoout_refresh, 0, 0, 0);

			
			}else if(mdown == 2){ /* resize */
				
				/*POINT pt;

				GetCursorPos(&pt);

				skin_settings.vid_w = max(pt.x - lx + dx, 100);
				skin_settings.vid_h = max(pt.y - ly + dy, 50);

				if(skin_settings.vid_d)
				{
					if(skin_main_width > skin_settings.vid_w - 10 && skin_main_width < skin_settings.vid_w + 10)
						skin_settings.vid_w = skin_main_width;

					if(skin_main_height > skin_settings.vid_h - 10 && skin_main_height < skin_settings.vid_h + 10)
						skin_settings.vid_h = skin_main_height;
				}

				SetWindowPos(hwnd, 0, 0, 0, skin_settings.vid_w, skin_settings.vid_h, SWP_NOMOVE | SWP_NOZORDER);

				vid_draw_background(maximized);

							
				skin.shared->call_function(call_videoout_refresh, 0, 0, 0);*/

			}


			if(!maximized)
			{
				/*if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vid.button_close, coords.window_vid.button_close_align, skin_settings.vid_w, skin_settings.vid_h))
				{
					show_tip(oooo_skins_close, 0);
					blt_coord_vpos_nozoom(hdc_vid, mdc_sheet, 1, &coords.window_vid.button_close, coords.window_vid.button_close_align, skin_settings.vid_w, skin_settings.vid_h);
				}else{
					blt_coord_vpos_nozoom(hdc_vid, mdc_sheet, 0, &coords.window_vid.button_close, coords.window_vid.button_close_align, skin_settings.vid_w, skin_settings.vid_h);
				}

				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vid.button_max, coords.window_vid.button_max_align, skin_settings.vid_w, skin_settings.vid_h))
				{
					show_tip(oooo_skins_maximize, 0);
					blt_coord_vpos_nozoom(hdc_vid, mdc_sheet, 1, &coords.window_vid.button_max, coords.window_vid.button_max_align, skin_settings.vid_w, skin_settings.vid_h);
				}else{
					blt_coord_vpos_nozoom(hdc_vid, mdc_sheet, 0, &coords.window_vid.button_max, coords.window_vid.button_max_align, skin_settings.vid_w, skin_settings.vid_h);
				} */
			}
		}
		break;

	case WM_MOUSEWHEEL:
		/* invert wheel messages: in video window just scrolling means seeking... */
		/*if(LOWORD(wParam) == MK_SHIFT)
		{
			WPARAM  newwp = MAKEWPARAM(0, HIWORD(wParam));
			SendMessage(skin.wnd, msg, newwp, lParam);
		}else{
			WPARAM  newwp = MAKEWPARAM(MK_SHIFT, HIWORD(wParam));
			SendMessage(skin.wnd, msg, newwp, lParam);
		}*/

		if(LOWORD(wParam) == MK_SHIFT) /* seek */
		{
			/*WPARAM  newwp = MAKEWPARAM(0, HIWORD(wParam));
			SendMessage(skin.wnd, msg, newwp, lParam); */

			double cpos, dur, steps;
			
			dur  = skin.shared->audio.output.getduration_ms();
			cpos = skin.shared->audio.output.getposition_ms();
			steps = (double)(short)HIWORD(wParam);
			cpos += (steps * 5000.0) / 120.0;
			if(cpos > dur)cpos = dur;
			else if(cpos < 0.0)cpos = 0.0;
			skin.shared->audio.output.setposition_ms(cpos);

		}else if(LOWORD(wParam) == MK_CONTROL){ /* switch list */
			SendMessage(skin.wnd, msg, wParam, lParam);
		
		}else{
			SendMessage(skin.wnd, msg, wParam, lParam);
			/*
			 */
		}

		break;

	
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_SPACE:
			skin.shared->audio.output.play();
			break;

		case VK_LEFT:
			{
				double cpos, dur;
			
				dur  = skin.shared->audio.output.getduration_ms();
				cpos = skin.shared->audio.output.getposition_ms();

				cpos -= 20000;

				if(cpos > dur)cpos = dur;
				else if(cpos < 0.0)cpos = 0.0;
				skin.shared->audio.output.setposition_ms(cpos);
			}
			break;

		case VK_RIGHT:
			{
				double cpos, dur;
			
				dur  = skin.shared->audio.output.getduration_ms();
				cpos = skin.shared->audio.output.getposition_ms();
				
				cpos += 20000;

				if(cpos > dur)cpos = dur;
				else if(cpos < 0.0)cpos = 0.0;
				skin.shared->audio.output.setposition_ms(cpos);
			}
			break;

		case VK_UP:
			{ 
				double cvol;

				skin.shared->audio.output.getvolume(&cvol, &cvol);
				cvol += 0.01;
				if(cvol > 1.0)cvol = 1.0;
				skin.shared->audio.output.setvolume(cvol, cvol);
			}
			break;

		case VK_DOWN:
			{ 
				double cvol;

				skin.shared->audio.output.getvolume(&cvol, &cvol);
				cvol -= 0.01;
				if(cvol < 0.0)cvol = 0.0;
				skin.shared->audio.output.setvolume(cvol, cvol);
			}
			break;
		}
		break;


	case WM_LBUTTONDBLCLK:
		{
			WINDOWPLACEMENT  wp;

			GetWindowPlacement(hwnd, &wp);

			if(wp.showCmd == SW_MAXIMIZE)
			{
				ShowWindow(hwnd, SW_RESTORE);
				maximized = 0;
				video_window_maximized = 0;
			}else{
				ShowWindow(hwnd, SW_MAXIMIZE);
				maximized = 1;
				video_window_maximized = 1;
			}

			/* let video timer know the changes */
			video_last_state = 0xbad;

			sys_sleep(30);
			vid_draw_background(maximized);
			skin.shared->call_function(call_videoout_refresh, 0, 0, 0);

		}
		break;

	case WM_LBUTTONDOWN:
		{
			/*RECT rct;

			if(maximized)break;

			if(!maximized)
			{
				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vid.button_close, coords.window_vid.button_close_align, skin_settings.vid_w, skin_settings.vid_h))
				{
					if(skin_settings.skin_lock)
					{
						SendMessage(skin.wnd, WM_DESTROY, 0, 0);
					}else{
						vid_close();
						skin_settings.vid_show = 0;
					}
					break;
				}
				
				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vid.button_max, coords.window_vid.button_max_align, skin_settings.vid_w, skin_settings.vid_h))
				{
					goto offset_fullscreen;
					break;
				}
			}


			GetClientRect(hwnd, &rct);

			dx = (int)LOWORD(lParam);
			dy = (int)HIWORD(lParam);

			if(skin_settings.skin_lock)
			{
				RECT  rctm;
				POINT pt;
				
				GetWindowRect(skin.wnd, &rctm);
				GetCursorPos(&pt);
				last_dx = pt.x - rctm.left;
				last_dy = pt.y - rctm.top;
			}

			if(dx > rct.right - 20 && dy > rct.bottom - 20 &&
			   dx <= rct.right && dy <= rct.bottom)
			{
				dx = rct.right - dx;
				dy = rct.bottom - dy;


				GetWindowRect(hwnd, &rct);
				lx = rct.left;
				ly = rct.top;
		
				mdown = 2;
			}else{
				mdown = 1;
			}
			*/
			SetCapture(hwnd);
		}
		break;

	case WM_USER:
		if(wParam == user_msg_start_keep_cursor)
			keep_cursor = 1;
		if(wParam == user_msg_end_keep_cursor)
			keep_cursor = 0;
		break;

	case WM_LBUTTONUP:
		mdown = 0;
		ReleaseCapture();
		break;

	case WM_CREATE:
		hdc_vid = GetDC(hwnd);
		//SetTimer(hwnd, 123, 200, 0);
		SetTimer(hwnd, 123, 35, 0);
		//InitializeCriticalSection(&video_cs);
		video_thread_terminate = 0;
		//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)th, 0, 0, 0);/* */
		break;

	case WM_PAINT:
		vid_draw_background(maximized);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
