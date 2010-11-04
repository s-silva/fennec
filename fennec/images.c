#include "fennec main.h"
#include "../libraries/libpng/png.h"

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

typedef struct _iodata_stream
{
	unsigned int  i;
	char         *m;
}iodata_stream;

int png_file_blit(HDC dc, const char *path, int x, int y, unsigned int w, unsigned int h, int sx, int sy)
{
	register unsigned int  i, ix, iy, im;
	
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read =	0;
	FILE         *fp;
	HDC           hmdc;
	HBITMAP       hbmp;
	char         *dbitmap;
	int           rbppx;

	fp = fopen(path, "rb");
	if(!fp)return -1;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)
	{
		fclose(fp);
		return -2;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, 0, 0);
		return -3;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return -4;
	}

	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	if(info_ptr->channels == 3)
	{
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return -5;
	}

	hbmp = CreateBitmap((int)info_ptr->width, (int)info_ptr->height, 1, rbppx, dbitmap);

	free(dbitmap);

	hmdc = CreateCompatibleDC(0);
	SelectObject(hmdc, hbmp);
	DeleteObject(hbmp);

	BitBlt(dc, x, y, info_ptr->width, info_ptr->height, hmdc, sx, sy, SRCCOPY);

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	return 0;
}

int png_file_load_bmp(const char *path, unsigned int *w, unsigned int *h, unsigned long *dbitmap)
{
	register unsigned int  i, ix, iy, im;
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read =	0;
	FILE         *fp;
	int           rbppx;

	fp = fopen(path, "rb");

	if(!fp)return -1;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)
	{
		fclose(fp);
		return -2;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, 0, 0);
		return -3;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return -4;
	}

	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	*w = (unsigned int)info_ptr->width;
	*h = (unsigned int)info_ptr->height;

	if(info_ptr->channels == 3)
	{
		dbitmap = (unsigned long*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (unsigned long*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return -5;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	return 0;
}

int png_free_bmp(unsigned long *dbitmap)
{
	if(!dbitmap)return -1;
	free(dbitmap);
	dbitmap = 0;
	return 0;
}

int png_file_load_dc(const char *path, unsigned int *w, unsigned int *h, HDC *odc)
{
	register unsigned int  i, ix, iy, im;
	
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read = 0;
	FILE         *fp;
	HDC           hmdc;
	HBITMAP       hbmp;
	char         *dbitmap;
	int           rbppx;

	fp = fopen(path, "rb");

	if(!fp)return	-1;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)
	{
		fclose(fp);
		return -2;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, 0, 0);
		return -3;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return -4;
	}

	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	*w = (unsigned int)info_ptr->width;
	*h = (unsigned int)info_ptr->height;

	if(info_ptr->channels == 3)
	{
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return -5;
	}

	hbmp = CreateBitmap((int)info_ptr->width, (int)info_ptr->height, 1, rbppx, dbitmap);

	free(dbitmap);

	hmdc = CreateCompatibleDC(0);
	SelectObject(hmdc, hbmp);
	DeleteObject(hbmp);

	*odc = hmdc;

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	return 0;
}

void res_read(png_structp ps, png_bytep obuffer, png_size_t z)
{
	memcpy(obuffer, (((iodata_stream*)ps->io_ptr)->m) + (((iodata_stream*)ps->io_ptr)->i), z);
	((iodata_stream*)ps->io_ptr)->i += (unsigned int)z;
}

int png_res_blit(HDC dc, HINSTANCE hinst, const char *sdir, const char *sname, int x, int y, unsigned int w, unsigned int h, int sx, int sy)
{
	register unsigned int  i, ix, iy, im;
	
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read =	0;
	HDC           hmdc;
	HBITMAP       hbmp;
	char         *dbitmap;
	char         *rdata;
	int           rbppx;
	HRSRC         hrsrc;
	HGLOBAL       hglob;
	iodata_stream dstream;

	if(!hinst)return -7;

	hrsrc = FindResource((HMODULE)hinst, (LPCTSTR)sname, (LPCTSTR)sdir);
	
	if(!hrsrc)return -6;

	hglob = LoadResource((HMODULE)hinst, hrsrc);
	rdata = LockResource(hglob);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)goto res_clean;

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)goto res_clean;

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	dstream.i = 0;
	dstream.m = rdata;

	png_set_read_fn(png_ptr, &dstream, res_read);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	if(info_ptr->channels == 3)
	{
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	hbmp = CreateBitmap((int)info_ptr->width, (int)info_ptr->height, 1, rbppx, dbitmap);

	free(dbitmap);

	hmdc = CreateCompatibleDC(0);
	SelectObject(hmdc, hbmp);
	DeleteObject(hbmp);

	BitBlt(dc, x, y, info_ptr->width, info_ptr->height, hmdc, sx, sy, SRCCOPY);

	DeleteDC(hmdc);

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	

res_clean:
	UnlockResource(hglob);
	FreeResource(hglob);
	return 0;
}

