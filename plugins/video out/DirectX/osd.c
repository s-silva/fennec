#include "main.h"
#include "data/resource.h"
#include "../../../include/ids.h"

#define user_msg_start_keep_cursor 0x1
#define user_msg_end_keep_cursor   0x2


extern string    subtitle_text;
extern string    subtitle_text_sec;


#define align_top    1
#define align_left   2
#define align_bottom 4
#define align_right  8

#define osd_id_play      1
#define osd_id_stop      2
#define osd_id_previous  3
#define osd_id_next      4
#define osd_id_rewind    5
#define osd_id_forward   6
#define osd_id_group     7
#define osd_id_volume    8
#define osd_id_seek      9

#define osd_id_video     10
#define osd_id_audio     11
#define osd_id_chapter   12
#define osd_id_subtitles 13
#define osd_id_browse    14
#define osd_id_reset     15

#define osd_id_mute      16
#define osd_id_hide      17
#define osd_id_load      18
#define osd_id_menu      19
#define osd_id_posup     20
#define osd_id_posreset  21
#define osd_id_posdown   22
#define osd_id_posleft   23
#define osd_id_posright  24
#define osd_id_zoomin    25
#define osd_id_zoomout   26

#define group_bottom     1
#define group_top        2
#define group_right      3

#define move_step 15

extern void     *current_frame;

struct osd_item
{
	int  id;
	int  align;
	int  moving_pos;
	int  moving_side;
	int  active;
	int  sx, sy, w, h, sx_h, sy_h;
	int  x, y; /* x and y from aligned corner */
	int  group;
};


void osd_drawitem(HDC dc, struct osd_item *oi, int mode);
int osd_inpos_ex(struct osd_item *oi, int x, int y, int *xv, int *yv);
int osd_inpos(struct osd_item *oi, int x, int y);
void osd_action(int action, int id, double v);
void set_mov_side_group(int gid, int mside);
void set_mov_group(int gid, int mpos, int active);

#define osd_logo_w 193
#define osd_logo_h 227

#define osd_item_count 26

#define top_bar_width 206

struct osd_item   osd_items[osd_item_count] = {/*mp ms ac   sx   sy    w    h sxh  syh    x    y */
	{osd_id_play,      align_bottom | align_left,  0, 0, 0,   0,   0,  47,  21,  0, 123,  20,  50, group_bottom},
	{osd_id_stop,      align_bottom | align_left,  0, 0, 0,  47,   0,  24,  21, 47, 123,  70,  50, group_bottom},
	{osd_id_previous,  align_bottom | align_left,  0, 0, 0,  71,   0,  24,  21, 71, 123,  96,  50, group_bottom},
	{osd_id_next,      align_bottom | align_left,  0, 0, 0,  95,   0,  24,  21, 95, 123, 122,  50, group_bottom},
	{osd_id_rewind,    align_bottom | align_left,  0, 0, 0, 119,   0,  24,  21,119, 123, 148,  50, group_bottom},
	{osd_id_forward,   align_bottom | align_left,  0, 0, 0, 143,   0,  24,  21,143, 123, 174,  50, group_bottom},
	{osd_id_group,     align_bottom | align_left,  0, 0, 0, 188,   0,  21,  21,188, 123, 200,  50, group_bottom},
	{osd_id_volume,    align_bottom | align_right, 0, 0, 0,   0,  21, 151,   8,  0,  29,  20,  45, group_bottom},
	{osd_id_seek,      align_bottom | align_left,  0, 0, 0,   0,  21, 151,   8,  0,  29, 223,  45, group_bottom},
	/*---------------------------------------------------------------------------------------------------------*/
	{osd_id_video,     align_top | align_left,     0, 0, 0,   0,  39,  37,  21, 36,  39,   5,  15, group_top},
	{osd_id_audio,     align_top | align_left,     0, 0, 0,   0,  60,  37,  21, 36,  60,  44,  15, group_top},
	{osd_id_chapter,   align_top | align_left,     0, 0, 0,   0,  81,  37,  21, 36,  81,  83,  15, group_top},
	{osd_id_subtitles, align_top | align_left,     0, 0, 0,   0, 102,  37,  21, 36, 102, 126,  15, group_top},
	{osd_id_browse,    align_top | align_left,     0, 0, 0, 167,   0,  21,  21,167, 123, 163,  15, group_top},
	{osd_id_reset,     align_top | align_left,     0, 0, 0, 188,   0,  21,  21,188, 123, 185,  15, group_top},
	/*---------------------------------------------------------------------------------------------------------*/
	{osd_id_mute,      align_top | align_right,    0, 0, 0, 122,  37,  53,  20,122,  37,   4,  47, group_right},
	{osd_id_hide,      align_top | align_right,    0, 0, 0, 122,  57,  53,  20,122,  57,   4,  70, group_right},
	{osd_id_load,      align_top | align_right,    0, 0, 0, 122,  77,  53,  20,122,  77,   4, 105, group_right},
	{osd_id_menu,      align_top | align_right,    0, 0, 0, 122,  97,  53,  20,122,  97,   4, 128, group_right},
	{osd_id_posup,     align_top | align_right,    0, 0, 0, 175,  67,  23,  23,221,  67,  28, 177, group_right},
	{osd_id_posreset,  align_top | align_right,    0, 0, 0, 175,  90,  23,  23,221,  90,  28, 201, group_right},
	{osd_id_posdown,   align_top | align_right,    0, 0, 0, 175,  44,  23,  23,221,  44,  28, 225, group_right},
	{osd_id_posleft,   align_top | align_right,    0, 0, 0, 198,  44,  23,  23,244,  44,  52, 201, group_right},
	{osd_id_posright,  align_top | align_right,    0, 0, 0, 198,  67,  23,  23,244,  67,   4, 201, group_right},
	{osd_id_zoomin,    align_top | align_right,    0, 0, 0, 175,  21,  23,  23,221,  21,  28, 259, group_right},
	{osd_id_zoomout,   align_top | align_right,    0, 0, 0, 198,  21,  23,  23,244,  21,   4, 259, group_right}};


