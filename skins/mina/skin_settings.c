#include "skin.h"
#include "skin settings.h"


skin_settings settings_data;



void sset_default(void)
{
	int i;

	settings_data.main.x    = 0;
	settings_data.main.y    = 0;
	settings_data.main.size = 1.0f;
	settings_data.main.media_window_mode = vmode_media_intro; /* introduction */

	settings_data.main.enable_transparency = 0;
	settings_data.main.transparency_amount = 50;

	settings_data.theme.use_theme = 0;
	
	settings_data.playlist.visible = 0;
	settings_data.playlist.display_mode = playlist_display_big;
	settings_data.playlist.wallpaper[0] = 0; /* default */

	settings_data.advanced.enable_album_art     = 1;
	settings_data.advanced.enable_artist_photo  = 1;
	settings_data.advanced.auto_download_covers = 0;

	settings_data.library.current_artist[0] = 0;
	settings_data.library.current_album[0]  = 0;
	settings_data.library.show_album        = 0;
	settings_data.library.current_dir       = 0;

	settings_data.vis.current_vis[0]       = 0;
	settings_data.vis.show_vis             = 0;
	settings_data.vis.video_when_available = 1;

	settings_data.video.force_hide         = 0;
	settings_data.video.detached           = 0;
	settings_data.video.x                  = 0;
	settings_data.video.y                  = 0;
	settings_data.video.h                  = 360;
	settings_data.video.w                  = 640;

	settings_data.dv.zoomv = 1.0f;

	for(i=0; i<32; i++)
		settings_data.playlist.xoff[i] = -1;

	settings_data.playlist.xoff[header_tag_title]  = 0;
	settings_data.playlist.xoff[header_tag_album]  = 279;
	settings_data.playlist.xoff[header_tag_artist] = 556;
	settings_data.playlist.xoff[header_tag_year]   = 787;
	settings_data.playlist.xoff[header_tag_genre]  = 822;
	settings_data.playlist.xoff[header_tag_index]  = 896;
}

void sset_load(void)
{
	int     i, er = 0;
	letter  tmp_buf[1024];
	FILE   *file_skin_settings = 0;

	GetModuleFileName(instance_skin, tmp_buf, sizeof(tmp_buf));

	i = (int)str_len(tmp_buf);

	while(i)
	{
		if(tmp_buf[i] == '/' || tmp_buf[i] == '\\')
		{
			tmp_buf[i + 1] = 0;
			break;
		}
		i--;
	}

	str_cat(tmp_buf, uni("settings - skin neo.fsd"));

	file_skin_settings = _wfopen(tmp_buf, uni("rb+"));

	if(file_skin_settings)
	{
		if(fread(&settings_data, sizeof(settings_data), 1, file_skin_settings) != 1)
			er = 1;
	}else{
		file_skin_settings = _wfopen(tmp_buf, uni("wb+"));

		if(!file_skin_settings) /* try read only */
		{
			file_skin_settings = _wfopen(tmp_buf, uni("rb"));
			
			if(file_skin_settings)
			{
				if(!fread(&settings_data, sizeof(settings_data), 1, file_skin_settings))
					er = 1;
				fclose(file_skin_settings);
				file_skin_settings = 0;
			}
		}else{
			er = 1;
		}
	}

	if(er) sset_default();
	if(file_skin_settings) fclose(file_skin_settings);
}

void sset_save(void)
{
	int     i;
	letter  tmp_buf[1024];
	FILE   *file_skin_settings = 0;

	GetModuleFileName(instance_skin, tmp_buf, sizeof(tmp_buf));

	i = (int)str_len(tmp_buf);

	while(i)
	{
		if(tmp_buf[i] == '/' || tmp_buf[i] == '\\')
		{
			tmp_buf[i + 1] = 0;
			break;
		}
		i--;
	}

	str_cat(tmp_buf, uni("settings - skin neo.fsd"));

	file_skin_settings = _wfopen(tmp_buf, uni("rb+"));
	if(!file_skin_settings) return;

	fwrite(&settings_data, sizeof(settings_data), 1, file_skin_settings);
					
	fclose(file_skin_settings);
}