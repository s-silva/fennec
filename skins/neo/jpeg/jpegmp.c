#include "jpegdec.h"

void *jpeg_load(const wchar_t* fname, int *w, int *h)
{
	FILE   *fp;
	DWORD   X_image, Y_image;
	BYTE   *our_image_buffer;
	

	fp = _wfopen(fname, L"rb");

	if(!fp) return 0;

	if(!load_JPEG_header(fp, &X_image, &Y_image))
	{
	 fclose(fp);
	 return 0;
	}

	decode_JPEG_image();

	if((unsigned int)*w < X_image) X_image = (unsigned int) *w;
	if((unsigned int)*h < Y_image) Y_image = (unsigned int) *h;

	if(!get_JPEG_buffer((unsigned short)X_image, (unsigned short)Y_image, &our_image_buffer))
	{
	 fclose(fp);
	 return 0;
	}

	*w = X_image;
	*h = Y_image;

	fclose(fp);

	return our_image_buffer;
}


