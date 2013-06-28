#include "skin.h"
#include "media.h"
#include "skin settings.h"

extern HDC   hdc_fview;
extern int   media_init;
extern HDC   fv_sheet;
extern HWND  window_media;

extern int	 scroll_value;
extern int   scroll_max;

HDC     hdc_backup;
HFONT   mintro_font_title = 0, old_font, mintro_font_tagline = 0;

HDC     hdc_grid;
int     pic_grid_w, pic_grid_h;

float   mizoom = 1.0f;

int     gridmatrix_alpha[2][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};
int     gridmatrix_change[2][6] = {{1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1}};

int     view_width, view_height;

unsigned int timer_mintro = 0;

int     mt_title_x = 0, mt_title_alpha = 0, mt_title_w = 0;
int     mt_dsc_x = 0, mt_dsc_alpha = 0, mt_dsc_w = 0;

string  mt_title_str = uni("Media Library...");
string  mt_dsc_str = uni("Start browsing by clicking on a picture above...");

#define  zoomv(x) ( (int)((float)(x) * mizoom) )




void CALLBACK mintro_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);





void mintro_init(HDC dc)
{
	letter    skin_path[512], skin_ap[512];

	hdc_backup = dc;

	mintro_font_title   = CreateFont(-36,
                                0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, uni("Tahoma"));

	mintro_font_tagline = CreateFont(-14,
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5,
                                DEFAULT_PITCH, uni("Tahoma"));

	old_font = SelectObject(hdc_backup, mintro_font_title);

	scroll_value = scroll_max = 0; /* disable */

	skin.shared->general.getskinspath(skin_path, sizeof(skin_path));
	str_cat(skin_path, uni("/neo/"));

	str_cpy(skin_ap, skin_path); str_cat(skin_ap, uni("introgrid.png"));
	hdc_grid   = png_get_hdc(skin_ap, 0);
	pic_grid_w = png_w;
	pic_grid_h = png_h;

	SetBkMode(dc, TRANSPARENT);

	timer_mintro = (unsigned int)SetTimer(0, 0, 35, (TIMERPROC) mintro_timer);
}

void mintro_uninit(void)
{
	DeleteDC(hdc_grid);

	KillTimer(0, timer_mintro);

	DeleteObject(mintro_font_title);
	DeleteObject(mintro_font_tagline);
	SelectObject(hdc_backup, old_font);
}

void mintro_draw(HDC dc, int w, int h)
{
	int pgx, pgy;

	hdc_backup  = dc;
	view_width  = w;
	view_height = h;

	if(pic_grid_w > w) mizoom = 0.7f;

	pgx = (w / 2) - (zoomv(pic_grid_w) / 2);
	pgy = (h / 2) - (zoomv(pic_grid_h) / 2);

	if(mizoom >= 1.0)
	{
		int i, j;

		for(i=0; i<2; i++)
		{
			for(j=0; j<6; j++)
			{
				alpha_blit(dc, hdc_grid, pgx + (j * (173 + 3)) + 5, pgy + (i * (173 + 3)) + 5, 173, 173, (j * (173 + 3)) + 5, (i * (173 + 3)) + 5, gridmatrix_alpha[i][j]);
			}
		}

	}else{
		int i, j;

		for(i=0; i<2; i++)
		{
			for(j=0; j<6; j++)
			{
				alpha_blit_resize(dc, hdc_grid, pgx + zoomv((j * (173 + 3)) + 5), pgy + zoomv((i * (173 + 3)) + 5), zoomv(173), zoomv(173), (j * (173 + 3)) + 5, (i * (173 + 3)) + 5, 173, 173, gridmatrix_alpha[i][j]);
			}
		}
	}
}

