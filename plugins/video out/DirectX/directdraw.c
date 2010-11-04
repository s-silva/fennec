#include "main.h"
#define WIN32_LEAN_AND_MEAN
#include "ddraw.h"

int default_sw = 860;
int default_sh = 430;

#define max_dmodes 32

extern string    subtitle_text;
extern string    subtitle_text_sec;

int show_subtitles = 1;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);



#define window_class_name uni("directdraw_fennec_video")


struct dmode
{
	int w, h;
}a_dmodes[max_dmodes];

int a_dmode_count = 0;

HWND  video_window = 0, video_window_parent = 0;
typedef HRESULT (WINAPI * DIRECTDRAWCREATE) (GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,IUnknown FAR *pUnkOuter);
static HMODULE library = 0;
static LPDIRECTDRAW lpDD = 0;
static LPDIRECTDRAWSURFACE lpDDS = 0;
static LPDIRECTDRAWSURFACE lpDDS_back;
static LPDIRECTDRAWSURFACE surface_osd = 0;
static WNDCLASS wc;
static LPDIRECTDRAWCLIPPER lpDDC = 0;
static LPDIRECTDRAWSURFACE lpDDS_secondary = 0;
int                        d_width = 100, d_height = 100, window_w = 0, window_h = 0;
int                        use_osd_surface = 1;
extern int                 frame_w, frame_h;
int                        osd_created = 0;
int                        size_is_ok = 0;

double                     video_zoom_x = 1.0, video_zoom_y = 1.0;
int                        video_dx = 0, video_dy = 0;
int                        crop_pos_x = 0, crop_pos_y = 0;

HPEN                       crop_rect_pen;
HBRUSH                     crop_rect_brush;
int                        show_crop_rect = 0;
int                        mouse_hold_count = 0;

HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC pddsd, LPVOID Context);

int directdraw_init(HWND hwndp)
{
    int                 width = 320, height = 200;
    RECT                rect;
    DDPIXELFORMAT       format;
    DDSURFACEDESC       descriptor;
    DIRECTDRAWCREATE    DirectDrawCreate;

	if(lpDDS) return 0;
	if(!hwndp) return 0;
	if(video_window) return 0; /* you gotta uninitialize it first */

    library = (HMODULE) LoadLibrary(uni("ddraw.dll"));
    if(!library) return 0;

    DirectDrawCreate = (DIRECTDRAWCREATE) GetProcAddress(library, "DirectDrawCreate");
    if(!DirectDrawCreate) return 0;

    if(FAILED(DirectDrawCreate(0,&lpDD,0))) return 0;

    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = 0;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = window_class_name;

    RegisterClass(&wc);

    video_window = CreateWindow(window_class_name, uni("Video"), WS_CHILD, 0, 0, width, height, hwndp, 0, 0, 0);

    ShowWindow(video_window, SW_NORMAL);

    if(FAILED(IDirectDraw2_SetCooperativeLevel(lpDD, video_window, DDSCL_NORMAL))) return 0;


    descriptor.dwSize         = sizeof(descriptor);
    descriptor.dwFlags        = DDSD_CAPS;
    descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

    if(FAILED(IDirectDraw2_CreateSurface(lpDD, &descriptor, &lpDDS, 0))) return 0;

    descriptor.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    descriptor.dwWidth        = d_width ;
    descriptor.dwHeight       = d_height;

    if(FAILED(IDirectDraw2_CreateSurface(lpDD, &descriptor, &lpDDS_secondary, 0))) return 0;
    
    if(FAILED(IDirectDraw2_CreateClipper(lpDD, 0, &lpDDC, 0))) return 0;

    if(FAILED(IDirectDrawClipper_SetHWnd(lpDDC, 0, video_window))) return 0;

    if(FAILED(IDirectDrawSurface2_SetClipper(lpDDS, lpDDC))) return 0;
    
    lpDDS_back    = lpDDS_secondary;
    format.dwSize = sizeof(format);

    if(FAILED(IDirectDrawSurface2_GetPixelFormat(lpDDS, &format))) return 0;

    if(!(format.dwFlags & DDPF_RGB)) return 0;


	IDirectDraw2_EnumDisplayModes(lpDD, DDEDM_STANDARDVGAMODES, 0, 0, EnumDisplayModesCallback);


	osd_initialize();
	osd_created = 0;

    return 1;
}