HDC      hdc_sheet = 0;
HBITMAP  bmp_sheet;
int      wnd_w, wnd_h;

int      mode_group = 1;

struct osd_item *highlight_oi;

int      seek_w = 0, volume_w = 0;
extern int window_w, window_h;
int      show_seek_position_tip = 0;
int      osd_lock_items = 0;

void osd_display(HDC dc, int w, int h)
{
	int i;
	struct osd_item *oi;

	wnd_w = window_w;
	wnd_h = window_h;

	if(!hdc_sheet)
		osd_initialize_in(dc);

	if(!current_frame || vdata.shared->audio.output.getplayerstate() == v_audio_playerstate_stopped)
	{
		if(wnd_w > osd_logo_w + 10 && wnd_h > osd_logo_h + 10)
		{
			BitBlt(dc, (wnd_w/2)-(osd_logo_w/2), (wnd_h/2)-(osd_logo_h/2), osd_logo_w, osd_logo_h, hdc_sheet, 269, 0, SRCCOPY);
		}else{
			int nw, nh, stretchmode;
			if(wnd_w < wnd_h)
			{
				nw = max(wnd_w - 20, 0);
				nh = (int)(nw * ((float)osd_logo_h / (float)osd_logo_w));
			}else{
				nh = max(wnd_h - 20, 0);
				nw = (int)(nh * ((float)osd_logo_w / (float)osd_logo_h));
			}

			stretchmode = GetStretchBltMode(dc);
			SetStretchBltMode(dc, STRETCH_HALFTONE);
			StretchBlt(dc, (wnd_w/2)-(nw/2), (wnd_h/2)-(nh/2), nw, nh, hdc_sheet, 269, 0, osd_logo_w, osd_logo_h, SRCCOPY);
			SetStretchBltMode(dc, stretchmode);
		}
	}

	seek_w   = ((wnd_w - 223 - 25) * 5) / 7;
	volume_w = ((wnd_w - 223 - 25) * 2) / 7;

	if(seek_w   < 0)seek_w   = 0;
	if(volume_w < 0)volume_w = 0;

	/* hmm... here we go */

	for(i=0; i<osd_item_count; i++)
	{
		oi = &osd_items[i];
		osd_drawitem(dc, oi, 0);

		if(oi->moving_side == 1)
		{
			if(oi->moving_pos < 100)
			{
				oi->moving_pos += move_step;
			}else{
				oi->moving_side = 0;
				oi->active      = 1;
			}

		}else if(oi->moving_side == -1){
		
			if(oi->moving_pos > 0)
			{
				if(!osd_lock_items)
					oi->moving_pos -= move_step;
			}else{
				oi->moving_pos  = 0;
				oi->moving_side = 0;
			}
		}
	}

	if(highlight_oi)
		osd_drawitem(dc, highlight_oi, 1);


}

