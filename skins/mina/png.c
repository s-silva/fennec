#include <png.h>
#include "skin.h"
#include "skin settings.h"

#define wrap_color(x)      ( (x) < 0 ? 0 : ( (x) > 255 ? 255 : (x) ) )
int set_color_32(unsigned char *buf);

int skin_color_hue   = 0;
int skin_color_sat   = 10;
int skin_color_light = 50;


int png_h = 0, png_w = 0;



HDC png_get_hdc(string fname, int color_apply)
{
	FILE        *pFile = 0;	
	BYTE       **pRowPointers = 0;
	BYTE         header[8]; 
	png_structp  pPng;
	png_infop    pPngInfo;
	int          bitDepth, colorType;
	ULONG        width, height;
	int          byteDepth;
	int          rowbytes;
	BYTE        *pdata;
	int          i, buffsize;
	BITMAPINFO   bi;
	HDC          dc;
	HBITMAP      hbm;
	LPVOID       dbits;

	png_w = png_h = 0;

	pFile = _wfopen(fname, uni("rb"));
	if(!pFile) return 0;

	

	//confirm png header
	fread(header, 1, 8, pFile);
	if (!png_check_sig(header, 8))
	{
		fclose(pFile);
		return 0;
	}

	//create pPng and info_png
	pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!pPng)
	{
		fclose(pFile);
		return 0;
	}

	pPngInfo = png_create_info_struct(pPng);
	if (!pPngInfo)
	{
		fclose(pFile);
		png_destroy_read_struct(&pPng, 0, 0); 
		return 0;
	}

	// it's a goto (in case png lib hits the bucket)
	if(setjmp(png_jmpbuf(pPng))) 
	{  
		fclose(pFile);
		png_destroy_read_struct(&pPng, &pPngInfo, 0);
		return 0;
	}

	png_init_io(pPng, pFile);
	png_set_sig_bytes(pPng, 8);  // we already read the 8 signature bytes 
	png_read_info(pPng, pPngInfo);  // read all PNG info up to image data 

	
	png_get_IHDR(pPng, pPngInfo, &width, &height, &bitDepth, &colorType, 0, 0, 0);

	// apply filters to image so we can get proper image data format
	if (colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(pPng);
	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		png_set_gray_1_2_4_to_8(pPng);
	if (png_get_valid(pPng, pPngInfo, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(pPng);

	
	if(bitDepth == 16)
	{
		png_set_strip_16(pPng); //make 8-bit
	}

	bitDepth = 8;
	byteDepth = bitDepth/8;

	
	switch(colorType)
	{
	case  PNG_COLOR_TYPE_GRAY:
		//m_colorType = COLOR_GRAY;
		//m_bytesPerPixel = byteDepth;
		break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
		//m_colorType = COLOR_RGBA;
		//m_bytesPerPixel = byteDepth*4;
		break;

	case PNG_COLOR_TYPE_RGB:
	default:
		//m_colorType = COLOR_RGB; 
		//m_bytesPerPixel = byteDepth*3;
		break;
	}

	png_set_bgr(pPng);


	png_read_update_info(pPng, pPngInfo);

	rowbytes = png_get_rowbytes(pPng, pPngInfo);
	buffsize = rowbytes * height;

	pdata = (BYTE*) sys_mem_alloc(rowbytes * height);
	pRowPointers = (BYTE**) sys_mem_alloc(sizeof(BYTE*) * height);

	for (i = 0;  i < (int)height;  ++i)
		pRowPointers[height - i - 1] = pdata + i*rowbytes;

	png_read_image(pPng, pRowPointers);

	png_read_end(pPng, 0);

	png_destroy_read_struct(&pPng, &pPngInfo, 0);

	fclose(pFile);


	switch(color_apply)
	{
	case 1:
		skin_color_hue   = settings_data.theme.base.h;
		skin_color_sat   = settings_data.theme.base.s;
		skin_color_light = settings_data.theme.base.l;
		break;
	case 2:
		skin_color_hue   = settings_data.theme.control.h;
		skin_color_sat   = settings_data.theme.control.s;
		skin_color_light = settings_data.theme.control.l;
		break;
	case 3:
		skin_color_hue   = settings_data.theme.text.h;
		skin_color_sat   = settings_data.theme.text.s;
		skin_color_light = settings_data.theme.text.l;
		break;
	}

	

	for(i=0; i<buffsize; i+=4)
	{
		/* set alpha levels */

		float av = (float)pdata[i + 3]  / 255.0f;
		pdata[i + 1] = (BYTE)((float)pdata[i + 1] * av);
		pdata[i + 2] = (BYTE)((float)pdata[i + 2] * av);
		pdata[i + 0] = (BYTE)((float)pdata[i + 0] * av);

		/* adjust theme colors */

		if(color_apply)
			set_color_32(pdata + i);
	}



	/* dib */

	dc = CreateCompatibleDC(0);

	memset(&bi, 0, sizeof(bi));

	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount    = 32;
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biWidth       = width;
	bi.bmiHeader.biHeight      = height;
	bi.bmiHeader.biCompression = BI_RGB;

	hbm = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &dbits, 0, 0);
	
	SelectObject(dc, hbm);

	// Copy data into DIB
	CopyMemory(dbits, pdata, buffsize);	

	
	png_w = width;
	png_h = height;

	DeleteObject(hbm);
	sys_mem_free(pdata);
	sys_mem_free(pRowPointers);

	return dc;

}









void PNGAPI user_error_fn(png_structp png, png_const_charp sz) { }
void PNGAPI user_warning_fn(png_structp png, png_const_charp sz) { }
 
HBITMAP LoadBitmapPNG(string szFile)
{
	HBITMAP hbm = NULL;
	int width;
	int height;
	int bpp;
	int memWidth;
	BOOL retVal = 0;
	int size = 0;
	BYTE header[8];
	png_infop info_ptr;
	png_structp png_ptr;
	png_infop end_info;
	int a_i, i;
	FILE *fp;
	
	error_in_last_image = 0;
	
	fp = _wfopen(szFile, uni("rb"));
	if (!fp)
		return 0;
	
	fread(header, 1, 8, fp);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fclose(fp);

	error_in_last_image = 1;

//if (png_sig_cmp(header, 0, 8))
	if (!png_check_sig(header, 8))
		return 0;
	//png_check_sig

	// now allocate stuff

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, user_error_fn, user_warning_fn);
	if (!png_ptr)
		return 0;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return 0;
	}
	 
	end_info = png_create_info_struct(png_ptr);
	if(!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return 0;
	}
	 
	fp = _wfopen(szFile, uni("rb"));
	if(fp)
	{
		LPBITMAPINFO lpbi;
		png_bytep* row_pointers;
		BYTE * pixelData;

		png_init_io(png_ptr, fp);
	 
		// should really use png_set_rows() to allocate space first, rather than doubling up
	 
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_BGR, NULL);
	 
		fclose(fp);
	 
		row_pointers = png_get_rows(png_ptr, info_ptr);
	 
		// now for a tonne of ugly DIB setup crap
	 
		width = info_ptr->width;
		height = info_ptr->height;
		bpp = info_ptr->channels * 8;
		memWidth = (width * (bpp >> 3) + 3) & ~3;
		 
		lpbi = (LPBITMAPINFO) sys_mem_alloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	 
		// create a greyscale palette
		for (a_i = 0; a_i < 256; a_i++)
		{
			lpbi->bmiColors[a_i].rgbRed = (BYTE)a_i;
			lpbi->bmiColors[a_i].rgbGreen = (BYTE)a_i;
			lpbi->bmiColors[a_i].rgbBlue = (BYTE)a_i;
			lpbi->bmiColors[a_i].rgbReserved = 0;
		}
		 
		lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		lpbi->bmiHeader.biWidth = width;
		lpbi->bmiHeader.biHeight = -height; // must be negative for top down
		lpbi->bmiHeader.biPlanes = 1;
		lpbi->bmiHeader.biBitCount = bpp;
		lpbi->bmiHeader.biCompression = BI_RGB;
		lpbi->bmiHeader.biSizeImage = memWidth * height;
		lpbi->bmiHeader.biXPelsPerMeter = 0;
		lpbi->bmiHeader.biYPelsPerMeter = 0;
		lpbi->bmiHeader.biClrUsed = 0;
		lpbi->bmiHeader.biClrImportant = 0;

		png_w = width;
		png_h = height;
	 
	
		hbm = CreateDIBSection(NULL, lpbi, DIB_RGB_COLORS, (void **)&pixelData, NULL, 0 );
		if(hbm && pixelData)
		{

			for (i = 0; i < height; i++)
				memcpy(pixelData + memWidth * i, row_pointers[i], width * info_ptr->channels);
		}
	 
		sys_mem_free(lpbi);
	}
	 
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	error_in_last_image = 0;
	 
	return hbm;
}

