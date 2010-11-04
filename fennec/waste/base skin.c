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

#include "base skin.h"
#include "Fennec Global Environment.h"
#include <shlobj.h>
#include "zmouse.h"
#include "keyboard.h"
#include "fennec audio.h"
#include "fennec help.h"

/* structures */

typedef  struct _SkinCoord
{
	int   x;  /* destination x */
	int   y;  /* destination y */
	int   sx; /* source x */
	int   sy; /* source y */
	int   w;  /* width */
	int   h;  /* height */
}SkinCoord;

/* local declarations */

int WINAPI PlaylistWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
unsigned long Local_Playlist_Draw(HDC dc, int x, int y, unsigned long fitem);

void BaseSkin_LocalBlitCoord(HDC ddc, const SkinCoord* sc, HDC sdc);

void CALLBACK TimerProc_Position(HWND hwnd, UINT uMsg, UINT_PTR idEvent, unsigned long dwTime);
int local_settitle(unsigned long i, char* txt);

/* defines */

#define BaseSkin_SkinName         "Skin"
#define BaseSkin_ActionsName      "Actions"
#define BaseSkin_MaskName         uni("Main")
#define BaseSkin_PlaylistMaskName "Playlist"
#define BaseSkin_MaskType         uni("Mask")
#define BaseSkin_PlaylistSkinName "Playlist"

#define BaseSkin_Button_Exit                 1
#define BaseSkin_Button_Minimize             2
#define BaseSkin_Button_Refresh              3
#define BaseSkin_Button_PowerOffWithoutSave  4
#define BaseSkin_Button_PowerOff             5
#define BaseSkin_Button_Sleep                6

#define BaseSkin_Button_ScreenLeft1          7
#define BaseSkin_Button_ScreenLeft2          8
#define BaseSkin_Button_ScreenLeft3          9
#define BaseSkin_Button_ScreenRight1         10
#define BaseSkin_Button_ScreenRight2         11
#define BaseSkin_Button_ScreenRight3         12

#define BaseSkin_Button_WindowPanel_DJ       13
#define BaseSkin_Button_WindowPanel_Piano    14
#define BaseSkin_Button_WindowPanel_Video    15
#define BaseSkin_Button_WindowPanel_Amp      16
#define BaseSkin_Button_WindowPanel_MixList  17
#define BaseSkin_Button_WindowPanel_Playlist 18
#define BaseSkin_Button_WindowPanel_Library  19
#define BaseSkin_Button_WindowPanel_Recorder 20

#define BaseSkin_Button_Previous             21
#define BaseSkin_Button_Rewind               22
#define BaseSkin_Button_Play                 23
#define BaseSkin_Button_Stop                 24
#define BaseSkin_Button_Forward              25
#define BaseSkin_Button_Next                 26
#define BaseSkin_Button_Open                 27
#define BaseSkin_Button_Eject                28
#define BaseSkin_Button_Select               29

#define BaseSkin_PlaylistButton_AutoSwitching 1
#define BaseSkin_PlaylistButton_Shuffle       2
#define BaseSkin_PlaylistButton_Information   3
#define BaseSkin_PlaylistButton_Repeat        4
#define BaseSkin_PlaylistButton_Insert        5
#define BaseSkin_PlaylistButton_Remove        6
#define BaseSkin_PlaylistButton_Sort          7
#define BaseSkin_PlaylistButton_Storage       8
#define BaseSkin_PlaylistButton_Options       9

#define BaseSkin_MemoryShift_Normals -1
#define BaseSkin_MemoryShift_Hovers  28
#define BaseSkin_MemoryShift_Downs   57

#define BaseSkin_Playlist_MemoryShift_Normals -1
#define BaseSkin_Playlist_MemoryShift_Hovers  11
#define BaseSkin_Playlist_MemoryShift_Downs   23
#define BaseSkin_Playlist_ButtonCount         12

#define position_forward 2
#define position_rewind  1
#define timer_position   10111

/* constants */

const SkinCoord SkinButtons[] =
{
/* normal */

/*main buttons exit normal     */{46   ,44 ,120,197,47 ,48},
/*main buttons minimize normal */{71   ,24 ,62 ,197,57 ,39},
/*main buttons refresh normal  */{26   ,68 ,168,197,37 ,58},
/*main buttons power off normal*/{14   ,102,206,197,28 ,57},
/*<not defined>                */{0    ,0  ,0  ,0  ,0  ,0 },
/*main buttons sleep normal    */{104  ,12 ,1  ,197,60 ,31},
/*display panel left normal 1  */{53   ,178,1  ,380,20 ,20},
/*display panel left normal 2  */{53   ,196,22 ,380,20 ,20},
/*display panel left normal 3  */{53   ,213,43 ,380,20 ,20},
/*display panel right normal 1 */{336  ,178,64 ,380,20 ,20},
/*display panel right normal 2 */{336  ,196,85 ,380,20 ,20},
/*display panel right normal 3 */{336  ,213,106,380,20 ,20},
/*panel DJ                     */{112  ,266,1  ,401,20 ,20},
/*panel piano                  */{130  ,266,22 ,401,20 ,20},
/*panel video                  */{149  ,266,43 ,401,20 ,20},
/*panel amplifier              */{167  ,266,64 ,401,20 ,20},
/*panel mixlist                */{97   ,287,85 ,401,20 ,20},
/*panel playlist               */{114  ,287,106,401,20 ,20},
/*panel library                */{133  ,287,127,401,20 ,20},
/*panel recorder               */{151  ,287,148,401,20 ,20},
/*playback previous normal     */{76   ,98 ,1  ,67 ,65 ,50},
/*playback rewind normal       */{112  ,72 ,67 ,67 ,59 ,62},
/*playback play/pause normal   */{154  ,62 ,127,67 ,51 ,55},
/*playback stop normal         */{202  ,62 ,179,67 ,51 ,54},
/*playback fast forward normal */{236  ,71 ,231,67 ,60 ,61},
/*playback next normal         */{267  ,96 ,292,67 ,61 ,52},
/*playback open normal         */{154  ,297,463,67 ,50 ,50},
/*playback eject normal        */{203  ,295,412,67 ,50 ,53},
/*playback select normal       */{237  ,281,354,67 ,57 ,57},

/* hover */

/*main buttons exit hover      */{46   ,44 ,120,258,47 ,48},
/*main buttons minimize hover  */{71   ,24 ,62 ,258,57 ,39},
/*main buttons refresh hover   */{26   ,68 ,168,258,37 ,58},
/*main buttons power off hover */{14   ,102,206,258,28 ,57},
/*<not defined>                */{0    ,0  ,0  ,0  ,0  ,0 },
/*main buttons sleep hover     */{104  ,12 ,1  ,258,60 ,31},
/*display panel left hover  1  */{53   ,178,1  ,422,20 ,20},
/*display panel left hover  2  */{53   ,196,22 ,422,20 ,20},
/*display panel left hover  3  */{53   ,213,43 ,422,20 ,20},
/*display panel right hover  1 */{336  ,178,64 ,422,20 ,20},
/*display panel right hover  2 */{336  ,196,85 ,422,20 ,20},
/*display panel right hover  3 */{336  ,213,106,422,20 ,20},
/*panel DJ                     */{112  ,266,1  ,443,20 ,20},
/*panel piano                  */{130  ,266,22 ,443,20 ,20},
/*panel video                  */{149  ,266,43 ,443,20 ,20},
/*panel amplifier              */{167  ,266,64 ,443,20 ,20},
/*panel mixlist                */{97   ,287,85 ,443,20 ,20},
/*panel playlist               */{114  ,287,106,443,20 ,20},
/*panel library                */{133  ,287,127,443,20 ,20},
/*panel recorder               */{151  ,287,148,443,20 ,20},
/*playback previous hover      */{76   ,98 ,1  ,1  ,65 ,50},
/*playback rewind hover        */{112  ,72 ,67 ,1  ,59 ,62},
/*playback play/pause hover    */{154  ,62 ,127,1  ,51 ,55},
/*playback stop hover          */{202  ,62 ,179,1  ,51 ,54},
/*playback fast forward hover  */{236  ,71 ,231,1  ,60 ,61},
/*playback next hover          */{267  ,96 ,292,1  ,61 ,52},
/*playback open hover          */{154  ,297,463,1  ,50 ,50},
/*playback eject hover         */{203  ,295,412,1  ,50 ,53},
/*playback select hover        */{237  ,281,354,1  ,57 ,57},

/* down */

/*main buttons exit down       */{46   ,44 ,120,319,47 ,48},
/*main buttons minimize down   */{71   ,24 ,62 ,319,57 ,39},
/*main buttons refresh down    */{26   ,68 ,168,319,37 ,58},
/*main buttons power off down  */{14   ,102,206,319,28 ,57},
/*<not defined>                */{0    ,0  ,0  ,0  ,0  ,0 },
/*main buttons sleep down      */{104  ,12 ,1  ,319,60 ,31},
/*display panel left down   1  */{53   ,178,169,380,20 ,20},
/*display panel left down   2  */{53   ,196,190,380,20 ,20},
/*display panel left down   3  */{53   ,213,211,380,20 ,20},
/*display panel right down   1 */{336  ,178,232,380,20 ,20},
/*display panel right down   2 */{336  ,196,253,380,20 ,20},
/*display panel right down   3 */{336  ,213,274,380,20 ,20},
/*panel DJ                     */{112  ,266,169,401,20 ,20},
/*panel piano                  */{130  ,266,190,401,20 ,20},
/*panel video                  */{149  ,266,211,401,20 ,20},
/*panel amplifier              */{167  ,266,232,401,20 ,20},
/*panel mixlist                */{97   ,287,253,401,20 ,20},
/*panel playlist               */{114  ,287,274,401,20 ,20},
/*panel library                */{133  ,287,295,401,20 ,20},
/*panel recorder               */{151  ,287,316,401,20 ,20},
/*playback previous down       */{76   ,98 ,1  ,132,65 ,50},
/*playback rewind down         */{112  ,72 ,67 ,132,59 ,62},
/*playback play/pause down     */{154  ,62 ,127,132,51 ,55},
/*playback stop down           */{202  ,62 ,179,132,51 ,54},
/*playback fast forward down   */{236  ,71 ,231,132,60 ,61},
/*playback next down           */{267  ,96 ,292,132,61 ,52},
/*playback open down           */{154  ,297,463,132,50 ,50},
/*playback eject down          */{203  ,295,412,132,50 ,53},
/*playback select down         */{237  ,281,354,132,57 ,57}
};

