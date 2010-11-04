/**----------------------------------------------------------------------------

 Fennec 7.1.2 Player 1.2
 Copyright (C) 2007 Chase <c-h@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/

#include "fennec main.h"
#include "fennec audio.h"




/* structs ------------------------------------------------------------------*/

struct media_file_header       /* file header */
{
	int8_t      sign[4];
	uint32_t    version;
	uint32_t    count;
	uint32_t    size;           /* file size */
	uint32_t    start_of_tags;
};

struct media_file_tag
{
	uint32_t   size;           /* size of the tag, inc. data and this header */
	double     score;
	int32_t    user;
	uint32_t   sorted_item_index_all_music;

	double     duration;
	double     abitrate;
	uint32_t   file_size;
	int8_t     rating;

	uint32_t   sizeof_title;
	uint32_t   sizeof_album;
	uint32_t   sizeof_artist;
	uint32_t   sizeof_original_artist;
	uint32_t   sizeof_composer;
	uint32_t   sizeof_lyricist;
	uint32_t   sizeof_band;
	uint32_t   sizeof_copyright;
	uint32_t   sizeof_publish;
	uint32_t   sizeof_genre;
	uint32_t   sizeof_year;
	uint32_t   sizeof_file_path;
	uint32_t   sizeof_bpm;
	uint32_t   sizeof_track;

	/* pointers from start of the tag to data */

	int        pointer_title;
	int        pointer_album;
	int        pointer_artist;
	int        pointer_original_artist;
	int        pointer_composer;
	int        pointer_lyricist;
	int        pointer_band;
	int        pointer_copyright;
	int        pointer_publish;
	int        pointer_genre;
	int        pointer_year;
	int        pointer_file_path;
	int        pointer_bpm;
	int        pointer_track;

	/* title, album, artist... */
};



/** - - - - - - - - - - - - - - - - - - - - - - -

file structure:

[media_file_header]
	[media_file_tag] + data
	...

- - - - - - - - - - - - - - - - - - - - - - - **/



/* defines ------------------------------------------------------------------*/

	/* imaginary/built-in directories */

#define media_library_dir_root     0x1 /* root (where you find other dirs like genres...) */
#define media_library_dir_all      0x2 /* all music */
#define media_library_dir_rating   0x3

	/* real directories */

#define media_library_dir_artists  0xa
#define media_library_dir_albums   0xb
#define media_library_dir_genres   0xc
#define media_library_dir_years    0xd




/* data ---------------------------------------------------------------------*/

int       ml_initialized = 0;
uint8_t  *ml_mapped_file = 0;
uint32_t  ml_mapped_file_size = 0;
int       ml_changed;

struct media_file_header   *ml_head;
int      *ml_tags = 0;

int       mode_sorted = 1;
int       mode_search_running;


#define ml_gtag(i) ((struct media_file_tag*) (ml_mapped_file + ml_tags[i]))





/* code ---------------------------------------------------------------------*/



/*
 * initialize (does not load the file).
 */
int media_library_initialize_base(void)
{
	if(ml_initialized) return -1;

	ml_mapped_file      = 0;
	ml_mapped_file_size = 0;
	ml_tags             = 0;

	ml_initialized = 1;

	ml_changed = 0;
	return 0;
}

/*
 * initialize and load the file.
 */
int media_library_initialize(void)
{
	letter fpath[v_sys_maxpath];

	if(media_library_initialize_base() >= 0)
	{
		fennec_get_abs_path(fpath, uni("/media library.fsd"));
		media_library_load(fpath);
		return 0;
	}
	
	return -1;
}



/*
 * uninitialize.
 */
int media_library_uninitialize(void)
{
	letter fpath[v_sys_maxpath];

	if(!ml_initialized) return -1;

	if(ml_changed)
	{
		fennec_get_abs_path(fpath, uni("/media library.fsd"));
		media_library_save(fpath);
	}

	if(ml_mapped_file)
		sys_mem_free(ml_mapped_file);

	if(ml_tags)
		sys_mem_free(ml_tags);

	ml_mapped_file      = 0;
	ml_mapped_file_size = 0;
	ml_tags             = 0;

	ml_initialized = 0;
	return 0;
}


/*
 * add file.
 */