HDC png_get_hdc_24(string fname)
{
	HDC          dc;
	HBITMAP      hbm;

	
	hbm = LoadBitmapPNG(fname);
	if(!hbm) return 0;

	
	dc = CreateCompatibleDC(0);

	SelectObject(dc, hbm);
	DeleteObject(hbm);

	return dc;
}


/*
 * hsv to rgb conversion.
 */
void color_hsv_2_rgb_fullint(int h, int s, int v, unsigned char* r, unsigned char* g, unsigned char* b)
{
	unsigned char _a, _b, _c, pc;
	unsigned int osc;

	/* optimized edition (only fixed-point maths used). */

	pc  = (unsigned char)((int)h / 60);
	osc = (((h * 100) / 6)) - ((h / 60) * 1000);

	_a = (unsigned char)(((255 - s) * v) / 255);
	_b = (unsigned char)(((255 - ((s * osc) / 1000)) * v) / 255);
	_c = (unsigned char)(((255 - ((s * (1000 - osc)) / 1000)) * v) / 255);

	switch(pc)
	{
	case 0: *r = (unsigned char)  v; *g = (unsigned char)_c; *b = (unsigned char)_a; break;
	case 1: *r = (unsigned char) _b; *g = (unsigned char) v; *b = (unsigned char)_a; break;
	case 2: *r = (unsigned char) _a; *g = (unsigned char) v; *b = (unsigned char)_c; break;
	case 3: *r = (unsigned char) _a; *g = (unsigned char)_b; *b = (unsigned char) v; break;
	case 4: *r = (unsigned char) _c; *g = (unsigned char)_a; *b = (unsigned char) v; break;
	case 5: *r = (unsigned char)  v; *g = (unsigned char)_a; *b = (unsigned char)_b; break;
	}
}


int set_color_32(unsigned char *buf)
{
	unsigned char    ar, ag, ab, rr, rg, rb;
	unsigned short   rh;
	unsigned char    rs, rl;

	rr = buf[0];
	rg = buf[1];
	rb = buf[2];

	rh = (unsigned short)(skin_color_hue * 3.5);
	rs = (unsigned char) (skin_color_sat * 2.5);
	rl = (unsigned char) (wrap_color( (((skin_color_light + 50) * 2) * ((rr + rg + rb) / 3)) / 255 ));

	color_hsv_2_rgb_fullint(rh, rs, rl, &ab, &ag, &ar);

	ar = (unsigned char)wrap_color( ((int)ar * 15) / 10 /* x1.5 */ );
	ag = (unsigned char)wrap_color( ((int)ag * 15) / 10 /* x1.5 */ );
	ab = (unsigned char)wrap_color( ((int)ab * 15) / 10 /* x1.5 */ );

	buf[0] = ar;
	buf[1] = ag;
	buf[2] = ab;

	return 1;
}

