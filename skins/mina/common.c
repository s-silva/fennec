/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.3
 Copyright (C) 2011 Chase <c-h@users.sf.net>

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

#include "skin.h"
#include "jpeg/jpegdec.h"

/*
 * simplification of the Rectangle function; to minimize GUI bugs.
 */
void drawrect(HDC dc, int x, int y, int w, int h, COLORREF c)
{
	HPEN pn   = CreatePen(PS_SOLID, 1, c), po;
	HBRUSH br = CreateSolidBrush(c), bo;
	
	po = SelectObject(dc, pn);
	bo = SelectObject(dc, br);

	Rectangle(dc, x, y, x + w, y + h);

	SelectObject(dc, po);
	SelectObject(dc, bo);

	DeleteObject(pn);
	DeleteObject(br);
}

void alpha_blit_resize(HDC ddc, HDC sdc, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int alphap)
{
	BLENDFUNCTION bf;

	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.AlphaFormat         = AC_SRC_ALPHA;
	bf.SourceConstantAlpha = (BYTE)((0xff * alphap) / 100);

	if(bf.SourceConstantAlpha > 0xff) bf.SourceConstantAlpha = 0xff;
	else if(bf.SourceConstantAlpha < 0) bf.SourceConstantAlpha = 0;

	/* draw left, right blocks */

	AlphaBlend(ddc, x, y, w, h, sdc, sx, sy, sw, sh, bf);
	return;
}

void alpha_blit(HDC ddc, HDC sdc, int x, int y, int w, int h, int sx, int sy, int alphap)
{
	BLENDFUNCTION bf;

	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.AlphaFormat         = AC_SRC_ALPHA;
	bf.SourceConstantAlpha = (BYTE)((0xff * alphap) / 100);

	if(bf.SourceConstantAlpha > 0xff) bf.SourceConstantAlpha = 0xff;
	else if(bf.SourceConstantAlpha < 0) bf.SourceConstantAlpha = 0;

	/* draw left, right blocks */

	AlphaBlend(ddc, x, y, w, h, sdc, sx, sy, w, h, bf);
	return;
}

void drawbar(HDC ddc, HDC sdc, int x, int y, int w, int h, int sxl, int syl, int swl, int sxm, int sym, int swm, int sxr, int syr, int swr, int alphap)
{
	BLENDFUNCTION bf;
	int i, c, xp;

	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.AlphaFormat         = AC_SRC_ALPHA;
	bf.SourceConstantAlpha = (BYTE)((0xff * alphap) / 100);

	/* one useless check here */

	if(bf.SourceConstantAlpha > 0xff) bf.SourceConstantAlpha = 0xff;
	else if(bf.SourceConstantAlpha < 0) bf.SourceConstantAlpha = 0;

	if(w <= 0) return;

	/* draw left, right blocks */

	if(w <= swl)
	{
		AlphaBlend(ddc, x, y, w, h, sdc, sxl, syl, w, h, bf);
		return;
	}

	AlphaBlend(ddc, x, y, swl, h, sdc, sxl, syl, swl, h, bf);
	
	if(w >= swl + swr)
		AlphaBlend(ddc, x + w - swr, y, swr, h, sdc, sxr, syr, swr, h, bf);
	else if(w > swl)
		AlphaBlend(ddc, x + swl, y, (w - swl), h, sdc, sxr, syr, (w - swl), h, bf);

	/* draw middle blocks */

	c =  w - swr - swl; /* middle length */
	c /= swm; /* number of blocks */

	xp = x + swl;

	for(i=0; i<c; i++)
	{
		AlphaBlend(ddc, xp, y, swm, h, sdc, sxm, sym, swm, h, bf);
		xp += swm;
	}

	/* finalize by filling any gaps */

	c = (w - swr - (xp - x)); /* gap length */

	if(c > 0)
		AlphaBlend(ddc, xp, y, c, h, sdc, sxm, sym, c, h, bf);

	return;
}



HDC jpeg_get(const string fname, int *w, int *h)
{
	BITMAPINFO   bi;
	HBITMAP      hbm;
	LPVOID       dbits;
	int          width = *w, height = *h;
	void        *buff;
	HDC          dc;

	buff = jpeg_load(fname, &width, &height);

	if(!buff) return 0;
	
	dc = CreateCompatibleDC(0);

	memset(&bi, 0, sizeof(bi));

	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount    = 32;
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biWidth       = width;
	bi.bmiHeader.biHeight      = -height;
	bi.bmiHeader.biCompression = BI_RGB;

	hbm = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &dbits, 0, 0);
	
	SelectObject(dc, hbm);

	CopyMemory(dbits, buff, width * height * 4);	

	*w = width;
	*h = height;

	DeleteObject(hbm);
	return dc;

}


/**----------------------------------------------------------------------------
 eof.
----------------------------------------------------------------------------**/