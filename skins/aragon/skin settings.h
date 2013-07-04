



enum
{
	skin_theme_default = 0,
	skin_theme_color_base,
	skin_theme_color_control,
	skin_theme_color_text
};

typedef struct _skin_settings
{
	struct{
		int   x, y;
		float size;
		int   media_window_mode;
		int   enable_transparency;
		int   transparency_amount;
	}main;

	struct{

		int   use_theme;
		
		struct{ int h, s, l; }base;
		struct{ int h, s, l; }control;
		struct{ int h, s, l; }text;

	}theme;

	struct{
		
		int     xoff[32];
		int     visible;
		int     sorted_column;
		int     sort_mode;
		int     display_mode;
		letter  wallpaper[260];
	}playlist;

	struct{
		
		float   zoomv;

	}dv;

	struct{

		letter  current_artist[1024];
		letter  current_album[1024];
		int     show_album;
		int     current_dir;

	}library;

	struct{

		letter  current_vis[1024];
		int     show_vis;
		int     video_when_available;

	}vis;

	struct{

		int     force_hide;
		int     detached;
		int     x, y, w, h;  /* if detached */

	}video;


	struct{
		
		int     enable_album_art;
		int     enable_artist_photo;
		int     auto_download_covers;

	}advanced;

} skin_settings;

extern skin_settings settings_data;

void sset_default(void);
void sset_load(void);
void sset_save(void);