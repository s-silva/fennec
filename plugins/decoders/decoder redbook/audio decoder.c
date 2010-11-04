#include "main.h"
#include "audio cd.h"


int decoder_load(unsigned long id, const string sname)
{	
	letter         buf[16];
	unsigned int   a, j, tn;

	pstreams[id].driveid = sname[0];

	pstreams[id].bitrate       = 0;
	pstreams[id].frequency     = 44100;
	pstreams[id].channels      = 2;
	pstreams[id].bitspersample = 16;
	pstreams[id].trackpos      = 0;

	pstreams[id].bufsector     = (char*)malloc(480400);
	pstreams[id].bufcache      = (char*)malloc(480400);
	pstreams[id].bnum          = 0;

	a = (unsigned int)str_len(sname);

	while(sname[a] != '.')a--;
	j = a - 1;
	while(isdigit(sname[j]))j--;

	str_ncpy(buf, sname + (j + 1), a - j - 1);
	buf[a - j - 1] = 0;

	tn = str_stoi(buf);

	pstreams[id].tracknumber = tn - 1;

#define  bpms ((pstreams[id].frequency * (pstreams[id].bitspersample / 8) * pstreams[id].channels) / 1000)
	
	pstreams[id].cachept = 0;
	pstreams[id].cachesize = 0;

	audiocd_initialize();

	audiocd_load_track((char)sname[0], tn - 1);

	pstreams[id].tracksize = audiocd_gettracksize((char)sname[0], tn - 1);
	pstreams[id].duration  = pstreams[id].tracksize / bpms;
    return 1;
}

int decoder_close(unsigned long id)
{
	audiocd_unload(pstreams[id].driveid, pstreams[id].tracknumber);
	
	if(pstreams[id].bufsector)
	{
		free(pstreams[id].bufsector);
		pstreams[id].bufsector = 0;
	}

	if(pstreams[id].bufcache)
	{
		free(pstreams[id].bufcache);
		pstreams[id].bufcache = 0;
	}
	return 1;
}

int decoder_seek(unsigned long id, double pos)
{
	pstreams[id].bnum = (unsigned int)(double)(pstreams[id].tracksize * pos) / audiocd_getblocksize();
	pstreams[id].trackpos = (unsigned long)(((double)pstreams[id].tracksize) * pos);
	return 1;
}

unsigned long getsector(unsigned long id, char* buffer, unsigned long dreq)
{
	long           bsize;
	unsigned long  nbr = audiocd_getblocksize();

	bsize = audiocd_read((char)pstreams[id].driveid, pstreams[id].tracknumber, pstreams[id].bnum, &nbr, buffer);

	pstreams[id].bnum += 1;

	if(pstreams[id].bnum * audiocd_getblocksize() > pstreams[id].tracksize)
		return 0;

	if(bsize > 0)
		return nbr;
	else 
		return 0;
}

unsigned long decoder_read(unsigned long id, char* adata, unsigned long isize)
{
	unsigned long i = 0, z;

	if(pstreams[id].cachesize && pstreams[id].cachept)
	{
		z = min(isize, pstreams[id].cachesize - pstreams[id].cachept);

		memcpy(adata, pstreams[id].bufcache + pstreams[id].cachept, z);

		pstreams[id].cachept += z;

		i += z;

		if(pstreams[id].cachept >= pstreams[id].cachesize)
			pstreams[id].cachept = 0;

		if(z >= isize) return isize;
	}

	for(;;)
	{
		pstreams[id].cachept = 0;

		pstreams[id].cachesize = getsector(id, pstreams[id].bufcache, 0);
		z = min(isize - i, pstreams[id].cachesize);

		memcpy(adata + i, pstreams[id].bufcache, z);

		pstreams[id].cachept += z;

		if(i + z >= isize)break;
		else if(!pstreams[id].cachesize) return i + z;

		i += z;
	}

	return isize;
}

int tagread(const string fname, struct fennec_audiotag* rtag)
{
	letter       tempbuffer[16];
	unsigned int tracknum;
	int          a, j;

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
		if(rtag->tag_filepath.tsize      ) { sys_mem_free(rtag->tag_filepath.tdata      ); rtag->tag_filepath.tsize      = 0; }
		if(rtag->tag_filename.tsize      ) { sys_mem_free(rtag->tag_filename.tdata      ); rtag->tag_filename.tsize      = 0; }
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

	memset(tempbuffer, 0, sizeof(tempbuffer));

	a = (int)str_len(fname);

	while(fname[a] != uni('.'))a--;
	j = a - 1;
	while(isdigit(fname[j]))j--;

	str_ncpy(tempbuffer, fname + (j + 1), a - j - 1);
	tempbuffer[a - j - 1] = 0;

	tracknum = str_stoi(tempbuffer);

	rtag->tag_title.tdata = (string) sys_mem_alloc(32);
	str_cpy(rtag->tag_title.tdata, uni("Track "));
	str_cat(rtag->tag_title.tdata, tempbuffer);
	rtag->tag_title.tsize = (unsigned int)str_size(rtag->tag_title.tdata);
	rtag->tag_title.tmode = tag_memmode_dynamic;

	rtag->tag_tracknum.tdata  = (string) sys_mem_alloc(16);
	str_cpy(rtag->tag_tracknum.tdata, tempbuffer);
	rtag->tag_tracknum.tsize  = (unsigned int)str_size(rtag->tag_tracknum.tdata);
	rtag->tag_tracknum.tmode  = tag_memmode_dynamic;
	rtag->tag_tracknum.tdatai = tracknum;

	rtag->tag_album.tdata  = (string) sys_mem_alloc(32);
	str_cpy(rtag->tag_album.tdata, uni("CD Audio Disc"));
	rtag->tag_album.tsize  = (unsigned int)str_size(rtag->tag_album.tdata);
	rtag->tag_album.tmode  = tag_memmode_dynamic;
	return 1;
}

int tagwrite(const string fname, struct fennec_audiotag* rtag)
{
	return 0;
}
