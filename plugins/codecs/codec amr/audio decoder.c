/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (Sound Files).
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

#include "plugin.h"
#include "amr/interf_dec.h"
#include "amr/sp_dec.h"

#ifndef ETSI
#	ifndef IF2
#		include <string.h>
#		define AMR_MAGIC_NUMBER "#!AMR\n"
#	endif
#endif

unsigned long local_get_samples_count(FILE *sfile);

int decoder_load(unsigned long id, const string sname)
{
	char magic[8];


	pstreams[id].hstream = _wfopen(sname, uni("rb"));
	
	if(!pstreams[id].hstream) return 0;

#	ifndef ETSI
#	ifndef IF2

	fread(magic, sizeof(char), strlen(AMR_MAGIC_NUMBER), pstreams[id].hstream);

	if(strncmp(magic, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER)))
	{
		/* error: invalid amr file */
		fclose(pstreams[id].hstream);
		return 0;
	}

#	endif
#	endif

	pstreams[id].fsize = local_get_samples_count(pstreams[id].hstream);

	pstreams[id].dstate = Decoder_Interface_init();

	pstreams[id].written = 0;
	pstreams[id].eos     = 0;
	pstreams[id].olen    = 0;

	pstreams[id].bitspersample = 16;
	pstreams[id].channels      = 1;
	pstreams[id].frequency     = 8000;
	pstreams[id].duration      = pstreams[id].fsize / 8;
	pstreams[id].bitrate       = 0;
	return 1;
}

int decoder_close(unsigned long id)
{
	if(!pstreams[id].hstream)return 0;

	Decoder_Interface_exit(pstreams[id].dstate);
	fclose(pstreams[id].hstream);
	pstreams[id].hstream = 0;
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	pstreams[id].written = 0;
	return 0;
}


unsigned long local_get_samples_count(FILE *sfile)
{
	int read_size;
#ifndef ETSI
   unsigned char analysis[32];
   enum Mode     dec_mode;

#	ifdef IF2
		short block_size[16] = { 12, 13, 15, 17, 18, 20, 25, 30, 5, 0, 0, 0, 0, 0, 0, 0 };
#	else
		short block_size[16] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
#	endif

#else
   short analysis[250];
#endif
   	int i = 0;

	fseek(sfile, (long)strlen(AMR_MAGIC_NUMBER), SEEK_SET);


	for(;;)
	{
#ifndef ETSI

		if(fread(analysis, sizeof(unsigned char), 1, sfile) == 0)
		{
			fseek(sfile, (long)strlen(AMR_MAGIC_NUMBER), SEEK_SET);
			return i;	
		}

#	ifdef IF2
		dec_mode = analysis[0] & 0x000F;
#	else
		dec_mode = (analysis[0] >> 3) & 0x000F;
#	endif

		read_size = block_size[dec_mode];

		fseek(sfile, read_size, SEEK_CUR);

#else

		read_size = 250;

		fseek(sfile, read_size, SEEK_CUR);

		if(fread(analysis, sizeof(unsigned char), 1, sfile) == 0)
		{
			fseek(sfile, (long)strlen(AMR_MAGIC_NUMBER), SEEK_SET);
			return i;	
		}

#endif

		i += 160;

	}
	return i;
}

int dec_amr_getblock(unsigned long id, void* oblock)
{
	int read_size;

#ifndef ETSI
   unsigned char analysis[32];
   enum Mode     dec_mode;

#	ifdef IF2
		short block_size[16] = { 12, 13, 15, 17, 18, 20, 25, 30, 5, 0, 0, 0, 0, 0, 0, 0 };
#	else
		short block_size[16] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
#	endif

#else
   short analysis[250];
#endif

#ifndef ETSI

	if(fread(analysis, sizeof(unsigned char), 1, pstreams[id].hstream) == 0)
	{
		pstreams[id].eos = 1;
		return 0;
	}

#	ifdef IF2
		dec_mode = analysis[0] & 0x000F;
#	else
		dec_mode = (analysis[0] >> 3) & 0x000F;
#	endif


	read_size = block_size[dec_mode];

	if(fread(&analysis[1], sizeof (char), read_size, pstreams[id].hstream) == 0)
	{
		pstreams[id].eos = 1;
		return 0;
	}

#else

	read_size = 250;

	/* read file */

	if(fread(analysis, sizeof (short), read_size, pstreams[id].hstream) == 0)
	{
		pstreams[id].eos = 1;
		return 0;
	}
		

#endif

	/* call decoder */
	Decoder_Interface_Decode(pstreams[id].dstate, analysis, oblock, 0);

	return sizeof(pstreams[id].synth);
}