int directdraw_uninit(void)
{
	if(!video_window) return 0;

	if(!lpDDS) return 0;

	KillTimer(video_window, 131);

    if(lpDDS_secondary) IDirectDrawSurface2_Release(lpDDS_secondary);
	if(lpDDS)           IDirectDrawSurface2_Release(lpDDS);
	if(surface_osd)     IDirectDrawSurface2_Release(surface_osd);
	if(lpDD)            IDirectDraw2_RestoreDisplayMode(lpDD);
	if(lpDD)            IDirectDraw2_SetCooperativeLevel(lpDD, video_window, DDSCL_NORMAL);
	if(lpDD)            IDirectDraw2_Release(lpDD);
	if(video_window)    DestroyWindow(video_window);
	if(library)         FreeLibrary(library);
	
	library         = 0;
	lpDD            = 0;
	lpDDS           = 0;
	lpDDS_secondary = 0;
	surface_osd     = 0;

	video_window    = 0;
	return 1;
}

int replace_i_str(string str, const string s, const string d)
{
	string mstr = str;
	int len = (int)str_len(str), pos = 0, i;
	int dlen = (int)str_len(d);
	int slen = (int)str_len(s);
	int ldiff = dlen - slen;
	int found = 0;

	while(*mstr)
	{
		if(str_incmp(mstr, s, slen) == 0)
		{
			found = 1;

			if(dlen < slen)
			{
				str_cpy(mstr + dlen, mstr + slen);

			}else if(dlen > slen){

				for(i=len; i>pos; i--)
				{
					str[i + ldiff] = str[i];
				}
			}

			memcpy(mstr, d, dlen * sizeof(letter));

			mstr += dlen;
			len += ldiff;
			pos += dlen;
		}
		mstr++;
		pos++;
	}
	return found;
}


