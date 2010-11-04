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

#include "Fennec Main.h"

// little discription
// General Skin ain't a skin file loader or such thing,
// it does some works on Skin-loaded features.

#define  Skin_MaskColor 0xFF00FF // RGB(255,0,255)
#define  Skin_Region_Memory 4096 // memory size

HRGN Skin_GetBitmapRegion(HBITMAP hbmp)
{
	// mask color = rgb(255,0,255)
	HRGN     hRgn = NULL;
    unsigned long*   pBitmapBits = NULL;
    unsigned long*   pBitmapCursor;
    BITMAP   bitmap;
    RGNDATA* pRGNData = NULL;
    int      iLastRectIDX;
    int      iRGNDataSize_Rects;
    int      iRowIDX, iColIDX;
    unsigned long    dwTransMasked;
    int     bDetectedTransparentPixel;

	COLORREF cTransparentColor = Skin_MaskColor;

    // Get the size of the source
    GetObject(hbmp, sizeof(bitmap), &bitmap);
    pBitmapBits = (unsigned long*)sys_mem_alloc(sizeof(unsigned long) * bitmap.bmWidth * bitmap.bmHeight);

    // Extract the bits of the bitmap
    {
        BITMAPINFO bmi;
        HDC dc;

        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biWidth = bitmap.bmWidth;
        bmi.bmiHeader.biHeight = -bitmap.bmHeight;

        dc = CreateCompatibleDC(NULL);
        GetDIBits(dc, hbmp, 0, bitmap.bmHeight, pBitmapBits, &bmi, DIB_RGB_COLORS);
        DeleteDC(dc);
    }

    // Step through bitmap row by row - building rects for the rows that arn't the
    // transparent colour
    dwTransMasked = cTransparentColor & 0x00FFFFFF;
    pBitmapCursor = pBitmapBits;
    iLastRectIDX = 0;
    iRGNDataSize_Rects = 0;
    bDetectedTransparentPixel = FALSE;
    for(iRowIDX =0; iRowIDX < bitmap.bmHeight; iRowIDX++)
    {
        int bInStrip = FALSE;
        for(iColIDX =0; iColIDX < bitmap.bmWidth; iColIDX++, pBitmapCursor++)
        {
            // Is the current pixel transparent?
            if( (((*pBitmapCursor)&0x00FFFFFF)^dwTransMasked) == 0L)
            {
                bDetectedTransparentPixel = TRUE;
                // If we are in a strip - close it
                if(bInStrip == TRUE)
                {
                    bInStrip = FALSE;
                    ((RECT*)pRGNData->Buffer)[iLastRectIDX].right = iColIDX;
                    iLastRectIDX++;
                }
            }
            else
            {
                // Open a new strip if we need to
                if(bInStrip == FALSE)
                {
                    bInStrip = TRUE;

                    // Ensure that we have enough memory allocated
                    if(iLastRectIDX == iRGNDataSize_Rects)
                    {
                        iRGNDataSize_Rects += Skin_Region_Memory;
                        pRGNData = (RGNDATA*)sys_mem_realloc(pRGNData, sizeof(RGNDATAHEADER) + (iRGNDataSize_Rects * sizeof(RECT)));
                    }
                    ((RECT*)pRGNData->Buffer)[iLastRectIDX].left = iColIDX;
                    ((RECT*)pRGNData->Buffer)[iLastRectIDX].top = iRowIDX;
                    ((RECT*)pRGNData->Buffer)[iLastRectIDX].bottom = iRowIDX+1;
                }
            }
        }

        // Close any open rects
        if(bInStrip == TRUE)
        {
            ((RECT*)pRGNData->Buffer)[iLastRectIDX].right = bitmap.bmWidth;
            iLastRectIDX++;
        }
    }
    sys_mem_free(pBitmapBits);

    // If there are some rects in this region - create the GDI object
    if(bDetectedTransparentPixel == TRUE)
    {
        pRGNData->rdh.dwSize = sizeof(RGNDATAHEADER);
        pRGNData->rdh.iType = RDH_RECTANGLES;
        pRGNData->rdh.nCount = iLastRectIDX;
        pRGNData->rdh.nRgnSize = sizeof(RGNDATAHEADER) + (iLastRectIDX * sizeof(RECT));
        pRGNData->rdh.rcBound.left = 0;
        pRGNData->rdh.rcBound.top = 0;
        pRGNData->rdh.rcBound.right = bitmap.bmWidth;
        pRGNData->rdh.rcBound.bottom = bitmap.bmHeight;
        hRgn = ExtCreateRegion(NULL, pRGNData->rdh.nRgnSize, pRGNData);
    }

    // Cleanup
    if(pRGNData)
    sys_mem_free(pRGNData);
    return hRgn;
}