int media_library_add_file(const string spath)
{
	struct fennec_audiotag        at, fake_at;
	unsigned long                 pid;
	struct media_file_tag         ftag;
	uint32_t                      lastpos, laststop;
	struct fennec_audiotag_item  *cti, *cdt /* current data tag (real) */;


	if(!ml_initialized)
		media_library_initialize();

	if(!ml_mapped_file)
	{
		ml_mapped_file_size = sizeof(struct media_file_header);
		ml_mapped_file      = (uint8_t*) sys_mem_alloc(ml_mapped_file_size);

		ml_head = (struct media_file_header*)ml_mapped_file;

		ml_head->sign[0]       = 'f';
		ml_head->sign[1]       = 'm';
		ml_head->sign[2]       = 'l';
		ml_head->sign[3]       = 'f';
		ml_head->size          = ml_mapped_file_size;
		ml_head->start_of_tags = ml_mapped_file_size;
		ml_head->version       = 1;
		ml_head->count         = 0;
	}

	pid = audio_input_tagread(spath, &at);
	
	if(pid == -1) return -1;

	ml_changed = 1;

	/* convert */

	ftag.score                       = 0;
	ftag.user                        = 0;
	ftag.sorted_item_index_all_music = ml_head->count;

	ftag.duration    = 0;
	ftag.abitrate    = 0;
	ftag.file_size   = 0;
	ftag.rating      = 0;


	fake_at.tag_title.tsize      = (at.tag_title.tsize      ? (unsigned int)str_size(at.tag_title.tdata)      : 0);
	fake_at.tag_album.tsize      = (at.tag_album.tsize      ? (unsigned int)str_size(at.tag_album.tdata)      : 0);
	fake_at.tag_artist.tsize     = (at.tag_artist.tsize     ? (unsigned int)str_size(at.tag_artist.tdata)     : 0);
	fake_at.tag_origartist.tsize = (at.tag_origartist.tsize ? (unsigned int)str_size(at.tag_origartist.tdata) : 0);
	fake_at.tag_composer.tsize   = (at.tag_composer.tsize   ? (unsigned int)str_size(at.tag_composer.tdata)   : 0);
	fake_at.tag_lyricist.tsize   = (at.tag_lyricist.tsize   ? (unsigned int)str_size(at.tag_lyricist.tdata)   : 0);
	fake_at.tag_band.tsize       = (at.tag_band.tsize       ? (unsigned int)str_size(at.tag_band.tdata)       : 0);
	fake_at.tag_copyright.tsize  = (at.tag_copyright.tsize  ? (unsigned int)str_size(at.tag_copyright.tdata)  : 0);
	fake_at.tag_publish.tsize    = (at.tag_publish.tsize    ? (unsigned int)str_size(at.tag_publish.tdata)    : 0);
	fake_at.tag_genre.tsize      = (at.tag_genre.tsize      ? (unsigned int)str_size(at.tag_genre.tdata)      : 0);
	fake_at.tag_year.tsize       = (at.tag_year.tsize       ? (unsigned int)str_size(at.tag_year.tdata)       : 0);
	fake_at.tag_filepath.tsize	 = (unsigned int)str_size(spath);
	fake_at.tag_bpm.tsize        = (at.tag_bpm.tsize        ? (unsigned int)str_size(at.tag_bpm.tdata)        : 0);
	fake_at.tag_tracknum.tsize   = (at.tag_tracknum.tsize   ? (unsigned int)str_size(at.tag_tracknum.tdata)   : 0);

	ftag.sizeof_title             = fake_at.tag_title.tsize      + sizeof(letter);
	ftag.sizeof_album             = fake_at.tag_album.tsize      + sizeof(letter);
	ftag.sizeof_artist            = fake_at.tag_artist.tsize     + sizeof(letter);
	ftag.sizeof_original_artist   = fake_at.tag_origartist.tsize + sizeof(letter);
	ftag.sizeof_composer          = fake_at.tag_composer.tsize   + sizeof(letter);
	ftag.sizeof_lyricist          = fake_at.tag_lyricist.tsize   + sizeof(letter);
	ftag.sizeof_band			  = fake_at.tag_band.tsize       + sizeof(letter);
	ftag.sizeof_copyright		  = fake_at.tag_copyright.tsize  + sizeof(letter);
	ftag.sizeof_publish			  = fake_at.tag_publish.tsize    + sizeof(letter);
	ftag.sizeof_genre			  = fake_at.tag_genre.tsize      + sizeof(letter);
	ftag.sizeof_year              = fake_at.tag_year.tsize       + sizeof(letter);
	ftag.sizeof_file_path		  = fake_at.tag_filepath.tsize	 + sizeof(letter);
	ftag.sizeof_bpm				  = fake_at.tag_bpm.tsize        + sizeof(letter);
	ftag.sizeof_track			  = fake_at.tag_tracknum.tsize   + sizeof(letter);

	ftag.size  = ftag.sizeof_title            +
				 ftag.sizeof_album            +
				 ftag.sizeof_artist           +
				 ftag.sizeof_original_artist  +
				 ftag.sizeof_composer         +
				 ftag.sizeof_lyricist         +
				 ftag.sizeof_band			  +
				 ftag.sizeof_copyright		  +
				 ftag.sizeof_publish		  +
				 ftag.sizeof_genre			  +
				 ftag.sizeof_year             +
				 ftag.sizeof_file_path		  +
				 ftag.sizeof_bpm			  +
				 ftag.sizeof_track			  +
				 sizeof(struct media_file_tag) + 4 /* padding */;


	lastpos             = ml_mapped_file_size;
	laststop            = ml_mapped_file_size;
	ml_mapped_file_size += ftag.size;
	ml_mapped_file      = (uint8_t*) sys_mem_realloc(ml_mapped_file, ml_mapped_file_size);

	ml_head = (struct media_file_header*)ml_mapped_file;

	{
		/* copy data */

		lastpos +=  sizeof(struct media_file_tag);


		cti = &fake_at.tag_title; cdt = &at.tag_title;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_title = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_album; cdt = &at.tag_album;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_album = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_artist; cdt = &at.tag_artist;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_artist = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_origartist; cdt = &at.tag_origartist;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_original_artist = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_composer; cdt = &at.tag_composer;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_composer = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_lyricist; cdt = &at.tag_lyricist;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_lyricist = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_band; cdt = &at.tag_band;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_band = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_copyright; cdt = &at.tag_copyright;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_copyright = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_publish; cdt = &at.tag_publish;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_publish = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_genre; cdt = &at.tag_genre;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_genre = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_year; cdt = &at.tag_year;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_year = lastpos;
			lastpos += cti->tsize + sizeof(letter);
		
		/* file path */
			memcpy(ml_mapped_file + lastpos, spath, ftag.sizeof_file_path - sizeof(letter));
			memset(ml_mapped_file + lastpos + (ftag.sizeof_file_path - sizeof(letter)), 0, sizeof(letter));
			ftag.pointer_file_path = lastpos;
			lastpos += ftag.sizeof_file_path;

		cti = &fake_at.tag_bpm; cdt = &at.tag_bpm;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_bpm = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		cti = &fake_at.tag_tracknum; cdt = &at.tag_tracknum;
			if(cti->tsize) memcpy(ml_mapped_file + lastpos, cdt->tdata, cti->tsize);
			memset(ml_mapped_file + lastpos + cti->tsize, 0, sizeof(letter));
			ftag.pointer_track = lastpos;
			lastpos += cti->tsize + sizeof(letter);

		/* padding */

		ml_mapped_file[lastpos] = 0xf;
		ml_mapped_file[lastpos + 1] = 0xa;
		ml_mapped_file[lastpos + 2] = 0xc;
		ml_mapped_file[lastpos + 3] = 0xe;
		lastpos += 4;

		/* add all details */


		memcpy(ml_mapped_file + laststop, &ftag, sizeof(struct media_file_tag));


			
		/* add to list */

		ml_head->count++;
		ml_tags = (int*) sys_mem_realloc(ml_tags, ml_head->count * sizeof(int*));
		ml_tags[ml_head->count - 1] = (int)laststop;
	}




	audio_input_tagread_known(pid, 0, &at); /* free */

	
	return 0;
}