void directdraw_update(int mode)
{
    RECT   source;
    RECT   destination;
	RECT   rectosd, rct, rectwindow;
    POINT  point;
	int    rw, rh, rx = 0, ry = 0; /* relative sizes */
	int    tmprw, tmprh, tmprx = 0, tmpry = 0; /* temporary relative sizes */
	int    ww, wh;                 /* video window sizes */
	float  sc;                     /* scale */
	int    r_frame_w, r_frame_h;   /* resized values of frame size (aspect ratio based) */
	

	if(!video_window) return;
	if(!lpDDS)return;
	if(use_osd_surface && !surface_osd)return;
	if(!lpDDS_secondary)return;

	//frame_w = default_sw;
	//frame_h = default_sh;

	if(lpDDS)
	{
		r_frame_w = frame_w;
		r_frame_h = frame_h;
		
		if(aspect_ratio > 0.0)
			r_frame_h = (int)((double)r_frame_h / aspect_ratio);

		source.left   = 0;
		source.top    = 0;
		source.right  = d_width;
		source.bottom = d_height;

		rectwindow.left   = 0;
		rectwindow.top    = 0;
		rectwindow.right  = window_w;
		rectwindow.bottom = window_h;


		point.x = 0;
		point.y = 0;
		ClientToScreen(video_window, &point);

		GetClientRect(video_window, &destination);
		GetClientRect(video_window, &rct);


		destination.left   += point.x;
		destination.top    += point.y;
		destination.right  += point.x;
		destination.bottom += point.y;

		//vdata.getdata(get_window_video_rect, 0, &rct, 0);
		//vdata.getdata(get_window_video_dc, 0, &hdc, 0);

		//if(w <= 0)w = 256;
		//if(h <= 0)h = 256;

		ww = rw = rct.right - rct.left;
		wh = rh = rct.bottom - rct.top;

		if(frame_w > frame_h)
		{
			sc = (float)rw / (float)r_frame_w;
		}else{
			sc = (float)rh / (float)r_frame_h;
		}

		/* to keep aspect ratio, both width and height should be scaled equally */
		
		rw = (int)((float)r_frame_w * sc);
		rh = (int)((float)r_frame_h * sc);

		/* we gotta handle video's corners vertically and check if it's gonna surpass the parent window's borders */

		if(rh > wh)
			sc = (float)wh / (float)r_frame_h;

		if(rw > ww)
			sc = (float)ww / (float)r_frame_w;

		/* any changes? calculate'em again */

		rw = (int)((float)r_frame_w * sc);
		rh = (int)((float)r_frame_h * sc);

		/* calculate video position */

		rx = (ww - rw) / 2;
		ry = (wh - rh) / 2;


		if(use_osd_surface)
		{

			
			if(video_zoom_x <= 1.0)
			{
				video_zoom_x = 1.0;
				crop_pos_x = 0;
			}
			if(video_zoom_y <= 1.0)
			{
				video_zoom_y = 1.0;
				crop_pos_y = 0;
			}


			
			//tmprw = (rx + rw) * video_zoom;
			//tmprh = (ry + rh) * video_zoom;

			/*if(tmprw < destination.right && tmprh < destination.bottom)
			{
				rw *= video_zoom;
				rh *= video_zoom;
			}else{
				source.right /= video_zoom;
				source.bottom /= video_zoom;
			}	*/

			//if(source.left + crop_pos_x > source.right)crop_pos_x = source.right - source.left;
			//if(source.top + crop_pos_y > source.bottom)crop_pos_y = source.bottom - source.top;

			source.left = crop_pos_x;
			source.top = crop_pos_y;
			source.right = crop_pos_x + (d_width / video_zoom_x);
			source.bottom = crop_pos_y + (d_height / video_zoom_y);

			if(source.left < 0)
			{
				source.left = 0;
				source.right = (d_width / video_zoom_x);
			}
			if(source.top < 0)
			{
				source.top = 0;
				source.bottom = (d_height / video_zoom_y);
			}

			if(source.right > d_width)
			{
				source.left = d_width - (d_width / video_zoom_x);
				source.right = d_width;
			}
			if(source.bottom > d_height)
			{
				source.top = d_height - (d_height / video_zoom_y);
				source.bottom = d_height;
			}

			rectosd.left   = rx;
			rectosd.top    = ry;
			rectosd.right  = rw + rx;
			rectosd.bottom = rh + ry;

			if(video_zoom_y > 1.0 || video_zoom_x > 1.0)
			{
				rectosd.left   = 0;
				rectosd.top    = 0;
				rectosd.right  = window_w;
				rectosd.bottom = window_h;
			}
			
		}


		if(!use_osd_surface)
		{
			IDirectDrawSurface2_Blt(lpDDS, &destination, lpDDS_secondary, &source, DDBLT_WAIT, 0);
		}else{
			HDC  hdc;
			DDBLTFX bfx;
			memset(&bfx, 0, sizeof(bfx));
			bfx.dwFillColor = 0x000000;
			bfx.dwSize      = sizeof(bfx);

			IDirectDrawSurface2_Blt(surface_osd, 0, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bfx);
			
			if(mode)
			{
				
				HRESULT res = IDirectDrawSurface2_Blt(surface_osd, &rectosd, lpDDS_secondary, &source, DDBLT_WAIT | DDBLT_ZBUFFER, 0);
				
				if(show_crop_rect)
				{
					RECT rcr;
					float ar;
					int   vsize = 100, vw;

					ar = d_width / d_height;
					vw = (vsize * ar);

					rcr.top = 5;
					rcr.left = 5;
					rcr.bottom = 5 + vsize;
					rcr.right = 5 + vw;
					
					IDirectDrawSurface2_Blt(surface_osd, &rcr, lpDDS_secondary, &source, DDBLT_WAIT | DDBLT_ZBUFFER, 0);

				}

			}

			if(!FAILED(IDirectDrawSurface2_GetDC(surface_osd, &hdc)))
			{
				float ar;
				int   vsize = 100, vw;
				HPEN    hold_pen;
				HBRUSH  hold_brush;

				if(subtitle_text && show_subtitles)
				{
					RECT   rct, nrct;
					HFONT  nfont, ofont;
					int    subs_fontsize, soffset;
					string subtitle_font_face = uni("Arial");
					static letter subtitle_str[4096];
					static int    sub_italic = 0, sub_bold = 0, sub_underlined = 0;
					static string last_substr;

					if(subtitle_text != last_substr)
					{
						last_substr = subtitle_text;
						str_cpy(subtitle_str, subtitle_text);
						if(replace_i_str(subtitle_str, uni("<i>"), uni("")))
						{
							sub_italic = 1;
							replace_i_str(subtitle_str, uni("</i>"), uni(""));
						}
						if(replace_i_str(subtitle_str, uni("<b>"), uni("")))
						{
							sub_bold = 1;
							replace_i_str(subtitle_str, uni("</b>"), uni(""));
						}
						if(replace_i_str(subtitle_str, uni("<u>"), uni("")))
						{
							sub_underlined = 1;
							replace_i_str(subtitle_str, uni("</u>"), uni(""));
						}
					}

					rct.top     = window_h;
					rct.bottom  = window_h;
					rct.left    = 0;
					rct.right   = window_w;

					subs_fontsize = min(max((min(window_h, window_w) * 15) / 250, 10), 22);
					soffset = min(subs_fontsize / 10, 2);

					nfont = CreateFont(-MulDiv(subs_fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72),
						0, 0, 0, sub_bold ? FW_BOLD : FW_NORMAL, sub_italic, sub_underlined, 0, DEFAULT_CHARSET,
										OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
										DEFAULT_PITCH, subtitle_font_face);

					ofont = (HFONT) SelectObject(hdc, nfont);

					SetBkMode(hdc, TRANSPARENT);

					nrct.left  = 0;
					nrct.right = window_w;

					DrawText(hdc, subtitle_str, str_len(subtitle_str), &nrct, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);

					rct.top -= nrct.bottom - nrct.top;
					rct.top -= window_h / 12;

					SetTextColor(hdc, 0x000000);

					rct.top   -= soffset; rct.left  -= soffset; rct.right -= soffset;
					DrawText(hdc, subtitle_str, str_len(subtitle_str), &rct, DT_CENTER | DT_WORDBREAK);
					rct.top   += soffset; rct.left  += soffset; rct.right += soffset;

					rct.top   += soffset; rct.left  += soffset; rct.right += soffset;
					DrawText(hdc, subtitle_str, str_len(subtitle_str), &rct, DT_CENTER | DT_WORDBREAK);
					rct.top   -= soffset; rct.left  -= soffset; rct.right -= soffset;

					SetTextColor(hdc, 0xffffff);
					DrawText(hdc, subtitle_str, str_len(subtitle_str), &rct, DT_CENTER | DT_WORDBREAK);

					SelectObject(hdc, ofont);
					DeleteObject(nfont);	
				} /* </if - subtitles_text> */

				if(subtitle_text_sec)
				{
					RECT   rct, nrct;
					HFONT  nfont, ofont;
					int    subs_fontsize, soffset;
					string subtitle_font_face = uni("Arial");
					static letter subtitle_str[4096];
					static int    sub_italic = 0, sub_bold = 0, sub_underlined = 0;
					static string last_substr;

					if(subtitle_text_sec != last_substr)
					{
						last_substr = subtitle_text_sec;
						str_cpy(subtitle_str, subtitle_text_sec);
						if(replace_i_str(subtitle_str, uni("<i>"), uni("")))
						{
							sub_italic = 1;
							replace_i_str(subtitle_str, uni("</i>"), uni(""));
						}
						if(replace_i_str(subtitle_str, uni("<b>"), uni("")))
						{
							sub_bold = 1;
							replace_i_str(subtitle_str, uni("</b>"), uni(""));
						}
						if(replace_i_str(subtitle_str, uni("<u>"), uni("")))
						{
							sub_underlined = 1;
							replace_i_str(subtitle_str, uni("</u>"), uni(""));
						}
					}

					rct.top     = window_h;
					rct.bottom  = window_h;
					rct.left    = 0;
					rct.right   = window_w;

					subs_fontsize = min(max((min(window_h, window_w) * 15) / 250, 10), 22);
					soffset = min(subs_fontsize / 10, 2);

					nfont = CreateFont(-MulDiv(subs_fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72),
										0, 0, 0, sub_bold ? FW_BOLD : FW_NORMAL, sub_italic, sub_underlined, 0, DEFAULT_CHARSET,
										OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
										DEFAULT_PITCH, subtitle_font_face);

					ofont = (HFONT) SelectObject(hdc, nfont);

					SetBkMode(hdc, TRANSPARENT);

					nrct.left  = 0;
					nrct.right = window_w;

					DrawText(hdc, subtitle_str, str_len(subtitle_str), &nrct, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);

					rct.top = window_h / 16;

					SetTextColor(hdc, 0x000000);

					rct.top   -= soffset; rct.left  -= soffset; rct.right -= soffset;
					DrawText(hdc, subtitle_str, str_len(subtitle_str), &rct, DT_CENTER | DT_WORDBREAK);
					rct.top   += soffset; rct.left  += soffset; rct.right += soffset;

					rct.top   += soffset; rct.left  += soffset; rct.right += soffset;
					DrawText(hdc, subtitle_str, str_len(subtitle_str), &rct, DT_CENTER | DT_WORDBREAK);
					rct.top   -= soffset; rct.left  -= soffset; rct.right -= soffset;

					SetTextColor(hdc, 0xffffff);
					DrawText(hdc, subtitle_str, str_len(subtitle_str), &rct, DT_CENTER | DT_WORDBREAK);

					SelectObject(hdc, ofont);
					DeleteObject(nfont);	
				} /* </if - subtitles_text> */


				/* draw osd controls */
				
				osd_display(hdc, rectosd.right, rectosd.bottom);
				
	
				
				if(show_crop_rect)
				{
					ar = d_width / d_height;
					vw = (vsize * ar);

					hold_pen = (HPEN)SelectObject(hdc, crop_rect_pen);
					hold_brush = (HBRUSH)SelectObject(hdc, crop_rect_brush);

					Rectangle(hdc, 5, 5, 5 + vw, 5 + vsize);
					Rectangle(hdc, 5 + ((source.left * vw) / d_width), 5 + ((source.top * vsize) / d_height), 5 + ((source.right * vw) / d_width), 5 + ((source.bottom * vsize) / d_height));

					SelectObject(hdc, hold_pen);
					SelectObject(hdc, hold_brush);
				}
				
				IDirectDrawSurface2_ReleaseDC(surface_osd, hdc);
			}

			IDirectDrawSurface2_Blt(lpDDS, &destination, surface_osd, &rectwindow, DDBLT_WAIT, 0);
		}
	}
}

