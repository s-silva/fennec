/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (MPEG).
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

#include "main.h"

/* genre list -------------------------------------------------------------------------------------------*/

string genres[] =
{ uni("Blues"), uni("Classic Rock"), uni("Country"), uni("Dance"), uni("Disco"), uni("Funk"), uni("Grunge"), uni("Hip-Hop"), uni("Jazz")
, uni("Metal"), uni("New Age"), uni("Oldies"), uni("Other"), uni("Pop"), uni("R&B"), uni("Rap"), uni("Reggae"), uni("Rock"), uni("Techno")
, uni("Industrial"), uni("Alternative"), uni("Ska"), uni("Death Metal"), uni("Pranks"), uni("Soundtrack"), uni("Euro-Techno")
, uni("Ambient"), uni("Trip-Hop"), uni("Vocal"), uni("Jazz+Funk"), uni("Fusion"), uni("Trance"), uni("Classical"), uni("Instrumental")
, uni("Acid"), uni("House"), uni("Game"), uni("Sound Clip"), uni("Gospel"), uni("Noise"), uni("Alternative Rock"), uni("Bass"), uni("Soul")
, uni("Punk"), uni("Space"), uni("Meditative"), uni("Instrumental Pop"), uni("Instrumental Rock"), uni("Ethnic"), uni("Gothic")
, uni("Darkwave"), uni("Techno-Industrial"), uni("Electronic"), uni("Pop-Folk"), uni("Eurodance"), uni("Dream"), uni("Southern Rock")
, uni("Comedy"), uni("Cult"), uni("Gangsta"), uni("Top 40"), uni("Christian Rap"), uni("Pop/Funk"), uni("Jungle"), uni("Native American")
, uni("Cabaret"), uni("New Wave"), uni("Psychadelic"), uni("Rave"), uni("Showtunes"), uni("Trailer"), uni("Lo-Fi"), uni("Tribal")
, uni("Acid Punk"), uni("Acid Jazz"), uni("Polka"), uni("Retro"), uni("Musical"), uni("Rock & Roll"), uni("Hard Rock"), uni("Folk")
, uni("Folk-Rock"), uni("National Folk"), uni("Swing"), uni("Fast Fusion"), uni("Bebob"), uni("Latin"), uni("Revival"), uni("Celtic")
, uni("Bluegrass"), uni("Avantgarde"), uni("Gothic Rock"), uni("Progressive Rock"), uni("Psychedelic Rock")
, uni("Symphonic Rock"), uni("Slow Rock"), uni("Big Band"), uni("Chorus"), uni("Easy Listening"), uni("Acoustic"), uni("Humour")
, uni("Speech"), uni("Chanson"), uni("Opera"), uni("Chamber Music"), uni("Sonata"), uni("Symphony"), uni("Booty Bass"), uni("Primus")
, uni("Porn Groove"), uni("Satire"), uni("Slow Jam"), uni("Club"), uni("Tango"), uni("Samba"), uni("Folklore"), uni("Ballad"), uni("Power Ballad")
, uni("Rhythmic Soul"), uni("Freestyle"), uni("Duet"), uni("Punk Rock"), uni("Drum Solo"), uni("A capella"), uni("Euro-House"), uni("Dance Hall")};

#define genres_count 125

/*-------------------------------------------------------------------------------------------------------*/

void str_make_win(char* str)
{
	unsigned int i = 0;                /* str (string to be replaced) pointer */
	unsigned int flen;                 /* full (str) length */

	flen = (unsigned int)strlen(str);

	while(str[i])
	{
		if(str[i] == '\n' || str[i] == '\r')
		{

			str[i] = '\r';
			memmove(str + i + 1, str + i, flen - i);
			str[i + 1] = '\n';
			flen++;
			str[flen] = 0;
			i++;
		}

		i++;
	}
}

void str_make_tag(char* str)
{
	unsigned int i = 0;                /* str (string to be replaced) pointer */
	unsigned int flen;                 /* full (str) length */

	flen = (unsigned int)strlen(str);

	while(str[i])
	{
		if(str[i] == '\r' && str[i + 1] == '\n')
		{

			str[i] = '\r';
			memmove(str + i, str + i + 1, flen - i - 1);
			flen--;
			str[flen] = 0;
			i++;
		}

		i++;
	}
}