/*
 * save the mapped file into disc.
 */
int media_library_save(const string spath)
{
	t_sys_file_handle   fhandle;

	if(!ml_initialized) return -1;
	if(!ml_mapped_file || !ml_mapped_file_size) return -1;


	fhandle = sys_file_createforce(spath, v_sys_file_forwrite);

	if(fhandle == v_error_sys_file_create)
	{
		reportx("couldn't save the media library file: %s", rt_suberror, spath);
		return -1;
	}

	sys_file_write(fhandle, ml_mapped_file, ml_mapped_file_size);
	sys_file_seteof(fhandle);


	sys_file_close(fhandle);

	return 0;
}


/*
 * map file from a disc, and make the array of items.
 */
int media_library_load(const string spath)
{
	t_sys_file_handle            fhandle;
	unsigned long                i;
	int                          tmpipt, tmpipt_sub;


	if(!ml_initialized) return -1;

	if(ml_mapped_file)
		sys_mem_free(ml_mapped_file);

	/* map file into memory */

	fhandle = sys_file_openshare(spath, v_sys_file_forread);
	
	if(fhandle == v_error_sys_file_open)
	{
		reportx("media library file cannot be loaded: %s", rt_suberror, spath);
		return -2;
	}

	ml_mapped_file_size = sys_file_getsize(fhandle);
	ml_mapped_file      = (uint8_t*) sys_mem_alloc(ml_mapped_file_size);

	if(!ml_mapped_file) report("out of memory! [ml load]", rt_error);

	sys_file_read(fhandle, ml_mapped_file, ml_mapped_file_size);

	sys_file_close(fhandle);


	/* decode into memory scheme */


	ml_head = (struct media_file_header*) ml_mapped_file;

	if(ml_head->version > 1)
		report("invalid version number!", rt_warning);

	ml_head->count;

	if(ml_tags)
		sys_mem_free(ml_tags);

	ml_tags = (int*) sys_mem_alloc(ml_head->count * sizeof(struct media_file_tag**));

	if(!ml_tags) report("out of memory! [ml load]", rt_error);


	/* add entries to the array */

	tmpipt  = 0;
	tmpipt += ml_head->start_of_tags; /* now we're at the start of the first tag */

	for(i=0; i<ml_head->count; i++)
	{
		ml_tags[i]  = tmpipt;
		

		tmpipt_sub  = tmpipt;
		tmpipt_sub += sizeof(struct media_file_tag);

		ml_gtag(i)->pointer_title               = tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_title;
		ml_gtag(i)->pointer_album               = tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_album;
		ml_gtag(i)->pointer_artist              = tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_artist;
		ml_gtag(i)->pointer_original_artist     = tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_original_artist;
		ml_gtag(i)->pointer_composer			= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_composer;
		ml_gtag(i)->pointer_lyricist			= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_lyricist;
		ml_gtag(i)->pointer_band                = tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_band;
		ml_gtag(i)->pointer_copyright			= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_copyright;
		ml_gtag(i)->pointer_publish				= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_publish;
		ml_gtag(i)->pointer_genre				= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_genre;
		ml_gtag(i)->pointer_year				= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_year;
		ml_gtag(i)->pointer_bpm                 = tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_bpm;
		ml_gtag(i)->pointer_track				= tmpipt_sub;  /**/  tmpipt_sub += ml_gtag(i)->sizeof_track;
					
		tmpipt  += ml_gtag(i)->size;
	}

	return 0;
}