void set_mov_side_group(int gid, int mside)
{
	struct osd_item *oi;
	int i;
	for(i=0; i<osd_item_count; i++)
	{
		oi = &osd_items[i];
		if(oi->group == gid)oi->moving_side = mside;
	}
}

struct osd_item * find_item_id(int iid)
{
	struct osd_item *oi;
	int i;
	for(i=0; i<osd_item_count; i++)
	{
		oi = &osd_items[i];
		if(oi->id == iid)return oi;
	}
	return 0;
}

void set_mov_group(int gid, int mpos, int active)
{
	struct osd_item *oi;
	int i;
	for(i=0; i<osd_item_count; i++)
	{
		oi = &osd_items[i];
		if(oi->group == gid)
		{
			oi->moving_pos = mpos;
			oi->active     = active;
		}
	}
}

string get_time_text(int t)
{
	static letter pbuf[64];
	letter tmb[32];

	memset(pbuf, 0, sizeof(pbuf));


	if(t > 3600)
	{
		str_itos(t / 3600, tmb, 10);
		str_cat(pbuf, tmb);
		str_cat(pbuf, uni(":"));
		t %= 3600;
	}

	if(t > 60)
	{
		str_itos(t / 60, tmb, 10);
		if(tmb[1] == uni('\0'))
		{
			tmb[1] = tmb[0];
			tmb[0] = uni('0');
			tmb[2] = uni('\0');
		}
		str_cat(pbuf, tmb);
		str_cat(pbuf, uni(":"));
		t %= 60;
	}else{
		str_cpy(pbuf, uni("00:"));
	}

	str_itos(t, tmb, 10);
	if(tmb[1] == uni('\0'))
	{
		tmb[1] = tmb[0];
		tmb[0] = uni('0');
		tmb[2] = uni('\0');
	}
	str_cat(pbuf, tmb);

	return pbuf;
}