void pic_resize(uint8_t *picd, uint8_t *pics, int w, int h, int nw, int nh)
{
	int x, y;
	float xf = (float)nw / (float)w;
	float yf = (float)nh / (float)h;

#define poso(ex)((((y * nw) + x) * 4) + ex)
#define posn(ex)((((y * w) + x) * 4) + ex)
	
	if(!picd)return;

	for(y=0; y<h; y++)
	{
		for(x=0; x<w; x++)
		{
			picd[poso(0)] = pics[posn(0)];//
			picd[poso(1)] = pics[posn(1)];
			picd[poso(2)] = pics[posn(2)];
			picd[poso(3)] = 0;
		}
	}
}

int size_in_list(int w, int h)
{
	int i;
	for(i=0; i<a_dmode_count; i++)
	{
		if(a_dmodes[i].w == w && a_dmodes[i].h == h)return 1;
	}
	return 0;
}


int directdraw_draw(void *pic, int w, int h)
{
	DDSURFACEDESC  descriptor;
	RECT           rct;
	HWND           hwndp;

#define fourcc(a,b,c,d) (( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) ))

	int bh = h, bw = w;
	//w = default_sw;
	//h = default_sh;

	vdata.getdata(get_window_video, 0, &hwndp, 0);

	if(!hwndp) return 0;
	if(video_window_parent != hwndp)
	{
		video_uninit();
		video_init(hwndp);
		video_window_parent = hwndp;
	}

	if(w != d_width || h != d_height)
	{

		if(size_in_list(w, h))size_is_ok = 1;
		else size_is_ok = 0;

		if(w && h)
		{
			d_width  = w;
			d_height = h;

			if(lpDDS_secondary)
				IDirectDrawSurface2_Release(lpDDS_secondary);

			memset(&descriptor, 0, sizeof(descriptor));
			
			descriptor.dwSize         = sizeof(descriptor);
			descriptor.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			descriptor.dwWidth        = d_width;
			descriptor.dwHeight       = d_height;

		
			if(FAILED(IDirectDraw2_CreateSurface(lpDD, &descriptor, &lpDDS_secondary, 0))) return 0;
	    	
			lpDDS_back    = lpDDS_secondary;
		}
	}

	if(use_osd_surface)
	{
		GetClientRect(video_window, &rct);

		if(rct.right != window_w || rct.bottom != window_h || osd_created == 0)
		{
			window_w = rct.right;
			window_h = rct.bottom;

			if(surface_osd)
				IDirectDrawSurface2_Release(surface_osd);

			memset( &descriptor, 0, sizeof(descriptor));

			descriptor.dwSize         = sizeof(descriptor);
			descriptor.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			descriptor.dwWidth        = window_w;
			descriptor.dwHeight       = window_h;

			if(FAILED(IDirectDraw2_CreateSurface(lpDD, &descriptor, &surface_osd, 0))) return 0;

			osd_created = 1;
		}
	}

    IDirectDrawSurface2_Restore(lpDDS);
    if(pic)
		IDirectDrawSurface2_Restore(lpDDS_secondary);

	memset(&descriptor, 0, sizeof(descriptor));
    descriptor.dwSize = sizeof(descriptor);

	if(pic)
	{
		HRESULT res;
		

		if(size_is_ok)
		{
			res = IDirectDrawSurface2_Lock(lpDDS_back, 0, &descriptor, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);
			if(FAILED(res)) return 0;

			memcpy(descriptor.lpSurface, pic, w * h * 4);

			IDirectDrawSurface2_Unlock(lpDDS_back, descriptor.lpSurface);

		}else{

			HBITMAP bmp;
			HDC     hdc, cdc;
						
			bmp = CreateBitmap(w, h, 1, 32, pic);

			if(!FAILED(IDirectDrawSurface2_GetDC(lpDDS_back, &hdc)))
			{
				cdc = CreateCompatibleDC(0);
				
				SelectObject(cdc, bmp);
				BitBlt(hdc, 0, 0, w, h, cdc, 0, 0, SRCCOPY);
				IDirectDrawSurface2_ReleaseDC(lpDDS_back, hdc);

				DeleteDC(cdc);
			}

			DeleteObject(bmp);
		}

		IDirectDrawSurface2_Flip(lpDDS, 0, DDFLIP_WAIT);
	}



	return 1;
}


