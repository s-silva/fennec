/*

	trash:

-------------------------------------------------------------------------------

*/

char declared_to_skip_warning_level_4_messages;

#ifdef do_not_define_this

	/* playlist -------------------------------------------------------------*/


/*
 * Remove all playlist items.
 */

int booom_audio_playlist_clear(void)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = internal_output_playlist_clear();
		fennec_refresh(fennec_v_refresh_force_less);

	}else{

		ret = external_output_playlist_clear();
		fennec_refresh(fennec_v_refresh_force_less);
	}

	return ret;
}


/*
 * Add item.
 */

int booom_audio_playlist_add(char* fname, int checkfile, int gettags)
{
	int ret = 0;

	if(!fname)return 0;
	if(!strlen(fname))return 0;
	
	if(isinternalout())
	{
		ret = internal_output_playlist_add(fname, checkfile, gettags);
		strcpy(settings.player.last_file, fname);

	}else{

		ret = external_output_playlist_add(fname, checkfile, gettags);
		strcpy(settings.player.last_file, fname);
	}

	return ret;
}


/*
 * Move item.
 * 'idd' - destination index, ids - source index.
 */

int booom_audio_playlist_move(unsigned long idd, unsigned long ids)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = internal_output_playlist_move(idd, ids);
		fennec_refresh(fennec_v_refresh_force_less);

	}else{

		ret = external_output_playlist_move(idd, ids);
		fennec_refresh(fennec_v_refresh_force_less);
	}

	return ret;
}


/*
 * Set shuffle.
 */

int booom_audio_playlist_setshuffle(int shuffle, int doaction)
{
	int ret = 0;
	
	if(isinternalout())
	{
		ret = internal_output_playlist_setshuffle(shuffle, doaction);
		fennec_refresh(fennec_v_refresh_force_less);
	
	}else{
	
		ret = external_output_playlist_setshuffle(shuffle, doaction);
		fennec_refresh(fennec_v_refresh_force_less);
	}

	return ret;
}


/*
 * Shuffle playlist.
 * 'entirelist':
 *		1. Shuffle all.
 *		2. Shuffle newly added items only (not played).
 */

int booom_audio_playlist_shuffle(int entirelist)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_shuffle(entirelist);
		fennec_refresh(fennec_v_refresh_force_less);

	}else{

		ret = external_output_playlist_shuffle(entirelist);
		fennec_refresh(fennec_v_refresh_force_less);
	}

	return ret;
}


/*
 * Unshuffle playlist.
 */

int booom_audio_playlist_unshuffle(void)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_unshuffle();
		fennec_refresh(fennec_v_refresh_force_less);

	}else{

		ret = external_output_playlist_unshuffle();
		fennec_refresh(fennec_v_refresh_force_less);
	}

	return ret;
}


/*
 * Remove an item from playlist.
 */

int booom_audio_playlist_remove(unsigned long idx)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_remove(idx);
		fennec_refresh(fennec_v_refresh_force_less);

	}else{

		ret = external_output_playlist_remove(idx);
		fennec_refresh(fennec_v_refresh_force_less);
	}

	return ret;
}


/*
 * Mark as played.
 */

int booom_audio_playlist_mark(unsigned long idx)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_mark(idx);

	}else{

		ret = external_output_playlist_mark(idx);
	}

	return ret;
}


/*
 * Unmark (not played).
 */

int booom_audio_playlist_unmark(unsigned long idx)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_unmark(idx);
	}else{
		ret = external_output_playlist_unmark(idx);
	}

	return ret;
}


/*
 * Get marked flag.
 */

int booom_audio_playlist_ismarked(unsigned long idx)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_ismarked(idx);
	}else{
		ret = external_output_playlist_ismarked(idx);
	}

	return ret;
}


/*
 * Get tags (not used)
 */

int booom_audio_playlist_getinfo(unsigned long idx, char* sname, char* sartist, char* salbum, char* sgenre, unsigned long* durationms, unsigned long* fsize)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_getinfo(idx, sname, sartist, salbum, sgenre, durationms, fsize);
	}else{
		ret = external_output_playlist_getinfo(idx, sname, sartist, salbum, sgenre, durationms, fsize);
	}

	return ret;
}


/*
 * Switch to next item.
 */

int booom_audio_playlist_next(void)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_next();
		fennec_refresh(fennec_v_refresh_force_not);

	}else{

		ret = external_output_playlist_next();
		fennec_refresh(fennec_v_refresh_force_not);
	}

	return ret;
}


/*
 * Switch to previous item.
 */

int booom_audio_playlist_previous(void)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_previous();
		fennec_refresh(fennec_v_refresh_force_not);

	}else{

		ret = external_output_playlist_previous();	
		fennec_refresh(fennec_v_refresh_force_not);
	}

	return ret;
}


/*
 * Directly switch into a list index.
 */

int booom_audio_playlist_switch_list(unsigned long idx)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_switch_list(idx);
	}else{
		ret = external_output_playlist_switch_list(idx);
	}

	return ret;
}


/*
 * Switch into virtual indices.
 * -1 - current index.
 */

int booom_audio_playlist_switch(unsigned long idx)
{
	int ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_switch(idx);
	}else{
		ret = external_output_playlist_switch(idx);
	}

	return ret;
}


/*
 * Get current index.
 */

unsigned long booom_audio_playlist_getcurrentindex(void)
{
	unsigned long ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_getcurrentindex();
	}else{
		ret = external_output_playlist_getcurrentindex();
	}

	return ret;
}


/*
 * Get real index (shuffled).
 */

unsigned long booom_audio_playlist_getrealindex(unsigned long idx)
{
	unsigned long ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_getrealindex(idx);
	}else{
		ret = external_output_playlist_getrealindex(idx);
	}

	return ret;
}


/*
 * Get list index (unshuffled).
 */

unsigned long booom_audio_playlist_getlistindex(unsigned long ridx)
{
	unsigned long ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_getlistindex(ridx);
	}else{
		ret = external_output_playlist_getlistindex(ridx);
	}

	return ret;
}


/*
 * Get playlist items count.
 */

unsigned long booom_audio_playlist_getcount(void)
{
	unsigned long ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_getcount();
	}else{
		ret = external_output_playlist_getcount();
	}

	return ret;
}


/*
 * Exchange items.
 */

int booom_audio_playlist_exchange(unsigned long idd, unsigned long ids)
{
	unsigned long ret = 0;
		
	if(isinternalout())
	{
		ret = internal_output_playlist_exchange(idd, ids);
	}else{
		ret = external_output_playlist_exchange(idd, ids);
	}

	return ret;
}


/*
 * Get source (file/stream) path.
 */

const char* booom_audio_playlist_getsource(unsigned long idx)
{
	if(isinternalout())
	{
		return internal_output_playlist_getsource(idx);
	}else{
		return external_output_playlist_getsource(idx);
	}
}

#endif