void draw_scrolls(HDC dc, HDC sdc, int x, int y, int w, double v, struct osd_item *oi)
{
	int lw = 1, rw = 1, mw = 10, i, xp = 0, lp = 0, ow = w;

	BitBlt(dc, x, y, lw, oi->h, sdc, oi->sx, oi->sy, SRCCOPY);
	lp = x + lw;

	for(i=0; i<=(w - rw - lw - mw); i+=mw)
	{
		BitBlt(dc, x + lw + i, y, mw, oi->h, sdc, oi->sx + lw, oi->sy, SRCCOPY);
		lp = x + lw + i + mw;
	}

	BitBlt(dc, lp, y, (x + w) - lp - rw, oi->h, sdc, oi->sx + lw, oi->sy, SRCCOPY);
	BitBlt(dc, x + w - rw, y, rw, oi->h, sdc, oi->sx + oi->w - rw, oi->sy, SRCCOPY);

	xp = lp = 0;

	w = (int)(w * v);

	if(w > 0)
	{
		BitBlt(dc, x, y, lw, oi->h, sdc, oi->sx_h, oi->sy_h, SRCCOPY);
		lp = x + lw;

		for(i=0; i<=(w - rw - lw - mw); i+=mw)
		{
			BitBlt(dc, x + lw + i, y, mw, oi->h, sdc, oi->sx_h + lw, oi->sy_h, SRCCOPY);
			lp = x + lw + i + mw;
		}

		BitBlt(dc, lp, y, (x + w) - lp - rw, oi->h, sdc, oi->sx_h + lw, oi->sy_h, SRCCOPY);
		BitBlt(dc, x + w - rw, y, rw, oi->h, sdc, oi->sx_h + oi->w - rw, oi->sy_h, SRCCOPY);
	}

	if(ow > 40)
	{
		if(oi->id == osd_id_seek)
		{
			HFONT nfont, ofont;
			SIZE  pz;
			string ttext;
			nfont = CreateFont(-MulDiv(10, GetDeviceCaps(dc, LOGPIXELSY), 72),
												0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
												OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
												DEFAULT_PITCH, uni("Tahoma"));

			ofont = (HFONT) SelectObject(dc, nfont);

			SetBkMode(dc, TRANSPARENT);
			

			ttext = get_time_text((int)(vdata.shared->audio.output.getduration_ms() / 1000));
			GetTextExtentPoint32(dc, ttext, (int)str_len(ttext), &pz);
			SetTextColor(dc, 0x000000);
			TextOut(dc, x + ow - pz.cx + 1, y + oi->h + 5 + 1, ttext, (int)str_len(ttext));
			SetTextColor(dc, 0xffffff);
			TextOut(dc, x + ow - pz.cx, y + oi->h + 5, ttext, (int)str_len(ttext));

			ttext = get_time_text((int)(vdata.shared->audio.output.getposition_ms() / 1000));
			GetTextExtentPoint32(dc, ttext, (int)str_len(ttext), &pz);
			SetTextColor(dc, 0x000000);
			TextOut(dc, x + 1, y + oi->h + 5 + 1, ttext, (int)str_len(ttext));
			SetTextColor(dc, 0xffffff);
			TextOut(dc, x, y + oi->h + 5, ttext, (int)str_len(ttext));

			if(show_seek_position_tip)
			{
				double posfrac = (double)show_seek_position_tip / (double)ow;
				int duration = (int)(vdata.shared->audio.output.getduration_ms() / 1000);
				ttext = get_time_text((int)((double)duration * posfrac));
				GetTextExtentPoint32(dc, ttext, (int)str_len(ttext), &pz);
				SetTextColor(dc, 0x000000);
				TextOut(dc, x + show_seek_position_tip + 1 - (pz.cx / 2), y - pz.cy - 5 + 1, ttext, (int)str_len(ttext));
				SetTextColor(dc, 0xffffff);
				TextOut(dc, x + show_seek_position_tip - (pz.cx / 2), y - pz.cy - 5 , ttext, (int)str_len(ttext));
			}


			SelectObject(dc, ofont);
			DeleteObject(nfont);
		}
	}
}


int osd_mouse_message(int action, int x, int y)
{
	struct osd_item *oi;
	int ret = 0, xv, yv;

	highlight_oi = 0;

	if(action <= 3) /* move, r down, l down, drag*/
	{
		int i;

		show_seek_position_tip = 0;
		

		for(i=0; i<osd_item_count; i++)
		{
			oi = &osd_items[i];

			if(oi->active)
			{
				if(osd_inpos_ex(oi, x, y, &xv, &yv))
				{
					highlight_oi = oi;

					if(oi->id == osd_id_seek)
					{
						show_seek_position_tip = xv;
					}

					if(action)
					{
						if(oi->id == osd_id_seek)
						{
							osd_action(action, oi->id, ((double)xv / (double)seek_w));
						}else if(oi->id == osd_id_volume){
							osd_action(action, oi->id, ((double)xv / (double)volume_w));

						}else{
							osd_action(action, oi->id, ((double)xv / (double)oi->w));
						}
						ret = 1;
					}
				}else{
					//highlight_oi = 0;
				}
			}

			if(!mode_group)
			{
				int ymp = 0;
				int nx = x, ny = y;

				if(oi->align & align_bottom)
				{
					if(y > ((wnd_h * 3) / 4))ymp = 1;
					ny = -1;
				}

				if(oi->align == (align_top | align_left))
				{
					if(y < (wnd_h / 4))ymp = 1;
					ny = -1;
				}

				if(oi->align == (align_top | align_right))
				{
					if(x > ((wnd_w * 3) / 4))ymp = 1;
					nx = -1;
				}

				if( osd_inpos(oi, nx, ny) && ymp)
				{
					oi->moving_side = 1;
					if(oi->id == osd_id_posup)
					{
						struct osd_item *xoi;
						xoi = find_item_id(osd_id_posdown);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posreset);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posleft);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posright);
						if(xoi)xoi->moving_side = 1;
						goto skip_search_other;
					}
					if(oi->id == osd_id_posreset)
					{
						struct osd_item *xoi;
						xoi = find_item_id(osd_id_posdown);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posup);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posleft);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posright);
						if(xoi)xoi->moving_side = 1;
						goto skip_search_other;
					}
					if(oi->id == osd_id_posdown)
					{
						struct osd_item *xoi;
						xoi = find_item_id(osd_id_posup);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posreset);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posleft);
						if(xoi)xoi->moving_side = 1;
						xoi = find_item_id(osd_id_posright);
						if(xoi)xoi->moving_side = 1;
						goto skip_search_other;
					}
				}else{

				
					oi->active      = 0;
					oi->moving_side = -1;
					
					if(oi->moving_side)
					{
						if(oi->moving_pos > 0)
						{
							oi->moving_side = -1;
						}else{
							oi->moving_side = 0;
						}
					}
				}


			}
		}