static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int mdown = 0, v;
	static int mdown_x = 0, mdown_y = 0;
	static int mup_x = 0, mup_y = 0;

	switch(message)
	{
	case WM_TIMER:
		if(vdata.shared->audio.output.getplayerstate() != v_audio_playerstate_playing)
			video_display();

		if(mouse_hold_count == 200) break;

		if(mouse_hold_count > 100)
		{
			osd_hide_all();
			mouse_hold_count = 200;
		}else{
			mouse_hold_count++;
		}
		break;

	case WM_PAINT:
		videoout_refresh(0);
		break;

	case WM_CREATE:
		crop_rect_pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		crop_rect_brush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		SetTimer(hwnd, 131, 10, 0);
		break;

	case WM_MOUSEMOVE:
		mouse_hold_count = 0;
		if(mdown)
			v = osd_mouse_message(3, LOWORD(lParam), HIWORD(lParam));
		else
			v = osd_mouse_message(0, LOWORD(lParam), HIWORD(lParam));

		if(!v)
		{
			if(mdown && (video_zoom_x > 1.0 || video_zoom_y > 1.0))
			{
				crop_pos_x = mup_x - LOWORD(lParam) + mdown_x;
				crop_pos_y = mup_y - HIWORD(lParam) + mdown_y;
				show_crop_rect = 1;
			}else{
				SendMessage(GetParent(hwnd), message, wParam, lParam);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		mdown = 1;
		mdown_x = LOWORD(lParam);
		mdown_y = HIWORD(lParam);
		osd_mouse_message(1, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_RBUTTONDOWN:
		osd_mouse_message(2, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_LBUTTONUP:
		mdown = 0;
		mup_x = crop_pos_x;
		mup_y = crop_pos_y;
		show_crop_rect = 0;
	case WM_RBUTTONUP:
		{
			RECT rct, rctp;
			GetWindowRect(GetParent(hwnd), &rctp);
			GetWindowRect(hwnd, &rct);
			lParam = MAKELONG(LOWORD(lParam) + (rct.left - rctp.left), HIWORD(lParam) + (rct.top - rctp.top));

			SendMessage(GetParent(hwnd), message, wParam, lParam);
		}
		break;

	case WM_KEYDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		if(!osd_mouse_message(1, LOWORD(lParam), HIWORD(lParam)))
		{
			SendMessage(GetParent(hwnd), message, wParam, lParam);
		}
		break;

	case WM_DESTROY:
		DeleteObject(crop_rect_pen);
		DeleteObject(crop_rect_brush);
		break;
	}

    return DefWindowProc(hwnd, message, wParam, lParam);
}

HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC pddsd, LPVOID Context)
{
	if(a_dmode_count >= max_dmodes)return DDENUMRET_OK;

	a_dmodes[a_dmode_count].w = pddsd->dwWidth;
	a_dmodes[a_dmode_count].w = pddsd->dwHeight;

	a_dmode_count++;
    return DDENUMRET_OK;
}