/*
 * get a pointer to "media_file_tag".
 */
string media_library_getsource(unsigned long id)
{
	if(!ml_initialized) return 0;
	if(id >= ml_head->count) return 0;

	return (string)(ml_mapped_file + ml_gtag(id)->pointer_file_path      );
}


/*
 * get source location by id
 */
void *media_library_get(unsigned long id)
{
	if(!ml_initialized) return 0;
	if(id >= ml_head->count) return 0;

	return (ml_mapped_file + ml_tags[id]);
}


/*
 * get a pointer to "media_file_tag".
 */
int media_library_remove(unsigned long id)
{
	struct media_file_tag   *mft;

	if(!ml_initialized) return 0;
	if(id >= ml_head->count) return 0;

	
	memcpy(ml_mapped_file + ml_tags[id], ml_mapped_file + ml_tags[ml_head->count - 1], sizeof(struct media_file_tag));

	mft = (struct media_file_tag*)(ml_mapped_file + ml_tags[ml_head->count - 1]);
	mft->size = 0;
	ml_head->count--;

	return 1;
}


/*
 * translate data to a "fennec_audiotag".
 */
int media_library_translate(unsigned long id, struct fennec_audiotag *at)
{
	if(!ml_initialized) return -1;
	if(!at) return -1;
	if(id >= ml_head->count) return -1;

	if(mode_sorted)
		id = ml_gtag(id)->sorted_item_index_all_music;

	memset(at, 0, sizeof(struct fennec_audiotag));

	at->tag_title.tsize       = max(0, ml_gtag(id)->sizeof_title           - sizeof(letter) );
	at->tag_album.tsize       = max(0, ml_gtag(id)->sizeof_album           - sizeof(letter) );
	at->tag_artist.tsize      = max(0, ml_gtag(id)->sizeof_artist          - sizeof(letter) );
	at->tag_origartist.tsize  = max(0, ml_gtag(id)->sizeof_original_artist - sizeof(letter) );
	at->tag_composer.tsize    = max(0, ml_gtag(id)->sizeof_composer        - sizeof(letter) );
	at->tag_lyricist.tsize    = max(0, ml_gtag(id)->sizeof_lyricist        - sizeof(letter) );
	at->tag_band.tsize        = max(0, ml_gtag(id)->sizeof_band            - sizeof(letter) );
	at->tag_copyright.tsize   = max(0, ml_gtag(id)->sizeof_copyright       - sizeof(letter) );
	at->tag_publish.tsize     = max(0, ml_gtag(id)->sizeof_publish         - sizeof(letter) );
	at->tag_genre.tsize       = max(0, ml_gtag(id)->sizeof_genre           - sizeof(letter) );
	at->tag_year.tsize        = max(0, ml_gtag(id)->sizeof_year            - sizeof(letter) );
	at->tag_filepath.tsize    = max(0, ml_gtag(id)->sizeof_file_path       - sizeof(letter) );
	at->tag_bpm.tsize         = max(0, ml_gtag(id)->sizeof_bpm             - sizeof(letter) );
	at->tag_tracknum.tsize    = max(0, ml_gtag(id)->sizeof_track           - sizeof(letter) );

	at->tag_title.tdata       = (string)(ml_mapped_file + ml_gtag(id)->pointer_title          );
	at->tag_album.tdata       = (string)(ml_mapped_file + ml_gtag(id)->pointer_album          );
	at->tag_artist.tdata      = (string)(ml_mapped_file + ml_gtag(id)->pointer_artist         );
	at->tag_origartist.tdata  = (string)(ml_mapped_file + ml_gtag(id)->pointer_original_artist);
	at->tag_composer.tdata    = (string)(ml_mapped_file + ml_gtag(id)->pointer_composer       );
	at->tag_lyricist.tdata    = (string)(ml_mapped_file + ml_gtag(id)->pointer_lyricist       );
	at->tag_band.tdata        = (string)(ml_mapped_file + ml_gtag(id)->pointer_band           );
	at->tag_copyright.tdata   = (string)(ml_mapped_file + ml_gtag(id)->pointer_copyright      );
	at->tag_publish.tdata     = (string)(ml_mapped_file + ml_gtag(id)->pointer_publish        );
	at->tag_genre.tdata       = (string)(ml_mapped_file + ml_gtag(id)->pointer_genre          );
	at->tag_year.tdata        = (string)(ml_mapped_file + ml_gtag(id)->pointer_year           );
	at->tag_filepath.tdata    = (string)(ml_mapped_file + ml_gtag(id)->pointer_file_path      );
	at->tag_bpm.tdata         = (string)(ml_mapped_file + ml_gtag(id)->pointer_bpm            );
	at->tag_tracknum.tdata    = (string)(ml_mapped_file + ml_gtag(id)->pointer_track          );

	return 0;
}