skip_search_other:;

		if(mode_group)
		{	
			if(y > ((wnd_h * 3) / 4))
			{
				set_mov_side_group(group_bottom, 1);
			}else{
				set_mov_side_group(group_bottom, -1);
			}

			if(y < ((wnd_h) / 4))
			{
				set_mov_side_group(group_top, 1);
			}else{
				set_mov_side_group(group_top, -1);
			}

			if(x > ((wnd_w * 3) / 4))
			{
				set_mov_side_group(group_right, 1);
			}else{
				set_mov_side_group(group_right, -1);
			}
		}
	}

	return ret;
}

int osd_inpos_ex(struct osd_item *oi, int x, int y, int *xv, int *yv)
{
	int dw = oi->w;
	int oih = oi->h;

	if(oi->id == osd_id_volume ||oi->id == osd_id_seek)
	{
		oih += 40;
		y += 10;
	}

	if(oi->align == (align_bottom | align_left))
	{
		*xv = x - oi->x;

		if(oi->id == osd_id_seek)
			dw  = seek_w;

		if(((y > (wnd_h - oi->y)) && (y < (wnd_h - oi->y) + oih)) || y < 0)
			if((x > oi->x) && (x < (oi->x + dw)))
				return 1;

	}else if(oi->align == (align_bottom | align_right)){
		
		if(oi->id == osd_id_volume)
			dw  = volume_w;

		*xv = x - (wnd_w - oi->x - dw);

		if(((y > (wnd_h - oi->y)) && (y < (wnd_h - oi->y) + oih)) || y < 0)
			if((x > (wnd_w - oi->x - dw)) && (y < (wnd_w - oi->x) ))
				return 1;

	}else if(oi->align == (align_top | align_left)){
		

		x -= (wnd_w / 2) - (top_bar_width / 2);

		if(((y > oi->y) && (y < (oi->y + oih))) || (y < 0))
			if(((x > oi->x) && (x < (oi->x + oi->w))))
				return 1;
	}else if(oi->align == (align_top | align_right)){
		

		if(((x > (wnd_w - oi->x - oi->w) && (x < (wnd_w - oi->x))) || (x < 0)))
			if(((y > oi->y) && (y < (oi->y + oih))))
				return 1;
	}

	return 0;
}
int osd_inpos(struct osd_item *oi, int x, int y)
{
	int xv, yv;
	return osd_inpos_ex(oi, x, y, &xv, &yv);
}


