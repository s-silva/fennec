#include "skin.h"

#define vis_window_class uni("fennec.skin.3.visualization")

#define  window_vis_crop_tl_x 1
#define  window_vis_crop_tl_y 301
#define  window_vis_crop_tl_w 61
#define  window_vis_crop_tl_h 31

#define  window_vis_crop_tr_x 95
#define  window_vis_crop_tr_y 301
#define  window_vis_crop_tr_w 35
#define  window_vis_crop_tr_h 31

#define  window_vis_crop_bl_x 185
#define  window_vis_crop_bl_y 301
#define  window_vis_crop_bl_w 21
#define  window_vis_crop_bl_h 21

#define  window_vis_crop_br_x 161
#define  window_vis_crop_br_y 301
#define  window_vis_crop_br_w 21
#define  window_vis_crop_br_h 21

#define  window_vis_crop_tm_x 63
#define  window_vis_crop_tm_y 301
#define  window_vis_crop_tm_w 31
#define  window_vis_crop_tm_h 31

#define  window_vis_crop_bm_x 207
#define  window_vis_crop_bm_y 301
#define  window_vis_crop_bm_w 22
#define  window_vis_crop_bm_h 19

#define  window_vis_crop_ml_x 131
#define  window_vis_crop_ml_y 301
#define  window_vis_crop_ml_w 13
#define  window_vis_crop_ml_h 22

#define  window_vis_crop_mr_x 146
#define  window_vis_crop_mr_y 301
#define  window_vis_crop_mr_w 14
#define  window_vis_crop_mr_h 22


LRESULT CALLBACK callback_vis_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int vis_lyrics_timercall(void);
int vis_lyrics_loadcurrent(void);
void vis_lyrics_draw(HDC dc);

int   vis_init = 0;
HWND  window_vis;
HDC   hdc_vis;
HRGN  rgn_vis = 0;



string   vis_lyric_tag = 0;
int      vis_lyric_tag_len = 0;
string   vis_lyric_current_text = 0;
int      vis_lyric_current_text_len = 0;
int      vis_lyric_current_text_pos = 0, vis_lyric_y = 0;
int      vis_lyric_current_action = 0, vis_lyric_current_laction = 0;
UINT_PTR vis_lyric_timer;
string   vis_markers = 0;
int      vis_marker_show, vis_marker_hide, vis_marker_pos, vis_marker_last;
HFONT    vis_lyric_font, vis_lyric_font_b;

letter   vis_lyrics_lastfile[v_sys_maxpath];

int		 vis_lyrics_font_size = 20;