void local_setmemtext(string *ds, string ss)
{
	*ds = sys_mem_alloc(str_size(ss) + sizeof(letter));
	str_cpy(*ds, ss);
}


/*
 * get dir names [while( foo(id, x, buffer) ) id++; ].
 */
int media_library_get_dir_names(unsigned long id, unsigned long pdir, string *dname)
{
	if(!ml_initialized) return 0;
	if(!ml_mapped_file) return 0;

	if(id == (unsigned long)-1 && dname) /* free */
	{
		sys_mem_free(dname);
		return 0;
	}

	switch(pdir)
	{
	case media_library_dir_root:
		switch(id)
		{
		case 0: local_setmemtext(dname, uni("All Music")); return 1;
		case 1: local_setmemtext(dname, uni("Artists"));   return 1;
		case 2: local_setmemtext(dname, uni("Albums"));    return 1;
		case 3: local_setmemtext(dname, uni("Genres"));    return 1;
		case 4: local_setmemtext(dname, uni("Years"));     return 1;
		/* rating */

		default: return 0;
		}
		break;

	case media_library_dir_artists:
		if(id >= ml_head->count) return 0;
		if(mode_sorted)
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(id)->pointer_artist));
		else
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(ml_gtag(id)->sorted_item_index_all_music)->pointer_artist));
		return 1;
		
	case media_library_dir_albums:
		if(id >= ml_head->count) return 0;
		if(mode_sorted)
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(id)->pointer_album));
		else
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(ml_gtag(id)->sorted_item_index_all_music)->pointer_album));
		return 1;

	case media_library_dir_genres:
		if(id >= ml_head->count) return 0;
		if(mode_sorted)
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(id)->pointer_genre));
		else
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(ml_gtag(id)->sorted_item_index_all_music)->pointer_genre));
		return 1;

	case media_library_dir_years:
		if(id >= ml_head->count) return 0;
		if(mode_sorted)
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(id)->pointer_year));
		else
			local_setmemtext(dname, (string)(ml_mapped_file + ml_gtag(ml_gtag(id)->sorted_item_index_all_music)->pointer_year));
		return 1;

	}
	
	return 0;
}