void osd_drawitem(HDC dc, struct osd_item *oi, int mode)
{
	int sx, sy, dx = 0, dy = 0;
	letter buf[10];

	if(!mode)
	{
		sx = oi->sx;
		sy = oi->sy;
	}else{
		sx = oi->sx_h;
		sy = oi->sy_h;
	}

	if(oi->align == (align_bottom | align_left))
	{
		dx = oi->x;
		dy = wnd_h - (oi->moving_pos * oi->y) / 100;

	}else if(oi->align == (align_bottom | align_right)){

		dx = wnd_w - oi->x - oi->w;
		dy = wnd_h - (oi->moving_pos * oi->y) / 100;
		
		if(oi->id == osd_id_volume)
			dx = wnd_w - oi->x - volume_w;
	}else if(oi->align == (align_top | align_left)){
	
		dx = (wnd_w / 2) - (top_bar_width / 2) + oi->x;
		dy = ((oi->moving_pos * (oi->y + oi->h)) / 80) - (oi->y + oi->h);

	}else if(oi->align == (align_top | align_right)){

		dx = wnd_w - (oi->moving_pos * (oi->w + oi->x)) / 100;
		dy = oi->y;
	}


	
	if(oi->id == osd_id_volume)
	{
		double v, vl, vr;
		vdata.shared->audio.output.getvolume(&vl, &vr);
		v = (vl + vr) / 2.0;

		if(volume_w)
			draw_scrolls(dc, hdc_sheet, dx, dy, volume_w, v, oi);

	}else if(oi->id == osd_id_seek){
		
		double v;
		vdata.shared->audio.output.getposition(&v);

		if(seek_w)
			draw_scrolls(dc, hdc_sheet, dx, dy, seek_w, v, oi);

	}else{

		BitBlt(dc, dx, dy, oi->w, oi->h, hdc_sheet, sx, sy, SRCCOPY);
	}

	if(oi->group == group_top)
	{
		HFONT nfont, ofont;
		struct video_dec *vd;

		vdata.shared->call_function(call_video_getvdec, 0, &vd, 0);

		/*
		if(vd)
		{

			nfont = CreateFont(-MulDiv(10, GetDeviceCaps(dc, LOGPIXELSY), 72),
											0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
											OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
											DEFAULT_PITCH, uni("Tahoma"));

			ofont = (HFONT) SelectObject(dc, nfont);

			switch(oi->id)
			{
			case osd_id_video:
				SetBkMode(dc, TRANSPARENT);
				SetTextColor(dc, RGB(0, 0, 128));
				str_itos(vd->video_decoder_trans_info(video_get_video_stream_current, 0, 0, 0), buf, 10);
				TextOut(dc, dx + oi->w - 20, dy + 2, buf, (int)str_len(buf));
				break;

			case osd_id_audio:
				SetBkMode(dc, TRANSPARENT);
				SetTextColor(dc, RGB(0, 0, 128));
				str_itos(vd->video_decoder_trans_info(video_get_audio_stream_current, 0, 0, 0), buf, 10);
				TextOut(dc, dx + oi->w - 20, dy + 2, buf, (int)str_len(buf));
				break;

			case osd_id_chapter:
				SetBkMode(dc, TRANSPARENT);
				SetTextColor(dc, RGB(0, 0, 128));
				str_itos(vd->video_decoder_trans_info(video_get_movie_chapter_current, 0, 0, 0), buf, 10);
				TextOut(dc, dx + oi->w - 20, dy + 2, buf, (int)str_len(buf));
				break;

			case osd_id_subtitles:
				{
					int     lid;
					letter  sub_name[128];

					SetBkMode(dc, TRANSPARENT);
					SetTextColor(dc, RGB(0, 0, 128));

					lid = vd->video_decoder_trans_info(video_get_sub_language_current, -1, 0, sub_name);
				
					if(lid)
					{
						//str_itos(lid, buf, 10);
						TextOut(dc, dx + oi->w - 20, dy + 2, sub_name, (int)str_len(sub_name));
					}else{
						TextOut(dc, dx + oi->w - 40, dy + 2, uni("None"), 4);
					}
				}
				break;
			} 

			SelectObject(dc, ofont);
			DeleteObject(nfont);
		}*/
	}

}

void osd_timer(void)
{

}

void osd_initialize(void)
{

}

void osd_uninitialize(void)
{
	if(hdc_sheet)
	{
		DeleteDC(hdc_sheet);
		DeleteObject(bmp_sheet);
		hdc_sheet = 0;
	}
}


