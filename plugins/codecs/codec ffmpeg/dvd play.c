
#include "plugin.h"

#ifndef DVD_VIDEO_LB_LEN
#	define DVD_VIDEO_LB_LEN 2048
#endif
#define dvd_buffer_size (1024 * DVD_VIDEO_LB_LEN)


int read_data(void *opaque, uint8_t *buf, int buf_size)
{
	struct URLContext *uc;
	int cnt = 0, id = 0, v;

	uc = (struct URLContext*)opaque;
	id = (int)(INT_PTR)uc->filename;

	cnt = buf_size;
	
	if(pstreams[id].dvd_cpt + buf_size >= pstreams[id].dvd_bufsize)
	{
		v =  pstreams[id].dvd_bufsize - pstreams[id].dvd_cpt;

		if(v > 0)
			memcpy(buf, pstreams[id].dvd_buffer + pstreams[id].dvd_cpt, v);

		pstreams[id].dvd_bufsize = dvddec_read(pstreams[id].dvd_info, pstreams[id].dvd_buffer);

		if(v > 0)
			v = buf_size - v;
		else
			v = 0;

		memcpy(buf + v, pstreams[id].dvd_buffer + pstreams[id].dvd_cpt, v);
		pstreams[id].dvd_cpt = v;
	}else{
		memcpy(buf, pstreams[id].dvd_buffer + pstreams[id].dvd_cpt, buf_size);
		pstreams[id].dvd_cpt += buf_size;
	}
	return cnt;
}


int dvdplay_open(unsigned long id, AVFormatContext **icm, int driveid)
{
	int            r;
	AVProbeData    pd;

	pstreams[id].dvd.fmt = 0;
	
	pd.filename = "";
	pd.buf_size = 2048;
	pd.buf      = av_malloc(2048);


	pstreams[id].dvd_buffer = malloc(dvd_buffer_size);
	pstreams[id].dvd_cpt   = 0;

	dvddec_open(0, &pstreams[id].dvd_info);
	pstreams[id].dvd_bufsize = dvddec_read(pstreams[id].dvd_info, pstreams[id].dvd_buffer);

	memcpy(pd.buf, pstreams[id].dvd_buffer, pd.buf_size);
	
	pstreams[id].dvd.fmt = av_probe_input_format(&pd, 1);



    pstreams[id].dvd.io_buffer_size      = 2048;
    pstreams[id].dvd.io_buffer           = av_malloc(pstreams[id].dvd.io_buffer_size);
   
	pstreams[id].dvd.url.priv_data       = 0;
    pstreams[id].dvd.url.prot            = &pstreams[id].dvd.prot;
	pstreams[id].dvd.url.prot->name      = "fennec";
    pstreams[id].dvd.url.prot->url_open  = 0;
    pstreams[id].dvd.url.prot->url_read  = read_data;
    pstreams[id].dvd.url.prot->url_write = 0;
    pstreams[id].dvd.url.prot->url_seek  = 0;
    pstreams[id].dvd.url.prot->url_close = 0;
    pstreams[id].dvd.url.prot->next      = 0;
	pstreams[id].dvd.url.filename        = (char*)(INT_PTR)id;


	init_put_byte( &pstreams[id].dvd.io, pstreams[id].dvd.io_buffer, pstreams[id].dvd.io_buffer_size, 0, &pstreams[id].dvd.url, read_data, 0, 0 );

    pstreams[id].dvd.fmt->flags |= AVFMT_NOFILE; /* libavformat must not fopen/fclose */

	r = av_open_input_stream(&pstreams[id].dvd.ic, &pstreams[id].dvd.io, "", pstreams[id].dvd.fmt, 0);

	*icm = pstreams[id].dvd.ic;
	return 0;
}