const SkinCoord SkinPlaylistButtons[] = {

/*playlist main auto switching normal */{226,12 ,235,197,43,23},
/*playlist main shuffle normal        */{254,12 ,279,197,48,23},
/*playlist main information normal    */{285,12 ,328,197,48,23},
/*playlist main repeat normal         */{315,12 ,377,197,43,23},

/*playlist control insert normal      */{366,40 ,421,197,10,18},
/*playlist control remove normal      */{366,59 ,432,197,10,18},
/*playlist control sort normal        */{366,78 ,443,197,10,18},
/*playlist control load/save normal   */{366,97 ,454,197,10,18},
/*playlist control options normal     */{366,116,465,197,10,18},	

/*reserved                            */{  0,0  ,  0,  0, 0, 0},
/*reserved                            */{  0,0  ,  0,  0, 0, 0},
/*reserved                            */{  0,0  ,  0,  0, 0, 0},

/*playlist main auto switching hover  */{226,12 ,235,258,43,23},
/*playlist main shuffle hover         */{254,12 ,279,258,48,23},
/*playlist main information hover     */{285,12 ,328,258,48,23},
/*playlist main repeat hover          */{315,12 ,377,258,43,23},

/*playlist control insert hover       */{366,40 ,421,258,10,18},
/*playlist control remove hover       */{366,59 ,432,258,10,18},
/*playlist control sort hover         */{366,78 ,443,258,10,18},
/*playlist control load/save hover    */{366,97 ,454,258,10,18},
/*playlist control options hover      */{366,116,465,258,10,18},

/*reserved                            */{  0,0  ,  0,  0, 0, 0},
/*reserved                            */{  0,0  ,  0,  0, 0, 0},
/*reserved                            */{  0,0  ,  0,  0, 0, 0},

/*playlist main auto switching down   */{226,12 ,235,319,43,23},
/*playlist main shuffle down          */{254,12 ,279,319,48,23},
/*playlist main information down      */{285,12 ,328,319,48,23},
/*playlist main repeat down           */{315,12 ,377,319,43,23},
			  
/*playlist control insert down        */{366,40 ,421,319,10,18},
/*playlist control remove down        */{366,59 ,432,319,10,18},
/*playlist control sort down          */{366,78 ,443,319,10,18},
/*playlist control load/save down     */{366,97 ,454,319,10,18},
/*playlist control options down       */{366,116,465,319,10,18},

/*reserved                            */{  0,0  ,  0,  0, 0, 0},
/*reserved                            */{  0,0  ,  0,  0, 0, 0},
/*reserved                            */{  0,0  ,  0,  0, 0, 0}
};

/* variables */

HDC     MemoryDC_Skin;
HBITMAP Bitmap_Skin;
HDC     MemoryDC_Actions;
HBITMAP Bitmap_Actions;
Mask    Mask_Main;

Mask    Mask_Playlist;

HRGN    BaseSkin_RegionMain;
HRGN    BaseSkin_RegionPlaylist;

int BaseSkin_Applied     = 0;
int BaseSkin_Initialized = 0;

int BaseSkin_Width  = 0;
int BaseSkin_Height = 0;

int BaseSkin_LastActionButton = 0;
int BaseSkin_LastAction       = 0;

int BaseSkin_Playlist_LastActionButton = 0;
int BaseSkin_Playlist_LastAction       = 0;

HWND Window_Playlist        = 0;
HDC  PlaylistWindowDC       = 0;
HDC  PlaylistMemoryDC       = 0;
int  PlaylistWindow_Width   = 0;
int  PlaylistWindow_Height  = 0;
int  PlaylistDisplay_X      = 88;
int  PlaylistDisplay_Y      = 43;
int  PlaylistDisplay_Width  = 249;
int  PlaylistDisplay_Height = 126;
int  PlaylistDisplay_ScrollWidth  = 10;
int  PlaylistDisplay_ScrollButtonHeight = 10;

int  PlaylistDisplay_ItemHeight = 14;
int  PlaylistDisplay_Max        = 9;

int  Position_Mode = 0;
UINT timer_position_id;

HFONT    Font_Playlist_NormalItem;
COLORREF Color_Playlist_NormalItem_OBackground = RGB(67, 58, 162);
COLORREF Color_Playlist_NormalItem_OForeground = RGB(255, 255, 255);
COLORREF Color_Playlist_NormalItem_EBackground = RGB(38, 33, 93);
COLORREF Color_Playlist_NormalItem_EForeground = RGB(255, 255, 255);
COLORREF Color_Playlist_SelectedItem_Background = RGB(127, 120, 201);
COLORREF Color_Playlist_SelectedItem_Foreground = RGB(255, 255, 255);
COLORREF Color_Playlist_PlayingItem_Background = RGB(150, 147, 176);
COLORREF Color_Playlist_PlayingItem_Foreground = RGB(255, 255, 255);
COLORREF Color_Playlist_Moving_OBackground = RGB(133, 127, 194);
COLORREF Color_Playlist_Moving_OForeground = RGB(255, 255, 255);
COLORREF Color_Playlist_Moving_EBackground = RGB(114, 110, 150);
COLORREF Color_Playlist_Moving_EForeground = RGB(255, 255, 255);
unsigned long    Playlist_SelectedItem = 0;
unsigned long    Playlist_FirstItem = 0;
int tips_display(int x, int y, string msg);
int local_tips_create(void);

WORD     CurrentColorH = 0;
WORD     CurrentColorS = 0;
WORD     CurrentColorL = 0;

int gseldrive_msgok = 0;

char  playlist_tip_text[1024];



// Data for windproc ////////////////////////////////////////////////

POINT  downPos = {0,0};
POINT  movePos;
SIZE   screenSize;
RECT   rClt;
RECT   rWork;
POINT  menupt;
HMENU  tpmenu;
POINT  tmppt;
char   proc_busy = 0;
HDWP   hpos = NULL;

int    last_action = 0;
int    onmoving = 0;

long   OPTION_DockChange = 10;
long   OPTION_Dock = 1;
int    FennecDefaultMessage = 0;



struct playlist_itemdata
{
	unsigned long itemid;
	char          itext[260];
};

struct playlist_itemdata *Playlist_ItemData;

/* functions */

/*
 * Initialize entire base skin system (should be 'Uninitialized' before swithcing
 * into another skin).
 */

int BaseSkin_Initialize(void)
{
	BITMAP bmpskin;
	unsigned int w = 0, h = 0;

	if(BaseSkin_Initialized)return 0;

	/* create mask */

	Mask_Create(&Mask_Main);
	if(!Mask_LoadMaskResource(instance_fennec, BaseSkin_MaskType, BaseSkin_MaskName, &Mask_Main))return 0;

	/* create skin dc */

	MemoryDC_Skin = CreateCompatibleDC(window_main_dc);

	Bitmap_Skin = png_res_load_winbmp(instance_fennec, uni("png"), uni("skin.window.main"));


	GetObject(Bitmap_Skin, sizeof(BITMAP), &bmpskin);
	BaseSkin_Width  = bmpskin.bmWidth;
	BaseSkin_Height = bmpskin.bmHeight;

	SelectObject(MemoryDC_Skin, Bitmap_Skin);

	BaseSkin_RegionMain = CreateRectRgn(0, 0, w, h);
	BaseSkin_RegionMain = Skin_GetBitmapRegion(Bitmap_Skin);

	DeleteObject(Bitmap_Skin);

	/* create actions dc */

	//MemoryDC_Actions = CreateCompatibleDC(0);
	//Bitmap_Actions   = LoadBitmap(instance_fennec, BaseSkin_ActionsName);
	//SelectObject(MemoryDC_Actions,Bitmap_Actions);
	//DeleteObject(Bitmap_Actions);

	png_res_load_dc(instance_fennec, uni("png"), uni("skin.actions.main"), &w, &h, &MemoryDC_Actions);


	BaseSkin_Initialized = 1;

	if((settings.environment.playlist_window_state != setting_window_hidden) && 
	   (settings.environment.playlist_window_state != setting_window_hidden_docked))
	{
		BaseSkin_Playlist_CreateWindow();
		BaseSkin_Playlist_ShowWindow();
	}
	
	return 1;
}

/*
 * Apply startup skin effects (region, background).
 */

int BaseSkin_ApplySkin(void)
{
	if(!BaseSkin_Initialized)return 0;

	SetWindowRgn(window_main, BaseSkin_RegionMain, 1);

	SetWindowPos(window_main, NULL, 0, 0, BaseSkin_Width, BaseSkin_Height, SWP_NOACTIVATE | SWP_NOMOVE);

	BaseSkin_Redraw(1, 0, 0, 0, 0);
	return 1;
}

/*
 * Set skin color by HSL values
 */

int BaseSkin_SetColor(int h, int s, int l)
{
	HBITMAP vbitmap;

	vbitmap = png_res_load_winbmp(instance_fennec, uni("png"), uni("skin.window.main"));
	AdjustColors(vbitmap, 0, h, s, l);
	SelectObject(MemoryDC_Skin, vbitmap);
	DeleteObject(vbitmap);

	vbitmap = png_res_load_winbmp(instance_fennec, uni("png"), uni("skin.actions.main"));
	AdjustColors(vbitmap, 0, h, s, l);
	SelectObject(MemoryDC_Actions, vbitmap);
	DeleteObject(vbitmap);

	vbitmap = png_res_load_winbmp(instance_fennec, uni("png"), uni("skin.window.playlist"));
	AdjustColors(vbitmap, 0, h, s, l);
	SelectObject(PlaylistMemoryDC, vbitmap);
	DeleteObject(vbitmap);

	CurrentColorH = (WORD)h;
	CurrentColorS = (WORD)s;
	CurrentColorL = (WORD)l;

	BaseSkin_Redraw(1, 0, 0, 0, 0);
	
	Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, 0);
	BitBlt(PlaylistWindowDC, 0, 0, PlaylistWindow_Width, PlaylistWindow_Height, PlaylistMemoryDC, 0, 0, SRCCOPY);

	settings.player.themecolor = RGB(h, s, l);
	return 1;
}


int gseldrive_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char tmpbuf[10];
				int csel = (int)SendDlgItemMessage(hwnd, IDC_DRIVES, CB_GETCURSEL, 0, 0);
				
				if(csel != -1)
				{
					SendDlgItemMessage(hwnd, IDC_DRIVES, CB_GETLBTEXT, csel, (LPARAM)tmpbuf);
					settings.player.selected_drive = tmpbuf[0];
				}else{
					settings.player.selected_drive = 0;
				}
			}
			gseldrive_msgok = 1;
			EndDialog(hwnd, 0);
			break;

		case IDCANCEL:
			gseldrive_msgok = 1;
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			unsigned int i;
			char droot[]  = "X:\\";
			char drname[] = "1:";

			for(i=0; i<26; i++)
			{
				droot[0] = (char)('A' + i);
				if(GetDriveType(droot) == DRIVE_CDROM)
				{
					drname[0] = (char)('A' + i);
					SendDlgItemMessage(hwnd, IDC_DRIVES, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)drname);
				}
			}

			if(drname[0] == '1')
			{
				MessageBox(hwnd, "No CD drives found", "Error", MB_ICONEXCLAMATION);
				settings.player.selected_drive = 0;
				gseldrive_msgok = 1;
				EndDialog(hwnd, 0);
			}
		}
		break;

	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
	}
	return 0;
}

void ShowSelDrive()
{
	gseldrive_msgok = 0;
	DialogBox(instance_fennec, MAKEINTRESOURCE(IDD_SELDRIVE), window_main, (DLGPROC)gseldrive_proc);
	while(!gseldrive_msgok)sys_pass();
}

/*
 * Mouse messages.
 */