HMENU user_create_menu(int mid, int flags)
{
	HMENU  m = CreatePopupMenu(), c;
	int i, count, sel_lng;


#	define addmenu(x, i, v)  (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_STRING, (i), (v)) )
#	define addmenu_chk(x, i, v)  (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_CHECKED | MF_STRING, (i), (v)) )
#	define menugroup(x)      (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, 0) )
#	define addchild(x, h, v) (InsertMenu(x, (UINT)-1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)(h), (v)) )
#	define menubegin(x)      (x = CreatePopupMenu())
#	define menuend(x)        (DestroyMenu(x))

	switch(mid)
	{
	case 1:
		{
			struct video_dec *vd;
			letter sub_name[128];

			vdata.shared->call_function(call_video_getvdec, 0, &vd, 0);
			

			menubegin(c);

			addmenu(c, 1001, uni("(None)"));

			count = vd->video_decoder_trans_info(video_get_sub_language_count, 0, 0, 0);
				
			/* get primary language id */
			sel_lng = vd->video_decoder_trans_info(video_get_sub_language_current, 0, 0, 0);

			for(i=0; i<count; i++)
			{
				vd->video_decoder_trans_info(video_get_sub_language_current, i, 0, sub_name);

				if(i != sel_lng)
					addmenu(c, 2000 + i, sub_name);
				else
					addmenu_chk(c, 2000 + i, sub_name);
			}

			addchild(m, c, uni("Primary"));

			menubegin(c);

			addmenu(c, 1002, uni("(None)"));

			/* get secondary language id */

			sel_lng = vd->video_decoder_trans_info(video_get_sub_language_current, 1, 0, 0);

			for(i=0; i<count; i++)
			{
				vd->video_decoder_trans_info(video_get_sub_language_current, i, 0, sub_name);
				
				if(i != sel_lng)
					addmenu(c, 8000 + i, sub_name);
				else
					addmenu_chk(c, 8000 + i, sub_name);
			}

			addchild(m, c, uni("Secondary"));
		}
		break;
	}

	return m;
}