int tagwrite(const string fname,  struct fennec_audiotag *wtag)
{	
	void* atag;

	if(!wtag)
	{
		tag_old_clear(fname);
		return tag_write_clear(fname);
	}

	atag = tag_fromfile(fname);

	if(!atag)
	{
		atag = tag_create(3);
	}

	if(wtag->tag_title.tsize         )atag = tag_write_setframe(atag, "TIT2", wtag->tag_title.tdata         );
	else tag_write_removeframe(atag, "TIT2");
	if(wtag->tag_album.tsize         )atag = tag_write_setframe(atag, "TALB", wtag->tag_album.tdata         );
	else tag_write_removeframe(atag, "TALB");
	if(wtag->tag_artist.tsize        )atag = tag_write_setframe(atag, "TPE1", wtag->tag_artist.tdata        );
	else tag_write_removeframe(atag, "TPE1");
	if(wtag->tag_origartist.tsize    )atag = tag_write_setframe(atag, "TOPE", wtag->tag_origartist.tdata    );
	else tag_write_removeframe(atag, "TOPE");
	if(wtag->tag_composer.tsize      )atag = tag_write_setframe(atag, "TCOM", wtag->tag_composer.tdata      );
	else tag_write_removeframe(atag, "TCOM");
	if(wtag->tag_lyricist.tsize      )atag = tag_write_setframe(atag, "TEXT", wtag->tag_lyricist.tdata      );
	else tag_write_removeframe(atag, "TEXT");
	if(wtag->tag_band.tsize          )atag = tag_write_setframe(atag, "TPE2", wtag->tag_band.tdata          );
	else tag_write_removeframe(atag, "TPE2");
	if(wtag->tag_copyright.tsize     )atag = tag_write_setframe(atag, "TCOP", wtag->tag_copyright.tdata     );
	else tag_write_removeframe(atag, "TCOP");
	if(wtag->tag_publish.tsize       )atag = tag_write_setframe(atag, "TPUB", wtag->tag_publish.tdata       );
	else tag_write_removeframe(atag, "TPUB");
	if(wtag->tag_encodedby.tsize     )atag = tag_write_setframe(atag, "TENC", wtag->tag_encodedby.tdata     );
	else tag_write_removeframe(atag, "TENC");
	if(wtag->tag_genre.tsize         )atag = tag_write_setframe(atag, "TCON", wtag->tag_genre.tdata         );
	else tag_write_removeframe(atag, "TCON");
	if(wtag->tag_year.tsize          )atag = tag_write_setframe(atag, "TYER", wtag->tag_year.tdata          );
	else tag_write_removeframe(atag, "TYER");
	if(wtag->tag_url.tsize           )atag = tag_write_setframe(atag, "WXXX", wtag->tag_url.tdata           );
	else tag_write_removeframe(atag, "WXXX");
	if(wtag->tag_offiartisturl.tsize )atag = tag_write_setframe(atag, "WOAR", wtag->tag_offiartisturl.tdata );
	else tag_write_removeframe(atag, "WOAR");

/*	if(wtag->tag_filepath.tsize      )atag = tag_write_setframe(atag,       , wtag->tag_filepath.tdata      );
	else tag_write_removeframe(atag, "    ");
	if(wtag->tag_filename.tsize      )atag = tag_write_setframe(atag,       , wtag->tag_filename.tdata      );
	else tag_write_removeframe(atag, "    ");   */

	if(wtag->tag_comments.tsize      )atag = tag_write_setframe(atag, "COMM", wtag->tag_comments.tdata      );
	else tag_write_removeframe(atag, "COMM");
	if(wtag->tag_lyric.tsize         )atag = tag_write_setframe(atag, "USLT", wtag->tag_lyric.tdata         );
	else tag_write_removeframe(atag, "USLT");
	if(wtag->tag_bpm.tsize           )atag = tag_write_setframe(atag, "TBPM", wtag->tag_bpm.tdata           );
	else tag_write_removeframe(atag, "TBPM");
	if(wtag->tag_tracknum.tsize      )atag = tag_write_setframe(atag, "TRCK", wtag->tag_tracknum.tdata      );
	else tag_write_removeframe(atag, "TRCK");
	

	if(tag_write_tofile(atag, fname))
	{
		string lpMsgBuf;

		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			0,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			0 );

		MessageBox( 0, (LPCTSTR)lpMsgBuf, uni("Error"), MB_OK | MB_ICONINFORMATION );
		LocalFree( lpMsgBuf );
	}

	sys_pass();
	tag_free(atag);
	return 1;
}