int BaseSkin_MouseMessage(int x, int y, unsigned short mdata)
{
	unsigned long pointid;
	int   faction;

#define FormatAction(x) (((x == 1) | (x == 2) | (x == 4)) ? x : 3)

	if(!BaseSkin_Initialized)return 0;

	pointid = Mask_Point(x, y, &Mask_Main);
	faction = FormatAction(mdata);

	if(faction == 1 && (BaseSkin_LastActionButton == (int) pointid) && (BaseSkin_LastAction == FormatAction(mdata)))faction = 6;

	switch(pointid)
	{
	case BaseSkin_Button_Exit:
		tips_display(0, 0, tip_text_exit);
		if(faction == 1)fennec_power_set(fennec_power_mode_default);
		goto CommonPoint;

	case BaseSkin_Button_Minimize:
		tips_display(0, 0, tip_text_minimize);
		if(faction == 1)ShowWindow(window_main, SW_MINIMIZE);
		goto CommonPoint;

	case BaseSkin_Button_Refresh:
		tips_display(0, 0, tip_text_refresh);
		if(faction == 1)fennec_refresh(fennec_v_refresh_force_less);
		goto CommonPoint;

	case BaseSkin_Button_PowerOff:
		tips_display(0, 0, tip_text_poweroff);
		if(faction == 1)fennec_power_set(fennec_power_mode_save);
		goto CommonPoint;

	case BaseSkin_Button_PowerOffWithoutSave:
		tips_display(0, 0, tip_text_poweroffwithoutsaving);
		if(faction == 1)fennec_power_set(fennec_power_mode_exit);
		goto CommonPoint;

	case BaseSkin_Button_Sleep:
		tips_display(0, 0, tip_text_sleep);
		if(faction == 1)fennec_power_set(fennec_power_mode_sleep);
		goto CommonPoint;

	case BaseSkin_Button_ScreenLeft1:
		tips_display(0, 0, tip_text_panelback);
		if(faction == 1)Display_ShowPanel_Back();
		goto CommonPoint;

	case BaseSkin_Button_ScreenLeft2:
		tips_display(0, 0, tip_text_panelcolor);
		if(faction == 1)Display_ShowPanel(PanelID_Color);
		goto CommonPoint;

	case BaseSkin_Button_ScreenLeft3:
		tips_display(0, 0, tip_text_panelnext);
		if(faction == 1)Display_ShowPanel_Next();
		goto CommonPoint;

	case BaseSkin_Button_ScreenRight1:
		tips_display(0, 0, tip_text_panelback);
		if(faction == 1)Display_ShowPanel_Back();
		goto CommonPoint;

	case BaseSkin_Button_ScreenRight2:
		tips_display(0, 0, tip_text_panelmain);
		if(faction == 1)Display_ShowPanel(PanelID_Main);
		goto CommonPoint;

	case BaseSkin_Button_ScreenRight3:
		tips_display(0, 0, tip_text_panelnext);
		if(faction == 1)Display_ShowPanel_Next();
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_DJ:
		tips_display(0, 0, tip_text_scratch);
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_Piano:
		tips_display(0, 0, tip_text_piano);
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_Video:
		tips_display(0, 0, tip_text_video);
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_Amp:
		tips_display(0, 0, tip_text_amp);
		if(faction == 1)GlobalFunction(Function_EqualizerPanel);
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_MixList:
		tips_display(0, 0, tip_text_mixlist);
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_Playlist:
		tips_display(0, 0, tip_text_playlist);
		if(faction == 1)
		{
			if(settings.environment.playlist_window_state != setting_window_hidden && settings.environment.playlist_window_state != setting_window_hidden_docked)
			{
				
				if(settings.environment.playlist_window_state == setting_window_docked)
				{
					settings.environment.playlist_window_state = setting_window_hidden_docked;
				}else{
					settings.environment.playlist_window_state = setting_window_hidden;
				}
				BaseSkin_Playlist_CloseWindow();
			}else{

				if(settings.environment.playlist_window_state == setting_window_hidden_docked)
				{
					settings.environment.playlist_window_state = setting_window_docked;
				}else{
					settings.environment.playlist_window_state = setting_window_normal;
				}			
				
				BaseSkin_Playlist_CreateWindow();
				BaseSkin_Playlist_ShowWindow();
				
			}
		}
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_Library:
		tips_display(0, 0, tip_text_library);
		goto CommonPoint;

	case BaseSkin_Button_WindowPanel_Recorder:
		tips_display(0, 0, tip_text_recoder);
		goto CommonPoint;

	case BaseSkin_Button_Previous:
		tips_display(0, 0, tip_text_previous);
		if(faction == 1)GlobalFunction(Function_Previous);
		goto CommonPoint;

	case BaseSkin_Button_Rewind:
		tips_display(0, 0, tip_text_rewind);
		if(mdata == 5)
		{
			if(Position_Mode)KillTimer(0, timer_position_id);
			Position_Mode = 0;
		}
			
		if(faction == 1)
		{
			if(!Position_Mode)
			{
				timer_position_id = (UINT)SetTimer(0, timer_position, 50, (TIMERPROC)TimerProc_Position);
			}
			Position_Mode = position_rewind;
		}
		
		goto CommonPoint;

	case BaseSkin_Button_Play:
		tips_display(0, 0, tip_text_play);
		if(faction == 1)GlobalFunction(Function_Play);
		goto CommonPoint;

	case BaseSkin_Button_Stop:
		tips_display(0, 0, tip_text_stop);
		if(faction == 1)GlobalFunction(Function_Stop);
		goto CommonPoint;

	case BaseSkin_Button_Forward:
		tips_display(0, 0, tip_text_forward);
		if(mdata == 5)
		{
			if(Position_Mode)KillTimer(0, timer_position_id);
			Position_Mode = 0;
		}
			
		if(faction == 1)
		{
			if(!Position_Mode)
			{
				timer_position_id = (UINT)SetTimer(0, timer_position, 50, (TIMERPROC)TimerProc_Position);
			}

			Position_Mode = position_forward;
		}

		goto CommonPoint;

	case BaseSkin_Button_Next:
		tips_display(0, 0, tip_text_next);
		if(faction == 1)GlobalFunction(Function_Next);
		goto CommonPoint;

	case BaseSkin_Button_Open:
		tips_display(0, 0, tip_text_load);
		if(faction == 1) /* can't use mouse up, because it defines both r/l ups */
		{
			GlobalFunction(Function_OpenFileDialog);
		}else if(faction == 4){
			basewindows_show_open("Advanced File Selection.", 0, 1);
		}
		goto CommonPoint;

	case BaseSkin_Button_Eject:
		tips_display(0, 0, tip_text_eject);
		if(faction == 1)
		{
			if(settings.player.selected_drive)
			{
				if(audio_playlist_getsource(audio_playlist_getcurrentindex())[0] == settings.player.selected_drive)audio_stop();
				audio_input_drive_eject(settings.player.selected_drive);
			}
		}
		goto CommonPoint;

	case BaseSkin_Button_Select:
		tips_display(0, 0, tip_text_select);
		if(faction == 1)
		{
			if(settings.player.selected_drive)
			{
				int i, c = audio_input_drive_gettracks(settings.player.selected_drive);
				char buf[128];
				char buf2[32];

				if(c > 0)
				{
					audio_playlist_clear();
					for(i=0; i<c; i++)
					{
						buf[0] = settings.player.selected_drive;
						strcpy(buf + 1, ":\\Track");
								
						memset(buf2, 0, sizeof(buf2));
						itoa(i + 1, buf2, 10);
						strcat(buf, buf2);
						strcat(buf, ".cda");

						audio_playlist_add(buf, 0, 0);
					}
					fennec_refresh(fennec_v_refresh_force_less);
				}
			}
		}else if(faction == 4){
			ShowSelDrive();
		}


CommonPoint:

		if(BaseSkin_LastActionButton)
		{

			if(BaseSkin_LastActionButton == (int) pointid && BaseSkin_LastAction == FormatAction(mdata))break;
			BaseSkin_LocalBlitCoord(window_main_dc, &SkinButtons[BaseSkin_LastActionButton + BaseSkin_MemoryShift_Normals], MemoryDC_Actions);
			BaseSkin_LastActionButton = 0;
		}

		if(mdata == 1 || mdata == 2 || mdata == 4)
		{
			BaseSkin_LocalBlitCoord(window_main_dc, &SkinButtons[pointid + BaseSkin_MemoryShift_Downs], MemoryDC_Actions);
			BaseSkin_LastAction = FormatAction(mdata);
		}else{
			BaseSkin_LocalBlitCoord(window_main_dc, &SkinButtons[pointid + BaseSkin_MemoryShift_Hovers], MemoryDC_Actions);
			BaseSkin_LastAction = FormatAction(mdata);
		}

		BaseSkin_LastActionButton = (int) pointid;

		break;

	default:
		return 0;
	}

	return 1;
}

/*
 Mouse message based skin updates.
*/

int BaseSkin_Redraw(int fr, int x, int y, int w, int h)
{
	if(!BaseSkin_Initialized)return 0;

	Display_BlitCurrentPanel(MemoryDC_Skin, 0xFFFF, 0xFFFF); /* 0xFFFF, default x and y */
	BitBlt(window_main_dc, 0, 0, BaseSkin_Width, BaseSkin_Height, MemoryDC_Skin, 0, 0, SRCCOPY);

	return 1;
}

/*
 Uninitialize and destroy (free) all the memory allocated for the skin.
*/

int BaseSkin_Uninitialize(void)
{
	if(!BaseSkin_Initialized)return 1;

	Mask_Free(&Mask_Main);

	DeleteDC(MemoryDC_Skin);
	DeleteDC(MemoryDC_Actions);

	DeleteObject(BaseSkin_RegionMain);
	BaseSkin_Initialized = 0;
	return 1;
}

/* ----------------------------------------------------------------------------
 Playlist
-----------------------------------------------------------------------------*/

int BaseSkin_Refresh_Playlist(void)
{
	if(Window_Playlist)
	{
		if(Playlist_FirstItem + PlaylistDisplay_Max > audio_playlist_getcount())Playlist_FirstItem = 0;
		Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, 0);
		BitBlt(PlaylistWindowDC, 0, 0, PlaylistWindow_Width, PlaylistWindow_Height, PlaylistMemoryDC, 0, 0, SRCCOPY);
	}
	return 1;
}