int png_res_load_bmp(HINSTANCE hinst, const char *sdir, const char *sname, unsigned int *w, unsigned int *h, unsigned long *dbitmap)
{
	register unsigned int  i, ix, iy, im;
	
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read = 0;
	char         *rdata;
	int           rbppx;
	HRSRC         hrsrc;
	HGLOBAL       hglob;
	iodata_stream dstream;

	if(!hinst)return -7;

	hrsrc = FindResource((HMODULE)hinst, (LPCTSTR)sname, (LPCTSTR)sdir);
	
	if(!hrsrc)return -6;

	hglob = LoadResource((HMODULE)hinst, hrsrc);
	rdata = LockResource(hglob);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)goto res_clean;

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)goto res_clean;

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	dstream.i = 0;
	dstream.m = rdata;

	png_set_read_fn(png_ptr, &dstream, res_read);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	*w = (unsigned int)info_ptr->width;
	*h = (unsigned int)info_ptr->height;

	if(info_ptr->channels == 3)
	{
		dbitmap = (unsigned long*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (unsigned long*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	

res_clean:
	UnlockResource(hglob);
	FreeResource(hglob);
	return 0;
}

int png_res_load_dc(HINSTANCE hinst, const string sdir, const string sname, unsigned int *w, unsigned int *h, HDC *dc)
{
	register unsigned int  i, ix, iy, im;
	
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read =	0;
	HDC           hmdc;
	HBITMAP       hbmp;
	char         *dbitmap;
	char         *rdata;
	int           rbppx;
	HRSRC         hrsrc;
	HGLOBAL       hglob;
	iodata_stream dstream;

	if(!hinst)return -7;

	hrsrc = FindResource((HMODULE)hinst, (LPCTSTR)sname, (LPCTSTR)sdir);
	
	if(!hrsrc)return -6;

	hglob = LoadResource((HMODULE)hinst, hrsrc);
	rdata = LockResource(hglob);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)goto res_clean;

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)goto res_clean;

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	dstream.i = 0;
	dstream.m = rdata;

	png_set_read_fn(png_ptr, &dstream, res_read);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	*w = (unsigned int)info_ptr->width;
	*h = (unsigned int)info_ptr->height;

	if(info_ptr->channels == 3)
	{
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	hbmp = CreateBitmap((int)info_ptr->width, (int)info_ptr->height, 1, rbppx, dbitmap);

	free(dbitmap);

	hmdc = CreateCompatibleDC(0);
	SelectObject(hmdc, hbmp);
	DeleteObject(hbmp);

	*dc = hmdc;

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	

res_clean:
	UnlockResource(hglob);
	FreeResource(hglob);
	return 0;
}


HBITMAP png_res_load_winbmp(HINSTANCE hinst, const string sdir, const string sname)
{
	register unsigned int  i, ix, iy, im;
	
	png_structp   png_ptr;
	png_infop     info_ptr;
	unsigned int  sig_read = 0;
	HBITMAP       hbmp = 0;
	char         *dbitmap;
	char         *rdata;
	int           rbppx;
	HRSRC         hrsrc;
	HGLOBAL       hglob;
	iodata_stream dstream;

	if(!hinst)return 0;

	hrsrc = FindResource((HMODULE)hinst, (LPCTSTR)sname, (LPCTSTR)sdir);
	
	if(!hrsrc)return 0;

	hglob = LoadResource((HMODULE)hinst, hrsrc);
	rdata = LockResource(hglob);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if(!png_ptr)goto res_clean;

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)goto res_clean;

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	dstream.i = 0;
	dstream.m = rdata;

	png_set_read_fn(png_ptr, &dstream, res_read);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, 1, 0);

	im = info_ptr->channels;

	if(info_ptr->channels == 3)
	{
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][(ix * im) + 2];
				dbitmap[i + 1] = info_ptr->row_pointers[iy][(ix * im) + 1];
				dbitmap[i + 2] = info_ptr->row_pointers[iy][(ix * im) + 0];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else if(info_ptr->channels == 1){
		
		dbitmap = (char*) malloc(4 * info_ptr->height * info_ptr->width);
	
		i     = 0;
		rbppx = 32;

		for(iy = 0;  iy < info_ptr->height;  iy++)
		{
			for(ix = 0;  ix < info_ptr->width;  ix++)
			{
				dbitmap[i    ] = info_ptr->row_pointers[iy][ix];
				dbitmap[i + 1] = dbitmap[i];
				dbitmap[i + 2] = dbitmap[i];
				dbitmap[i + 3] = 0;

				i += 4;
			}
		}

	}else{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		goto res_clean;
	}

	hbmp = CreateBitmap((int)info_ptr->width, (int)info_ptr->height, 1, rbppx, dbitmap);

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	

res_clean:
	UnlockResource(hglob);
	FreeResource(hglob);
	return hbmp;
}