/*
 * get dir tag indexes [while( foo(id, x, x, buffer) ) id++; ].
 * pdir - parent dir.
 * cdir - child dir.
 */
int media_library_get_dir_files(unsigned long id, unsigned long pdir, string cdir, uint32_t *tid)
{
	if(!ml_initialized) return 0;
	if(!ml_mapped_file) return 0;

	/* all music */

	if(pdir == media_library_dir_all)
	{
		if(id == (unsigned long)-1)
		{
			if(tid)
				*tid = (uint32_t)ml_head->count;
			return (int)ml_head->count;

		}else{
			if(id >= ml_head->count) return 0;
			*tid = (uint32_t)id;
			return 1;
		}
	}

	/* --------------------- */

	if((pdir == media_library_dir_artists) && cdir)
	{
		unsigned long i, f = 0, d;

		if(id >= ml_head->count) return 0;
			
		i = *tid;


		for(; i<ml_head->count; i++)
		{
			if(mode_sorted)
				d = ml_gtag(i)->sorted_item_index_all_music;
			else
				d = i;

			if(!str_icmp((string)(ml_mapped_file + ml_gtag(d)->pointer_artist), cdir))
			{
				f = 1;
				*tid = i;
				break;
			}
		}
		return f;
	}

	/* --------------------- */

	if((pdir == media_library_dir_albums) && cdir)
	{
		unsigned long i, f = 0, d;

		if(id >= ml_head->count) return 0;
			
		i = *tid;


		for(; i<ml_head->count; i++)
		{
			if(mode_sorted)
				d = ml_gtag(i)->sorted_item_index_all_music;
			else
				d = i;

			if(!str_icmp((string)(ml_mapped_file + ml_gtag(d)->pointer_album), cdir))
			{
				f = 1;
				*tid = i;
				break;
			}
		}
		return f;
	}

	/* --------------------- */

	if((pdir == media_library_dir_genres) && cdir)
	{
		unsigned long i, f = 0, d;

		if(id >= ml_head->count) return 0;
			
		i = *tid;


		for(; i<ml_head->count; i++)
		{
			if(mode_sorted)
				d = ml_gtag(i)->sorted_item_index_all_music;
			else
				d = i;

			if(!str_icmp((string)(ml_mapped_file + ml_gtag(d)->pointer_genre), cdir))
			{
				f = 1;
				*tid = i;
				break;
			}
		}
		return f;
	}

	/* --------------------- */

	if((pdir == media_library_dir_years) && cdir)
	{
		unsigned long i, f = 0, d;

		if(id >= ml_head->count) return 0;
			
		i = *tid;


		for(; i<ml_head->count; i++)
		{
			if(mode_sorted)
				d = ml_gtag(i)->sorted_item_index_all_music;
			else
				d = i;

			if(!str_icmp((string)(ml_mapped_file + ml_gtag(d)->pointer_year), cdir))
			{
				f = 1;
				*tid = i;
				break;
			}
		}
		return f;
	}
	return 0;
}


/*
 * search sub directories and add files.
 */
int media_library_add_sub_dir(const string fpath)
{
	HANDLE           hf;
	WIN32_FIND_DATA  fd;
	unsigned long    ide = 0;
	letter           ext[128];
	letter           einfo[260];
	letter           dpath[v_sys_maxpath];
	letter           ipath[v_sys_maxpath];

	ext[0]   = 0;
	einfo[0] = 0;
	dpath[0] = 0;

	while(audio_input_getextensionsinfo(ide, ext, einfo))
	{
		str_cpy(dpath, fpath);
		str_cat(dpath, uni("\\*."));
		str_cat(dpath, ext);

		hf = FindFirstFile(dpath, &fd);

		if(hf == INVALID_HANDLE_VALUE)goto extepoint;

		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		basewindows_wait_function(wait_window_set_detail, 0, ipath);
		media_library_add_file(ipath);

		while(FindNextFile(hf, &fd))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			basewindows_wait_function(wait_window_set_detail, 0, ipath);
			media_library_add_file(ipath);
		}

		FindClose(hf);

extepoint:
		ide++;
		ext[0]   = 0;
		dpath[0] = 0;
	}

	str_cpy(dpath, fpath);
	str_cat(dpath, uni("\\*"));

	hf = FindFirstFile(dpath, &fd);

	if(hf == INVALID_HANDLE_VALUE) return 0;

	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
	{
		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);
		
		sys_sleep(1);

		if(!mode_search_running)
		{
			FindClose(hf);
			return 1;
		}

		media_library_add_sub_dir(ipath);
	}

	while(FindNextFile(hf, &fd))
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			sys_sleep(1);

			if(!mode_search_running)
			{
				FindClose(hf);
				return 1;
			}

			media_library_add_sub_dir(ipath);
		}
	}

	FindClose(hf);

	return 1;
}