int BaseSkin_Playlist_CreateWindow(void)
{
#define PlaylistWindowClass "FennecPlaylist"
	unsigned long i;
	WNDCLASS plwclass;

	plwclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	plwclass.lpfnWndProc   = (WNDPROC)PlaylistWindowProc;
	plwclass.cbClsExtra    = 0;
	plwclass.cbWndExtra    = 0;
	plwclass.hInstance     = instance_fennec;
	plwclass.hIcon         = LoadIcon(instance_fennec, (LPCTSTR)IDI_MAIN);
	plwclass.hCursor       = LoadCursor(0, IDC_ARROW);
	plwclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	plwclass.lpszMenuName  = 0;
	plwclass.lpszClassName = PlaylistWindowClass;

	RegisterClass(&plwclass);

	Playlist_ItemData = (struct playlist_itemdata*) sys_mem_alloc(sizeof(struct playlist_itemdata) * (PlaylistDisplay_Max + 1));
	for(i=0; i<=(unsigned int)PlaylistDisplay_Max; i++)
	{
		Playlist_ItemData[i].itemid = (unsigned long)-1;
	}

	Window_Playlist = CreateWindow(PlaylistWindowClass, "Fennec Playlist", WS_POPUP, 0, 0,200,200, window_main, 0, instance_fennec, 0);
	
	Mask_Create(&Mask_Playlist);
	Mask_LoadMaskResource(instance_fennec, BaseSkin_MaskType, BaseSkin_PlaylistMaskName, &Mask_Playlist);

	if(settings.environment.playlist_window_state == setting_window_docked)
	{
		RECT rct;

		GetWindowRect(window_main, &rct);
		SetWindowPos(Window_Playlist, 0, rct.left + 346,rct.top + 151, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	CheckMenuItem(menu_main, ID_MENU_PLAYLISTWINDOW, MF_CHECKED);

	return 1;
}

int BaseSkin_Playlist_ShowWindow(void)
{
	ShowWindow(Window_Playlist, SW_SHOW);
	UpdateWindow(Window_Playlist);
	return 1;
}

int BaseSkin_Playlist_HideWindow(void)
{
	ShowWindow(Window_Playlist, SW_HIDE);
	return 1;
}

int BaseSkin_Playlist_CloseWindow(void)
{
	if(!Window_Playlist)return 0;

	DeleteObject(Font_Playlist_NormalItem);
	DeleteObject(BaseSkin_RegionPlaylist);
	DeleteDC(PlaylistMemoryDC);
	DestroyWindow(Window_Playlist);
	Mask_Free(&Mask_Playlist);
	sys_mem_free(Playlist_ItemData);

	CheckMenuItem(menu_main, ID_MENU_PLAYLISTWINDOW, MF_UNCHECKED);
	Window_Playlist = 0;
	return 1;
}

/* ----------------------------------------------------------------------------
 local functions.
---------------------------------------------------------------------------- */

void BaseSkin_LocalBlitCoord(HDC ddc, const SkinCoord* sc, HDC sdc)
{
	BitBlt(ddc, sc->x, sc->y, sc->w, sc->h, sdc, sc->sx, sc->sy, SRCCOPY);
}

void Local_FastRect(HDC dc, COLORREF rc, int x, int y, int w, int h)
{
	HPEN   oldpen;
	HBRUSH oldbrush;
	HPEN   hpen;
	HBRUSH hbrush;
	COLORREF bc;

#define clipc(a)(a > 255 ? 255 : (a < 0 ? 0 : a))

	if(y >= (PlaylistDisplay_Y + PlaylistDisplay_Height))return;

	bc = GetAdjustedColor(163, settings.player.displaycolor_h, settings.player.displaycolor_s, settings.player.displaycolor_l);

	Color_Playlist_SelectedItem_Background = RGB( clipc(GetRValue(bc))
												, clipc(GetGValue(bc))
												, clipc(GetBValue(bc)));

	Color_Playlist_Moving_EBackground      = RGB( clipc(GetRValue(bc) - 20)
												, clipc(GetGValue(bc) - 20)
												, clipc(GetBValue(bc) - 20));

	Color_Playlist_Moving_OBackground      = RGB( clipc(GetRValue(bc) - 50)
												, clipc(GetGValue(bc) - 50)
												, clipc(GetBValue(bc) - 50));

	Color_Playlist_NormalItem_EBackground  = RGB( clipc(GetRValue(bc) - 100)
												, clipc(GetGValue(bc) - 100)
												, clipc(GetBValue(bc) - 100));

	Color_Playlist_NormalItem_OBackground  = RGB( clipc(GetRValue(bc) - 70)
												, clipc(GetGValue(bc) - 70)
												, clipc(GetBValue(bc) - 70));

	Color_Playlist_PlayingItem_Background  = RGB( clipc(GetRValue(bc) + 50)
												, clipc(GetGValue(bc) + 50)
												, clipc(GetBValue(bc) + 50));
	Color_Playlist_PlayingItem_Foreground  = 0;


	hpen = CreatePen(PS_SOLID, 0, rc);
	hbrush = CreateSolidBrush(rc);

	oldpen = SelectObject(dc, hpen);
	oldbrush = SelectObject(dc, hbrush);

	Rectangle(dc, x, y, x + w, y + h);

	DeleteObject(hpen);
	DeleteObject(hbrush);

	SelectObject(dc, oldpen);
	SelectObject(dc, oldbrush);
}

int sort_filenames(const char *f1, const char *f2)
{
	size_t c1 = strlen(f1), c2 = strlen(f2);

	while(c1 && (f1[c1] != '/' && f1[c1] != '\\'))c1--;
	while(c2 && (f2[c2] != '/' && f2[c2] != '\\'))c2--;

	return stricmp(f1 + c1 + 1, f2 + c2 + 1);
}



int Local_Playlist_WindowMouseMessage(int x, int y, int msg)
{
	int pointid;

	if(!BaseSkin_Initialized)return 0;

	//if(x > (PlaylistDisplay_X + PlaylistDisplay_Width) && x <= (PlaylistDisplay_X + PlaylistDisplay_Width + 10))
	//{
	//	if(msg == 1 && y > PlaylistDisplay_Y && y <= PlaylistDisplay_Y + 10)
	//	{
	//		if(audio_playlist_getcount() > (unsigned long)PlaylistDisplay_Max)
	//		{

	//			if(Playlist_FirstItem >= 1)
	//			{
	//				Playlist_FirstItem -= 1;	
	//			}else{
	//				Playlist_FirstItem = 0;
	//			}
	//			Local_Playlist_Scroll(-1.0f);
	//		}

	//	}
	//}

	//if(x > (PlaylistDisplay_X + PlaylistDisplay_Width) && x <= (PlaylistDisplay_X + PlaylistDisplay_Width + 10))
	//{
	//	if(msg == 1 && y > (PlaylistDisplay_Y + PlaylistDisplay_Height - 10) && y <= (PlaylistDisplay_Y + PlaylistDisplay_Height))
	//	{
	//		if(audio_playlist_getcount() > (unsigned long)PlaylistDisplay_Max)
	//		{

	//			if((int)Playlist_FirstItem <= ((int)audio_playlist_getcount()) - PlaylistDisplay_Max - 3)
	//			{
	//				Playlist_FirstItem += 3;
	//			}else{
	//				Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
	//			
	//			}
	//			Local_Playlist_Scroll(-1.0f);
	//		}
	//	}
	//}

	pointid = Mask_Point(x, y, &Mask_Playlist);

	if(msg != 0 && (msg == BaseSkin_Playlist_LastAction && pointid == BaseSkin_Playlist_LastActionButton))return 0;

	if(BaseSkin_Playlist_LastActionButton)
	{
		BaseSkin_LocalBlitCoord(PlaylistWindowDC, &SkinPlaylistButtons[BaseSkin_Playlist_LastActionButton + BaseSkin_Playlist_MemoryShift_Normals], MemoryDC_Actions);
		BaseSkin_Playlist_LastActionButton = 0;
	}

	if(!pointid)return 0;

	if(msg == 0) /* mouse move */
	{
		if(pointid > 0 && pointid < BaseSkin_Playlist_ButtonCount)
		{
			if(!(msg == BaseSkin_Playlist_LastAction && pointid == BaseSkin_Playlist_LastActionButton))
			{
				BaseSkin_LocalBlitCoord(PlaylistWindowDC, &SkinPlaylistButtons[pointid + BaseSkin_Playlist_MemoryShift_Hovers], MemoryDC_Actions);
				BaseSkin_Playlist_LastActionButton = pointid;
			}
		}

		switch(pointid)
		{
		case BaseSkin_PlaylistButton_AutoSwitching:
			if(settings.player.auto_switching)
				tips_display(0, 0, tip_text_playlist_autoswitching_on);
			else
				tips_display(0, 0, tip_text_playlist_autoswitching_off);
			break;
	
		case BaseSkin_PlaylistButton_Shuffle:
			if(settings.player.playlist_shuffle)
				tips_display(0, 0, tip_text_playlist_shuffle_on);
			else
				tips_display(0, 0, tip_text_playlist_shuffle_off);
			break;

		case BaseSkin_PlaylistButton_Information:
			tips_display(0, 0, tip_text_playlist_information); break;

		case BaseSkin_PlaylistButton_Repeat:
			if(settings.player.playlist_repeat_list)
			{
				if(settings.player.playlist_repeat_single)
					tips_display(0, 0, tip_text_playlist_repeat_both);
				else
					tips_display(0, 0, tip_text_playlist_repeat_list);
			}else{
				if(settings.player.playlist_repeat_single)
					tips_display(0, 0, tip_text_playlist_repeat_single);
				else
					tips_display(0, 0, tip_text_playlist_repeat_none);
			}
				break;

		case BaseSkin_PlaylistButton_Insert:
			tips_display(0, 0, tip_text_playlist_insert); break;

		case BaseSkin_PlaylistButton_Remove:
			tips_display(0, 0, tip_text_playlist_remove); break;

		case BaseSkin_PlaylistButton_Sort:
			tips_display(0, 0, tip_text_playlist_sort); break;

		case BaseSkin_PlaylistButton_Storage:
			tips_display(0, 0, tip_text_playlist_storage); break;

		case BaseSkin_PlaylistButton_Options:
			tips_display(0, 0, tip_text_playlist_settings); break;
		}

	}

	if(pointid > 0 && pointid < BaseSkin_Playlist_ButtonCount && msg == 1) /* mouse down */
	{
		BaseSkin_LocalBlitCoord(PlaylistWindowDC, &SkinPlaylistButtons[pointid + BaseSkin_Playlist_MemoryShift_Downs], MemoryDC_Actions);
		BaseSkin_Playlist_LastActionButton = pointid;
	}

	if(pointid > 0 && pointid < BaseSkin_Playlist_ButtonCount && msg == 2) /* mouse up */
	{
		BaseSkin_LocalBlitCoord(PlaylistWindowDC, &SkinPlaylistButtons[pointid + BaseSkin_Playlist_MemoryShift_Normals], MemoryDC_Actions);
		BaseSkin_Playlist_LastActionButton = 0;

		switch(pointid)
		{
		case BaseSkin_PlaylistButton_AutoSwitching:
			settings.player.auto_switching ^= 1;
			break;

		case BaseSkin_PlaylistButton_Shuffle:
			if(settings.player.playlist_shuffle)
				audio_playlist_setshuffle(0, 1);
			else
				audio_playlist_setshuffle(1, 1);

			break;

		case BaseSkin_PlaylistButton_Information:
			if(Playlist_SelectedItem < audio_playlist_getcount())
				basewindows_show_tagging(0, audio_playlist_getsource(Playlist_SelectedItem));
			break;

		case BaseSkin_PlaylistButton_Repeat:
			settings.player.playlist_repeat_single ^= 1;
			break;

		case BaseSkin_PlaylistButton_Insert:
			GlobalFunction(Function_AddFileDialog);
			BaseSkin_Playlist_ClearTitleCache();
			BaseSkin_Refresh_Playlist();
			break;

		case BaseSkin_PlaylistButton_Remove:
			audio_playlist_remove(Playlist_SelectedItem);
			BaseSkin_Playlist_ClearTitleCache();
			BaseSkin_Refresh_Playlist();
			break;

		case BaseSkin_PlaylistButton_Sort:
			{
				unsigned int i, k, c, b;
			
				c = audio_playlist_getcount();

				for(k=0; k<c; k++)
				{
				for(i=0; i<c; i++)
					{
						b = i + 1;

						if(b >= c)continue;

						if(sort_filenames(audio_playlist_getsource(b), audio_playlist_getsource(i)) == -1)
							audio_playlist_exchange(b, i);
					}
				}

				fennec_refresh(fennec_v_refresh_force_high);

			}

			break;

		case BaseSkin_PlaylistButton_Storage:
			{
				unsigned int i, c;
				char         fpath[1024];
				OPENFILENAME lofn;

				memset(&lofn, 0, sizeof(lofn));

				fpath[0] = 0;

				lofn.lStructSize     = sizeof(lofn);
				lofn.lpstrTitle      = "Save Playlist File";
				lofn.hwndOwner       = window_main;
				lofn.lpstrFile       = fpath;
				lofn.nMaxFile        = sizeof(fpath);
				lofn.lpstrFilter     = "Text file (*.txt)\0*.txt\0M3U file\0*.m3u";
				lofn.nFilterIndex    = 0;
				lofn.lpstrFileTitle  = 0;
				lofn.nMaxFileTitle   = 0;
				lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
				lofn.hInstance       = instance_fennec;

				GetSaveFileName(&lofn);

				c = (unsigned int) strlen(fpath);
				
				if(c)
				{
					if(c >= 4)
					{
						i = c - 4;

						if((stricmp(fpath + i, ".txt") != 0) && (stricmp(fpath + i, ".m3u") != 0))
						{
							if(lofn.nFilterIndex == 1)
							{
								strcat(fpath, ".txt");
							}else{
								strcat(fpath, ".m3u");
							}
						}
					}else{
						if(lofn.nFilterIndex == 1)
						{
							strcat(fpath, ".txt");
						}else{
							strcat(fpath, ".m3u");
						}
					}

					playlist_t_save_current(fpath);
				}
			}
			break;

		case BaseSkin_PlaylistButton_Options:
			settings_ui_show();
			break;

		}
	}

	if(pointid > 0 && pointid < BaseSkin_Playlist_ButtonCount && msg == 5) /* right click */
	{
		BaseSkin_LocalBlitCoord(PlaylistWindowDC, &SkinPlaylistButtons[pointid + BaseSkin_Playlist_MemoryShift_Normals], MemoryDC_Actions);
		BaseSkin_Playlist_LastActionButton = 0;

		switch(pointid)
		{
		case BaseSkin_PlaylistButton_Insert:
			{
				char       fpath[260];
				BROWSEINFO bi;
				LPITEMIDLIST lpi;

				fpath[0] = 0;

				bi.hwndOwner = Window_Playlist;
				bi.lpszTitle = "Add to playlist.";
				bi.pszDisplayName = fpath;
				bi.lpfn = 0;
				bi.iImage = 0;
				bi.lParam = 0;
				bi.pidlRoot = 0;
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
	
				lpi = SHBrowseForFolder(&bi);
				SHGetPathFromIDList(lpi, fpath);

				if(strlen(fpath))
				{
					AddDirectory(fpath);
				}
				fennec_refresh(fennec_v_refresh_force_high);
			}
			return 1;

		case BaseSkin_PlaylistButton_Repeat:
			settings.player.playlist_repeat_list ^= 1;
			break;

		case BaseSkin_PlaylistButton_Sort:
			{
#				define getrand(max) (rand() % (int)((max) + 1))

				unsigned int i, c, b, p;
			
				srand(timeGetTime());
				c = audio_playlist_getcount();
				p = audio_playlist_getcurrentindex();

				for(i=0; i<c; i++)
				{
					b = getrand(c);

					if(b >= c)continue;

					if((i == p) || (b == p))continue;

					audio_playlist_exchange(b, i);
				}

				fennec_refresh(fennec_v_refresh_force_high);
			}
			break;

		}
	}
	return 1;
}

int Local_Playlist_MouseMessage(int x, int y, int msg)
{
	static unsigned long lastcitem = 0;
	static unsigned long lastmdown = 0;
//	char*  str;
	HRGN   crgn;
	unsigned long  citem = Playlist_FirstItem + (y / PlaylistDisplay_ItemHeight);
	unsigned long  cditem = y / PlaylistDisplay_ItemHeight;
	char   pls_title[512];
	char   tstyle[512];

	if(msg == 1)
	{
		lastmdown = 1;
		Playlist_SelectedItem = citem;

	/* right down */

	}else if(msg == 4){ /* mouse up */

		lastmdown = 0;
	
	}else if(msg == 3){ /* double click */

		if(audio_playlist_switch_list(citem))audio_play();
	
	}else if(msg == 0){

		if(lastmdown) /* drag */
		{
			audio_playlist_move(citem, Playlist_SelectedItem);

			Playlist_SelectedItem = citem;
			BaseSkin_Playlist_ClearTitleCache();
			Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, Playlist_FirstItem);
			BitBlt(PlaylistWindowDC, PlaylistDisplay_X, PlaylistDisplay_Y, PlaylistDisplay_Width, PlaylistDisplay_Height, PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, SRCCOPY);

		}else{

			if(citem == lastcitem)return 0;
			
			Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, Playlist_FirstItem);

			if(audio_playlist_getcount() <= (citem))return 0;

			memset(pls_title, 0, sizeof(pls_title));
			memset(tstyle, 0, sizeof(tstyle));
			itoa(citem + 1, tstyle, 10);
			strcat(tstyle, ". ");
			strcat(tstyle, settings.formatting.playlist_item);
			local_settitle(citem, tstyle);
			
			if((cditem) & 1) /* odd */
			{
				Local_FastRect(PlaylistMemoryDC, Color_Playlist_Moving_OBackground, PlaylistDisplay_X, PlaylistDisplay_Y + (cditem * PlaylistDisplay_ItemHeight), PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
				SetTextColor(PlaylistMemoryDC, Color_Playlist_Moving_OForeground);
			}else{
				Local_FastRect(PlaylistMemoryDC, Color_Playlist_Moving_EBackground, PlaylistDisplay_X, PlaylistDisplay_Y + (cditem * PlaylistDisplay_ItemHeight), PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
				SetTextColor(PlaylistMemoryDC, Color_Playlist_Moving_EForeground);
			}
			
			
			SetBkMode(PlaylistMemoryDC, TRANSPARENT);

			crgn = CreateRectRgn(PlaylistDisplay_X, PlaylistDisplay_Y, PlaylistDisplay_X + PlaylistDisplay_Width, PlaylistDisplay_Y + PlaylistDisplay_Height);
			SelectClipRgn(PlaylistMemoryDC, crgn);

			/* draw text with x + 2 extent */
	  		TextOut(PlaylistMemoryDC, PlaylistDisplay_X + 2, PlaylistDisplay_Y + (cditem * PlaylistDisplay_ItemHeight), tstyle, (int)strlen(tstyle));

			SelectClipRgn(PlaylistMemoryDC, 0);
			DeleteObject(crgn);	

		}
	}
	BitBlt(PlaylistWindowDC, PlaylistDisplay_X, PlaylistDisplay_Y, PlaylistDisplay_Width, PlaylistDisplay_Height, PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, SRCCOPY);

	lastcitem = citem;
	
	return 1;
}

int BaseSkin_Playlist_ClearTitleCache(void)
{
	unsigned long i;
	if(!Window_Playlist)return 0;

	for(i=0; i<=(unsigned int)PlaylistDisplay_Max; i++)
	{
		Playlist_ItemData[i].itemid = (unsigned long)-1;
	}
	return 1;
}

int local_settitle(unsigned long i, string txt)
{
	struct fennec_audiotag at;
	unsigned long ipid;
	unsigned long j, fnd = (unsigned long)-1;

	if(!Window_Playlist)return 0;

	for(j=0; j<=(unsigned int)PlaylistDisplay_Max; j++)
	{
		if(Playlist_ItemData[j].itemid == i)
		{
			fnd = j;
			break;
		}
	}

	if(fnd == -1)
	{
direct_ret:

		at.tag_album.tsize           = 0;
		at.tag_title.tsize           = 0;
		at.tag_album.tsize           = 0;
		at.tag_artist.tsize          = 0;
		at.tag_origartist.tsize      = 0;
		at.tag_composer.tsize        = 0;
		at.tag_lyricist.tsize        = 0;
		at.tag_band.tsize            = 0;
		at.tag_copyright.tsize       = 0;
		at.tag_publish.tsize         = 0;
		at.tag_encodedby.tsize       = 0;
		at.tag_genre.tsize           = 0;
		at.tag_year.tsize            = 0;
		at.tag_url.tsize             = 0;
		at.tag_offiartisturl.tsize   = 0;
		at.tag_filepath.tsize        = 0;
		at.tag_filename.tsize        = 0;
		at.tag_comments.tsize        = 0;
		at.tag_lyric.tsize           = 0;
		at.tag_bpm.tsize             = 0;
		at.tag_tracknum.tsize        = 0;

		ipid = audio_input_tagread(audio_playlist_getsource(i), &at);

		tags_translate(txt, &at);

		j = i - Playlist_FirstItem;

		if(j >= 0 && j <= (unsigned int)PlaylistDisplay_Max)
		{
			Playlist_ItemData[j].itemid = i;
			str_cpy(Playlist_ItemData[j].itext, txt);
		}

		audio_input_tagread_known(ipid, 0, &at); /* free */
	}else{
		if(str_len(Playlist_ItemData[fnd].itext))
		{
			str_cpy(txt, Playlist_ItemData[fnd].itext);
			return 1;
		}else{
			goto direct_ret;
		}

	}
	return 1;
}

unsigned long Local_Playlist_Draw(HDC dc, int x, int y, unsigned long fitem)
{
	unsigned long i;
	unsigned long icount;
	int t, yp = y;
	HRGN crgn;
	letter  pls_title[512];
	letter  tstyle[512];

	if(fitem == 0)fitem = 1; /* first item is 1 not 0 */

	t = (int)(((((float)Playlist_FirstItem) / ((float)audio_playlist_getcount() - PlaylistDisplay_Max)) * ((float)(PlaylistDisplay_Height - 30))) + 15);

	Local_FastRect(PlaylistWindowDC, Color_Playlist_NormalItem_EBackground, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + 10, 10, PlaylistDisplay_Height - 20);
	Local_FastRect(PlaylistWindowDC, Color_Playlist_SelectedItem_Background, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + t - 5, 10, 10);

	Local_FastRect(PlaylistMemoryDC, Color_Playlist_NormalItem_EBackground, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + 10, 10, PlaylistDisplay_Height - 20);
	Local_FastRect(PlaylistMemoryDC, Color_Playlist_SelectedItem_Background, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + t - 5, 10, 10);


	icount = min(audio_playlist_getcount(), (unsigned long)PlaylistDisplay_Max);

	crgn = CreateRectRgn(x, y, x + PlaylistDisplay_Width, y + PlaylistDisplay_Height);
	SelectClipRgn(dc, crgn);

	for(i=Playlist_FirstItem; i<icount + Playlist_FirstItem; i++)
	{
		memset(pls_title, 0, sizeof(pls_title));
		memset(tstyle, 0, sizeof(tstyle));
		_itow(i + 1, tstyle, 10);
		str_cat(tstyle, uni(". "));
		str_cat(tstyle, settings.formatting.playlist_item);
		local_settitle(i, tstyle);
		//general_playlist_maketitle(i, pls_title, tstyle);

		if(i == Playlist_SelectedItem)
		{
			if(yp <= PlaylistDisplay_Width - PlaylistDisplay_ItemHeight)
			{
				Local_FastRect(dc, Color_Playlist_SelectedItem_Background, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
				SetTextColor(dc, Color_Playlist_SelectedItem_Foreground);
			}

		}else if(i == audio_playlist_getcurrentindex()){

			Local_FastRect(dc, Color_Playlist_PlayingItem_Background, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
			SetTextColor(dc, Color_Playlist_PlayingItem_Foreground);
		}else{
			if(!(i & 1)) /* odd */
			{
				Local_FastRect(dc, Color_Playlist_NormalItem_OBackground, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
				SetTextColor(dc, Color_Playlist_NormalItem_OForeground);
			}else{
				Local_FastRect(dc, Color_Playlist_NormalItem_EBackground, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
				SetTextColor(dc, Color_Playlist_NormalItem_EForeground);
			}
		}
		
		SetBkMode(dc, TRANSPARENT);

		/* draw text with x + 2 extent */
		TextOut(dc, x + 2, yp, tstyle, (int)str_len(tstyle));

		yp += PlaylistDisplay_ItemHeight;
	}

	for(i=icount; i<(unsigned long)PlaylistDisplay_Max; i++)
	{
		if(i == Playlist_SelectedItem)
		{
			Local_FastRect(dc, Color_Playlist_SelectedItem_Background, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
		}else{
			if(!(i & 1)) /* odd */
			{
				Local_FastRect(dc, Color_Playlist_NormalItem_OBackground, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
			}else{
				Local_FastRect(dc, Color_Playlist_NormalItem_EBackground, x, yp, PlaylistDisplay_Width, PlaylistDisplay_ItemHeight);
			}
		}
		yp += PlaylistDisplay_ItemHeight;
	}



	SelectClipRgn(dc, 0);
	DeleteObject(crgn);	
	return fitem; /* the first item placed */
}

int Local_Playlist_Scroll(float pfrac)
{
	unsigned long ac;

	if(pfrac == -1.0f)
	{
		Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, 0);
		BitBlt(PlaylistWindowDC, 0, 0, PlaylistWindow_Width, PlaylistWindow_Height, PlaylistMemoryDC, 0, 0, SRCCOPY);
		Sleep(0);
		return 1;
	}
	
	if(pfrac > 1.0f)pfrac = 1.0f;
	if(pfrac < 0.0f)pfrac = 0.0f;
	ac = audio_playlist_getcount();

	if(ac < (unsigned long)PlaylistDisplay_Max)
	{
		Playlist_FirstItem = 0;
		return 0;
	}

	Playlist_FirstItem = (unsigned long)(pfrac * (ac - PlaylistDisplay_Max));
	Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, 0);
	BitBlt(PlaylistWindowDC, 0, 0, PlaylistWindow_Width, PlaylistWindow_Height, PlaylistMemoryDC, 0, 0, SRCCOPY);
	Sleep(0);
	return 1;
}

/* callback functions */

int WINAPI PlaylistWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int mldown = 0;
	static int mx = 0;
	static int my = 0;
	static int plscroll = 0;

	switch(msg)
	{
	case WM_MOUSEMOVE:
		if(LOWORD(lParam) >= PlaylistDisplay_X && LOWORD(lParam) <= PlaylistDisplay_X + PlaylistDisplay_Width &&
		   HIWORD(lParam) >= PlaylistDisplay_Y && HIWORD(lParam) <= PlaylistDisplay_Y + PlaylistDisplay_Height)
		{
			unsigned int i = ((HIWORD(lParam) - PlaylistDisplay_Y) / PlaylistDisplay_ItemHeight) + Playlist_FirstItem;
			
			if(i < audio_playlist_getcount())
			{
				static int offs = 0;
				static unsigned int lasti = (unsigned int)-1;

				if(lasti != i)
				{
					offs ^= 1;
					lasti = i;
				}

				strcpy(playlist_tip_text + offs, Playlist_ItemData[i - Playlist_FirstItem].itext);
				strcat(playlist_tip_text + offs, "\n\r\n\r");
				strcat(playlist_tip_text + offs, audio_playlist_getsource(i));

				tips_display(0, 0, playlist_tip_text + offs);
			}
		}

		if(mldown)
		{
			POINT pt;
			unsigned long ptf;
			DockWindowPoints dpts[1];
			int x,y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if(plscroll)
			{
				int t, v = y - PlaylistDisplay_Y;
				if(v > PlaylistDisplay_Height - 15)v = PlaylistDisplay_Height - 15;
				if(v < 15)v = 15;
	
				Local_Playlist_Scroll(((float)v - 15) / (((float)(PlaylistDisplay_Height - 30))));

				t = (int)(((((float)Playlist_FirstItem) / ((float)audio_playlist_getcount() - PlaylistDisplay_Max)) * ((float)(PlaylistDisplay_Height - 30))) + 15);

				if(PlaylistDisplay_Y + t - 5 >= PlaylistDisplay_Y + 10)
				{
					Local_FastRect(PlaylistWindowDC, Color_Playlist_NormalItem_EBackground, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + 10, 10, PlaylistDisplay_Height - 20);
					Local_FastRect(PlaylistWindowDC, Color_Playlist_SelectedItem_Background, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + t - 5, 10, 10);

					Local_FastRect(PlaylistMemoryDC, Color_Playlist_NormalItem_EBackground, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + 10, 10, PlaylistDisplay_Height - 20);
					Local_FastRect(PlaylistMemoryDC, Color_Playlist_SelectedItem_Background, PlaylistDisplay_X + PlaylistDisplay_Width + 2, PlaylistDisplay_Y + t - 5, 10, 10);
				}
				break;
			}

			if(x >= PlaylistDisplay_X && x <= PlaylistDisplay_X + PlaylistDisplay_Width &&
			   y >= PlaylistDisplay_Y && y <= PlaylistDisplay_Y + PlaylistDisplay_Height)
			{
				Local_Playlist_MouseMessage(x - PlaylistDisplay_X, y - PlaylistDisplay_Y, 4);
			}else{

				GetCursorPos(&pt);

				//c - 48,29
				//p - 393, 180

				dpts[0].cx = 0;
				dpts[0].cy = 0;
				dpts[0].px = 346 + settings.environment.main_window_x;
				dpts[0].py = 151 + settings.environment.main_window_y;


				ptf = MoveWindow_Docking(hwnd, dpts, 1, 10, &pt.x, &pt.y, mx, my);

				settings.environment.playlist_window_x = pt.x;
				settings.environment.playlist_window_y = pt.y;

				if(ptf != 0xFFFF)
				{
					settings.environment.playlist_window_state = setting_window_docked;
				}else{
					settings.environment.playlist_window_state = setting_window_normal;
				}
			}
		}else{
			int x,y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(x >= PlaylistDisplay_X && x <= PlaylistDisplay_X + PlaylistDisplay_Width &&
			   y >= PlaylistDisplay_Y && y <= PlaylistDisplay_Y + PlaylistDisplay_Height)
			{
				Local_Playlist_MouseMessage(x - PlaylistDisplay_X, y - PlaylistDisplay_Y, 0);
			}else{

				Local_Playlist_WindowMouseMessage(x, y, 0);
			}

		}
		break;

	case WM_MOUSEWHEEL:
		{
			if(audio_playlist_getcount() <= (unsigned long)PlaylistDisplay_Max)break;

			if((short)HIWORD(wParam) < 0)
			{
				if((int)Playlist_FirstItem <= ((int)audio_playlist_getcount()) - PlaylistDisplay_Max - 3)
				{
					Playlist_FirstItem += 3;
				}else{
					Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
					
				}
				Local_Playlist_Scroll(-1.0f);

			}else{
				if(Playlist_FirstItem >= 3)
				{
					Playlist_FirstItem -= 3;	
				}else{
					Playlist_FirstItem = 0;
				}
				Local_Playlist_Scroll(-1.0f);
			}
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_DOWN:
			if(audio_playlist_getcount() <= (unsigned long)PlaylistDisplay_Max)break;

			if(Playlist_FirstItem <= audio_playlist_getcount() - PlaylistDisplay_Max - 1)
			{
				Playlist_FirstItem += 1;
			}else{
				Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
			}

			Local_Playlist_Scroll(-1.0f);
			break;

		case VK_UP:
			if(audio_playlist_getcount() <= (unsigned long)PlaylistDisplay_Max)break;

			if(Playlist_FirstItem >= 1)
			{
				Playlist_FirstItem -= 1;
			}else{
				Playlist_FirstItem = 0;
			}

			Local_Playlist_Scroll(-1.0f);
			break;

		case VK_RIGHT:
			if(Playlist_SelectedItem < Playlist_FirstItem) /* outta range */
			{
				Playlist_FirstItem = Playlist_SelectedItem;
				if(Playlist_FirstItem + PlaylistDisplay_Max >= audio_playlist_getcount())Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
			
			}else if(Playlist_SelectedItem > Playlist_FirstItem + PlaylistDisplay_Max){
				Playlist_FirstItem = Playlist_SelectedItem;
				if(Playlist_FirstItem + PlaylistDisplay_Max >= audio_playlist_getcount())Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
			
			}

			if(Playlist_SelectedItem - Playlist_FirstItem < (unsigned long)PlaylistDisplay_Max - 1)
			{
				if(Playlist_SelectedItem < audio_playlist_getcount() - 1)Playlist_SelectedItem++;
				Local_Playlist_Scroll(-1.0f);
				break;
			}

			if(audio_playlist_getcount() <= (unsigned long)PlaylistDisplay_Max)break;

			if(Playlist_FirstItem <= audio_playlist_getcount() - PlaylistDisplay_Max - 1)
			{
				Playlist_FirstItem += 1;
			}else{
				Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
			}

			if(Playlist_SelectedItem < audio_playlist_getcount() - 1)Playlist_SelectedItem++;
			
			Local_Playlist_Scroll(-1.0f);
			break;

		case VK_LEFT:
			if(Playlist_SelectedItem < Playlist_FirstItem) /* outta range */
			{
				Playlist_FirstItem = Playlist_SelectedItem;
				if(Playlist_FirstItem + PlaylistDisplay_Max >= audio_playlist_getcount())Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
			
			}else if(Playlist_SelectedItem > Playlist_FirstItem + PlaylistDisplay_Max){
				Playlist_FirstItem = Playlist_SelectedItem;
				if(Playlist_FirstItem + PlaylistDisplay_Max >= audio_playlist_getcount())Playlist_FirstItem = audio_playlist_getcount() - PlaylistDisplay_Max;
			
			
			}

			if(Playlist_SelectedItem - Playlist_FirstItem > 0)
			{
				if(Playlist_SelectedItem > 0)Playlist_SelectedItem--;
				Local_Playlist_Scroll(-1.0f);
				break;
			}

			if(audio_playlist_getcount() <= (unsigned long)PlaylistDisplay_Max)break;

			if(Playlist_FirstItem >= 1)
			{
				Playlist_FirstItem -= 1;
			}else{
				Playlist_FirstItem = 0;
			}

			if(Playlist_SelectedItem > 0)Playlist_SelectedItem--;
			Local_Playlist_Scroll(-1.0f);
			break;
		
		case VK_RETURN:
		case VK_SPACE:
			if(audio_playlist_switch(Playlist_SelectedItem))audio_play();
			break;

		default:
			if(!settings.shortcuts.enable_local)break;

			{
				unsigned short a;
				a = fennec_convertkey((unsigned short*)&wParam);

				if(GetKeyState(VK_SHIFT)   & 0x8000) a |= fennec_key_shift;
				if(GetKeyState(VK_CONTROL) & 0x8000) a |= fennec_key_control;
				if(GetKeyState(VK_MENU)    & 0x8000) a |= fennec_key_alternative;
				
				kb_action(a, settings.shortcuts.localkeys, sizeof(settings.shortcuts.localkeys) / sizeof(settings.shortcuts.localkeys[0]));
			}
			break;
		}

		
		break;

	case WM_CREATE:
		PlaylistWindowDC = GetDC(hwnd);

		{
			HBITMAP hmempic;
			BITMAP  bmp;

			PlaylistMemoryDC = CreateCompatibleDC(0);
			hmempic = png_res_load_winbmp(instance_fennec, uni("png"), uni("skin.window.playlist"));

			GetObject(hmempic, sizeof(BITMAP), &bmp);
			PlaylistWindow_Width  = bmp.bmWidth;
			PlaylistWindow_Height = bmp.bmHeight;


			BaseSkin_RegionPlaylist = CreateRectRgn(0, 0, 0, 0);
			BaseSkin_RegionPlaylist = Skin_GetBitmapRegion(hmempic);
			
			AdjustColors(hmempic, 0, settings.player.themecolor_h, settings.player.themecolor_s, settings.player.themecolor_l);

			SelectObject(PlaylistMemoryDC, hmempic);

			DeleteObject(hmempic);
		}

		SetWindowPos(hwnd, 0, settings.environment.playlist_window_x, settings.environment.playlist_window_y, PlaylistWindow_Width, PlaylistWindow_Height, SWP_NOZORDER);
		
		SetWindowRgn(hwnd, BaseSkin_RegionPlaylist, 1);

		BitBlt(PlaylistWindowDC, 0, 0, PlaylistWindow_Width, PlaylistWindow_Height, PlaylistMemoryDC, 0, 0, SRCCOPY);
		

		Font_Playlist_NormalItem =  CreateFont(-MulDiv(8, GetDeviceCaps(PlaylistMemoryDC, LOGPIXELSY), 72),
							0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,
							DEFAULT_PITCH ,"Verdana");

		SelectObject(PlaylistMemoryDC, Font_Playlist_NormalItem);
		break;

	case WM_PAINT:
		Local_Playlist_Draw(PlaylistMemoryDC, PlaylistDisplay_X, PlaylistDisplay_Y, 0);
		BitBlt(PlaylistWindowDC, 0, 0, PlaylistWindow_Width, PlaylistWindow_Height, PlaylistMemoryDC, 0, 0, SRCCOPY);
		break;

	case WM_LBUTTONDOWN:
		{
			int x,y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if(x >= PlaylistDisplay_X && x <= PlaylistDisplay_X + PlaylistDisplay_Width &&
				y >= PlaylistDisplay_Y && y <= PlaylistDisplay_Y + PlaylistDisplay_Height)
			{
				Local_Playlist_MouseMessage(x - PlaylistDisplay_X, y - PlaylistDisplay_Y, 1);
			}else{

				if(x >= PlaylistDisplay_X && x <= PlaylistDisplay_X + PlaylistDisplay_Width + PlaylistDisplay_ScrollWidth &&
				   y >= PlaylistDisplay_Y + PlaylistDisplay_ScrollButtonHeight && y <= PlaylistDisplay_Y + PlaylistDisplay_Height - PlaylistDisplay_ScrollButtonHeight)
				{
					int v = y - PlaylistDisplay_Y;
					if(v > PlaylistDisplay_Height)v = PlaylistDisplay_Height;
					if(v < 0)v = 0;
				
					plscroll = 1;
					mldown = 1;
					SetCapture(hwnd);

	
					Local_Playlist_Scroll(((float)v) / ((float)PlaylistDisplay_Height));
					break;
				}


				mx = LOWORD(lParam);
				my = HIWORD(lParam);
				
				if(!Local_Playlist_WindowMouseMessage(mx, my, 1)) /* not belong to actions */
				{
					mldown = 1;
					SetCapture(hwnd);
				}
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			int x,y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if(x >= PlaylistDisplay_X && x <= PlaylistDisplay_X + PlaylistDisplay_Width &&
				y >= PlaylistDisplay_Y && y <= PlaylistDisplay_Y + PlaylistDisplay_Height)
			{
				Local_Playlist_MouseMessage(x - PlaylistDisplay_X, y - PlaylistDisplay_Y, 3);
			}
		}

		Local_Playlist_WindowMouseMessage(LOWORD(lParam), HIWORD(lParam), 6);

		break;

	case WM_LBUTTONUP:
	
		if(mldown)ReleaseCapture();
		Local_Playlist_MouseMessage(0, 0, 4);
		mldown = 0;

		if(plscroll)
		{
			plscroll = 0;
			ReleaseCapture();
		}

		Local_Playlist_WindowMouseMessage(LOWORD(lParam), HIWORD(lParam), 2);
		break;
	
	case WM_RBUTTONUP:
		Local_Playlist_WindowMouseMessage(LOWORD(lParam), HIWORD(lParam), 5);
		
		if(mldown)ReleaseCapture();
		mldown = 0;
		if(plscroll)
		{
			plscroll = 0;
			ReleaseCapture();
		}
		break;

	}

	return (int)DefWindowProc(hwnd, msg, wParam, lParam);
}

void CALLBACK TimerProc_Position(HWND hwnd, UINT uMsg, UINT_PTR idEvent, unsigned long dwTime)
{
	if(Position_Mode == position_forward)
	{
		GlobalFunction(Function_Forward);
	
	}else if(Position_Mode == position_rewind){
		
		GlobalFunction(Function_Rewind);
	}
}

int FennecWndMain_GiveMessage(int msgowner)
{
	if(msgowner < 0)return FennecDefaultMessage;
	FennecDefaultMessage = msgowner;
	return 1;
}

void MainWindow_OnMove(HWND hwnd,WPARAM wParam, LPARAM lParam)
{
	if(proc_busy)return;

	if(settings.skins.selected[0])return;

	movePos.x = LOWORD(lParam);
	movePos.y = HIWORD(lParam);

	if(FennecDefaultMessage & DISPLAY_MOUSEMESSAGE)
	{
		POINT tmppt;
		tmppt.x = movePos.x - Coord_DisplayX;
		tmppt.y = movePos.y - Coord_DisplayY;
		Display_MouseMessage(tmppt,Display_Mouse_Move);
		return;
	}

	if(movePos.x >= Coord_DisplayX && movePos.y >= Coord_DisplayY &&
		movePos.x <= (Coord_DisplayW + Coord_DisplayX) && movePos.y <= (Coord_DisplayH + Coord_DisplayY) && (!onmoving))
	{
		POINT tmppt;
		tmppt.x = movePos.x - Coord_DisplayX;
		tmppt.y = movePos.y - Coord_DisplayY;
        Display_MouseMessage(tmppt,Display_Mouse_Move);

	}else{

		if(wParam & MK_LBUTTON)
		{
			if(!last_action)
			{
				onmoving = 1;

				GetClientRect(hwnd,&rClt);

				SystemParametersInfo(SPI_GETWORKAREA,0,&rWork,0);

				GetCursorPos(&movePos);

				movePos.x = movePos.x - downPos.x;
				movePos.y = movePos.y - downPos.y;

				if(OPTION_Dock == 1)
				{
					if(movePos.x <= OPTION_DockChange && movePos.x >= -OPTION_DockChange) movePos.x = 0;
					if(movePos.y <= OPTION_DockChange && movePos.y >= -OPTION_DockChange) movePos.y = 0;
					if(movePos.x >= screenSize.cx - OPTION_DockChange - rClt.right && movePos.x <= screenSize.cx + OPTION_DockChange - rClt.right) movePos.x = screenSize.cx - rClt.right;
					if(movePos.y >= rWork.bottom - OPTION_DockChange - rClt.bottom && movePos.y <= rWork.bottom + OPTION_DockChange - rClt.bottom) movePos.y = rWork.bottom - rClt.bottom;
				}
				hpos = BeginDeferWindowPos(1);
				DeferWindowPos(hpos,hwnd,0,movePos.x,movePos.y,0,0,SWP_NOSIZE |  SWP_NOACTIVATE);
				
				/* [todo] to handle all windows */
				if(settings.environment.playlist_window_state == setting_window_docked)
				{
					DeferWindowPos(hpos, Window_Playlist, 0, movePos.x + 346, movePos.y + 151, 0, 0, SWP_NOSIZE |  SWP_NOACTIVATE);
					settings.environment.playlist_window_x = movePos.x + 346;
					settings.environment.playlist_window_y = movePos.y + 151;
					
				}
					
				EndDeferWindowPos(hpos);
				Sleep(0);
				SendMessage(hwnd,WM_PAINT,0,0);
				Sleep(0);

				settings.environment.main_window_x = movePos.x;
				settings.environment.main_window_y = movePos.y;

				

			}
		}else{
			/*BaseSkin_MouseMessage(movePos,(WORD)wParam);*/
			BaseSkin_MouseMessage(movePos.x, movePos.y, 3);
		}
	}
	return;
}

LRESULT CALLBACK callback_base_skin(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	EqualizerPreset eqp;

	switch(umsg)
	{
	case WM_MOUSEMOVE:
		MainWindow_OnMove(hwnd,wParam,lParam);
		break;

	case WM_PAINT:
		BaseSkin_Redraw(1,0,0,0,0);
		Display_Refresh(fennec_v_refresh_force_not);
		break;
	
	case WM_LBUTTONDOWN:
		proc_busy = 1;

		downPos.x = LOWORD(lParam);
		downPos.y = HIWORD(lParam);

		SetCapture(hwnd);

		SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCLIENT, 0);

		if(movePos.x >= Coord_DisplayX && movePos.y >= Coord_DisplayY &&
		   movePos.x <= (Coord_DisplayW + Coord_DisplayX) &&
		   movePos.y <= (Coord_DisplayH + Coord_DisplayY))
		{
			tmppt.x = movePos.x - Coord_DisplayX;
			tmppt.y = movePos.y - Coord_DisplayY;
			Display_MouseMessage(tmppt,Display_Mouse_LDown);
			last_action = 1; /* there is an action */

		}else{
			movePos.x = LOWORD(lParam);
			movePos.y = HIWORD(lParam);
			last_action = BaseSkin_MouseMessage(movePos.x, movePos.y, 1) ? 1 : 0;
		}

		proc_busy = 0;
		break;

	case WM_RBUTTONDOWN:
		menupt.x = LOWORD(lParam);
		menupt.y = HIWORD(lParam);

		SetCapture(hwnd);

		tmppt.x = LOWORD(lParam) - Coord_DisplayX;
		tmppt.y = HIWORD(lParam) - Coord_DisplayY;

		if(movePos.x >= Coord_DisplayX && movePos.y >= Coord_DisplayY &&
		   movePos.x <= (Coord_DisplayW + Coord_DisplayX) &&
		   movePos.y <= (Coord_DisplayH + Coord_DisplayY))
		{
			if(Display_MouseMessage(tmppt, Display_Mouse_RDown))break;
		}

		proc_busy = 1;

		if(BaseSkin_MouseMessage(menupt.x ,menupt.y, 4))
		{
			proc_busy = 0;
			break;
		}

		ClientToScreen(hwnd,&menupt);
		
		TrackPopupMenu(menu_main,TPM_LEFTALIGN | TPM_LEFTBUTTON,menupt.x,menupt.y,0,hwnd,NULL);

		proc_busy = 0;
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		movePos.x = LOWORD(lParam);
		movePos.y = HIWORD(lParam);

		tmppt.x = -1;
		tmppt.y = -1;

		Display_Debug(LOWORD(lParam));

		BaseSkin_MouseMessage(movePos.x, movePos.y, 5);

		tmppt.x = movePos.x - Coord_DisplayX;
		tmppt.y = movePos.y - Coord_DisplayY;

		if(umsg == WM_LBUTTONUP)
		{
			Display_MouseMessage(tmppt,Display_Mouse_LUp);
		}else{
			Display_MouseMessage(tmppt,Display_Mouse_RUp);
		}

		last_action = 0;
		onmoving = 0;

		ReleaseCapture();
		break;

	case WM_COMMAND:

		if(wParam >= GeneralMenu_Equalizer_PresetSelect && wParam <= (unsigned long)(GeneralMenu_Equalizer_PresetSelect + settings.player.equalizer_presets))
		{
			unsigned long retz = 0;
			settings_data_get(setting_id_equalizer_preset, ((unsigned long)wParam) - GeneralMenu_Equalizer_PresetSelect, &eqp, &retz);

			memset(settings.player.equalizer_last_preset, 0, sizeof(settings.player.equalizer_last_preset));
			strcpy(settings.player.equalizer_last_preset, eqp.pname);

			settings.player.equalizer_last_preset_id = (unsigned short)(wParam - GeneralMenu_Equalizer_PresetSelect);

			SetEqualizer(0,eqp.pbands,10);

			settings.player.equalizer_last_bands[ 0] = eqp.pbands[ 0];
			settings.player.equalizer_last_bands[ 1] = eqp.pbands[ 1];
			settings.player.equalizer_last_bands[ 2] = eqp.pbands[ 2];
			settings.player.equalizer_last_bands[ 3] = eqp.pbands[ 3];
			settings.player.equalizer_last_bands[ 4] = eqp.pbands[ 4];
			settings.player.equalizer_last_bands[ 5] = eqp.pbands[ 5];
			settings.player.equalizer_last_bands[ 6] = eqp.pbands[ 6];
			settings.player.equalizer_last_bands[ 7] = eqp.pbands[ 7];
			settings.player.equalizer_last_bands[ 8] = eqp.pbands[ 8];
			settings.player.equalizer_last_bands[ 9] = eqp.pbands[ 9];
			settings.player.equalizer_last_bands[10] = eqp.pbands[10];
			settings.player.equalizer_last_bands[11] = eqp.pbands[11];
			settings.player.equalizer_last_bands[12] = eqp.pbands[12];
			settings.player.equalizer_last_bands[13] = eqp.pbands[13];
			settings.player.equalizer_last_bands[14] = eqp.pbands[14];
			settings.player.equalizer_last_bands[15] = eqp.pbands[15];
			settings.player.equalizer_last_bands[16] = eqp.pbands[16];
			settings.player.equalizer_last_bands[17] = eqp.pbands[17];
			settings.player.equalizer_last_bands[18] = eqp.pbands[18];
			settings.player.equalizer_last_bands[19] = eqp.pbands[19];
			settings.player.equalizer_last_bands[20] = eqp.pbands[20];
			settings.player.equalizer_last_bands[21] = eqp.pbands[21];
			Display_Refresh(fennec_v_refresh_force_not);
		}

		switch(wParam)
		{
		case GeneralMenu_Equalizer_DeletePreset:
			if(settings.player.equalizer_last_preset_id < settings.player.equalizer_presets)
			{
				settings_data_remove(setting_id_equalizer_preset, settings.player.equalizer_last_preset_id);
				settings.player.equalizer_presets--;

				Display_EqFlush = 1;
				Display_Refresh(fennec_v_refresh_force_not);
			}
			break;
		
		case GeneralMenu_Equalizer_RenamePreset:
			if(settings.player.equalizer_last_preset_id < settings.player.equalizer_presets)
			{
				char* uinret = basewindows_getuserinput("Rename Equalizer Preset", "Preset name :", "");
				
				if(uinret)
				{
					if(strlen(uinret))
					{
						settings_data_set(setting_id_equalizer_preset, settings.player.equalizer_last_preset_id, uinret);
				
						Display_EqFlush = 1;
						Display_Refresh(fennec_v_refresh_force_not);	
					}
				}
			}
			break;

		case GeneralMenu_Equalizer_SaveToDisc:
			{
				unsigned int i;
				char         fpath[1024];
				OPENFILENAME lofn;
				HANDLE       hfile;
				unsigned long        nbr = 0;
				unsigned long        pcount;

				memset(&lofn, 0, sizeof(lofn));

				fpath[0] = 0;

				lofn.lStructSize     = sizeof(lofn);
				lofn.lpstrTitle      = "Save Equalizer Presets File";
				lofn.hwndOwner       = window_main;
				lofn.lpstrFile       = fpath;
				lofn.nMaxFile        = sizeof(fpath);
				lofn.lpstrFilter     = "Equalizer Preset (*.feq)\0*.feq";
				lofn.nFilterIndex    = 0;
				lofn.lpstrFileTitle  = 0;
				lofn.nMaxFileTitle   = 0;
				lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
				lofn.hInstance       = instance_fennec;

				GetSaveFileName(&lofn);

				if(strlen(fpath))
				{
					hfile = CreateFile(fpath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);

					if(hfile != INVALID_HANDLE_VALUE)
					{
						EqualizerPreset eqp;

						pcount = settings.player.equalizer_presets;

						WriteFile(hfile, &pcount, sizeof(unsigned long), &nbr, 0);

						for(i=0; i<settings.player.equalizer_presets; i++)
						{
							nbr = sizeof(EqualizerPreset);
							settings_data_get(setting_id_equalizer_preset, i, &eqp, &nbr);
							WriteFile(hfile, &eqp, sizeof(EqualizerPreset), &nbr, 0);
						}

						CloseHandle(hfile);
					}
				}
			}
			break;

		case GeneralMenu_Equalizer_Clean:
			{
				unsigned int i, c = settings.player.equalizer_presets;

				for(i=0; i<c; i++)
				{
					settings_data_remove(setting_id_equalizer_preset, settings.player.equalizer_last_preset_id);
					settings.player.equalizer_presets--;
				}

				Display_EqFlush = 1;
				Display_Refresh(fennec_v_refresh_force_not);
			}
			break;

		case GeneralMenu_Equalizer_LoadFromDisc:
			{
				unsigned int i;
				char         fpath[1024];
				OPENFILENAME lofn;
				HANDLE       hfile;
				unsigned long        nbr = 0;
				unsigned long        pcount;

				memset(&lofn, 0, sizeof(lofn));

				fpath[0] = 0;

				lofn.lStructSize     = sizeof(lofn);
				lofn.lpstrTitle      = "Load Equalizer Presets File";
				lofn.hwndOwner       = window_main;
				lofn.lpstrFile       = fpath;
				lofn.nMaxFile        = sizeof(fpath);
				lofn.lpstrFilter     = "Equalizer Preset (*.feq)\0*.feq";
				lofn.nFilterIndex    = 0;
				lofn.lpstrFileTitle  = 0;
				lofn.nMaxFileTitle   = 0;
				lofn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
				lofn.hInstance       = instance_fennec;

				GetOpenFileName(&lofn);

				if(strlen(fpath))
				{
					hfile = CreateFile(fpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

					if(hfile != INVALID_HANDLE_VALUE)
					{
						int x;
						EqualizerPreset eqp;

						ReadFile(hfile, &pcount, sizeof(unsigned long), &nbr, 0);
						
						x = (int)settings.player.equalizer_presets;
						x += pcount;

						settings.player.equalizer_presets = (unsigned short)x;

						for(i=0; i<pcount; i++)
						{
							ReadFile(hfile, &eqp, sizeof(EqualizerPreset), &nbr, 0);

							settings_data_add(setting_id_equalizer_preset, &eqp, sizeof(EqualizerPreset));
							
						}
						CloseHandle(hfile);
					}
				}
			}
			break;

		case GeneralMenu_Equalizer_SavePreset:
				{
					char* uinret = basewindows_getuserinput("Save Equalizer Preset", "Preset name :", "");
					if(strlen(uinret))
					{
						memcpy(eqp.pname, uinret, 30);
						eqp.pbands[ 0] = settings.player.equalizer_last_bands[ 0];
						eqp.pbands[ 1] = settings.player.equalizer_last_bands[ 1];
						eqp.pbands[ 2] = settings.player.equalizer_last_bands[ 2];
						eqp.pbands[ 3] = settings.player.equalizer_last_bands[ 3];
						eqp.pbands[ 4] = settings.player.equalizer_last_bands[ 4];
						eqp.pbands[ 5] = settings.player.equalizer_last_bands[ 5];
						eqp.pbands[ 6] = settings.player.equalizer_last_bands[ 6];
						eqp.pbands[ 7] = settings.player.equalizer_last_bands[ 7];
						eqp.pbands[ 8] = settings.player.equalizer_last_bands[ 8];
						eqp.pbands[ 9] = settings.player.equalizer_last_bands[ 9];
						eqp.pbands[10] = settings.player.equalizer_last_bands[10];
						eqp.pbands[11] = settings.player.equalizer_last_bands[11];
						eqp.pbands[12] = settings.player.equalizer_last_bands[12];
						eqp.pbands[13] = settings.player.equalizer_last_bands[13];
						eqp.pbands[14] = settings.player.equalizer_last_bands[14];
						eqp.pbands[15] = settings.player.equalizer_last_bands[15];
						eqp.pbands[16] = settings.player.equalizer_last_bands[16];
						eqp.pbands[17] = settings.player.equalizer_last_bands[17];
						eqp.pbands[18] = settings.player.equalizer_last_bands[18];
						eqp.pbands[19] = settings.player.equalizer_last_bands[19];
						eqp.pbands[20] = settings.player.equalizer_last_bands[20];
						eqp.pbands[21] = settings.player.equalizer_last_bands[21];
						settings_data_add(setting_id_equalizer_preset, &eqp, sizeof(EqualizerPreset));
						settings.player.equalizer_presets++;

						Display_EqFlush = 1;
						Display_Refresh(fennec_v_refresh_force_not);
					}
				}
				break;
		}
		break;
	}

	return 1;

}

int SwitchToBaseSkin(int initialize)
{
	if(settings.skins.selected[0])
	{
		settings.skins.selected[0] = 0;
		
		
		SetWindowLong(window_main, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MINIMIZEBOX);
		SetWindowPos(window_main, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
		
		skins_uninitialize();

		BaseSkin_Initialize();

		Display_Initialize();
		Display_ShowPanel(PanelID_Main);

		BaseSkin_ApplySkin();
		BaseSkin_SetColor(GetRValue(settings.player.themecolor), GetGValue(settings.player.themecolor), GetBValue(settings.player.themecolor));
		
		skinproc = (WNDPROC)callback_base_skin;

		Display_Refresh(fennec_v_refresh_force_not);
	}else if(initialize){
		skinproc = (WNDPROC)callback_base_skin;
		
		BaseSkin_Initialize();

		Display_Initialize();
		Display_ShowPanel(PanelID_Main);

		BaseSkin_ApplySkin();
		BaseSkin_SetColor(GetRValue(settings.player.themecolor), GetGValue(settings.player.themecolor), GetBValue(settings.player.themecolor));
		
		skinproc = (WNDPROC)callback_base_skin;

		Display_Refresh(fennec_v_refresh_force_not);
	}
	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/