void vis_create(HWND hwndp)
{
	WNDCLASS wndc;
	RECT rct;

	if(vis_init)return;
	if(!skin_settings.vis_show)return;

	wndc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndc.lpfnWndProc   = (WNDPROC)callback_vis_window;
	wndc.cbClsExtra    = 0;
	wndc.cbWndExtra    = 0;
	wndc.hInstance     = instance_skin;;
	wndc.hIcon         = LoadIcon(skin.finstance, (LPCTSTR)0);
	wndc.hCursor       = LoadCursor(0, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndc.lpszMenuName  = 0;
	wndc.lpszClassName = vis_window_class;

	RegisterClass(&wndc);

	/* create window */
	GetClientRect(hwndp, &rct);

	window_vis = CreateWindow(vis_window_class, uni("Visualization"), WS_CHILD, 0, 45, rct.right, rct.bottom - 117 - 45, hwndp, 0, instance_skin, 0);

	//setwinpos_clip(window_vis, 0, skin_settings.vis_x, skin_settings.vis_y, max(skin_settings.vis_w, 30), max(skin_settings.vis_h, 30), SWP_NOSIZE | SWP_NOZORDER);
	
	if(window_ml) ShowWindow(window_ml, SW_HIDE);
	if(window_vid) ShowWindow(window_vid, SW_HIDE);

	ShowWindow(window_vis, SW_SHOW);
	UpdateWindow(window_vis);

	str_cpy(skin.shared->settings.general->visualizations.selected, skin_settings.current_vis);
	skin.shared->call_function(call_visualizations_select_next, 0, 0, 0);

	vis_init = 1;

	vis_lyrics_lastfile[0] = 0;
	vis_lyrics_refresh(v_fennec_refresh_force_high);

}


void vis_close(void)
{
	if(!vis_init)return;

	if(window_ml) ShowWindow(window_ml, SW_SHOW);

	str_cpy(skin_settings.current_vis, skin.shared->settings.general->visualizations.selected);
	skin.shared->call_function(call_visualizations_select_none, 0, 0, 0);

	if(hdc_vis)DeleteDC(hdc_vis);
	DestroyWindow(window_vis);
	if(rgn_vis)DeleteObject(rgn_vis);
	vis_init = 0;
}

void vis_draw_background(int maxd)
{
	RECT  rct;
	int   i, c;

	GetClientRect(window_vis, &rct);	
	
	/* change font sizes */

	vis_lyrics_font_size = MulDiv(max(min(rct.bottom / 20, 18), 8), GetDeviceCaps(hdc_vis, LOGPIXELSY), 72);

	DeleteObject(vis_lyric_font);
	DeleteObject(vis_lyric_font_b);

	vis_lyric_font   = CreateFont(-vis_lyrics_font_size,
                        0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                        DEFAULT_PITCH, skin_settings.font_display);
	vis_lyric_font_b = CreateFont(-vis_lyrics_font_size,
                        0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                        DEFAULT_PITCH, skin_settings.font_display);

	/* maximized mode won't need a border */

	if(maxd)
	{
		//SetWindowRgn(window_vis, 0, 1);

		drawrect(hdc_vis, 0, 0, rct.right, rct.bottom, 0);
		
		skin.shared->call_function(call_visualizations_refresh, v_fennec_refresh_force_less, 0, 0);
		return;

	}else{
		drawrect(hdc_vis, 0, 0, rct.right, rct.bottom, 0);
		
		skin.shared->call_function(call_visualizations_refresh, v_fennec_refresh_force_less, 0, 0);
	}


	skin.shared->call_function(call_visualizations_refresh, v_fennec_refresh_force_less, 0, 0);

	vis_lyric_current_action = vis_lyric_current_laction;
	vis_lyrics_draw(hdc_vis);
}

int vis_refresh(void)
{
	WINDOWPLACEMENT  wp;

	GetWindowPlacement(window_vis, &wp);

	vis_draw_background((wp.showCmd == SW_MAXIMIZE));

	return 1;
}

int vis_get_position(RECT *retp)
{
	RECT             rct;
	WINDOWPLACEMENT  wp;

	if(!IsWindow(window_vis))
	{
		retp->left   = 0;
		retp->top    = 0;
		retp->right  = 0;
		retp->bottom = 0;
		return 0;
	}

	GetClientRect(window_vis, &rct);

	GetWindowPlacement(window_vis, &wp);

	if(wp.showCmd == SW_MAXIMIZE)
	{
		retp->left   = 0;
		retp->top    = 0;
		retp->right  = rct.right;
		retp->bottom = rct.bottom;

	}else{

		retp->left   = coords.window_vis.vis_l;
		retp->top    = coords.window_vis.vis_t;
		retp->right  = rct.right  - coords.window_vis.vis_r - retp->left;
		retp->bottom = rct.bottom - coords.window_vis.vis_b - retp->top;
	}
}

int visualization_messages(int id, int mdata, int sdata)
{
	if(vis_message)
		return vis_message(id, mdata, sdata);
	else
		return 0;
}

LRESULT CALLBACK callback_vis_window(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, dx, dy, lx, ly, maximized = 0;


	switch(msg)
	{
	case WM_TIMER:

		
		break;

	case WM_USER: /* overlay message: lparam = &hdc */
		{
			HDC odc = *((HDC*)lParam);
			vis_lyrics_draw(odc);
		}
		break;

	case WM_MOUSEMOVE:
		{
			RECT rct;
			int  x = (int)LOWORD(lParam), y = (int)HIWORD(lParam);

			GetClientRect(hwnd, &rct);

			if(x > rct.right - 20 && y > rct.bottom - 20 &&
			   x <= rct.right && y <= rct.bottom)
			{
				SetCursor(LoadCursor(0, IDC_SIZENWSE));
			}

			if(mdown == 1)
			{
				POINT pt;

				GetCursorPos(&pt);

				move_docking_window(window_id_vis, pt.x - dx, pt.y - dy);
			
			}else if(mdown == 2){ /* resize */
				
				POINT pt;

				GetCursorPos(&pt);

				skin_settings.vis_w = max(pt.x - lx + dx, 100);
				skin_settings.vis_h = max(pt.y - ly + dy, 50);

				if(skin_settings.vis_d)
				{
					if(skin_main_width > skin_settings.vis_w - 10 && skin_main_width < skin_settings.vis_w + 10)
						skin_settings.vis_w = skin_main_width;

					if(skin_main_height > skin_settings.vis_h - 10 && skin_main_height < skin_settings.vis_h + 10)
						skin_settings.vis_h = skin_main_height;
				}

				SetWindowPos(hwnd, 0, 0, 0, skin_settings.vis_w, skin_settings.vis_h, SWP_NOMOVE | SWP_NOZORDER);

				vis_draw_background(maximized);
			}else{

				visualization_messages(2 /* mouse move */, x, y);

			}

			if(!maximized)
			{
				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vis.button_close, coords.window_vis.button_close_align, skin_settings.vis_w, skin_settings.vis_h))
				{
					show_tip(oooo_skins_close, 0);
					blt_coord_vpos_nozoom(hdc_vis, mdc_sheet, 1, &coords.window_vis.button_close, coords.window_vis.button_close_align, skin_settings.vis_w, skin_settings.vis_h);
				}else{
					blt_coord_vpos_nozoom(hdc_vis, mdc_sheet, 0, &coords.window_vis.button_close, coords.window_vis.button_close_align, skin_settings.vis_w, skin_settings.vis_h);
				}

				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vis.button_max, coords.window_vis.button_max_align, skin_settings.vis_w, skin_settings.vis_h))
				{
					show_tip(oooo_skins_maximize, 0);
					blt_coord_vpos_nozoom(hdc_vis, mdc_sheet, 1, &coords.window_vis.button_max, coords.window_vis.button_max_align, skin_settings.vis_w, skin_settings.vis_h);
				}else{
					blt_coord_vpos_nozoom(hdc_vis, mdc_sheet, 0, &coords.window_vis.button_max, coords.window_vis.button_max_align, skin_settings.vis_w, skin_settings.vis_h);
				}
			}
		}
		break;

	case WM_MOUSEWHEEL:
		SendMessage(skin.wnd, msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
		if(visualization_messages(msg_keys, (int)wParam, (int)lParam))break;
		SendMessage(skin.wnd, msg, wParam, lParam);
		break;

	case WM_LBUTTONDBLCLK:
offset_fullscreen:
		{
			WINDOWPLACEMENT  wp;

			GetWindowPlacement(hwnd, &wp);

			if(wp.showCmd == SW_MAXIMIZE)
			{
				ShowWindow(hwnd, SW_RESTORE);
				maximized = 0;
			}else{
				ShowWindow(hwnd, SW_MAXIMIZE);
				maximized = 1;
			}

			sys_sleep(30);
			vis_draw_background(maximized);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			RECT rct;

			if(visualization_messages(msg_leftdown, (int)LOWORD(lParam), (int)HIWORD(lParam)))break;

			if(maximized)break;

			if(!maximized)
			{
				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vis.button_close, coords.window_vis.button_close_align, skin_settings.vis_w, skin_settings.vis_h))
				{
					if(skin_settings.skin_lock)
					{
						SendMessage(skin.wnd, WM_DESTROY, 0, 0);
					}else{
						vis_close();
						skin_settings.vis_show = 0;
					}
					break;
				}

				if(incoord_vpos_nozoom((int)LOWORD(lParam), (int)HIWORD(lParam), &coords.window_vis.button_max, coords.window_vis.button_max_align, skin_settings.vis_w, skin_settings.vis_h))
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

			SetCapture(hwnd);
		}
		break;

	case WM_LBUTTONUP:
		mdown = 0;

		ReleaseCapture();

		if(visualization_messages(msg_leftup, (int)LOWORD(lParam), (int)HIWORD(lParam)))break;
		break;

	case WM_RBUTTONDOWN:
		visualization_messages(msg_rightdown, (int)LOWORD(lParam), (int)HIWORD(lParam));
		break;

	case WM_RBUTTONUP:
		if(!visualization_messages(msg_rightup, (int)LOWORD(lParam), (int)HIWORD(lParam)))
		{
			skin.shared->general.show_settings(0, 0, panel_visualizations);
		}
		break;

	case WM_CREATE:
		hdc_vis = GetDC(hwnd);

		vis_lyrics_font_size = MulDiv(20, GetDeviceCaps(hdc_vis, LOGPIXELSY), 72);

		vis_lyric_font   = CreateFont(-vis_lyrics_font_size,
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                DEFAULT_PITCH, skin_settings.font_display);
		vis_lyric_font_b = CreateFont(-vis_lyrics_font_size,
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                DEFAULT_PITCH, skin_settings.font_display);


		vis_lyric_timer = SetTimer(hwnd, 4000101, 10, 0);
		break;

	case WM_DESTROY:
		DeleteObject(vis_lyric_font);
		KillTimer(hwnd, vis_lyric_timer);
		break;

	case WM_PAINT:
		vis_draw_background(maximized);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}