/*
 * search directories and add files.
 */
int media_library_add_dir(const string fpath)
{
	HANDLE           hf;
	WIN32_FIND_DATA  fd;
	unsigned long    ide = 0;
	letter           ext[128];
	letter           einfo[260];
	letter           dpath[v_sys_maxpath];
	letter           ipath[v_sys_maxpath];

	ext[0]   = 0;
	einfo[0] = 0;
	dpath[0] = 0;

	mode_search_running = 1;

	basewindows_show_wait(window_main);
	basewindows_wait_function(wait_window_set_text, 0, (void*)uni("Searching and adding files to media library..."));
	basewindows_wait_function(wait_window_setcancel, 0, (void*)&mode_search_running);


	while(audio_input_getextensionsinfo(ide, ext, einfo))
	{
		str_cpy(dpath, fpath);
		str_cat(dpath, uni("\\*."));
		str_cat(dpath, ext);

		hf = FindFirstFile(dpath, &fd);

		if(hf == INVALID_HANDLE_VALUE)goto extepoint;

		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		basewindows_wait_function(wait_window_set_detail, 0, ipath);
		media_library_add_file(ipath);

		while(FindNextFile(hf, &fd))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			basewindows_wait_function(wait_window_set_detail, 0, ipath);
			media_library_add_file(ipath);
		}

		FindClose(hf);

extepoint:
		ide++;
		ext[0]   = 0;
		dpath[0] = 0;
	}

	str_cpy(dpath, fpath);
	str_cat(dpath, uni("\\*"));

	hf = FindFirstFile(dpath, &fd);

	if(hf == INVALID_HANDLE_VALUE)
	{
		basewindows_wait_function(wait_window_end, 0, 0);
		return 0;
	}

	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
	{
		str_cpy(ipath, fpath);
		str_cat(ipath, uni("\\"));
		str_cat(ipath, fd.cFileName);

		sys_sleep(1);

		if(!mode_search_running)
		{
			FindClose(hf);
			return 1;
		}

		media_library_add_sub_dir(ipath);
	}

	while(FindNextFile(hf, &fd))
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != uni('.'))
		{
			str_cpy(ipath, fpath);
			str_cat(ipath, uni("\\"));
			str_cat(ipath, fd.cFileName);

			sys_sleep(1);
			
			if(!mode_search_running)
			{
				FindClose(hf);
				return 1;
			}

			media_library_add_sub_dir(ipath);
		}
	}

	FindClose(hf);

	basewindows_wait_function(wait_window_end, 0, 0);
	return 1;
}

int fennec_tag_item_compare(struct fennec_audiotag_item *a, struct fennec_audiotag_item *b)
{
	int v;
	if(a->tsize && a->tsize)
		v = abs(str_cmp(a->tdata, a->tdata));
	else
		v = 1;
	return v;
}

/* 
 * general function for tag comparison *.
 */
int fennec_tag_compare(struct fennec_audiotag *a, struct fennec_audiotag *b)
{
	int v;

	v = 0;

	v += fennec_tag_item_compare(&a->tag_title         , &b->tag_title        ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_album         , &b->tag_album        ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_artist        , &b->tag_artist       ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_origartist    , &b->tag_origartist   ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_composer      , &b->tag_composer     ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_lyricist      , &b->tag_lyricist     ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_band          , &b->tag_band         ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_copyright     , &b->tag_copyright    ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_publish       , &b->tag_publish      ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_encodedby     , &b->tag_encodedby    ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_genre         , &b->tag_genre        ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_year          , &b->tag_year         ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_url           , &b->tag_url          ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_offiartisturl , &b->tag_offiartisturl); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_comments      , &b->tag_comments     ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_lyric         , &b->tag_lyric        ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_bpm           , &b->tag_bpm          ); if(v)return v;
	v += fennec_tag_item_compare(&a->tag_tracknum      , &b->tag_tracknum     ); if(v)return v;
	return v;
}