COLORREF GetAdjustedColor(int scol, int h, int s, int l)
{
	#define wrapc(x)(x < 0 ? 0 : (x > 255 ? 255 : x))

	unsigned char ar, ag, ab;
	color_hsv_2_rgb_fullint((unsigned short)(h*3.5), (unsigned char)(s*2.5), (unsigned char)(wrapc( (((l + 50) * 2) * scol) / 255 )), &ar, &ag, &ab);
	
	ar = (unsigned char)wrapc(ar * 1.5);
	ag = (unsigned char)wrapc(ag * 1.5);
	ab = (unsigned char)wrapc(ab * 1.5);

	return RGB(ar, ag, ab);
}

int AdjustColors(HBITMAP hbmp, HBITMAP hmask, int h, int s, int l)
{
    HDC        dc;
	unsigned long*     bmem;
	BITMAPINFO bmi;
	BITMAP     bmp;
	int        x,y;//,r,g,b;
	unsigned char ar, ag, ab;
	COLORREF   c;

	#define MapCoords(x, y, w)((y * w) + x)
  	#define wrapc(x)(x < 0 ? 0 : (x > 255 ? 255 : x))

    GetObject(hbmp, sizeof(BITMAP), &bmp);
    bmem = (unsigned long*)sys_mem_alloc(sizeof(unsigned long) * bmp.bmWidth * bmp.bmHeight);

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth       = bmp.bmWidth;
    bmi.bmiHeader.biHeight      = -bmp.bmHeight;

    dc = CreateCompatibleDC(0);
    GetDIBits(dc, hbmp, 0, bmp.bmHeight, bmem, &bmi, DIB_RGB_COLORS);

	for(y=0; y<bmp.bmHeight; y++)
	{
		for(x=0; x<bmp.bmWidth; x++)
		{
			c = bmem[MapCoords(x, y, bmp.bmWidth)];
			//r = wrapc(GetRValue(c) + ar - 50);
			//g = wrapc(GetGValue(c) + ag - 50);
			//b = wrapc(GetBValue(c) + ab - 50);

			color_hsv_2_rgb_fullint((unsigned short)(h*3.5), (unsigned char)(s*2.5), (unsigned char)(wrapc( (((l + 50) * 2) * GetRValue(c)) / 255 )), &ab, &ag, &ar);

			ar = (unsigned char)wrapc(ar * 1.5);
			ag = (unsigned char)wrapc(ag * 1.5);
			ab = (unsigned char)wrapc(ab * 1.5);

			c = RGB(ar, ag, ab);
			bmem[MapCoords(x, y, bmp.bmWidth)] = c;
		}
	}

	SetDIBits(dc, hbmp, 0, bmp.bmHeight, bmem, &bmi, DIB_RGB_COLORS);

	DeleteDC(dc);
	sys_mem_free(bmem);
	return 1;
}