int vis_lyrics_loadcurrent(void)
{
	int                     id, i;
	struct fennec_audiotag  ctag;
	const string            fpath = skin.shared->audio.output.playlist.getsource(skin.shared->audio.output.playlist.getcurrentindex());


	if(!fpath)return 0;
	if(str_icmp(vis_lyrics_lastfile, fpath) == 0)return 0;

	str_cpy(vis_lyrics_lastfile, fpath);

	vis_lyric_tag_len = 0;
	vis_lyric_current_text = 0;
	vis_lyric_current_text_len = 0;
	vis_lyric_current_text_pos = 0;
	vis_lyric_current_action = 0;
	vis_markers = 0;
	vis_marker_last = 0;


	if(vis_lyric_tag)sys_mem_free(vis_lyric_tag);
		vis_lyric_tag = 0;

	id = skin.shared->audio.input.tagread(fpath, &ctag);

	if(ctag.tag_lyric.tsize)
	{
		vis_lyric_tag = (string) sys_mem_alloc((ctag.tag_lyric.tsize + 16) * sizeof(letter));

		str_cpy(vis_lyric_tag, ctag.tag_lyric.tdata);
	}else{
		if(vis_lyric_tag)sys_mem_free(vis_lyric_tag);
		vis_lyric_tag = 0;
	}

	skin.shared->audio.input.tagread_known(id, 0, &ctag);


	if(vis_lyric_tag)
	{
		vis_lyric_tag_len = (int)str_len(vis_lyric_tag);
		for(i=0; i<vis_lyric_tag_len; i++)
		{
			if(vis_lyric_tag[i] == uni('\r'))vis_lyric_tag[i] = uni('\0');
			else if(vis_lyric_tag[i] == uni('\n'))vis_lyric_tag[i] = uni('\0');

			if(!vis_markers)
			{
				if(vis_lyric_tag[i] == uni('[') && (vis_lyric_tag_len - i) > 9)
				{
					if(str_ncmp(vis_lyric_tag + i, uni("[markers:"), 9) == 0)
					{
						vis_markers = vis_lyric_tag + i;
						vis_marker_pos = 9;
						vis_marker_show = -1;
						vis_marker_hide = -1;
						vis_lyric_current_text_pos = 0;
					}
				}
			}
		}
	}

	return 1;
}