/*
 * get advanced function pointer
 */
int media_library_advanced_function(int fid, int fdata, void *rf)
{
	switch(fid)
	{
	case 1: /* safe initialize */
		if(!ml_initialized)media_library_initialize();
		break;

	case 2: /* exchange (fdata <- rt) */
		{
			unsigned long di = (unsigned long)fdata, si = PtrToUlong(rf), bki;

			if(di >= ml_head->count) return -1;
			if(si >= ml_head->count) return -1;

			bki = ml_gtag(si)->sorted_item_index_all_music;
			ml_gtag(si)->sorted_item_index_all_music = ml_gtag(di)->sorted_item_index_all_music;
			ml_gtag(di)->sorted_item_index_all_music = bki;
			
			ml_changed = 1;
		}
		break;

	case 3: /* set/get sorting mode */

		if(fdata == -1)return mode_sorted;
		else mode_sorted = fdata;
		break;

	case 4: /* jump into a dir */

		switch(fdata)
		{
		case 0:  return media_library_dir_all;
		case 1:  return media_library_dir_artists;
		case 2:  return media_library_dir_albums;
		case 3:  return media_library_dir_genres;
		case 4:  return media_library_dir_years;
		/* rating */
		default: return media_library_dir_root;
		}
		break;

	case 5: /* clear */

		media_library_uninitialize();
		media_library_initialize_base();

		if(!ml_mapped_file)
		{
			ml_mapped_file_size = sizeof(struct media_file_header);
			ml_mapped_file      = (uint8_t*) sys_mem_alloc(ml_mapped_file_size);

			ml_head = (struct media_file_header*)ml_mapped_file;

			ml_head->sign[0]       = 'f';
			ml_head->sign[1]       = 'm';
			ml_head->sign[2]       = 'l';
			ml_head->sign[3]       = 'f';
			ml_head->size          = ml_mapped_file_size;
			ml_head->start_of_tags = ml_mapped_file_size;
			ml_head->version       = 1;
			ml_head->count         = 0;
		}

		ml_changed = 1;
		break;

	case 6: /* import */
	case 7: /* export */
		{
			unsigned int  c;
			letter        fpath[v_sys_maxpath];
			OPENFILENAME  lofn;

			memset(&lofn, 0, sizeof(lofn));

			fpath[0] = 0;

			lofn.lStructSize     = sizeof(lofn);
			
			if(fid == 6)
				lofn.lpstrTitle      = uni("Load Media Library File");
			else
				lofn.lpstrTitle      = uni("Export Media Library File");
				
			lofn.hwndOwner       = window_main;
			lofn.lpstrFile       = fpath;
			lofn.nMaxFile        = sizeof(fpath);
			lofn.lpstrFilter     = uni("Fennec Settings File (*.fsd)\0*.fsd\0\0");
			lofn.nFilterIndex    = 0;
			lofn.lpstrFileTitle  = 0;
			lofn.nMaxFileTitle   = 0;
			lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
			lofn.hInstance       = instance_fennec;

			if(fid == 6)
				GetOpenFileName(&lofn);
			else
				GetSaveFileName(&lofn);

			c = (unsigned int) str_len(fpath);
			
			if(c)
			{
				if(!ml_initialized)media_library_initialize();

				if(fid == 6)
				{
					media_library_load(fpath);
				}else{
					
					if(str_icmp(fpath + (c - 4), uni(".fsd")) != 0)
						str_cat(fpath, uni(".fsd"));

					media_library_save(fpath);
				}
			}
		}
		break;

	case 8: /* file update: remove if the source doesn't exist/change tags */
		/* rf: file name */
		
		{
			unsigned long           i, fid;
			string                  fname = (string)rf;
			string                  tpath;
			
			/* validate */
			if(!ml_initialized) break;

			for(i=0; i<ml_head->count; i++)
			{
				tpath = (string)(ml_mapped_file + ml_gtag(i)->pointer_file_path);
				if(!str_icmp(tpath, fname))
				{
					/* check for availability */

					fid = i;

					if(!sys_fs_file_exist(fname))
					{
						media_library_remove(fid);
						return 0;
					}
					break;
				}
			}

			/* update tags */

			//media_library_translate(fid, &ml_tag);
			//pid = audio_input_tagread(fname, &rtag);

			//if(fennec_tag_compare(&rtag, &ml_tag))
			//{
				/* easy fix for updates */
			//	media_library_remove(fid);
			//	media_library_add_file(fname);
			//}

		}
		break;
	}
	return 0;
}



/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