unsigned long MoveWindow_Docking(HWND hwnd, DockWindowPoints* dpts, unsigned long dptcount, int dextent, long* x, long* y, int dx, int dy)
{
	RECT rclt;
	RECT rwork;
	int  rx, ry;
	unsigned long i, f;
	HDWP hpos;

	GetClientRect(hwnd, &rclt);

	SystemParametersInfo(SPI_GETWORKAREA,0,&rwork,0);

	rx = *x - dx;
	ry = *y - dy;

	if(rx <= dextent && rx >= -dextent) rx = 0;
	if(ry <= dextent && ry >= -dextent) ry = 0;
	if(rx >= rwork.right  - dextent - rclt.right && rx <= rwork.right + dextent - rclt.right) rx = rwork.right - rclt.right;
	if(ry >= rwork.bottom - dextent - rclt.bottom && ry <= rwork.bottom + dextent - rclt.bottom) ry = rwork.bottom - rclt.bottom;

	/* finding points

	parent window point - p
	docking points - *
	dextent = 4

	p***
	****
	****
    ****

	*/

	f = 0xFFFF;

	for(i=0; i<dptcount; i++)
	{
		if(dpts[i].cx + rx <= dpts[i].px + dextent && dpts[i].cx + rx >= dpts[i].px - dextent)
		{
			if(dpts[i].cy + ry <= dpts[i].py + dextent && dpts[i].cy + ry >= dpts[i].py - dextent)
			{
				rx = dpts[i].px - dpts[i].cx;
				ry = dpts[i].py - dpts[i].cy;
				f = i;
				break;	
			}
		}
	}

	hpos = BeginDeferWindowPos(1);
	DeferWindowPos(hpos, hwnd, 0, rx, ry, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	EndDeferWindowPos(hpos);

	*x = rx;
	*y = ry;

	return f;
}

/* ----------------------------------------------------------------------------
 common mask functions.
---------------------------------------------------------------------------- */

int Mask_LoadMaskFile(const string mfile, Mask* omask)
{
	HANDLE mfid;
	unsigned long  nbr = 0;
	unsigned long w,h;

	mfid = CreateFile(mfile, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
	if(mfid == INVALID_HANDLE_VALUE)return 0;

	ReadFile(mfid, &w, sizeof(unsigned long), &nbr, 0);
	ReadFile(mfid, &h, sizeof(unsigned long), &nbr, 0);
	
	if((w * h > 0) && (w * h < 0x2000000 /* 32mb */))
	{
		omask->points = (unsigned char*) sys_mem_alloc(500 * 500 * sizeof(unsigned char));
		omask->w = w;
		omask->h = h;

		ReadFile(mfid, omask->points, w * h * sizeof(unsigned char), &nbr, 0);
		
		CloseHandle(mfid);
		return 1;
	}

	CloseHandle(mfid);

	return 0;
}

int Mask_LoadMaskResource(HMODULE hmod, const string rtype, const string rname, Mask* omask)
{
	HRSRC fres;
	HGLOBAL lres;
	unsigned char* rpt;

	fres = FindResource(hmod, rname, rtype);
	if(!fres)return 0;
	lres = LoadResource(hmod, fres);
	rpt  = (unsigned char*) LockResource(lres);

	omask->w = ((unsigned long*)rpt)[0];
	omask->h = ((unsigned long*)rpt)[1];

	if((omask->w * omask->h <= 0) || (omask->w * omask->h >= 0x2000000))return 0;
	
	omask->points = (unsigned char*) sys_mem_alloc(omask->w * omask->h * sizeof(unsigned char));
	
	memcpy(omask->points, rpt + (sizeof(unsigned long) * 2), (omask->w * omask->h * sizeof(unsigned char)) - (sizeof(unsigned long) * 2));
	
	UnlockResource(lres);
	FreeLibrary(lres);
	return 1;
}

/* return -1 for an error */

int Mask_Point(int x, int y, Mask* imask)
{
	if((unsigned long)x > imask->w || (unsigned long)y > imask->h)return -1;
	if(!imask->points)return -1;
	return imask->points[x + (y * imask->w)];
}

int Mask_Create(Mask* cmask)
{
	cmask->w = 0;
	cmask->h = 0;
	cmask->points = 0;
	return 1;
}

int Mask_Free(Mask* imask)
{
	if(imask->points)
	{
		sys_mem_free(imask->points);
		imask->w = 0;
		imask->h = 0;
		return 1;
	}

	imask->w = 0;
	imask->h = 0;

	return 0;
}

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/