void mintro_mousemsg(int x, int y, int action)
{
	static int lasti = -1;
	static int lastj = -1;
	int i, j, pgx, pgy;

	if(action == mm_move)
	{
		pgx = (view_width / 2) - (zoomv(pic_grid_w) / 2);
		pgy = (view_height / 2) - (zoomv(pic_grid_h) / 2);
	
		i = (y - (pgy - 5)) / zoomv(173 + 3);
		j = (x - (pgx - 5)) / zoomv(173 + 3);

		if(i >= 0 && i < 2 && j >= 0 && j < 6) /* check if it's valid */
		{
			if((i != lasti) || (j != lastj))
			{
				
				gridmatrix_alpha[i][j] = 0;
				gridmatrix_change[i][j] = 0;

				if(lasti >= 0 && lastj >= 0)
				{
					gridmatrix_change[lasti][lastj] = -20;
				}

				if(j / 2 != lastj / 2)
				{
					mt_title_x = zoomv(pic_grid_w) + 200;
					mt_dsc_x = zoomv(pic_grid_w) + 500;

					mt_title_w = mt_dsc_w = 0;

					switch(j/2)
					{
					case 0:
						mt_title_str = uni("Artists...");
						mt_dsc_str = uni("Search through the artists...");
						break;
					case 1:
						mt_title_str = uni("Genres...");
						mt_dsc_str = uni("Different genres, different sounds...");
						break;
					case 2:
						mt_title_str = uni("Years...");
						mt_dsc_str = uni("Welcome to the timeline of music...");
						break;
					}
				}

				lasti = i;
				lastj = j;


				
			}
		}
	}

	if(action == mm_down_l)
	{
		pgx = (view_width / 2) - (zoomv(pic_grid_w) / 2);
		pgy = (view_height / 2) - (zoomv(pic_grid_h) / 2);
	
		i = (y - (pgy - 5)) / zoomv(173 + 3);
		j = (x - (pgx - 5)) / zoomv(173 + 3);

		if(i >= 0 && i < 2 && j >= 0 && j < 6) /* check if it's valid */
		{
			switch(j/2)
			{
			case 0: /* artists */
				ml_current_dir  = media_library_dir_artists;
				break;
			case 1: /* genres */
				ml_current_dir  = media_library_dir_genres;
				break;
			case 2: /* years */
				ml_current_dir  = media_library_dir_years;
				break;
			}

			if(j/2 == 0 || j/2 == 1 || j/2 == 2)
			{
				ml_current_cdir = 0;
				ml_dir_changed = 1;
				ml_in_dir = ((ml_current_dir != media_library_dir_all) && !ml_current_cdir);
				ml_pl_startid = 0;
				ml_cache_uninit();
				ml_cache_init();
				fullview_switch(vmode_library);
				fullview_drawgeneral();
			}
		}
	}

}

void CALLBACK mintro_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	static int rm = 100;
	int i, j, pgx, pgy;

	
	

	if(rm > 20)
	{
		for(i=0; i<2; i++)
		{
			for(j=0; j<6; j++)
			{
				if(gridmatrix_change[i][j])
				{
					gridmatrix_change[i][j] = (int)(rand() % 20) - 10;
				
					if(gridmatrix_change[i][j] == 0) gridmatrix_change[i][j] = 1;
				}
			
			}
		}

		rm = 0;
	}

	for(i=0; i<2; i++)
	{
		for(j=0; j<6; j++)
		{
			if(gridmatrix_change[i][j])
				gridmatrix_alpha[i][j] += gridmatrix_change[i][j];
			else
				gridmatrix_alpha[i][j] += 10;

			if(gridmatrix_alpha[i][j] > 100) gridmatrix_alpha[i][j] = 100;
			else if(gridmatrix_alpha[i][j] < 20) gridmatrix_alpha[i][j] = 20;
		}
	}


	pgx = (view_width / 2) - (zoomv(pic_grid_w) / 2);
	pgy = (view_height / 2) - (zoomv(pic_grid_h) / 2);
	
	fullview_clear(pgx, pgy, zoomv(pic_grid_w), zoomv(pic_grid_h) + 80);


	mintro_draw(hdc_backup, view_width, view_height);


	mt_title_x = (mt_title_x * 3) / 4;
	mt_dsc_x = (mt_title_x * 3) / 12;
	
	SetTextColor(hdc_backup, 0x000000);
	SelectObject(hdc_backup, mintro_font_title);

	if(mt_title_w == 0)
	{
		SIZE sz;
		GetTextExtentPoint(hdc_backup, mt_title_str, str_len(mt_title_str), &sz);
		mt_title_w = sz.cx;
	}
	TextOut(hdc_backup, -mt_title_x + (pgx + zoomv(pic_grid_w)) - mt_title_w, pgy + zoomv(pic_grid_h) + 8, mt_title_str, str_len(mt_title_str));
	
	
	SelectObject(hdc_backup, mintro_font_tagline);
	if(mt_dsc_w == 0)
	{
		SIZE sz;
		GetTextExtentPoint(hdc_backup, mt_dsc_str, str_len(mt_dsc_str), &sz);
		mt_dsc_w = sz.cx;
	}

	TextOut(hdc_backup, mt_dsc_x + (pgx + zoomv(pic_grid_w)) - mt_dsc_w + 1, pgy + zoomv(pic_grid_h) + 8 + 36 + 1, mt_dsc_str,str_len(mt_dsc_str));
	SetTextColor(hdc_backup, 0xffffff);
	TextOut(hdc_backup, mt_dsc_x + (pgx + zoomv(pic_grid_w)) - mt_dsc_w, pgy + zoomv(pic_grid_h) + 8 + 36, mt_dsc_str,str_len(mt_dsc_str));

	fullview_render(pgx, pgy, zoomv(pic_grid_w), zoomv(pic_grid_h) + 80);

	rm++;
}