unsigned long decoder_read(unsigned long id, char* adata, unsigned long dsize)
{
	int i = 0, z, r;

	if(pstreams[id].eos)
		return 0;
 
	if(pstreams[id].written)
	{
		z = sizeof(pstreams[id].synth) - pstreams[id].written;
		r = min(z, dsize);

		memcpy(adata, ((char*)pstreams[id].synth) + pstreams[id].written, r);

		i += r;

		if(z > dsize)
		{	
			pstreams[id].written += r;
			return dsize;
		}else{
			pstreams[id].written = 0;
		}
	}


	for(;;)
	{
		z = dec_amr_getblock(id, pstreams[id].synth);

		if(!z)
			return i;

		r = dsize - i; /* remaining length */

		memcpy(adata + i, pstreams[id].synth, min(z, r));

		if(z >= r)
		{
			pstreams[id].written = min(z, r);

			if(pstreams[id].written >= z)
				pstreams[id].written = 0;

			break;
		}

		i += min(z, r);
	}

    return dsize;
}



int tagread(const string fname,  struct fennec_audiotag *rtag)
{
	if(!rtag)return 0;

	if(!fname)
	{
		if(rtag->tag_title.tsize         ) { sys_mem_free(rtag->tag_title.tdata         ); rtag->tag_title.tsize         = 0; }
		if(rtag->tag_album.tsize         ) { sys_mem_free(rtag->tag_album.tdata         ); rtag->tag_album.tsize         = 0; }
		if(rtag->tag_artist.tsize        ) { sys_mem_free(rtag->tag_artist.tdata        ); rtag->tag_artist.tsize        = 0; }
		if(rtag->tag_origartist.tsize    ) { sys_mem_free(rtag->tag_origartist.tdata    ); rtag->tag_origartist.tsize    = 0; }
		if(rtag->tag_composer.tsize      ) { sys_mem_free(rtag->tag_composer.tdata      ); rtag->tag_composer.tsize      = 0; }
		if(rtag->tag_lyricist.tsize      ) { sys_mem_free(rtag->tag_lyricist.tdata      ); rtag->tag_lyricist.tsize      = 0; }
		if(rtag->tag_band.tsize          ) { sys_mem_free(rtag->tag_band.tdata          ); rtag->tag_band.tsize          = 0; }
		if(rtag->tag_copyright.tsize     ) { sys_mem_free(rtag->tag_copyright.tdata     ); rtag->tag_copyright.tsize     = 0; }
		if(rtag->tag_publish.tsize       ) { sys_mem_free(rtag->tag_publish.tdata       ); rtag->tag_publish.tsize       = 0; }
		if(rtag->tag_encodedby.tsize     ) { sys_mem_free(rtag->tag_encodedby.tdata     ); rtag->tag_encodedby.tsize     = 0; }
		if(rtag->tag_genre.tsize         ) { sys_mem_free(rtag->tag_genre.tdata         ); rtag->tag_genre.tsize         = 0; }
		if(rtag->tag_year.tsize          ) { sys_mem_free(rtag->tag_year.tdata          ); rtag->tag_year.tsize          = 0; }
		if(rtag->tag_url.tsize           ) { sys_mem_free(rtag->tag_url.tdata           ); rtag->tag_url.tsize           = 0; }
		if(rtag->tag_offiartisturl.tsize ) { sys_mem_free(rtag->tag_offiartisturl.tdata ); rtag->tag_offiartisturl.tsize = 0; }
	  //if(rtag->tag_filepath.tsize      ) { sys_mem_free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
	  //if(rtag->tag_filename.tsize      ) { sys_mem_free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; }
		if(rtag->tag_comments.tsize      ) { sys_mem_free(rtag->tag_comments.tdata      ); rtag->tag_comments.tsize      = 0; }
		if(rtag->tag_lyric.tsize         ) { sys_mem_free(rtag->tag_lyric.tdata         ); rtag->tag_lyric.tsize         = 0; }
		if(rtag->tag_bpm.tsize           ) { sys_mem_free(rtag->tag_bpm.tdata           ); rtag->tag_bpm.tsize           = 0; }
		if(rtag->tag_tracknum.tsize      ) { sys_mem_free(rtag->tag_tracknum.tdata      ); rtag->tag_tracknum.tsize      = 0; }
		
		return 1;
	}

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

	{
		unsigned int c = (unsigned int)str_len(fname);
		unsigned int i = c;
		unsigned int p = 0;

		while(i)
		{
			if(fname[i] == uni('\\') || fname[i] == uni('/'))break;
			if(!p && (fname[i] == uni('.')))p = i;
			i--;
		}

		if(i)i++;

		if(c - i > 0)
		{
			rtag->tag_title.tsize = (c - i) * sizeof(letter);
			rtag->tag_title.tdata = sys_mem_alloc(rtag->tag_title.tsize + sizeof(letter));
			str_cpy(rtag->tag_title.tdata, fname + i);
			rtag->tag_title.tmode = tag_memmode_dynamic;
			if(p && (p - i) > 0)rtag->tag_title.tdata[p - i] = 0;
		}
	}
	return 1;
}

/*-----------------------------------------------------------------------------
 2007
-----------------------------------------------------------------------------*/