void osd_action(int action, int id, double v)
{
	switch(id)
	{
	case osd_id_subtitles:
		{
			POINT pt;
			HWND  hwinvid = 0;
			HMENU mc = user_create_menu(1, 0);
			int   midr;
			struct video_dec *vd;

			osd_lock_items = 1;
			
			vdata.getdata(get_window_video, 0, &hwinvid, 0);
			if(hwinvid) SendMessage(hwinvid, WM_USER, (WPARAM)user_msg_start_keep_cursor, 0);


			vdata.shared->call_function(call_video_getvdec, 0, &vd, 0);
			
			
			GetCursorPos(&pt);

			midr = TrackPopupMenu(mc, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, GetFocus(), 0);

			if(midr == 1001)
			{
				vd->video_decoder_trans_info(video_set_sub_language, -1, 0, 0);
				subtitle_text = 0;
			}

			if(midr == 1002)
			{
				vd->video_decoder_trans_info(video_set_sub_language, -1, 1.0, 0);
				subtitle_text_sec = 0;
			}
				
			if(midr >= 2000 && midr < 8000)
				vd->video_decoder_trans_info(video_set_sub_language, midr - 2000, 0, 0);

			if(midr >= 8000 && midr < 14000)
				vd->video_decoder_trans_info(video_set_sub_language, midr - 8000, 1.0, 0);

			DestroyMenu(mc);

			osd_lock_items = 0;
			if(hwinvid) SendMessage(hwinvid, WM_USER, (WPARAM)user_msg_end_keep_cursor, 0);

		}
		break;

	case osd_id_reset:
		show_subtitles ^= 1;
		break;

	case osd_id_play:
		if(action == 1)
			vdata.shared->audio.output.play();
		break;

	case osd_id_stop:
		if(action == 1)
		{	vdata.shared->audio.output.stop();
			current_frame = 0;
		}
		break;

	case osd_id_volume:
		if(v > 1.0)v = 1.0;
		else if(v < 0.0)v = 0.0;
		vdata.shared->audio.output.setvolume(v, v);
		break;

	case osd_id_seek:
		if(v > 1.0)v = 1.0;
		else if(v < 0.0)v = 0.0;
		vdata.shared->audio.output.setposition(v);
		break;

	case osd_id_next:
		if(action == 1)
			vdata.shared->audio.output.playlist.next();
		break;

	case osd_id_previous:
		if(action == 1)
			vdata.shared->audio.output.playlist.previous();
		break;

	case osd_id_rewind:
		{
			double p;

			vdata.shared->audio.output.getposition(&p);
			p -= 0.01;
			if(p < 0.0)p = 0.0;
			vdata.shared->audio.output.setposition(p);
		}
		break;

	case osd_id_forward:
		{
			double p;

			vdata.shared->audio.output.getposition(&p);
			p += 0.01;
			if(p > 1.0)p = 1.0;
			vdata.shared->audio.output.setposition(p);
		}
		break;

	case osd_id_group:
		mode_group ^= 1;
		break;

	case osd_id_zoomin:
		video_zoom_x *= 1.2;
		video_zoom_y = video_zoom_x;
		break;

	case osd_id_zoomout:
		video_zoom_x /= 1.2;
		video_zoom_y = video_zoom_x;
		break;

	case osd_id_posreset:
		video_zoom_y = video_zoom_x = 1.0;
		video_dx = 0;
		video_dy = 0;
		break;

	case osd_id_posleft:
		video_zoom_x /= 1.2;
		break;

	case osd_id_posright:
		video_zoom_x *= 1.2;
		break;

	case osd_id_posup:
		video_zoom_y /= 1.2;
		break;

	case osd_id_posdown:
		video_zoom_y *= 1.2;
		break;

	case osd_id_hide:
		if(action == 1)
		{
			vdata.shared->audio.output.pause();
			current_frame = 0;
		}
		break;

	case osd_id_mute:
		if(action == 1)
		{
			static double vl = 1.0, vr = 1.0;
			static int mute_set = 0;

			if(!mute_set)
			{
				vdata.shared->audio.output.getvolume(&vl, &vr);
				vdata.shared->audio.output.setvolume(0, 0);
				mute_set = 1;
			}else{
				vdata.shared->audio.output.setvolume(vl, vr);
				mute_set = 0;
			}
			current_frame = 0;
		}
		break;

	case osd_id_load:
		if(action == 1)
		{
			vdata.shared->simple.show_openfile();
		}
		break;

	case osd_id_browse:
		if(action == 1)
		{
			unsigned int i;
			letter       fpath[1024];
			OPENFILENAME lofn;
			HANDLE       hfile;
			unsigned long        nbr = 0;
			unsigned long        pcount;
			struct video_dec *vd;

			memset(&lofn, 0, sizeof(lofn));

			fpath[0] = 0;

			lofn.lStructSize     = sizeof(lofn);
			lofn.lpstrTitle      = uni("Load Subtitles File");
			lofn.hwndOwner       = GetFocus();
			lofn.lpstrFile       = fpath;
			lofn.nMaxFile        = sizeof(fpath);
			lofn.lpstrFilter     = uni("Supported Subtitle Formats (*.srt)\0*.srt");
			lofn.nFilterIndex    = 0;
			lofn.lpstrFileTitle  = 0;
			lofn.nMaxFileTitle   = 0;
			lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
			lofn.hInstance       = hinstance;

			vdata.shared->call_function(call_video_getvdec, 0, &vd, 0);

			if(vd)
			{
				vd->video_decoder_trans_info(video_get_subfile, 260, 0, (void*)lofn.lpstrInitialDir);
			

				GetOpenFileName(&lofn);

				if(str_len(fpath))
				{
					vd->video_decoder_trans_info(video_set_subfile, 0, 0, fpath);
				}
			}

		}
		break;
	}
}

void osd_initialize_in(HDC dc)
{
	bmp_sheet = LoadBitmap(hinstance, MAKEINTRESOURCE(bitmap_sheet));
	hdc_sheet = CreateCompatibleDC(dc);
	SelectObject(hdc_sheet, bmp_sheet);
}

int osd_hide_all(void)
{
	set_mov_side_group(group_bottom, -1);
	set_mov_side_group(group_right, -1);
	set_mov_side_group(group_top, -1);
	return 1;
}