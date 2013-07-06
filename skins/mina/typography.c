#include "skin.h"

HFONT     typo_fonts[typo_count];
int       last_font = -1;



HFONT typo_makefont(const string fface, int size, int bold, int italic)
{
	return CreateFont(-MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0,
		(bold ? FW_BOLD : FW_NORMAL), italic, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY , DEFAULT_PITCH, fface);
}

void typo_create_fonts(void)
{
	
	typo_fonts[typo_song_title]    = typo_makefont(uni("Verdana"), 18, 1, 0);
	typo_fonts[typo_song_artist]   = typo_makefont(uni("Verdana"), 12, 1, 0);
	typo_fonts[typo_song_album]    = typo_makefont(uni("Verdana"), 12, 0, 1);
	typo_fonts[typo_song_position] = typo_makefont(uni("Verdana"), 12, 1, 0);

}

void typo_print_shadow(HDC hdc, const string text, int x, int y, COLORREF color, int ifont)
{
	if(ifont != last_font)
	{
		SelectObject(hdc, typo_fonts[ifont]);
		last_font = ifont;
		SetBkMode(hdc, TRANSPARENT);
	}

	SetTextColor(hdc, 0x00);
	TextOut(hdc, x+1, y+1, text, str_len(text));

	SetTextColor(hdc, color);
	TextOut(hdc, x, y, text, str_len(text));

	
}