string genre_dec(string ig, unsigned int *ogint)
{
	if(!ig)
	{
		if(ogint)
			*ogint = -1;
		return 0;
	}

	if(ig[0] == uni('('))
	{
		/* UTF-16 */

		letter  ibuf[10];
		int     i;

		memset(ibuf, 0, sizeof(ibuf));

		for(i=0; ;i++)
		{
			if(!ig[i + 1])break;
			if( iswdigit(ig[i + 1]) )
				ibuf[i] = ig[i + 1];
			else
				break;
		}

		if(ig[i + 1] == uni(')') )i++;

		str_cpy(ig, ig + i + 1);

		if(ogint)
			*ogint = (unsigned int)str_stoi(ibuf);

		return ig;

	}else{
		if(ogint)
			*ogint = -1;
		return ig;
	}
}

int tagread(const string fname, struct fennec_audiotag *rtag)
{ 
	void                          *atag;
	struct fennec_audiotag_item   *ct;

	string v1_title;
	string v1_album;
	string v1_artist;
	string v1_comments;
	string v1_year;
	string v1_genre;
	string v1_tracknum;

	letter v1t_title     [32];
	letter v1t_album     [32];
	letter v1t_artist    [32];
	letter v1t_comments  [32];
	letter v1t_year      [6];
	int    v1t_genre;
	int    v1t_tracknum;

	if(!rtag)return 0;

	if(!fname)
	{
		if(rtag->tag_title.tsize         ) { tag_string_free(rtag->tag_title.tdata         ); rtag->tag_title.tsize         = 0; }
		if(rtag->tag_album.tsize         ) { tag_string_free(rtag->tag_album.tdata         ); rtag->tag_album.tsize         = 0; }
		if(rtag->tag_artist.tsize        ) { tag_string_free(rtag->tag_artist.tdata        ); rtag->tag_artist.tsize        = 0; }
		if(rtag->tag_origartist.tsize    ) { tag_string_free(rtag->tag_origartist.tdata    ); rtag->tag_origartist.tsize    = 0; }
		if(rtag->tag_composer.tsize      ) { tag_string_free(rtag->tag_composer.tdata      ); rtag->tag_composer.tsize      = 0; }
		if(rtag->tag_lyricist.tsize      ) { tag_string_free(rtag->tag_lyricist.tdata      ); rtag->tag_lyricist.tsize      = 0; }
		if(rtag->tag_band.tsize          ) { tag_string_free(rtag->tag_band.tdata          ); rtag->tag_band.tsize          = 0; }
		if(rtag->tag_copyright.tsize     ) { tag_string_free(rtag->tag_copyright.tdata     ); rtag->tag_copyright.tsize     = 0; }
		if(rtag->tag_publish.tsize       ) { tag_string_free(rtag->tag_publish.tdata       ); rtag->tag_publish.tsize       = 0; }
		if(rtag->tag_encodedby.tsize     ) { tag_string_free(rtag->tag_encodedby.tdata     ); rtag->tag_encodedby.tsize     = 0; }
		if(rtag->tag_genre.tsize         ) { tag_string_free(rtag->tag_genre.tdata         ); rtag->tag_genre.tsize         = 0; }
		if(rtag->tag_year.tsize          ) { tag_string_free(rtag->tag_year.tdata          ); rtag->tag_year.tsize          = 0; }
		if(rtag->tag_url.tsize           ) { tag_string_free(rtag->tag_url.tdata           ); rtag->tag_url.tsize           = 0; }
		if(rtag->tag_offiartisturl.tsize ) { tag_string_free(rtag->tag_offiartisturl.tdata ); rtag->tag_offiartisturl.tsize = 0; }
	  //if(rtag->tag_filepath.tsize      ) { tag_string_free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
	  //if(rtag->tag_filename.tsize      ) { tag_string_free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; }
		if(rtag->tag_comments.tsize      ) { tag_string_free(rtag->tag_comments.tdata      ); rtag->tag_comments.tsize      = 0; }
		if(rtag->tag_lyric.tsize         ) { tag_string_free(rtag->tag_lyric.tdata         ); rtag->tag_lyric.tsize         = 0; }
		if(rtag->tag_bpm.tsize           ) { tag_string_free(rtag->tag_bpm.tdata           ); rtag->tag_bpm.tsize           = 0; }
		if(rtag->tag_tracknum.tsize      ) { tag_string_free(rtag->tag_tracknum.tdata      ); rtag->tag_tracknum.tsize      = 0; }

	}else{

		atag = tag_fromfile(fname);

		rtag->tag_title.tsize         = 0;
		rtag->tag_album.tsize         = 0;
		rtag->tag_artist.tsize        = 0;
		rtag->tag_origartist.tsize    = 0;
		rtag->tag_composer.tsize      = 0;
		rtag->tag_lyricist.tsize      = 0;
		rtag->tag_band.tsize          = 0;
		rtag->tag_copyright.tsize     = 0;
		rtag->tag_publish.tsize       = 0;
		rtag->tag_encodedby.tsize     = 0;
		rtag->tag_genre.tsize         = 0;
		rtag->tag_year.tsize          = 0;
		rtag->tag_url.tsize           = 0;
		rtag->tag_offiartisturl.tsize = 0;
		rtag->tag_filepath.tsize      = 0;
		rtag->tag_filename.tsize      = 0;
		rtag->tag_comments.tsize      = 0;
		rtag->tag_lyric.tsize         = 0;
		rtag->tag_bpm.tsize           = 0;
		rtag->tag_tracknum.tsize      = 0;

		if(!atag) goto point_v1_read;



		ct = &rtag->tag_title;
		ct->tdata  = tag_get_string(atag, "TIT2");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;

		ct = &rtag->tag_album;
		ct->tdata  = tag_get_string(atag, "TALB");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		

		ct = &rtag->tag_artist;
		ct->tdata  = tag_get_string(atag, "TPE1");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_origartist;
		ct->tdata  = tag_get_string(atag, "TOPE");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_composer;
		ct->tdata  = tag_get_string(atag, "TCOM");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_lyricist;
		ct->tdata  = tag_get_string(atag, "TEXT");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_band;
		ct->tdata  = tag_get_string(atag, "TPE2");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_copyright;
		ct->tdata  = tag_get_string(atag, "TCOP");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_publish;
		ct->tdata  = tag_get_string(atag, "TPUB");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_encodedby;
		ct->tdata  = tag_get_string(atag, "TENC");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_genre;

		ct->tdata  = tag_get_string(atag, "TCON");
		ct->tdatai = 0;
		genre_dec(ct->tdata, &ct->tdatai);
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_year;
		ct->tdata  = tag_get_string(atag, "TYER");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_url;
		ct->tdata  = tag_get_string(atag, "WXXX");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_offiartisturl;
		ct->tdata  = tag_get_string(atag, "WOAR");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_comments;
		ct->tdata  = tag_get_string(atag, "COMM");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_lyric;
		ct->tdata  = tag_get_string(atag, "USLT");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_bpm;
		ct->tdata  = tag_get_string(atag, "TBPM");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
		
		ct = &rtag->tag_tracknum;
		ct->tdata  = tag_get_string(atag, "TRCK");
		ct->tdatai = 0;
		ct->tmode  = ct->tdata ? tag_memmode_dynamic : tag_memmode_static;
		ct->tsize  = ct->tdata ? (unsigned int)str_size(ct->tdata) : 0;
				

		/* ---------------------- */


		rtag->tag_filepath.tdata = 0; 
		rtag->tag_filepath.tsize = 0; 
		rtag->tag_filename.tdata = 0; 
		rtag->tag_filename.tsize = 0; 
		
		tag_free(atag);

		return 1;

point_v1_read:

		/* idv1 fill */

		memset(v1t_title    , 0, sizeof(v1t_title    ));
		memset(v1t_album    , 0, sizeof(v1t_album    ));
		memset(v1t_artist   , 0, sizeof(v1t_artist   ));
		memset(v1t_comments , 0, sizeof(v1t_comments ));
		memset(v1t_year     , 0, sizeof(v1t_year     ));
	   
		v1t_genre     = 0;
		v1t_tracknum  = 0;

		if(!tag_old_read(fname, v1t_artist, v1t_title, v1t_album, v1t_year, v1t_comments, &v1t_genre, &v1t_tracknum))
			return 0;
 
		if(rtag->tag_title.tsize)
		{
			if(!str_len(rtag->tag_title.tdata))
			{
				goto point_v1_title;
			}
		}else{
point_v1_title:;

			if(str_len(v1t_title))
			{
				v1_title = sys_mem_alloc(32 * sizeof(letter));
				if(v1_title)
				{
					ct = &rtag->tag_title;
					str_cpy(v1_title, v1t_title);
					ct->tdata  = v1_title;
					ct->tsize  = (unsigned int)str_size(v1_title);
					ct->tdatai = 0;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}

		/* album */

		if(rtag->tag_album.tsize)
		{
			if(!str_len(rtag->tag_album.tdata))
			{
				goto point_v1_album;
			}
		}else{
point_v1_album:;

			if(str_len(v1t_album))
			{
				v1_album = sys_mem_alloc(32 * sizeof(letter));
				if(v1_album)
				{
					ct = &rtag->tag_album;

					str_cpy(v1_album, v1t_album);
					ct->tdata  = v1_album;
					ct->tsize  = (unsigned int)str_size(v1_album);
					ct->tdatai = 0;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}

		/* artist */

		if(rtag->tag_artist.tsize)
		{
			if(!str_len(rtag->tag_artist.tdata))
			{
				goto point_v1_artist;
			}
		}else{
point_v1_artist:;

			if(str_len(v1t_artist))
			{
				v1_artist = sys_mem_alloc(32 * sizeof(letter));
				if(v1_artist)
				{
					ct = &rtag->tag_artist;

					str_cpy(v1_artist, v1t_artist);
					ct->tdata  = v1_artist;
					ct->tsize  = (unsigned int)str_size(v1_artist);
					ct->tdatai = 0;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}

		/* year */

		if(rtag->tag_year.tsize)
		{
			if(!str_len(rtag->tag_year.tdata))
			{
				goto point_v1_year;
			}
		}else{
point_v1_year:;

			if(str_len(v1t_year))
			{
				v1_year = sys_mem_alloc(32 * sizeof(letter));
				if(v1_year)
				{
					ct = &rtag->tag_year;

					str_cpy(v1_year, v1t_year);
					ct->tdata  = v1_year;
					ct->tsize  = (unsigned int)str_size(v1_year);
					ct->tdatai = 0;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}

		/* comments */

		if(rtag->tag_comments.tsize)
		{
			if(!str_len(rtag->tag_comments.tdata))
			{
				goto point_v1_comments;
			}
		}else{
point_v1_comments:;

			if(str_len(v1t_comments))
			{
				v1_comments = sys_mem_alloc(32 * sizeof(letter));
				if(v1_comments)
				{
					ct = &rtag->tag_comments;

					str_cpy(v1_comments, v1t_comments);
					ct->tdata  = v1_comments;
					ct->tsize  = (unsigned int)str_size(v1_comments);
					ct->tdatai = 0;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}
		
		/* genre */

		if(rtag->tag_genre.tsize)
		{
			if(!str_len(rtag->tag_genre.tdata))
			{
				goto point_v1_genre;
			}
		}else{
point_v1_genre:;

			if(v1t_genre < genres_count)
			{
				v1_genre = sys_mem_alloc(32 * sizeof(letter));
				if(v1_genre)
				{
					ct = &rtag->tag_genre;
					
					str_cpy(v1_genre, genres[v1t_genre]);

					ct->tdata  = v1_genre;
					ct->tsize  = (unsigned int)str_size(v1_genre);
					ct->tdatai = v1t_genre;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}

		/* track number */

		if(rtag->tag_tracknum.tsize)
		{
			if(!str_len(rtag->tag_tracknum.tdata))
			{
				goto point_v1_tracknum;
			}
		}else{
point_v1_tracknum:;

			if(v1t_tracknum)
			{
				v1_tracknum = sys_mem_alloc(32 * sizeof(letter));
				if(v1_tracknum)
				{
					ct = &rtag->tag_tracknum;

					memset(v1_tracknum, 0, 32 * sizeof(letter));
					_itow(v1t_tracknum, v1_tracknum, 10);

					ct->tdata  = v1_tracknum;
					ct->tsize  = (unsigned int)str_size(v1_tracknum);
					ct->tdatai = v1t_tracknum;
					ct->tmode  = tag_memmode_dynamic;
				}
			}
		}
	}
	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
