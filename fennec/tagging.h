/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
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

#ifndef header_tagging
#define header_tagging

#include "systems/system.h"

#define    tag_memmode_static  0 /* don't need to be deallocated */
#define    tag_memmode_dynamic 1 /* 'free' to deallocate */

struct fennec_audiotag_item
{
	unsigned int   tsize;     /* tag size (zero: no tag) */
	unsigned short tmode;     /* memory allocation mode */
	string         tdata;     /* data */
	unsigned int   tdatai;    /* numeric data */
	unsigned int   treserved; /* reserved */
};

struct fennec_audiotag
{
	struct fennec_audiotag_item    tag_title;
	struct fennec_audiotag_item    tag_album;
	struct fennec_audiotag_item    tag_artist;
	struct fennec_audiotag_item    tag_origartist;
	struct fennec_audiotag_item    tag_composer;
	struct fennec_audiotag_item    tag_lyricist;
	struct fennec_audiotag_item    tag_band;
	struct fennec_audiotag_item    tag_copyright;
	struct fennec_audiotag_item    tag_publish;
	struct fennec_audiotag_item    tag_encodedby;
	struct fennec_audiotag_item    tag_genre;
	struct fennec_audiotag_item    tag_year;
	struct fennec_audiotag_item    tag_url;
	struct fennec_audiotag_item    tag_offiartisturl;
	struct fennec_audiotag_item    tag_filepath;
	struct fennec_audiotag_item    tag_filename;
	struct fennec_audiotag_item    tag_comments;
	struct fennec_audiotag_item    tag_lyric;
	struct fennec_audiotag_item    tag_bpm;
	struct fennec_audiotag_item    tag_tracknum;
	struct fennec_audiotag_item    tag_image;

	unsigned int                   tid;
	unsigned int                   treserved;
};

extern const string tag_genres[];

#define tag_genres_count 125

int tags_translate(string mem, struct fennec_audiotag* tags, string fpath);

#endif

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/