int vis_lyrics_getnextmarker(void)
{
	int      i, d;
	letter   str_m[255];


	if(vis_marker_pos == -1)return 0;
	for(i=0, d=0; ;i++, d++)
	{
		str_m[d] = vis_markers[vis_marker_pos + i];

		if(str_m[d] == uni('/') || str_m[d] == uni('\\'))
		{
			str_m[d] = uni('\0');
			vis_marker_show = str_stoi(str_m);
			d = 0;
		}
		if(str_m[d] == uni(',') || str_m[d] == uni(']'))
		{
			if(str_m[d] == ']')
				vis_marker_pos = -1;
			else
				vis_marker_pos += i + 1;

			str_m[d] = uni('\0');
			vis_marker_hide = str_stoi(str_m);

			
			break;
		}
	}

	return 1;
}

int vis_lyrics_timercall(void)
{
	double pos;
	
	if(!vis_lyric_tag) return 0;
	if(!vis_markers) return 0;


	if(skin.shared->audio.output.getplayerstate() == v_audio_playerstate_playing)
	{
		pos = skin.shared->audio.output.getposition_ms();
	}else{
		return 0;
	}
	
	if(pos < (double)vis_marker_last)
	{
		/* oh, you seeked it! */
		vis_marker_pos = 9;
		vis_lyrics_getnextmarker();
		vis_marker_last = 0;
		vis_lyric_current_text_pos = 0;
		vis_lyric_current_action = 2;
		return 0;
	}
	
	if(vis_marker_pos == -1)
	{
		vis_lyric_current_text_len = 0;
		return 0;
	}

	if(vis_marker_show == -1 && vis_marker_hide == -1)
	{
		if(!vis_lyrics_getnextmarker())return 0;
	}

	if(vis_marker_show != -1)
	{
		if((double)vis_marker_show <= pos)
		{
			vis_lyric_current_text_len = (int)str_len(vis_lyric_tag + vis_lyric_current_text_pos);
			vis_lyric_current_text = vis_lyric_tag + vis_lyric_current_text_pos;

			vis_marker_last = vis_marker_show;
			vis_marker_show = -1;
			vis_lyric_current_action = 1; /* show */
		}
	
	}else if(vis_marker_hide != -1){
		if((double)vis_marker_hide <= pos)
		{
			vis_lyric_current_text_pos += vis_lyric_current_text_len;

			for(;;)
			{
				if(vis_lyric_tag[vis_lyric_current_text_pos] == uni('\0'))vis_lyric_current_text_pos++;
				else goto point_hidesub;
			}

point_hidesub:

			vis_lyric_current_text_len = 0;
			vis_lyric_current_text = vis_lyric_tag;

			vis_marker_last = vis_marker_hide;
			vis_marker_hide = -1;

			vis_lyric_current_action = 2; /* clear */
		}
	
	}

	return 1;
}

void vis_lyrics_refresh(int inum)
{
	if(inum >= v_fennec_refresh_force_high)
	{
		vis_lyrics_loadcurrent();
	}
}

void vis_lyrics_draw(HDC dc)
{
	vis_lyrics_timercall();

	if(vis_lyric_current_text_len && vis_lyric_current_action == 1)
	{
		int      bkmod;
		COLORREF col;
		RECT     rct;
		WINDOWPLACEMENT  wp;
		HFONT    oldfont;

		GetWindowPlacement(window_vis, &wp);

		GetClientRect(window_vis, &rct);

		bkmod = GetBkMode(dc);
		col = GetTextColor(dc);

		SetBkMode(dc, TRANSPARENT);


		if(wp.showCmd != SW_MAXIMIZE)
		{
			rct.top     = rct.bottom - 13 -  (vis_lyrics_font_size + 10) ;
			rct.bottom  -= 13;
			rct.left    =  20;
			rct.right   -= 28;
		}else{
			rct.top     = rct.bottom -  (vis_lyrics_font_size + 10) ;
		}

		//SetTextColor(dc, 0x0);
		//oldfont = (HFONT)SelectObject(dc, vis_lyric_font_b);
		//DrawText(dc, vis_lyric_current_text, vis_lyric_current_text_len, &rct, DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER | DT_SINGLELINE);
		//SelectObject(dc, oldfont);

		rct.left    += 1;
		rct.right   += 1;

		SetTextColor(dc, 0xffffff);
		oldfont = (HFONT)SelectObject(dc, vis_lyric_font);
		DrawText(dc, vis_lyric_current_text, vis_lyric_current_text_len, &rct, DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER | DT_SINGLELINE);
		SelectObject(dc, oldfont);



		SetBkMode(dc, bkmod);
		SetTextColor(dc, col);

		//vis_lyric_current_laction = vis_lyric_current_action;
		//vis_lyric_current_action = 0;
	}

	if(vis_lyric_current_action == 2)
	{
		RECT     rct;
		WINDOWPLACEMENT  wp;

		GetWindowPlacement(window_vis, &wp);


		GetClientRect(window_vis, &rct);

		if(wp.showCmd != SW_MAXIMIZE)
		{
			BitBlt(dc, 10, rct.bottom - 13 - 20, rct.right - (10 + 12), 20, 0, 0, 0, BLACKNESS);
		}else{
			BitBlt(dc, 0, rct.bottom - 20, rct.right, 20, 0, 0, 0, BLACKNESS);
		}

		vis_lyric_current_laction = vis_lyric_current_action;
		vis_lyric_current_action = 0;
	}
}