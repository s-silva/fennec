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

Return value types

    Return 1 : (int)

    1 - all right
    0 - error and function process stopped message will be displayed with log or
        message box

Data type information for this file

    1. Panel ID : unsigned long (unsigned long/32bit unsigned)
    2. Components (type id) : int for calls, unsigned char to storage
    3. Coordinates : int (unsigned short can be used for storage, just for the
                    coordinates inside the display)

Specific names

    1. npanel  : panel id for calls, also represents 'new panel'.
    2. comptid : component type id
    3. compid  : component id (array index)

Specific prefixes

    1. 'PanelID_' - id of a panel, double word data.
    2. 'Coord_'   - coordinate (define for static, variable for dynamic)
    3. 'Local_'   - local function for the file
    4. 'Comp_'    - Component type id

Information on file

    ComponentProc : int(int compid,unsigned long msg,unsigned long hdata,void* data)
----------------------------------------------------------------------------**/

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "base skin.h"
#include "Fennec Audio.h"
#include "fennec help.h"

/* type definitions */

typedef  int (__stdcall * ComponentProc) (int,unsigned long,unsigned long,void*);

/* structures */

typedef  struct _DisplayComponent
{
    int   comptid;                     /* component id */
    int   subid;                       /* id 2 (eg: button id) */
    int   dx;                          /* destination x */
    int   dy;                          /* destination y */
    char* ctip;                        /* tip text */
    int   value;                       /* current value or state */
    int   ndata;                       /* numeric data */
	char  inuse;
    char* cap;                         /* caption/title text pointer */
    ComponentProc cproc;
}DisplayComponent;

typedef  struct _Coord
{
    int   x;
    int   y;
    int   w;
    int   h;
}Coord;

int display_panelhistory[16];
int display_panelhistory_current = 0;
int display_panelhistory_count   = 0;

/* defines */

#define  ComponentCountAdd   5         /* component memory increase by */
#define  ComponentCountInit  10        /* component memory increase by */

#define  ResizeMode_Null        0x0    /* no resize */
#define  ResizeMode_Horizontal  0x1    /* resize horizontally */
#define  ResizeMode_Vertical    0x2    /* resize vertically */
#define  ResizeMode_Both        0x3    /* full resize */

/* defined in "Fennec Main.h" for global usage

#define  PanelID_Null      0x0
#define  PanelID_Main      0x1
#define  PanelID_Playlist  0x2
#define  PanelID_Equalizer 0x3
#define  PanelID_MediaInfo 0x4
#define  PanelID_Color     0x5

*/

#define  Comp_Null         0x0         /* null, not used as a component type id */
#define  Comp_Button       0x1         /* push button component */
#define  Comp_ScrollH      0x2         /* horizontal scroll bar */
#define  Comp_ScrollV      0x3         /* vertical scroll bar */
#define  Comp_TriScrollV   0x4         /* Triangular vertical scroll bar */
#define  Comp_LevelH       0x5         /* horizontal level bar */
#define  Comp_LevelV       0x6         /* vertical level bar */
#define  Comp_KnobL        0x7         /* leveling knob */
#define  Comp_KnobB        0x8         /* balancing knob */
#define  Comp_Check        0x9         /* check box */
#define  Comp_Option       0xA         /* option (radio) button */
#define  Comp_CheckFill    0xB         /* check box with filled background for text */
#define  Comp_OptionFill   0xC         /* option (radio) button filled background for text */

#define  CompData_ButtonRecord   0x1   /* record button (option/dialog) */
#define  CompData_ButtonMute     0x2   /* mute button (option) */
#define  CompData_ButtonDisk     0x3   /* disc button (option/dialog) */
#define  CompData_ButtonSWave    0x4   /* sonaz wave out button (option/dialog) */
#define  CompData_ButtonMixer    0x5   /* mixer button (option/long time) */
#define  CompData_ButtonAutoMix  0x6   /* auto mix button (option/dialog) */
#define  CompData_ButtonRepeat   0x7   /* repeat button (option) */
#define  CompData_ButtonLoop     0x8   /* loop button (push) */
#define  CompData_ButtonSwitch   0x9   /* switch button (push) */
#define  CompData_ButtonPush     0xA   /* generic push */
#define  CompData_ButtonOption   0xB   /* generic option */
#define  CompData_ButtonMInfo    0xC   /* more information '->' button */

#define  CompMsg_Null            0x0   /* component message null */
#define  CompMsg_MouseDown       0x1
#define  CompMsg_MouseUp         0x2
#define  CompMsg_Select          0x3
#define  CompMsg_Scroll          0x4
#define  CompMsg_ScrollL         0x5
#define  CompMsg_ScrollR         0x6
#define  CompMsg_ScrollM         0x7
#define  CompMsg_MDownL          0x8
#define  CompMsg_MDownR          0x9
#define  CompMsg_MDownM          0xA
#define  CompMsg_MUpL            0xB
#define  CompMsg_MUpR            0xC
#define  CompMsg_MUpM            0xD

#define  TimerID_Display         5001
#define  DisplayTimer_Interval   30

#define  PanelColor_Skin         0x1
#define  PanelColor_Display      0x2

#define  ConvertNumToPercent(n,m)(( ((float)n) / ((float)m) ) * 100)
#define  ConvertPercentToNum(p,m)(( ((float)p) / 100 ) * ((float)m))
#define  ClipToDistance(a,s,e)((a < s) ? s : ((a > e) ? e : a ))
#define  ConvertBPercentToNum(p,m)((((((float)p) - 50.0f)) / 50.0f) * ((float)m))

/* order to create components */

/*
#define  PanelMain_ButtonRecord  0x0
#define  PanelMain_ButtonMute    0x1
#define  PanelMain_ButtonDisk    0x2
#define  PanelMain_ButtonSWave   0x3
#define  PanelMain_ButtonMixer   0x4
#define  PanelMain_ButtonAutoMix 0x5
#define  PanelMain_ButtonRepeat  0x6
#define  PanelMain_ButtonLoop    0x7
#define  PanelMain_ButtonSwitch1 0x8
#define  PanelMain_ButtonSwitch2 0x9
#define  PanelMain_ButtonMInfo   0xA
#define  PanelMain_ScrollVolume  0xB
#define  PanelMain_ScrollPlayer1 0xC
#define  PanelMain_ScrollPlayer2 0xD
#define  PanelMain_ScrollLevel1  0xE
#define  PanelMain_ScrollLevel2  0xF
*/

#define  PanelMain_ButtonMute    0x0
#define  PanelMain_ButtonDisk    0x1
#define  PanelMain_ButtonSWave   0x2
#define  PanelMain_ButtonMixer   0x3
#define  PanelMain_ButtonMInfo   0x4
#define  PanelMain_ScrollVolume  0x5
#define  PanelMain_ScrollPlayer1 0x6
//#define  PanelMain_ScrollPlayer2 0x7
#define  PanelMain_ScrollLevel1  0x7
#define  PanelMain_ScrollLevel2  0x8

/* order to create components */

#define  PanelEqualizer_PreampLeft  0x0
#define  PanelEqualizer_PreampRight 0x1
#define  PanelEqualizer_BandLeft1   0x2
#define  PanelEqualizer_BandLeft2   0x3
#define  PanelEqualizer_BandLeft3   0x4
#define  PanelEqualizer_BandLeft4   0x5
#define  PanelEqualizer_BandLeft5   0x6
#define  PanelEqualizer_BandLeft6   0x7
#define  PanelEqualizer_BandLeft7   0x8
#define  PanelEqualizer_BandLeft8   0x9
#define  PanelEqualizer_BandLeft9   0xA
#define  PanelEqualizer_BandLeft10  0xB
#define  PanelEqualizer_BandRight1  0xC
#define  PanelEqualizer_BandRight2  0xD
#define  PanelEqualizer_BandRight3  0xE
#define  PanelEqualizer_BandRight4  0xF
#define  PanelEqualizer_BandRight5  0x10
#define  PanelEqualizer_BandRight6  0x11
#define  PanelEqualizer_BandRight7  0x12
#define  PanelEqualizer_BandRight8  0x13
#define  PanelEqualizer_BandRight9  0x14
#define  PanelEqualizer_BandRight10 0x15
#define  PanelEqualizer_OnOff       0x16
#define  PanelEqualizer_Preset      0x17

/* order to create components */

#define  PanelColor_ScrollH  0x0
#define  PanelColor_ScrollS  0x1
#define  PanelColor_ScrollL  0x2
#define  PanelColor_Apply    0x3
#define  PanelColor_Switch   0x4

/* local declarations */

void local_panelhistory_add(int pid);
int local_panelhistory_next(void);
int local_panelhistory_back(void);
void local_panelhistory_clear(void);

int  Local_CompGetCoord(unsigned long compid,int clight,Coord* cd,Coord* cb);
int  Local_InsideCoord(Coord* cd,int x,int y);
int  Local_InsideCoordP(Coord* cd,POINT* pt);
int  Local_ComponentInitializeLibrary(void);
int  Local_ComponentUninitializeLibrary(void);

int  Local_PanelMain_Show(void);
int  Local_PanelMain_Close(void);
int  Local_PanelMain_Scroll(int rx);
int  Local_PanelTimer_Main(void);
int  Local_PanelCredits_Show(void);
int  Local_PanelCredits_Timer(void);
int  Local_PanelCredits_Close(void);
int  Local_PanelEqualizer_Show(void);
int  __stdcall Local_PanelMain_Buttons(int compid,unsigned long msg,unsigned long ndata,void* vdata);
int  __stdcall Local_PanelMain_Scrolls(int compid,unsigned long msg,unsigned long ndata,void* vdata);
int  __stdcall Local_PanelEqualizer_Buttons(int compid,unsigned long msg,unsigned long ndata,void* vdata);
int  __stdcall Local_PanelEqualizer_Scrolls(int compid,unsigned long msg,unsigned long ndata,void* vdata);
int  Local_PanelEqualizer_Close(void);
int Local_PanelVisualization_Show(void);
int Local_PanelVisualization_Timer(void);
int Local_PanelVisualization_Close(void);

int  __stdcall  Local_PanelColor_Proc(int compid,unsigned long msg,unsigned long ndata,void* vdata);
int  Local_PanelColor_Close(void);
int  Local_PanelColor_Show(void);

int  Local_ComponentClear(void);
int  Local_ComponentDestroy(unsigned long compid);
void Local_CompDraw(unsigned long compid,HDC dc,int cdt,int xd,int yd);
void Local_CompDrawFast(unsigned long compid,int cdt);
int  Local_CompGetHeight(unsigned long compid);
int  Local_CompGetWidth(unsigned long compid);

int  Local_DrawElectronicDigits_Time(HDC dc,int x,int y,int cmin,int csec);

void __stdcall TimerProc_Display(HWND hwnd,UINT uMsg,UINT_PTR idEvent,unsigned long dwTime);

int Local_SetPanelBackground(const char *resid);
void Local_DrawRect(HDC dc, int x, int y, int w, int h, COLORREF rcl);

/* constants */

const Coord    Coord_ButtonRecordD   = {0  ,0 ,23,12};
const Coord    Coord_ButtonRecordL   = {0  ,26,23,12};
const Coord    Coord_ButtonMuteD     = {1, 1, 32, 32};/*{24 ,0 ,23,12};*/
const Coord    Coord_ButtonMuteL     = {187 + 1, 167, 32, 32};/*{24 ,26,23,12};*/
const Coord    Coord_ButtonDiskD     = {33, 1, 32, 32};//{48 ,0 ,23,12};
const Coord    Coord_ButtonDiskL     = {187 + 33, 167, 32, 32};//{48 ,26,23,12};
const Coord    Coord_ButtonSWaveD    = {66, 1, 32, 32};//{72 ,0 ,23,12};
const Coord    Coord_ButtonSWaveL    = {187 + 66, 167, 32, 32};//{72 ,26,23,12};
const Coord    Coord_ButtonMixerD    = {99, 1, 32, 32};//{96 ,0 ,23,12};
const Coord    Coord_ButtonMixerL    = {187 + 99, 167, 32, 32};//{96 ,26,23,12};
const Coord    Coord_ButtonAutoMixD  = {120,0 ,23,12};
const Coord    Coord_ButtonAutoMixL  = {120,26,23,12};
const Coord    Coord_ButtonRepeatD   = {144,0 ,23,12};
const Coord    Coord_ButtonRepeatL   = {144,26,23,12};
const Coord    Coord_ButtonLoopD     = {0  ,13,49,12};
const Coord    Coord_ButtonLoopL     = {0  ,39,49,12};
const Coord    Coord_ButtonSwitchD   = {50 ,13,49,12};
const Coord    Coord_ButtonSwitchL   = {50 ,39,49,12};
const Coord    Coord_ButtonMInfoD    = {0 ,181,10,8};
const Coord    Coord_ButtonMInfoL    = {12,181,10,8};

const Coord    Coord_TriScrollVD     = {76,52,13,50};
const Coord    Coord_TriScrollVL     = {91,52,13,50};

const Coord    Coord_ScrollH         = {1,108,233,7};
const Coord    Coord_ScrollHBtn      = {1,92 ,11 ,7};

const Coord    Coord_ScrollH_L       = {1 ,108,4 ,7}; /* resize crops (left) */
const Coord    Coord_ScrollH_M       = {5 ,108,20,7}; /* middle crop */
const Coord    Coord_ScrollH_R       = {26,108,4 ,7}; /* right crop */
const Coord    Coord_ScrollH_LM      = {2 ,116,20,7}; /* light middle (value) */

const Coord    Coord_ImageStopped    = {1,53,71,13};
const Coord    Coord_ImagePlaying    = {1,66,71,13};
const Coord    Coord_ImagePaused     = {1,79,71,13};

const Coord    Coord_CheckBox        = {0 ,138,13,13};
const Coord    Coord_CheckBoxMove    = {14,138,13,13};
const Coord    Coord_CheckBoxDown    = {28,138,13,13};
const Coord    Coord_CheckBoxFill    = {42,138,12,13};

const Coord    Coord_OptionBox       = {0 ,151,13,13};
const Coord    Coord_OptionBoxMove   = {14,151,13,13};
const Coord    Coord_OptionBoxDown   = {28,151,13,13};

const Coord    Coord_TailingFill     = {4,165,25,15};
const Coord    Coord_TailingFillL    = {0,165,3,15};
const Coord    Coord_TailingFillR    = {30,165,3,15};
const Coord    Coord_TailingFillH    = {38,165,25,15};
const Coord    Coord_TailingFillHL   = {34,165,3,15};
const Coord    Coord_TailingFillHR   = {64,165,3,15};

const Coord    Coord_ScrollV_T       = {25,192,7,7};
const Coord    Coord_ScrollV_M       = {9,192,7,7};
const Coord    Coord_ScrollV_B       = {17,192,7,7};
const Coord    Coord_ScrollV_Btn     = {1,192,7,7};

const Coord    Coord_ButtonPushL     = {168,0, 5,16};
const Coord    Coord_ButtonPushM     = {174,0,15,16};
const Coord    Coord_ButtonPushR     = {191,0, 5,16};

const Coord    Coord_ButtonPushHL    = {168,17, 5,16};
const Coord    Coord_ButtonPushHM    = {174,17,15,16};
const Coord    Coord_ButtonPushHR    = {191,17, 5,16};

/* const Coord    Coord_...FillR
   const Coord    Coord_...FillL if non-solid */

/* variables */

int      Coord_ENumberX    = 0;
int      Coord_ENumberY    = 125;
int      Coord_ENumberW    = 6;
int      Coord_ENumberH    = 11;

int      Coord_DisplayX    = 79;       /* display x position */
int      Coord_DisplayY    = 166 - 2;      /* display y position */
int      Coord_DisplayW    = 253;      /* display width */
int      Coord_DisplayH    = 83;       /* display height */

int      Coord_DynamicW    = 300;      /* resize and decrease size [afterbuild] */
int      Coord_DynamicH    = 100;      /* [afterbuild] */

int      Coord_TextScrollX = 1;
int      Coord_TextScrollW = 253 - 1 - 17;

COLORREF Color_CurrentText = RGB(0,0,0);
HFONT    Font_Current;                 /* current font */
TEXTMETRIC Font_CurrentMetric;         /* current font text metrics */

HFONT    Font_Default;                 /* backup memory for last/windows sys font */
HFONT    Font_Bold;                    /* bold font */
HFONT    Font_Italic;                  /* italic font */
HFONT    Font_ULined;                  /* underlined font */
HFONT    Font_SizeSmall;               /* small sized font */
HFONT    Font_SizeLarge;               /* large sized font */

HFONT    Font_ComponentDark;           /* default component font (button etc.) dark. */
HFONT    Font_ComponentLight;          /* default component font light. */
HFONT    Font_ComponentOld;            /* old font store for component functions. */
COLORREF Color_Font_ComponentDark;     /* color of dark font */
COLORREF Color_Font_ComponentLight;    /* color of light font */

HBRUSH   PanelBrushDefault;            /* back up memory for last brush */
HBRUSH   PanelBrush1;                  /* brush 1 for panel painting */
HBRUSH   PanelBrush2;                  /* brush 2 for panel painting */
HPEN     PanelPenDefault;              /* back up memory for last pen */
HPEN     PanelPen1;                    /* pen 1 for panel painting */
HPEN     PanelPen2;                    /* pen 2 for panel painting */
HBRUSH   PanelBrushDefaultEx1;
HPEN    PanelPenDefaultEx1;

TEXTMETRIC Font_BoldMetric;            /* bold font metrics */
TEXTMETRIC Font_SizeSmallMetric;       /* small font metrics */
TEXTMETRIC Font_SizeLargeMetric;       /* large font metrics */

letter   PanelMain_Title[1024];        /* scrolling title for panel Main */
letter   PanelMain_TitleTemp[1024];
letter   PanelMain_LastPath[1024]; 
short    PanelMain_TitleWidth;         /* title source (in dynamic display) width */
unsigned long    PanelMain_DurationMS;         /* duration in miliseconds */
unsigned long    PanelMain_DurationMSL;        /* duration in miliseconds of last selected player */

HMENU    PanelEqualizer_PresetMain;
HMENU    PanelEqualizer_PresetSelect;

int      PanelColor_Selected = PanelColor_Skin;

char*    PanelVisualization_AudioBuffer = 0;
float*   PanelVisualization_Audio = 0;
float*   PanelVisualization_FFTOut = 0;
unsigned long    PanelVisualization_AudioSize;
unsigned long    PanelVisualization_FFTSize;
float*   PanelVisualization_Peaks = 0;
float*   PanelVisualization_Falloff = 0;
float*   PanelVisualization_fft_i;
float*   PanelVisualization_fft_r;
float*   PanelVisualization_PeaksMax;

short    ComponentIDBase = 0;          /* for public use */

unsigned long    CurrentPanel = PanelID_Null;

HDC      DynamicDisplay;               /* memory dc for runtime graphics */
HDC      MemoryDisplay;                /* memory dc for display. */
HDC      ActionDisplay;                /* action source. */

HBITMAP  bmpDynamicDisplay;            /* bitmap for DynamicDisplay */
HBITMAP  bmpMemoryDisplay;             /* bitmap for MemoryDisplay */
HBITMAP  bmpActionDisplay;             /* bitmap for ActionDisplay */
HBITMAP  bmpPanelBackground = 0;
HDC      dcPanelBackground;
int      panelbk_black = 1;

unsigned long    ComponentCount;               /* component counter */
unsigned long    ComponentsSize;               /* Components array size */

ComponentProc Display_CurrentGeneralProc = 0;
int PanelVisualization_Mode  = 0;
#define PanelVisualization_Modes 4

int Display_EqFlush = 0;

UINT_PTR Timer_Display;

int display_refresh_level = 4;

DisplayComponent* Components = 0;      /* component pointer (heap/virtual memory) */
char     internal_call_showpanel = 0;
/*
 * initialize the display.
 * return type : 1
 */

int Display_Initialize(void)
{
    /* create color memory dc */

    MemoryDisplay = CreateCompatibleDC(window_main_dc);

    if(!MemoryDisplay){LogMsgS("couldn't create mem display.");return 0;}

    bmpMemoryDisplay = CreateCompatibleBitmap(window_main_dc,Coord_DisplayW,Coord_DisplayH);

    if(!bmpMemoryDisplay){LogMsgS("couldn't create display mem bitmap.");return 0;}

    SelectObject(MemoryDisplay,bmpMemoryDisplay);

    /* create dynamic dc (color) */

    DynamicDisplay = CreateCompatibleDC(window_main_dc);

    if(!DynamicDisplay){LogMsgS("couldn't create dynamic display.");return 0;}

    bmpDynamicDisplay = CreateCompatibleBitmap(window_main_dc,Coord_DynamicW,Coord_DynamicH);
    
    if(!bmpDynamicDisplay){LogMsgS("couldn't create display dynamic bitmap.");return 0;}

    SelectObject(DynamicDisplay,bmpDynamicDisplay);

    /* initialize source bitmaps */

    ActionDisplay = CreateCompatibleDC(window_main_dc);

    if(!ActionDisplay){LogMsgS("couldn't create action display dc");return 0;}

    bmpActionDisplay = png_res_load_winbmp(instance_fennec, uni("png"), uni("display.actions"));

	AdjustColors(bmpActionDisplay, 0, settings.player.displaycolor_h,
						              settings.player.displaycolor_s,
						              settings.player.displaycolor_l);


    if(!bmpActionDisplay){LogMsgS("couldn't load action display bmp");return 0;}

    SelectObject(ActionDisplay,bmpActionDisplay);

    Font_Current = CreateFont(-MulDiv(10, GetDeviceCaps(MemoryDisplay, LOGPIXELSY), 72),
                                0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,
                                DEFAULT_PITCH ,"FixedSys");

    SelectObject(MemoryDisplay,Font_Current);
    SelectObject(DynamicDisplay,Font_Current);
    GetTextMetrics(MemoryDisplay,&Font_CurrentMetric);

	bmpPanelBackground = CreateCompatibleBitmap(window_main_dc,Coord_DisplayW,Coord_DisplayW);

	dcPanelBackground = CreateCompatibleDC(window_main_dc);

	SelectObject(dcPanelBackground, bmpPanelBackground);

    Local_ComponentInitializeLibrary();

    Timer_Display = SetTimer(0, TimerID_Display, DisplayTimer_Interval, TimerProc_Display);

	local_panelhistory_clear();
	internal_call_showpanel = 0;

	Display_ShowPanel(PanelID_Credits);
    return 1;
}

/*
 * uninitialize the display
 * return type : 1
 */

int Display_Uninitialize(void)
{
	Display_ShowPanel(0);

    if(bmpMemoryDisplay) DeleteObject(bmpMemoryDisplay);
    if(MemoryDisplay)    DeleteDC(MemoryDisplay);
    if(bmpActionDisplay) DeleteObject(bmpActionDisplay);
    if(ActionDisplay)    DeleteDC(ActionDisplay);
    if(bmpDynamicDisplay)DeleteObject(bmpDynamicDisplay);
    if(DynamicDisplay)   DeleteDC(DynamicDisplay);
    if(Font_Current)     DeleteObject(Font_Current);

	if(bmpPanelBackground) DeleteObject(bmpPanelBackground);
	bmpPanelBackground = 0;
	if(dcPanelBackground)  DeleteDC(dcPanelBackground);
	dcPanelBackground  = 0;

	bmpMemoryDisplay  = 0;
	MemoryDisplay     = 0;
	bmpActionDisplay  = 0;
	ActionDisplay     = 0;
	bmpDynamicDisplay = 0;
	DynamicDisplay    = 0;
	Font_Current      = 0;

    KillTimer(0, Timer_Display);

    Local_ComponentUninitializeLibrary();
    return 1;
}

/*
 * set selected panel and show it
 * return type : 1
 */

int Display_ShowPanel(unsigned long npanel)
{
    unsigned long i;		

	Color_CurrentText = GetAdjustedColor(163, settings.player.displaycolor_h, settings.player.displaycolor_s, settings.player.displaycolor_l);

    if(CurrentPanel != npanel)
    {
		Display_CurrentGeneralProc = 0;

		display_refresh_level = fennec_v_refresh_force_full;

		/* set backgrounds */

		switch(npanel)
		{
		case     PanelID_Color:         Local_SetPanelBackground("display.background.shade"); break;
		case     PanelID_Equalizer:     Local_SetPanelBackground("display.background.equalizer"); break;
		default: Local_SetPanelBackground(0);
		}
					

        /* close existing first */
        switch(CurrentPanel)
        {
        case PanelID_Main:
            Local_PanelMain_Close();
            break;

        case PanelID_Credits:
            Local_PanelCredits_Close();
            break;

        case PanelID_Equalizer:
            Local_PanelEqualizer_Close();
            break;

        case PanelID_Color:
            Local_PanelColor_Close();
            break;

		case PanelID_Visualization:
			Local_PanelVisualization_Close();
			break;

        default:
            break;
        }

		if(!internal_call_showpanel)
		{
			local_panelhistory_add((int)npanel);
		}else{
			internal_call_showpanel = 0;
		}
    }

    switch(npanel)
    {
    case PanelID_Main:
        Local_PanelMain_Show();
        CurrentPanel = PanelID_Main;
        break;

    case PanelID_Credits:
        Local_PanelCredits_Show();
        CurrentPanel = PanelID_Credits;
        break;

    case PanelID_Equalizer:
        Local_PanelEqualizer_Show();
        CurrentPanel = PanelID_Equalizer;
        break;

    case PanelID_Color:
        Local_PanelColor_Show();
        CurrentPanel = PanelID_Color;
        break;

	case PanelID_Visualization:
		Local_PanelVisualization_Show();
		CurrentPanel = PanelID_Visualization;
		break;

    default:
        break;
    }

    for(i=1;i<=ComponentCount;i++)
    {
        Local_CompDraw(i,MemoryDisplay,0,0,0);
    }

    BitBlt(window_main_dc,Coord_DisplayX,Coord_DisplayY,Coord_DisplayW,Coord_DisplayH,MemoryDisplay,0,0,SRCCOPY);


    return 1;
}

int Display_ShowPanel_Next(void)
{
	int pid = local_panelhistory_next();
	internal_call_showpanel = 1;
	Display_ShowPanel((unsigned long)pid);
	return pid;
}

int Display_ShowPanel_Back(void)
{
	int pid = local_panelhistory_back();
	internal_call_showpanel = 1;
	Display_ShowPanel((unsigned long)pid);
	return pid;
}

/*
 * blit current panel into a DC
 */

int Display_BlitCurrentPanel(HDC dc,int x,int y)
{
    Display_ShowPanel(CurrentPanel);

    if(x == 0xFFFF && y == 0xFFFF)
    {
        BitBlt(dc,Coord_DisplayX,Coord_DisplayY,Coord_DisplayW,Coord_DisplayH,MemoryDisplay,0,0,SRCCOPY);
    }else{
        BitBlt(dc,x,y,Coord_DisplayW,Coord_DisplayH,MemoryDisplay,0,0,SRCCOPY);
    }
    return 1;
}

/*
 * get display status
 * return type : 1
 */

int Display_GetDisplayStatus(void)
{
    return 1;
}

/*
 * get panel status
 * return type : 1
 */

int Display_GetPanelStatus(void)
{
    return 1;
}

/*
 * redraw the current panel
 * return type : 1
 */

int Display_Refresh(int level)
{
    unsigned long i;

	display_refresh_level = level;

    Display_ShowPanel(CurrentPanel);

    for(i=1;i<=ComponentCount;i++)
    {
        Local_CompDraw(i,MemoryDisplay,0,0,0);
    }

    BitBlt(window_main_dc,Coord_DisplayX,Coord_DisplayY,Coord_DisplayW,Coord_DisplayH,MemoryDisplay,0,0,SRCCOPY);

    return 1;
}

/*
 * erase all data and draw the background
 * return type : 1
 */

int Display_Erase(void)
{
	if(panelbk_black)
	{
		Local_DrawRect(MemoryDisplay, 0, 0, Coord_DisplayW, Coord_DisplayH, 0x0);
		BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);

		return 1;
	}

	if(bmpPanelBackground)
	{
		BitBlt(MemoryDisplay, 0, 0, Coord_DisplayW, Coord_DisplayH, dcPanelBackground, 0, 0, SRCCOPY);
		BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);
	}

    return 1;
}

int Display_Erase_MemoryDC(void)
{
	if(panelbk_black)
	{
		Local_DrawRect(MemoryDisplay, 0, 0, Coord_DisplayW, Coord_DisplayH, 0x0);
		return 1;
	}

	if(bmpPanelBackground)
	{
		BitBlt(MemoryDisplay, 0, 0, Coord_DisplayW, Coord_DisplayH, dcPanelBackground, 0, 0, SRCCOPY);
	}

    return 1;
}

/*
 * mouse message proc for display (from parent window)
 * return type : 1
 */

int Display_MouseMessage(POINT ppt,int mac)
{
    #define CaptureForDisplay FennecWndMain_GiveMessage(DISPLAY_MOUSEMESSAGE);

    static int   LastMouseDown = 0;      /* to handle mouse ups */
    static unsigned long LastMouseDownComp = 0;  /* to handle drags */
    static unsigned long LastComp = 0;           /* last component */

    int   compsel = 0; /* is component selected */
    int   capmsg  = 0; /* captured message */
    Coord coordsc;     /* selected components coord */
    Coord coordscb;    /* selected components buttons coord */
    unsigned long  i;
    POINT          pt;
    int            vals,mv;
    int            msgval = 0;
	int            quickin = 0;

	if(Display_CurrentGeneralProc)
	{
		switch(mac)
		{
		case Display_Mouse_LDown:
        case Display_Mouse_RDown:
        case Display_Mouse_Down:
			Display_CurrentGeneralProc(-1, Display_Mouse_Down, 0, 0);
			break;

		case Display_Mouse_LUp:
        case Display_Mouse_RUp:
        case Display_Mouse_Up:
			Display_CurrentGeneralProc(-1, Display_Mouse_Up, 0, 0);
			break;
		}

	}


    /* check if a component */

    if(//(mac != Display_Mouse_LUp) &&
       //(mac != Display_Mouse_RUp) &&
       //(mac != Display_Mouse_Up ) &&
       (LastMouseDownComp))
    {
        i      = LastMouseDownComp;
        pt.x   = ppt.x;
        pt.y   = ppt.y;
        capmsg = 1;

		if((mac == Display_Mouse_LUp) ||
           (mac == Display_Mouse_RUp) ||
           (mac == Display_Mouse_Up ))
		{
		    LastMouseDownComp = 0;
			FennecWndMain_GiveMessage(0);
			pt.x = ppt.x;
			pt.y = ppt.y;
		}
		quickin = 1;
        goto lQuickJump;
    }else{
        LastMouseDownComp = 0;
        FennecWndMain_GiveMessage(0);
        pt.x = ppt.x;
        pt.y = ppt.y;
		capmsg = 0;
    }

    if(ComponentCount)
    {
        for(i=1;i<=ComponentCount;i++)
        {
			
lQuickJump:

            switch(Components[i].comptid)
            {
            case Comp_ScrollH:
            case Comp_LevelH:
                Local_CompGetCoord(i,0,&coordsc,&coordscb);
                coordsc.x = Components[i].dx;
                coordsc.y = Components[i].dy;
                coordsc.w = Components[i].ndata;

                if(Local_InsideCoordP(&coordsc,&pt) || capmsg)
                {
                    compsel = 1;

					if(mac == Display_Mouse_Move)
						if(Components[i].ctip)
							tips_display(0, 0, Components[i].ctip);

                    if(capmsg)
                    {
                        pt.x = ClipToDistance(ppt.x,coordsc.x,coordsc.x + coordsc.w);
                        pt.y = ClipToDistance(ppt.y,coordsc.y,coordsc.y + coordsc.h);
                    }

                    if(Components[i].cproc)
                    {
                        switch(mac)
                        {
                        case Display_Mouse_LDown:
                        case Display_Mouse_RDown:
                        case Display_Mouse_Down:
                            if(Components[i].cproc(i,CompMsg_MouseDown,Components[i].value,0))
                            {
                                vals = pt.x - Components[i].dx;
                                if(Components[i].cproc(i,CompMsg_Scroll,(unsigned long)vals,0))
                                {
                                    Local_CompDrawFast(i,0);
                                }
                            }

                            LastMouseDown = 1;
                            LastMouseDownComp = i;
                            CaptureForDisplay;
                            break;

                        case Display_Mouse_LUp:
                        case Display_Mouse_RUp:
                        case Display_Mouse_Up:

                            if(Components[i].cproc(i,CompMsg_MouseUp,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            LastMouseDown = 0;
                            break;

                        case Display_Mouse_Move:
                            if(LastMouseDown && LastMouseDownComp == i) /* drag or scroll */
                            {
                                vals = pt.x - Components[i].dx;
                                if(Components[i].cproc(i,CompMsg_Scroll,(unsigned long)vals,0))
                                {
                                    Local_CompDrawFast(i,0);
                                }
                            }
                            break;

                        }
                    }

                }
                break;

            case Comp_ScrollV:
            case Comp_LevelV:
                Local_CompGetCoord(i,0,&coordsc,&coordscb);
                coordsc.x = Components[i].dx;
                coordsc.y = Components[i].dy;
                coordsc.h = Components[i].ndata;

                if(Local_InsideCoordP(&coordsc,&pt) || capmsg)
                {
                    compsel = 1;

					if(mac == Display_Mouse_Move)
						if(Components[i].ctip)
							tips_display(0, 0, Components[i].ctip);

                    if(capmsg)
                    {
                        pt.x = ClipToDistance(ppt.x,coordsc.x,coordsc.x + coordsc.w);
                        pt.y = ClipToDistance(ppt.y,coordsc.y,coordsc.y + coordsc.h);
                    }

                    if(Components[i].cproc)
                    {
                        switch(mac)
                        {
                        case Display_Mouse_RDown:
                            if(Components[i].cproc(i,CompMsg_MDownR,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            LastMouseDown = 2;
                            LastMouseDownComp = i;
                            CaptureForDisplay;
                            break;

                        case Display_Mouse_RUp:
                            if(Components[i].cproc(i,CompMsg_MUpR,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            LastMouseDown = 0;
                            break;

                        case Display_Mouse_LUp:
                        case Display_Mouse_Up:
                            if(Components[i].cproc(i,CompMsg_MUpL,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            LastMouseDown = 0;
                            break;

						case Display_Mouse_LDown:
                        case Display_Mouse_Down:
                            if(Components[i].cproc(i,CompMsg_MDownL,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            LastMouseDown = 1;
                            LastMouseDownComp = i;
                            CaptureForDisplay;
                            
                        case Display_Mouse_Move:

                            if(LastMouseDown && LastMouseDownComp == i) /* drag or scroll */
                            {
                                vals = pt.y - (Components[i].dy + Coord_ScrollV_T.h + (Coord_ScrollV_Btn.h / 2));
                                mv   = coordsc.h - (Coord_ScrollV_T.h + Coord_ScrollV_B.h + Coord_ScrollV_Btn.h);

                                vals = ClipToDistance(vals,0,mv);
                                vals = (int)ConvertNumToPercent(vals,mv);
                                vals = 100 - vals; /* invert percent. */

                                if(LastMouseDown == 1)
                                {
                                    if(Components[i].cproc(i,CompMsg_ScrollL,(unsigned long)vals,0))
                                    {
                                        Local_CompDrawFast(i,0);
                                    }
                                }else{
                                    if(Components[i].cproc(i,CompMsg_ScrollR,(unsigned long)vals,0))
                                    {
                                        Local_CompDrawFast(i,0);
                                    }
                                }
                            }
                            break;

                        }
                    }

                }
                break;

            case Comp_TriScrollV:
                Local_CompGetCoord(i,0,&coordsc,&coordscb);
                coordsc.x = Components[i].dx;
                coordsc.y = Components[i].dy;

                if(Local_InsideCoordP(&coordsc,&pt) || capmsg)
                {
                    compsel = 1;

					if(mac == Display_Mouse_Move)
						if(Components[i].ctip)
							tips_display(0, 0, Components[i].ctip);

                    if(capmsg)
                    {
                        pt.x = ClipToDistance(ppt.x,coordsc.x,coordsc.x + coordsc.w);
                        pt.y = ClipToDistance(ppt.y,coordsc.y,coordsc.y + coordsc.h);
                    }

                    if(Components[i].cproc)
                    {
                        switch(mac)
                        {
                        case Display_Mouse_LDown:
                        case Display_Mouse_RDown:
                        case Display_Mouse_Down:
                            if(Components[i].cproc(i,CompMsg_MouseDown,Components[i].value,0))
                            {
                                vals = coordsc.h - (pt.y - Components[i].dy);
                                if(Components[i].cproc(i,CompMsg_Scroll,(unsigned long)vals,0))
                                {
                                    Local_CompDrawFast(i,0);
                                }
                            }

                            LastMouseDown = 1;
                            LastMouseDownComp = i;
                            CaptureForDisplay;
                            break;

                        case Display_Mouse_LUp:
                        case Display_Mouse_RUp:
                        case Display_Mouse_Up:
                            if(Components[i].cproc(i,CompMsg_MouseUp,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            LastMouseDown = 0;
                            break;

                        case Display_Mouse_Move:

                            if(LastMouseDown && LastMouseDownComp == i) /* drag or scroll */
                            {
                                vals = coordsc.h - (pt.y - Components[i].dy);
                                if(Components[i].cproc(i,CompMsg_Scroll,(unsigned long)vals,0))
                                {
                                    Local_CompDrawFast(i,0);
                                }
                            }
                            break;

                        }
                    }
                }
                break;

            default:
                Local_CompGetCoord(i,0,&coordsc,&coordscb);
                coordsc.x = Components[i].dx;
                coordsc.y = Components[i].dy;

                if(Components[i].comptid == Comp_OptionFill || Components[i].comptid == Comp_CheckFill)
                {
                    coordsc.w += Components[i].ndata;
                }

                if(Components[i].comptid == Comp_Button && Components[i].subid == CompData_ButtonPush)
                {
                    coordsc.w = Components[i].ndata;
                }

                if(Local_InsideCoordP(&coordsc,&pt))
                {
                    compsel = 1;
		
					if(mac == Display_Mouse_Move)
						if(Components[i].ctip)
							tips_display(0, 0, Components[i].ctip);


                    if(Components[i].cproc)
                    {
                        switch(mac)
                        {
                        case Display_Mouse_LDown:
                        case Display_Mouse_RDown:
                        case Display_Mouse_Down:
                            if(Components[i].cproc(i,CompMsg_MouseDown,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,1);
                            }
                            break;

                        case Display_Mouse_LUp:
                        case Display_Mouse_RUp:
                        case Display_Mouse_Up:

                            if(mac == Display_Mouse_Up)
                            {
                                msgval = CompMsg_MouseUp;
                            }else if(mac == Display_Mouse_LUp){
                                msgval = CompMsg_MUpL;
                            }else if(mac == Display_Mouse_RUp){
                                msgval = CompMsg_MUpR;

                            }

                            if(Components[i].cproc(i,msgval,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,0);
                            }

                            if(msgval != CompMsg_MouseUp)
                            {
                                if(Components[i].cproc(i,CompMsg_MouseUp,Components[i].value,0))
                                {
                                    Local_CompDrawFast(i,0);
                                }
                            }

                            break;

                        case Display_Mouse_Move:

                            if(LastComp == i)break;

                            if(LastComp)Local_CompDrawFast(LastComp,0);

                            if(Components[i].cproc(i,CompMsg_Scroll,Components[i].value,0))
                            {
                                Local_CompDrawFast(i,1);
                                LastComp = i;
                            }

                            break;

                        }
                    }
                    return 1;
                }
                break;
            }

			if(quickin)break;
        }

        if(LastComp && !compsel)Local_CompDrawFast(LastComp,0);
        LastComp = 0;
    }
    if(compsel)
    {
        return 1;
    }else{
        return 0;
    }
}

/*
 * key message proc for display (from parent window)
 * return type : 1
 */

int Display_KeyMessage(void)
{
    return 1;
}

/* ----------------------------------------------------------------------------
 local functions.
---------------------------------------------------------------------------- */

int Local_InsideCoordP(Coord* cd,POINT* pt)
{
    if(pt->x >= cd->x && pt->x <= (cd->w + cd->x) &&
       pt->y >= cd->y && pt->y <= (cd->h + cd->y))
    {
        return 1;
    }else{
        return 0;
    }
}

int Local_InsideCoord(Coord* cd,int x,int y)
{
    if(x >= cd->x && x <= (cd->w + cd->x) &&
       y >= cd->y && y <= (cd->h + cd->y))
    {
        return 1;
    }else{
        return 0;
    }
}

void Local_DisplayFastRect_Vis(HDC dc, COLORREF rc, int x, int y, int w, int h)
{
	if(h > Coord_DisplayH)
	{
		h = Coord_DisplayH;
		y = 0;
	}
	y |= 1;
	BitBlt(dc, x, y, w, h, ActionDisplay, 207, 1, SRCCOPY);
}

void Local_DisplayFastRect(HDC dc, COLORREF rc, int x, int y, int w, int h)
{
	HPEN   oldpen;
	HBRUSH oldbrush;
	HPEN   hpen;
	HBRUSH hbrush;

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

int Local_DisplayTextOutFast(int x, int y, const char* txt)
{
    return (TextOut(window_main_dc,Coord_DisplayX + x,Coord_DisplayY + y,txt,(int)strlen(txt)) ? 1 : 0);
}

int Local_DisplayTextOutFastEx(int x, int y, const char* txt,unsigned int len)
{
    return (TextOut(window_main_dc,Coord_DisplayX + x,Coord_DisplayY + y,txt,len) ? 1 : 0);
}

int Local_DisplayTextOut(int x, int y, const char* txt)
{
    return (TextOut(MemoryDisplay,x,y,txt,(int)strlen(txt)) ? 1 : 0);
}

int Local_DisplayTextOutEx(int x, int y, const char* txt,unsigned int len)
{
    return (TextOut(MemoryDisplay,x,y,txt,len) ? 1 : 0);
}

void _inline Local_BlitCoord(HDC ddc,HDC sdc,int x,int y,const Coord* cd)
{
    BitBlt(ddc,x,y,cd->w,cd->h,sdc,cd->x,cd->y,SRCCOPY);
}

/* To get faster (for any compiler) use '#define' [afterbuild] */

void __inline Local_FastBlit(int x,int y,int w,int h,HDC sdc,int sx,int sy)
{
    BitBlt(window_main_dc,Coord_DisplayX + x,Coord_DisplayY + y,w,h,sdc,sx,sy,SRCCOPY);
}

void Local_EraseDynamic(void)
{
    BitBlt(DynamicDisplay,0,0,Coord_DynamicW,Coord_DynamicH,0,0,0,BLACKNESS);
}

void Local_EraseWindowDC(void)
{
    BitBlt(window_main_dc,Coord_DisplayX,Coord_DisplayY,Coord_DisplayW,Coord_DisplayH,0,0,0,BLACKNESS);
}

void Local_ViewMemory(void)
{
    BitBlt(window_main_dc,Coord_DisplayX,Coord_DisplayY,Coord_DisplayW,Coord_DisplayH,MemoryDisplay,0,0,SRCCOPY);
}

void Local_ComponentTextOut(int fctype, HDC dc, int x, int y, const char * otxt)
{
    int      lastbk;
    COLORREF lastcolor;

    lastbk = GetBkMode(dc);
    lastcolor = GetTextColor(dc);
    SetBkMode(dc, TRANSPARENT);

    if(fctype)
    {
        SetTextColor(dc,Color_Font_ComponentLight);
        Font_ComponentOld = (HFONT) SelectObject(dc, Font_ComponentLight);
    }else{
        SetTextColor(dc,Color_Font_ComponentDark);
        Font_ComponentOld =  (HFONT)SelectObject(dc, Font_ComponentDark);
    }

    TextOut(dc,x,y,otxt,(int)strlen(otxt));

    SetBkMode(dc,lastbk);
    SetTextColor(dc,lastcolor);
    SelectObject(dc, Font_ComponentOld);
}

/*
 * functions for effects and graphics.
 */

void Local_PanelEffect_Slide(int sdirec)
{

}

void Local_DrawRect(HDC dc, int x, int y, int w, int h, COLORREF rcl)
{
	HBRUSH hb, hlb;

	hb = CreateSolidBrush(rcl);
	hlb = (HBRUSH) SelectObject(dc, hb);

	Rectangle(dc, x, y, x + w, y + h);
	SelectObject(dc, hlb);
	DeleteObject(hb);
}

/*
 * Initialize component library.
 */

int Local_ComponentInitializeLibrary(void)
{
	#define clipc(a)(a > 255 ? 255 : (a < 0 ? 0 : a))

    /* action and sources bitmap has been loaded by 'Display_Initialize'. */
    int fheight;

    ComponentCount = 0;
    ComponentsSize = ComponentCountInit;

    Components = (DisplayComponent*) sys_mem_alloc(ComponentsSize * sizeof(DisplayComponent));


    fheight = -MulDiv(8, GetDeviceCaps(ActionDisplay, LOGPIXELSY), 72);

    Font_ComponentDark = CreateFont(fheight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,5,0,"Verdana");

	Color_Font_ComponentDark = GetAdjustedColor(80, settings.player.displaycolor_h, settings.player.displaycolor_s, settings.player.displaycolor_l);


    Font_ComponentLight = CreateFont(fheight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,5,0,"Verdana");
    Color_Font_ComponentLight = 0xFFFFFF;
    return 1;
}

/*
 * Get coordinates of a component.
 */

int  Local_CompGetCoord(unsigned long compid,int clight,Coord* cd,Coord* cb)
{
    if(compid > ComponentCount)return 0;

    switch(Components[compid].comptid)
    {
    case Comp_Button:
        switch(Components[compid].subid)
        {
        case CompData_ButtonRecord:    /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonRecordL;
            }else{
                *cd = Coord_ButtonRecordD;
            }
            break;

        case CompData_ButtonMute:      /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonMuteL;
            }else{
                *cd = Coord_ButtonMuteD;
            }
            break;

        case CompData_ButtonDisk:      /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonDiskL;
            }else{
                *cd = Coord_ButtonDiskD;
            }
            break;

        case CompData_ButtonSWave:     /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonSWaveL;
            }else{
                *cd = Coord_ButtonSWaveD;
            }
            break;

        case CompData_ButtonMixer:     /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonMixerL;
            }else{
                *cd = Coord_ButtonMixerD;
            }
            break;

        case CompData_ButtonAutoMix:   /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonAutoMixL;
            }else{
                *cd = Coord_ButtonAutoMixD;
            }
            break;

        case CompData_ButtonRepeat:    /* option */ 
            if(clight)
            {
                *cd = Coord_ButtonRepeatL;
            }else{
                *cd = Coord_ButtonRecordD;
            }
            break;

        case CompData_ButtonLoop:      /* push */ 
            if(clight)
            {
                *cd = Coord_ButtonLoopL;
            }else{
                *cd = Coord_ButtonLoopD;
            }
            break;

        case CompData_ButtonSwitch:    /* push */ 
            if(clight)
            {
                *cd = Coord_ButtonSwitchL;
            }else{
                *cd = Coord_ButtonSwitchD;
            }
            break;

        case CompData_ButtonPush:
            if(clight)
            {
                *cd = Coord_ButtonPushHM;
            }else{
                *cd = Coord_ButtonPushM;
            }
            break;

        case CompData_ButtonOption:

            break;

        case CompData_ButtonMInfo:
            if(clight)
            {
                *cd = Coord_ButtonMInfoL;
            }else{
                *cd = Coord_ButtonMInfoD;
            }
            break;

        }
        break;

    case Comp_ScrollH:
        if(clight)
        {
            *cb = Coord_ScrollHBtn;
        }else{
            *cd = Coord_ScrollH;
        }
        break;

    case Comp_LevelH:

        break;

    case Comp_TriScrollV:
        if(clight)
        {
            *cd = Coord_TriScrollVL;
        }else{
            *cd = Coord_TriScrollVD;
        }
        break;

    case Comp_Check:
    case Comp_CheckFill:
        /* light on  : cd - move, cb - checked
           light off : cd - default, cb - fill */

        if(clight)
        {
            *cd = Coord_CheckBoxMove;
            *cb = Coord_CheckBoxDown;
        }else{
            *cd = Coord_CheckBox;
            *cb = Coord_CheckBoxFill;
        }

        break;

    case Comp_Option:
    case Comp_OptionFill:
        /* light on  : cd - move, cb - checked
           light off : cd - default, cd - fill */

        if(clight)
        {
            *cd = Coord_OptionBoxMove;
            *cb = Coord_OptionBoxDown;
        }else{
            *cd = Coord_OptionBox;
            *cb = Coord_TailingFill;
        }

        break;

    case Comp_ScrollV:

        if(clight)
        {
            *cd = Coord_ScrollV_Btn;
        }else{
            *cd = Coord_ScrollV_M;
        }
        break;

    /* null, unknown, not availables */ 

    case Comp_Null:
        return 0;


    case Comp_LevelV:
    case Comp_KnobL:
    case Comp_KnobB:
        LogMsgS("Fennec Display, Local_GetCoord. Component is not available.");
        return 0;

    default:
        LogMsgD("Fennec Display, Local_GetCoord. Component is not defined.",compid);
        return 0;
    }
    return 1;
}

int  Local_CompGetWidth(unsigned long compid)
{
    Coord wc,bc;
    switch(Components[compid].comptid)
    {
    case Comp_LevelH:
    case Comp_ScrollH:
        return Components[compid].ndata;
    default:
        Local_CompGetCoord(compid,0,&wc,&bc);
        return wc.w;
    }
}

int  Local_CompGetHeight(unsigned long compid)
{
    Coord wc,bc;
    switch(Components[compid].comptid)
    {
    case Comp_LevelV:
    case Comp_ScrollV:
        return Components[compid].ndata;
    default:
        Local_CompGetCoord(compid,0,&wc,&bc);
        return wc.h;
    }
}

/*
 * panel history buffer functions.
 */

void local_panelhistory_add(int pid)
{
	if(display_panelhistory_current < display_panelhistory_count - 1)
	/* clear forward buffer */
		display_panelhistory_count = display_panelhistory_current;

	if(display_panelhistory_count >= 16)
	{
		/* shift back the buffer */
		int i;

		for(i=0; i<16 - 1; i++)
			display_panelhistory[i] = display_panelhistory[i + 1];

		display_panelhistory_count   = 15;
		display_panelhistory_current = 14;
	}

	display_panelhistory_count++;
	display_panelhistory[display_panelhistory_count - 1] = pid;
	display_panelhistory_current = display_panelhistory_count - 1;
}

int local_panelhistory_next(void)
{
	if(display_panelhistory_current < (display_panelhistory_count - 1))display_panelhistory_current++;
	return display_panelhistory[display_panelhistory_current];
}

int local_panelhistory_back(void)
{
	if(display_panelhistory_current > 0)display_panelhistory_current--;
	return display_panelhistory[display_panelhistory_current];
}

void local_panelhistory_clear(void)
{
	display_panelhistory_current = 0;
	display_panelhistory_count   = 0;
}

/*
 * Blit variable sized bitmaps
 * ddc : [Windows] destination dc
 * sdc : [Windows] source dc
 * lc  : left crop
 * mc  : middle crop
 * rc  : right crop
 */

void Local_BlitSized(HDC ddc,HDC sdc,int x,int y,int w,const Coord* lc,const Coord* mc,const Coord* rc)
{
    int i,sx,fc;

    sx = x;

    Local_BlitCoord(ddc,sdc,sx,y,lc);

    sx += lc->w;

    fc = (w - lc->w - rc->w) / mc->w;

    for(i=0;i<fc;i++)
    {
        Local_BlitCoord(ddc,sdc,sx,y,mc);
        sx += mc->w;
    }

    BitBlt(ddc,sx,y, ((x + w) - rc->w) - sx, mc->h,sdc,mc->x,mc->y,SRCCOPY);

    Local_BlitCoord(ddc,sdc,(x + w) - rc->w,y,rc);
    return;
}

/*
 * Draw resized components such as scrollbars etc.
 * dc    : [Windows] destination DC
 * val   : current value (percent)
 * lc    : left crop
 * mc    : middle crop
 * rc    : right crop
 * lmc   : light middle crop or button
 * sizem : resize mode (ResizeMode_...)
 */

void Local_CompDrawResized(HDC dc,int comptid,int subid,int x,int y,int w,int h,int val,const Coord* lc,const Coord* mc,const Coord* rc,const Coord* lmc, int sizem)
{
    int i,sx,vp,sy;
    int mfills;

    /* int sw,rw; */

    if(comptid == Comp_LevelH && sizem == ResizeMode_Horizontal)
    {
        /* calculate value position (percent) by width */
        vp = (int)ConvertPercentToNum(val,w);
        vp = (vp > w ? w : (vp < 0 ? 0 : vp));

        /* the scroll images (appending ones) have to be placed in even
        numberd pixels, so here's a fast way to do this !*/

        //vp >>= 1;vp <<= 1;

        /* blit the light crop from start point */

        sx     = x;
        mfills = vp / lmc->w;

        for(i=0;i<mfills;i++)
        {
            BitBlt(dc,sx,y,lmc->w,lmc->h,ActionDisplay,lmc->x,lmc->y,SRCCOPY);
            sx += lmc->w;
        }

        /* fill the rest light crop */

        BitBlt(dc,sx,y,(x + vp)-sx,lmc->h,ActionDisplay,lmc->x,lmc->y,SRCCOPY);
        sx = x + vp;

        /* blit the dark crop from vp */

        mfills = (w - vp) / mc->w;

        for(i=0;i<mfills;i++)
        {
            BitBlt(dc,sx,y,mc->w,mc->h,ActionDisplay,mc->x,mc->y,SRCCOPY);
            sx += mc->w;
        }

        /* fill the rest of the scroll (wanna be fast ? remove that !) */

        BitBlt(dc,sx,y,(x + w)-sx,mc->h,ActionDisplay,mc->x,mc->y,SRCCOPY);

    }

    /* lmc becomes the scroll button */

    if(comptid == Comp_ScrollH && sizem == ResizeMode_Horizontal)
    {
        vp = (int)ConvertPercentToNum(val,w);
        vp = (vp > w ? w : (vp < 0 ? 0 : vp));

        /* confusing ? read the comments above ! */

        /* there's an add-on code, try if you're gonna use an odd placement
            if(vp & 1) vp |= 1;
            else       vp >>= 1;vp <<= 1; */

        //vp >>= 1;vp <<= 1;

        /* little bit a flickering could be appeared, but this is the fastest */

        /* first, blit the background */

        mfills = w / mc->w;
        sx     = x;

        for(i=0;i<mfills;i++)
        {
            BitBlt(dc,sx,y,mc->w,mc->h,ActionDisplay,mc->x,mc->y,SRCCOPY);
            sx += mc->w;
        }

        /* the rest of background */

        BitBlt(dc,sx,y,(w + x) - sx,mc->h,ActionDisplay,mc->x,mc->y,SRCCOPY);

        /* blit the scroll button */

        if(vp <= (lmc->w / 2))
        {
            BitBlt(dc,x,y,lmc->w,lmc->h,ActionDisplay,lmc->x,lmc->y,SRCCOPY);
        }else{
            if(vp >= (w - (lmc->w / 2)))
            {
                BitBlt(dc,(x + w) - lmc->w,y,lmc->w,lmc->h,ActionDisplay,lmc->x,lmc->y,SRCCOPY);
            }else{
                BitBlt(dc,x + vp - (lmc->w / 2),y,lmc->w,lmc->h,ActionDisplay,lmc->x,lmc->y,SRCCOPY);
            }
        }
    }

    if(comptid == Comp_ScrollV && sizem == ResizeMode_Vertical)
    {
        vp = (int)ConvertPercentToNum(100 - val,(h - (lc->h + rc->h + mc->h)));
        /* vals percent has been inverted, because we need the visuals
        for the value correctly */
        vp = (vp > h ? h : (vp < 0 ? 0 : vp));

        sy = y;

        /* blit the top */

        Local_BlitCoord(dc,ActionDisplay,x,sy,&Coord_ScrollV_T);
        sy += lc->h;

        /* fill the middle */

        mfills = (h - lc->h - rc->h) / mc->h;

        for(i=0;i<mfills;i++)
        {
            BitBlt(dc,x,sy,mc->w,mc->h,ActionDisplay,mc->x,mc->y,SRCCOPY);
            sy += mc->h;
        }

        /* fill the rest */

        BitBlt(dc,x,sy,mc->w,(h + y) - (sy + rc->h),ActionDisplay,mc->x,mc->y,SRCCOPY);

        /* blit the bottom */

        Local_BlitCoord(dc,ActionDisplay,x,y + (h - rc->h),&Coord_ScrollV_B);

        /* place the button */

        Local_BlitCoord(dc,ActionDisplay,x,(y + lc->h) + vp,lmc);
    }
}

/*
 * Draw a component using general values (including non resized components)
 * dc  : [Windows] destination DC
 * cdt :  component details (lit or not etc.)
 */

void Local_CompDraw(unsigned long compid,HDC dc,int cdt,int xd,int yd)
{
    int sizew = 0; /* resize horizontally */
    int sizeh = 0; /* resize vertically */
    Coord  cCoord;
    Coord  cocCoord;
    int dloopback = 0;

    if(compid > ComponentCount)return;

    switch(Components[compid].comptid)
    {
    case Comp_ScrollH:
    case Comp_LevelH:
        sizew = 1;
        break;
    case Comp_ScrollV:
    case Comp_LevelV:
        sizeh = 1;
        break;
    default:
        break;
    }

    if(!Local_CompGetCoord(compid,cdt,&cCoord,&cocCoord))return;

    if(Components[compid].comptid == Comp_TriScrollV)
    {
        int lval;

        if(!Local_CompGetCoord(compid,0,&cCoord,&cocCoord))return;
        if(Components[compid].value < 0 || Components[compid].value > 100)return;

        lval = cCoord.h - ((int) (((float)cCoord.h) * (((float)Components[compid].value) / 100)));

        BitBlt(dc,Components[compid].dx + xd,Components[compid].dy + yd
               ,cCoord.w,lval,ActionDisplay,cCoord.x,cCoord.y,SRCCOPY);

        if(!Local_CompGetCoord(compid,1,&cCoord,&cocCoord))return;

        BitBlt(dc,Components[compid].dx + xd,Components[compid].dy + yd + lval
               ,cCoord.w,cCoord.h - lval,ActionDisplay,cCoord.x,cCoord.y + lval,SRCCOPY);
        goto Point_DrawEnd;
    }

    /* check box (static width) cdt - 0, normal; 1,light */
    if(Components[compid].comptid == Comp_Check || Components[compid].comptid == Comp_Option)
    {
Point_DrawButton:
        if(Components[compid].value) /* checked */
        {
            if(!Local_CompGetCoord(compid,1,&cCoord,&cocCoord))return;

            Local_BlitCoord(dc,ActionDisplay,Components[compid].dx + xd,
                            Components[compid].dy + yd,&cocCoord);
            if(dloopback)
            {
                goto Point_DrawFill;
            }else{
                goto Point_DrawEnd;
            }
        }

        if((!cdt) && (!Components[compid].value)) /* normal */
        {
            if(!Local_CompGetCoord(compid,0,&cCoord,&cocCoord))return;

            Local_BlitCoord(dc,ActionDisplay,Components[compid].dx + xd,
                            Components[compid].dy + yd,&cCoord);

            if(dloopback)
            {
                goto Point_DrawFill;
            }else{
                goto Point_DrawEnd;
            }
        }

        if(cdt && (!Components[compid].value)) /* move */
        {
            if(!Local_CompGetCoord(compid,1,&cCoord,&cocCoord))return;
            Local_BlitCoord(dc,ActionDisplay,Components[compid].dx + xd,
                            Components[compid].dy + yd,&cCoord);
            if(dloopback)
            {
                goto Point_DrawFill;
            }else{
                goto Point_DrawEnd;
            }
        }
    }

    /* check box (variable width) */
    if(Components[compid].comptid == Comp_CheckFill || Components[compid].comptid == Comp_OptionFill)
    {
        dloopback = 1;

        if(!Local_CompGetCoord(compid,0,&cCoord,&cocCoord))return;
        /* draw fill background */

        if(!cdt)
        {

            Local_BlitSized(dc,ActionDisplay,Components[compid].dx + xd + Coord_OptionBox.w + 1,
                Components[compid].dy + yd - ((Coord_TailingFill.h - Coord_OptionBox.h)/2),Components[compid].ndata,&Coord_TailingFillL,&Coord_TailingFill,&Coord_TailingFillR);

            Local_ComponentTextOut(0,dc,Components[compid].dx + xd + Coord_OptionBox.w + 5,
                                Components[compid].dy + yd - ((Coord_TailingFill.h - Coord_OptionBox.h)/2) + 1,
                                Components[compid].cap);
        }else{

            Local_BlitSized(dc,ActionDisplay,Components[compid].dx + xd + Coord_OptionBox.w + 1,
                Components[compid].dy + yd - ((Coord_TailingFill.h - Coord_OptionBox.h)/2),Components[compid].ndata,&Coord_TailingFillHL,&Coord_TailingFillH,&Coord_TailingFillHR);

            Local_ComponentTextOut(1,dc,Components[compid].dx + xd + Coord_OptionBox.w + 5,
                                Components[compid].dy + yd - ((Coord_TailingFill.h - Coord_OptionBox.h)/2) + 1,
                                Components[compid].cap);
        }

        goto Point_DrawButton;

        Point_DrawFill:

        goto Point_DrawEnd;
    }

    /* Push button */
    if(Components[compid].comptid == Comp_Button && Components[compid].subid == CompData_ButtonPush)
    {
        HRGN crgn = CreateRectRgn(Components[compid].dx + xd, Components[compid].dy + yd, Components[compid].ndata + Components[compid].dx + xd, Coord_ButtonPushM.h + Components[compid].dy + yd);
        SelectClipRgn(dc, crgn);
        if(!cdt)
        {
            Local_BlitSized(dc, ActionDisplay, Components[compid].dx + xd, Components[compid].dy + yd, Components[compid].ndata, &Coord_ButtonPushL,&Coord_ButtonPushM,&Coord_ButtonPushR);
            Local_ComponentTextOut(0, dc, Components[compid].dx + xd + 2, Components[compid].dy + yd + 1, Components[compid].cap);
        }else{
            Local_BlitSized(dc, ActionDisplay, Components[compid].dx + xd, Components[compid].dy + yd, Components[compid].ndata, &Coord_ButtonPushHL,&Coord_ButtonPushHM,&Coord_ButtonPushHR);
            Local_ComponentTextOut(1, dc, Components[compid].dx + xd + 2, Components[compid].dy + yd + 1, Components[compid].cap);
        }
        SelectClipRgn(dc, 0);
        DeleteObject(crgn);
        goto Point_DrawEnd;
    }


    if(sizew == 0 && sizeh == 0)
    {
        BitBlt(dc,Components[compid].dx + xd,Components[compid].dy + yd
               ,cCoord.w,cCoord.h,ActionDisplay,cCoord.x,cCoord.y,SRCCOPY);

        goto Point_DrawEnd;
    }

    if(sizew && sizeh == 0)
    {
        if(Components[compid].comptid == Comp_ScrollH || Components[compid].comptid == Comp_LevelH)
        {

            Local_CompDrawResized(dc,Components[compid].comptid,0,Components[compid].dx + xd,Components[compid].dy + yd
                                  ,Components[compid].ndata,0,Components[compid].value /* ConvertPercentToNum(Components[compid].value,Components[compid].ndata) */
                                  ,&Coord_ScrollH_L,&Coord_ScrollH_M,&Coord_ScrollH_R
                                  ,&Coord_ScrollH_LM,ResizeMode_Horizontal);

        }


        goto Point_DrawEnd;
    }

    if(sizew == 0 && sizeh)
    {
        if(Components[compid].comptid == Comp_ScrollV || Components[compid].comptid == Comp_LevelV)
        {

            Local_CompDrawResized(dc,Components[compid].comptid,0,Components[compid].dx + xd,Components[compid].dy + yd
                                  ,0,Components[compid].ndata,Components[compid].value /* ConvertPercentToNum(Components[compid].value,Components[compid].ndata) */
                                  ,&Coord_ScrollV_T ,&Coord_ScrollV_M,&Coord_ScrollV_B
                                  ,&Coord_ScrollV_Btn,ResizeMode_Vertical);

        }


        goto Point_DrawEnd;
    }



Point_DrawEnd:
    return;

}

/*
 * set panel background bitmap.
 */

int Local_SetPanelBackground(const char *resid)
{
	if(!resid)
	{
		panelbk_black = 1;
		return 1;
	}

	if(bmpPanelBackground)
	{
		DeleteObject(bmpPanelBackground);

		bmpPanelBackground = png_res_load_winbmp(instance_fennec, uni("png"), resid);

		if(bmpPanelBackground)
		{
			HDC tdc = CreateCompatibleDC(window_main_dc);

			SelectObject(tdc, bmpPanelBackground);

			BitBlt(dcPanelBackground, 0, 0, Coord_DisplayW, Coord_DisplayH, tdc, 0, 0, SRCCOPY);

			DeleteDC(tdc);

			panelbk_black = 0;
			return 1;
		}
	}
	return 0;
}

/*
 * Draws a component into visible display (DC)
 * cdt : component details (lit or not etc.)
 */

void Local_CompDrawFast(unsigned long compid,int cdt)
{
    Local_CompDraw(compid,window_main_dc,cdt,Coord_DisplayX,Coord_DisplayY);
}


/*
 * Create a component
 * Note : the component id may change due to 'destruction' functions
 * Tip  : to delete use '...Destroy' or '...Clear'
 * 
 * cdata' : component data
 * cap'   : caption
 * ctip'  : tip text
 * cproc' : messages callback
 * return : component id (compid)
 */

int Local_ComponentCreate(int comptid,int x,int y,int w,int h,int cdata,char* cap,char* ctip,ComponentProc cproc)
{
#define ReturnError -1

    int    cid = Comp_Null; /* component id */
    int    sid = 0;         /* sub id */
    int    nd  = 0;         /* numeric data store */

    /* every error must 'return ReturnError' not break. */
    switch(comptid)
    {
    case Comp_Button:
        cid = Comp_Button;

        switch(cdata)
        {
        case CompData_ButtonRecord:    /* option */
        case CompData_ButtonMute:      /* option */
        case CompData_ButtonDisk:      /* option */
        case CompData_ButtonSWave:     /* option */
        case CompData_ButtonMixer:     /* option */
        case CompData_ButtonAutoMix:   /* option */
        case CompData_ButtonRepeat:    /* option */
        case CompData_ButtonLoop:      /* push */
        case CompData_ButtonSwitch:    /* push */
        case CompData_ButtonPush:
        case CompData_ButtonOption:
        case CompData_ButtonMInfo:
            sid = cdata;
            nd  = w;
            break;

        default:
            break;
        }
        break;

    /* resized ones (ndata = width) */

    case Comp_ScrollH:
    case Comp_LevelH:
    case Comp_CheckFill:
    case Comp_OptionFill:
        cid = comptid;
        nd  = w;
        break;

    /* resized ones (ndata = height) */

    case Comp_ScrollV:
    case Comp_LevelV:
        cid = comptid;
        nd  = h;
        break;

    /* static size components */

    case Comp_Check:
    case Comp_Option:
        cid = comptid;
        break;

    /* others */

    /* vertical triangular scroll (volume in panel main) */
    case Comp_TriScrollV:
        cid = Comp_TriScrollV;
        break;

    /* null, unknown, not availables */

    case Comp_Null:
        return ReturnError;

    case Comp_KnobL:
    case Comp_KnobB:
        LogMsgS("Fennec Display, Local_ComponentCreate. Component is not available.");
        return ReturnError;

    default:
        LogMsgS("Fennec Display, Local_ComponentCreate. Component is not defined.");
        return ReturnError;
    }

    /* creation start */

    ComponentCount++;

    if(ComponentCount >= ComponentsSize)
    {
        ComponentsSize += ComponentCountAdd;

        Components = sys_mem_realloc(Components, ((ComponentsSize + 5) * sizeof(DisplayComponent)));
    }

    Components[ComponentCount].comptid = cid;
    Components[ComponentCount].subid   = sid;
    Components[ComponentCount].dx      = x;
    Components[ComponentCount].dy      = y;
    Components[ComponentCount].ndata   = nd;
    Components[ComponentCount].value   = 0;
	Components[ComponentCount].inuse   = 0;
    Components[ComponentCount].cproc   = cproc;
    Components[ComponentCount].cap     = cap;
	Components[ComponentCount].ctip    = ctip;

    Local_CompDraw(ComponentCount,MemoryDisplay,0,0,0);

    return (int)ComponentCount;
}

/*
 * Update component and its data
 * cdata : component data
 * cap   : caption
 * ctip  : tip text
 */

int Local_ComponentUpdate(unsigned long compid,int x,int y,int w,int h,int cdata,int value,char* cap,char* ctip)
{
    if(compid > (int)ComponentCount)return 0;

    if(x >= 0)    Components[compid].dx      = x;
    if(y >= 0)    Components[compid].dy      = y;
    if(value >= 0)Components[compid].value   = value;

    if(Components[compid].comptid == Comp_ScrollH && w >= 0)
    {
        Components[compid].ndata = w;
    }

    Local_CompDraw(compid,MemoryDisplay,0,0,0);
    return 1;
}

int Local_ComponentSetValue(unsigned long compid,int val)
{
    if(compid > (int)ComponentCount)return 0;

    if(val >= 0)Components[compid].value   = val;

    /*Local_CompDraw(compid,MemoryDisplay,0,0,0);*/
    Local_CompDraw(compid,window_main_dc,0,Coord_DisplayX,Coord_DisplayY);
    return 1;
}

/*
 * Delete component by its id
 * Note - lower component ids can be changed thus try '...Clean'
 */

int Local_ComponentDestroy(unsigned long compid)
{

    /* this part really sucks, hay wait.. I found something to speed up
    oh it should work. (Think while writing comments, you'll get new ideas
    like that happend to me :-> ). we don't need to rearrange all, just
    replace the item with the last item and decrease count, (but one
    thing, we can't use static item handles) */

    if(((unsigned long)compid) > ComponentCount)return 0;
    if(compid == ComponentCount) /* just do nothing :-> */
    {
        ComponentCount--;
    }

    Components[compid] = Components[ComponentCount];
    ComponentCount--;
    return 1;
}

/*
 * Clear components list
 */

int Local_ComponentClear(void)
{
    /* really we don't need to do this cause the array should be
       resized to the maximum components count automatically
       but may be in the future we'll need this code (more componets)

    if(ComponentsSize >= ComponentCountInit)
    {
        ComponentsSize = ComponentCountInit;
        Components = (DisplayComponent*) sys_mem_realloc(Components,ComponentsSize * sizeof(DisplayComponent));
    }
    */

    ComponentCount = 0;
    return 1;
}

/*
 *Uninitialize component library
 */

int Local_ComponentUninitializeLibrary(void)
{
    if(Components)
    {
        /*free(Components);*/
    }
    return 1;
}

/*-----------------------------------------------------------------------------
 Local panel functions
 some'o the base calls should be on standard, three basic functions formatted
 to be like 'int Local_Panel.1._.2.' 1 - panel name, 2 - function show,timer
 and close, any'o Additional functions should be placed in middle (above close
 , the last function, and below timer, second function)
-----------------------------------------------------------------------------*/

int __inline Local_ScrollToPercent(int compid,int ndata)
{
    return ((int) ((((float)ndata) / ((float)Components[compid].ndata)) * 100));
}

/*
 * Panel main .................................................................
 */

char *local_gettitletext(string titlemem)
{
	struct fennec_audiotag at;
	unsigned long ipid;

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

	ipid = audio_input_tagread(audio_playlist_getsource(audio_playlist_getcurrentindex()), &at);

	tags_translate(titlemem, &at);

	audio_input_tagread_known(ipid, 0, &at); /* free */
	return titlemem;
}

int Local_PanelMain_Show(void)
{
//    BaseInfo*  bcurrentinfo;
/*  char       IABuffer[16]; */
    SIZE       titlesize;
    int        ps = 0;

    /* just place the buttons where they should be (I got a note :-p) */

    if(CurrentPanel != PanelID_Main)
    {
		PanelMain_LastPath[0] = 0;

		ComponentIDBase = (short)
		Local_ComponentCreate(Comp_Button    , 84                 , 17, 0  , 0, CompData_ButtonMute , 0, tip_text_conversion   , Local_PanelMain_Buttons);
		Local_ComponentCreate(Comp_Button    , 84 + 35            , 17, 0  , 0, CompData_ButtonDisk , 0, tip_text_ripping      , Local_PanelMain_Buttons);
		Local_ComponentCreate(Comp_Button    , 84 + 70            , 17, 0  , 0, CompData_ButtonSWave, 0, tip_text_joining      , Local_PanelMain_Buttons);
		Local_ComponentCreate(Comp_Button    , 84 + 105           , 17, 0  , 0, CompData_ButtonMixer, 0, tip_text_visualization, Local_PanelMain_Buttons);
        Local_ComponentCreate(Comp_Button    , Coord_DisplayW - 15, 3,  0  , 0, CompData_ButtonMInfo, 0, tip_text_tageditor    , Local_PanelMain_Buttons);
        Local_ComponentCreate(Comp_TriScrollV, 240                , 33, 0  , 0, 0                   , 0, tip_text_volume       , Local_PanelMain_Scrolls);
		Local_ComponentCreate(Comp_ScrollH   , 4                  , 55, 232, 0, 0                   , 0, 0                     , Local_PanelMain_Scrolls);
        Local_ComponentCreate(Comp_LevelH    , 4                  , 66, 232, 0, 0                   , 0, 0                     , Local_PanelMain_Scrolls);
        Local_ComponentCreate(Comp_LevelH    , 4                  , 74, 232, 0, 0                   , 0, 0                     , Local_PanelMain_Scrolls);


		Font_Bold = CreateFont(-MulDiv(8, GetDeviceCaps(DynamicDisplay, LOGPIXELSY), 72),
									0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
									DEFAULT_PITCH ,"Verdana");

		Font_Default = (HFONT)SelectObject(DynamicDisplay, Font_Bold);

        /* [todo] */ Components[ComponentIDBase + PanelMain_ScrollVolume].value = 100;

        SetBkColor(DynamicDisplay,0);
        SetBkMode(DynamicDisplay,OPAQUE);

		BitBlt(DynamicDisplay,0,0,Coord_DynamicW,Coord_DynamicH,0,0,0,BLACKNESS);
        SetTextColor(DynamicDisplay,Color_CurrentText);
    }

	if(audio_playlist_getcount())
	{
		if(display_refresh_level >= fennec_v_refresh_force_less)
		{
			str_cpy(PanelMain_Title, settings.formatting.main_title);

			str_cpy(PanelMain_LastPath, audio_playlist_getsource(audio_playlist_getcurrentindex()) );
			local_gettitletext(PanelMain_Title);

			GetTextExtentPoint(DynamicDisplay,PanelMain_Title,(int)str_len(PanelMain_Title),&titlesize);
			PanelMain_TitleWidth = (short) titlesize.cx;

			BitBlt(DynamicDisplay,0,0,Coord_DynamicW,Font_CurrentMetric.tmHeight,0,0,0,BLACKNESS);
			TextOut(DynamicDisplay,0,0,PanelMain_Title,(int)str_len(PanelMain_Title));

			Local_FastBlit(Coord_TextScrollX,0,Coord_TextScrollW,Font_CurrentMetric.tmHeight,DynamicDisplay,0,0);
			BitBlt(MemoryDisplay, Coord_TextScrollX, 0, Coord_TextScrollW,Font_CurrentMetric.tmHeight,DynamicDisplay,0,0, SRCCOPY);

			Local_PanelMain_Scroll(1);
		}

	}else{

        str_cpy(PanelMain_TitleTemp, PanelMain_Title);

        memset(PanelMain_Title,0,sizeof(PanelMain_Title));
        str_cpy(PanelMain_Title, uni("Fennec 7.1.1 Player - Stranger - Spring 2007, See About->Credits For More Information.   "));

        if(strcmp(PanelMain_Title,PanelMain_TitleTemp))
        {
            BitBlt(DynamicDisplay,0,0,Coord_DynamicW,Font_CurrentMetric.tmHeight,0,0,0,BLACKNESS);
            TextOut(DynamicDisplay,0,0,PanelMain_Title,(int)str_len(PanelMain_Title));

                        Local_FastBlit(Coord_TextScrollX,0,Coord_TextScrollW,Font_CurrentMetric.tmHeight,DynamicDisplay,0,0);
            BitBlt(MemoryDisplay, Coord_TextScrollX, 0, Coord_TextScrollW,Font_CurrentMetric.tmHeight,DynamicDisplay,0,0, SRCCOPY);

            Local_PanelMain_Scroll(1);
        }

        GetTextExtentPoint(DynamicDisplay,PanelMain_Title,(int)str_len(PanelMain_Title),&titlesize);
        PanelMain_TitleWidth = (short) titlesize.cx;
    }

	switch(audio_getplayerstate())
	{
	case audio_v_playerstate_null:
	case audio_v_playerstate_notinit:
	case audio_v_playerstate_init:
	case audio_v_playerstate_loaded:
	case audio_v_playerstate_buffering:
	case audio_v_playerstate_stopped:
	case audio_v_playerstate_opening:

        Local_BlitCoord(MemoryDisplay,ActionDisplay,4,36,&Coord_ImageStopped);
        break;

	case audio_v_playerstate_playing:
	case audio_v_playerstate_playingandbuffering:

        Local_BlitCoord(MemoryDisplay,ActionDisplay,4,36,&Coord_ImagePlaying);
        break;

	case audio_v_playerstate_paused:

        Local_BlitCoord(MemoryDisplay,ActionDisplay,4,36,&Coord_ImagePaused);
	}


	PanelMain_DurationMSL = PanelMain_DurationMS; /* kinda tricky optimization, should work :~) */
	PanelMain_DurationMS  = (unsigned long)audio_getduration_ms();

    Local_DrawElectronicDigits_Time(MemoryDisplay,42,20,PanelMain_DurationMS / 60000,(PanelMain_DurationMS / 1000) % 60);
    Local_DrawElectronicDigits_Time(window_main_dc,42 + Coord_DisplayX,20 + Coord_DisplayY,PanelMain_DurationMS / 60000,(PanelMain_DurationMS / 1000) % 60);

    ps = (int)(audio_getposition_ms() / 1000.0f);
    Local_DrawElectronicDigits_Time(MemoryDisplay,3,20,ps / 60,ps % 60);
    Local_DrawElectronicDigits_Time(window_main_dc,3  + Coord_DisplayX,20 + Coord_DisplayY,ps / 60,ps % 60);

	{
		double vl, vr;

		audio_getvolume(&vl, &vr);
		Components[ComponentIDBase + PanelMain_ScrollVolume].value = (int)((vl + vr) * 50.0f) ;
		Local_CompDrawFast(ComponentIDBase + PanelMain_ScrollVolume,0);
	}
    return 1;
}

int __stdcall Local_PanelMain_Buttons(int compid,unsigned long msg,unsigned long ndata,void* vdata)
{
	if(msg == CompMsg_MouseUp)
    {
        Components[compid].value ^= 1;

        switch(compid - ComponentIDBase)
        {
		case 0:
			BaseWindows_ShowConversion(1);
			break;

		case 1:
			BaseWindows_ShowRipping(1);
			break;

		case 2:
			BaseWindows_ShowJoining(1);
			break;

        case 3:
			Display_ShowPanel(PanelID_Visualization);
            break;

		case 4:
			basewindows_show_tagging(0, audio_playlist_getsource(audio_playlist_getcurrentindex()));
			break;

        default:
            break;
        }
	}
    return 1;
}

int __stdcall Local_PanelMain_Scrolls(int compid,unsigned long msg,unsigned long ndata,void* vdata)
{
    //if(msg != CompMsg_Scroll && msg != CompMsg_MouseDown)return 0;

    if(compid == (ComponentIDBase + PanelMain_ScrollPlayer1))
    {
		if(msg == CompMsg_Scroll || msg == CompMsg_MouseDown)
		{
			Components[compid].inuse = 1;
			Components[compid].value = (int)ConvertNumToPercent(ndata,Local_CompGetWidth(compid));
		}

		if(msg == CompMsg_MouseUp)
		{
			Components[compid].inuse = 0;
			audio_setposition((double)Components[compid].value / 100.0f);
		}
		/*
		Components[ComponentIDBase + PanelMain_ScrollPlayer2].value = Components[compid].value;

		Local_CompDrawFast(ComponentIDBase + PanelMain_ScrollPlayer2, 1);

    }else if(compid == ComponentIDBase + PanelMain_ScrollPlayer2){
        //if(LastPlayer != SelectedPlayer)
        //{
        Components[compid].value = (int)ConvertNumToPercent(ndata,Local_CompGetWidth(compid));
		Components[ComponentIDBase + PanelMain_ScrollPlayer1].value = Components[compid].value;
	
		Local_CompDrawFast(ComponentIDBase + PanelMain_ScrollPlayer1, 1);
		
		audio_setposition((double)Components[compid].value / 100.0f);

		//    InternalOut_SetPositionPercent(LastPlayer,Components[compid].value);
        //}*/

    }else if(compid == (ComponentIDBase + PanelMain_ScrollVolume)){
		if(msg != CompMsg_MouseUp)
		{
			Components[compid].value = (int)ConvertNumToPercent(ndata,Local_CompGetHeight(compid));
	      
			audio_setvolume(((double)Components[compid].value) / 100.0f, ((double)Components[compid].value) / 100.0f);
			
			if(Components[compid].value > 100)Components[compid].value = 100;
			if(Components[compid].value < 0)Components[compid].value = 0;

			if(Components[compid].value >= 98)
			{
				settings.player.volume_left  = 10000;
				settings.player.volume_right = 10000;

			}else if(Components[compid].value <= 2){

				settings.player.volume_left  = 0;
				settings.player.volume_right = 0;

			}else{

				settings.player.volume_left  = (unsigned short)(Components[compid].value * 100);
				settings.player.volume_right = (unsigned short)(Components[compid].value * 100);
			}
		}
	}
    return 1;
}

int Local_PanelMain_Scroll(int rx)
{
#define ExtraLength 50
#define TextInvisible 0xFFFF

    static int text1x = 0; /* real x axis 1*/
    static int text2x = TextInvisible; /* real x axis 2 (invisible at startup)*/
    static int cx     = 0; /* minor increase x (0 to ExtraLength)*/
    static int text1a = 1; /* active text 1 */
    static int text2a = 1; /* active text 2 */

    if(rx) /* reset values */
    {
        text1x = 0;
        text2x = TextInvisible;
        cx     = 0;
        text1a = 1;
        text2a = 1;
    }

    if(PanelMain_TitleWidth <= Coord_TextScrollW + 40)
		return 0;

    if(text1x != TextInvisible)
    {
        if(-(text1x - cx) > PanelMain_TitleWidth - Coord_TextScrollW && text1a)
        {
            text2x = PanelMain_TitleWidth - abs(text1x);
            text1a = 0;
            TextOut(DynamicDisplay, text2x, 0, PanelMain_Title, (int)str_len(PanelMain_Title));
        }

    }

    if(text2x != TextInvisible)
    {
        if(-(text2x - cx) > PanelMain_TitleWidth - Coord_TextScrollW && text2a)
        {
            text1x = PanelMain_TitleWidth - abs(text2x);
            text2a = 0;
            TextOut(DynamicDisplay, text1x, 0, PanelMain_Title, (int)str_len(PanelMain_Title));
        }
    }

    if(cx >= ExtraLength)
    {
        cx = 0;

        if(text1x != TextInvisible)
        {
            text1x -= ExtraLength;
            TextOut(DynamicDisplay, text1x, 0, PanelMain_Title, (int)str_len(PanelMain_Title));

            if(-text1x > PanelMain_TitleWidth)
            {
                text1x = TextInvisible;
                text1a = 1;
            }
        }


        if(text2x != TextInvisible)
        {
            text2x -= ExtraLength;
            TextOut(DynamicDisplay, text2x, 0, PanelMain_Title, (int)str_len(PanelMain_Title));

            if(-text2x > PanelMain_TitleWidth)
            {
                text2x = TextInvisible;
                text2a = 1;
            }
        }
    }


    Local_FastBlit(Coord_TextScrollX,0,Coord_TextScrollW,Font_CurrentMetric.tmHeight,DynamicDisplay,cx,0);
    BitBlt(MemoryDisplay, Coord_TextScrollX, 0, Coord_TextScrollW,Font_CurrentMetric.tmHeight,DynamicDisplay,cx,0, SRCCOPY);

    cx++;

    return 1;
}

int Local_PanelMain_Timer(void)
{
#define HoldDec 4

    static char spp = 0;
    static char spp2 = 0;
    static int pholdl = 0;
    static int pholdr = 0;
    unsigned long  peakl = 0,peakr = 0;

    int  pp;

    int ps = (int)audio_getposition_ms() / 1000;//InternalOut_GetPositionSeconds(SelectedPlayer);

    Local_DrawElectronicDigits_Time(window_main_dc,3  + Coord_DisplayX,20 + Coord_DisplayY,ps / 60,ps % 60);

    pp = (int)ConvertNumToPercent(ps, audio_getduration_ms() / 1000);//(int)ConvertNumToPercent(ps,(InternalOut_GetDurationMiliseconds(SelectedPlayer) / 1000));

    if(pp != spp && !Components[ComponentIDBase + PanelMain_ScrollPlayer1].inuse)
    {
		double audio_pos;
		audio_getposition(&audio_pos);

        Local_ComponentSetValue(ComponentIDBase + PanelMain_ScrollPlayer1, (int)(audio_pos * 100.0));
    }

    spp = (char)pp;

    /* else, use the player 2 or setting as the bottom seek */

    Local_PanelMain_Scroll(0);

    /* blit level (VU/Peak) meters */

	audio_getpeakpower(&peakl, &peakr);
	
	peakl /= 100;
	peakr /= 100;

    if(peakl >= (unsigned long)pholdl)
    {
        pholdl = peakl;
    }else{
        pholdl -= (pholdl >= HoldDec) ? HoldDec : pholdl;
        peakl = pholdl;
    }

    if(peakr >= (unsigned long)pholdr)
    {
        pholdr = peakr;
    }else{
        pholdr -= (pholdr >= HoldDec) ? HoldDec : pholdr;
        peakr = pholdr;
    }


    Local_ComponentSetValue(ComponentIDBase + PanelMain_ScrollLevel1,peakl);
    Local_ComponentSetValue(ComponentIDBase + PanelMain_ScrollLevel2,peakr);
    return 1;
}

int Local_PanelMain_Close(void)
{
    memset(PanelMain_Title,0,sizeof(PanelMain_Title));
	
	SelectObject(DynamicDisplay, Font_Default);
	DeleteObject(Font_Bold);
	
    Local_ComponentClear();
    Display_Erase();
    return 1;
}

/*
 * Panel about.credits ........................................................
 */

int Local_PanelCredits_Show(void)
{
    if(CurrentPanel == PanelID_Credits)return 1;

    Display_Erase();

    SetTextColor(MemoryDisplay,RGB(0,192,0));
    SetBkMode(MemoryDisplay,TRANSPARENT);

    Font_Bold = CreateFont(-MulDiv(8, GetDeviceCaps(MemoryDisplay, LOGPIXELSY), 72),
                                0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,
                                DEFAULT_PITCH ,"Verdana");
    Font_SizeSmall =  CreateFont(-MulDiv(8, GetDeviceCaps(MemoryDisplay, LOGPIXELSY), 72),
                                0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,
                                DEFAULT_PITCH ,"Verdana");

    PanelBrush1 = CreateSolidBrush(RGB(0,128,0));
    PanelPen1   = CreatePen(PS_SOLID,1,RGB(0,64,0));
    PanelBrush2 = CreateSolidBrush(RGB(0,0,128));
    PanelPen2   = CreatePen(PS_SOLID,1,RGB(0,0,64));

    PanelBrushDefault = (HBRUSH) SelectObject(window_main_dc,PanelBrush1);
    PanelPenDefault   = (HPEN)   SelectObject(window_main_dc,PanelPen1);

    PanelBrushDefaultEx1 = (HBRUSH) SelectObject(MemoryDisplay,PanelBrush2);
    PanelPenDefaultEx1   = (HPEN)   SelectObject(MemoryDisplay,PanelPen2);
    return 1;
}


/*
 * credits text formatting for credits display engine
 * % - normal text
 * ~ - Bold text
 * ` - End of the slide
 * $ - New line
 */

int Local_PanelCredits_Timer(void)
{

    return 1;
}

int Local_PanelCredits_Close(void)
{
    SelectObject(MemoryDisplay,Font_Default);
    SelectObject(window_main_dc,PanelBrushDefault);
    SelectObject(window_main_dc,PanelPenDefault);
    SelectObject(MemoryDisplay,PanelBrushDefaultEx1);
    SelectObject(MemoryDisplay,PanelPenDefaultEx1);
    DeleteObject(PanelBrush1);
    DeleteObject(PanelPen1);
    DeleteObject(PanelBrush2);
    DeleteObject(PanelPen2);
    Local_ComponentClear();
    Display_Erase();
    return 1;
}

/*
 * Panel Equalizer ............................................................
 */


int Local_PanelEqualizer_Show(void)
{
	static unsigned long last_eqp_count = 0;
    int cr = 10;
    int i;
    EqualizerPreset eqp;
    unsigned long dsz = 0;

    if(CurrentPanel != PanelID_Equalizer)
    {

        Display_Erase();

        ComponentIDBase = (short)

        Local_ComponentCreate(Comp_ScrollV, cr + 2 + ((Coord_ScrollV_M.w + 2) * 0),20,0,  Coord_DisplayH - 22 - 10,0,0,tip_text_equalizer_preampleft,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 2 + ((Coord_ScrollV_M.w + 2) * 1),20,0,  Coord_DisplayH - 22 - 10,0,0,tip_text_equalizer_preampright,Local_PanelEqualizer_Scrolls);

        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 2),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 3),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 4),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 5),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 6),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 7),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 8),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 9),20,0,  Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 10),20,0, Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 8 + ((Coord_ScrollV_M.w + 3) * 11),20,0, Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);

        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 12),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 13),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 14),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 15),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 16),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 17),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 18),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 19),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 20),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);
        Local_ComponentCreate(Comp_ScrollV, cr + 16 + ((Coord_ScrollV_M.w + 3) * 21),20,0,Coord_DisplayH - 22 - 10,0,0,0,Local_PanelEqualizer_Scrolls);

        if(settings.player.equalizer_enable)
        {
            Local_ComponentCreate(Comp_Button,2,2,50,0,CompData_ButtonPush,"On",0,Local_PanelEqualizer_Buttons);
        }else{
            Local_ComponentCreate(Comp_Button,2,2,50,0,CompData_ButtonPush,"Off",0,Local_PanelEqualizer_Buttons);
        }

        Local_ComponentCreate(Comp_Button,Coord_DisplayW - 122,2,120,0,CompData_ButtonPush,"Rock",tip_text_equalizer_presets,Local_PanelEqualizer_Buttons);
	
		PanelEqualizer_PresetMain = CreatePopupMenu();

		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_STRING, GeneralMenu_Equalizer_SavePreset,   "Save Preset");
		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_STRING, GeneralMenu_Equalizer_DeletePreset, "Delete Preset");
		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_STRING, GeneralMenu_Equalizer_RenamePreset, "Rename Preset");
  		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_STRING, GeneralMenu_Equalizer_SaveToDisc,   "Save Presets As...");
  		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_STRING, GeneralMenu_Equalizer_LoadFromDisc, "Load Preset Files...");
  		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
		InsertMenu(PanelEqualizer_PresetMain, (UINT)-1, MF_BYPOSITION | MF_STRING, GeneralMenu_Equalizer_Clean,        "Clean");

		PanelEqualizer_PresetSelect = CreatePopupMenu();
		for(i=0; i<settings.player.equalizer_presets; i++)
		{
			memset(eqp.pname, 0, 30);
			settings_data_get(setting_id_equalizer_preset, i, &eqp, &dsz);
			if(dsz)
			{
				InsertMenu(PanelEqualizer_PresetSelect,(UINT)-1,MF_BYPOSITION | MF_STRING,GeneralMenu_Equalizer_PresetSelect + i,eqp.pname);
			}
		}
		last_eqp_count = settings.player.equalizer_presets;
	}

	if((settings.player.equalizer_presets != last_eqp_count) || Display_EqFlush)
	{
		Display_EqFlush = 0;
		if(PanelEqualizer_PresetSelect)DestroyMenu(PanelEqualizer_PresetSelect);

		PanelEqualizer_PresetSelect = CreatePopupMenu();
		for(i=0; i<settings.player.equalizer_presets; i++)
		{
			memset(eqp.pname, 0, 30);
			settings_data_get(setting_id_equalizer_preset, i, &eqp, &dsz);
			if(dsz)
			{
				InsertMenu(PanelEqualizer_PresetSelect,(UINT)-1,MF_BYPOSITION | MF_STRING,GeneralMenu_Equalizer_PresetSelect + i,eqp.pname);
			}
		}
		last_eqp_count = settings.player.equalizer_presets;
	}

    Components[ComponentIDBase + PanelEqualizer_Preset].cap = settings.player.equalizer_last_preset;

    Components[ComponentIDBase + PanelEqualizer_PreampLeft].value  = (int) ((GetDecibelPreampValue(0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_PreampRight].value = (int) ((GetDecibelPreampValue(1) / 12.0f) * 50) + 50;

    Components[ComponentIDBase + PanelEqualizer_BandLeft1].value  = (int) ((GetDecibelBandValue(0,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft2].value  = (int) ((GetDecibelBandValue(1,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft3].value  = (int) ((GetDecibelBandValue(2,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft4].value  = (int) ((GetDecibelBandValue(3,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft5].value  = (int) ((GetDecibelBandValue(4,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft6].value  = (int) ((GetDecibelBandValue(5,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft7].value  = (int) ((GetDecibelBandValue(6,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft8].value  = (int) ((GetDecibelBandValue(7,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft9].value  = (int) ((GetDecibelBandValue(8,0) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandLeft10].value = (int) ((GetDecibelBandValue(9,0) / 12.0f) * 50) + 50;

    Components[ComponentIDBase + PanelEqualizer_BandRight1].value  = (int) ((GetDecibelBandValue(0,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight2].value  = (int) ((GetDecibelBandValue(1,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight3].value  = (int) ((GetDecibelBandValue(2,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight4].value  = (int) ((GetDecibelBandValue(3,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight5].value  = (int) ((GetDecibelBandValue(4,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight6].value  = (int) ((GetDecibelBandValue(5,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight7].value  = (int) ((GetDecibelBandValue(6,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight8].value  = (int) ((GetDecibelBandValue(7,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight9].value  = (int) ((GetDecibelBandValue(8,1) / 12.0f) * 50) + 50;
    Components[ComponentIDBase + PanelEqualizer_BandRight10].value = (int) ((GetDecibelBandValue(9,1) / 12.0f) * 50) + 50;

    return 1;
}

int __stdcall  Local_PanelEqualizer_Buttons(int compid,unsigned long msg,unsigned long ndata,void* vdata)
{
    switch(compid - ComponentIDBase)
    {
    case PanelEqualizer_OnOff:
        if(msg == CompMsg_MouseUp)settings.player.equalizer_enable ^= 1;
        if(settings.player.equalizer_enable)
        {
            Components[compid].cap = "On";
        }else{
            Components[compid].cap = "Off";
        }
        break;

    case PanelEqualizer_Preset:

        if(msg == CompMsg_MUpL)
        {
            POINT menupt;
            GetCursorPos(&menupt);
            TrackPopupMenu(PanelEqualizer_PresetSelect, TPM_LEFTALIGN | TPM_LEFTBUTTON, menupt.x,menupt.y,0,window_main,0);

        }else if(msg == CompMsg_MUpR){
            POINT menupt;
            GetCursorPos(&menupt);
            TrackPopupMenu(PanelEqualizer_PresetMain, TPM_LEFTALIGN | TPM_LEFTBUTTON, menupt.x,menupt.y,0,window_main,0);
		}
        break;
    }

    return 1;
}

int __stdcall  Local_PanelEqualizer_Scrolls(int compid,unsigned long msg,unsigned long ndata,void* vdata)
{
    int rv = 0;
    float bv = 0.0f;

    if(ndata < 0)ndata = 0;
    if(ndata > 100)ndata = 100;

    if(settings.player.equalizer_last_preset_id != 0xFFFF) /* custom */
    {
        settings.player.equalizer_last_preset_id = 0xFFFF;
        memset(settings.player.equalizer_last_preset, 0, sizeof(settings.player.equalizer_last_preset));
        strcpy(settings.player.equalizer_last_preset, "Custom");
    }

    switch(compid - ComponentIDBase)
    {
    case PanelEqualizer_PreampLeft:
        if(ndata < 55 && ndata > 45)
        {
            Components[compid].value = 50;
            rv = 50;
        }else{

            rv = ndata;
        }

        Components[compid].value = rv;
        if(msg == CompMsg_ScrollL)Components[ComponentIDBase + PanelEqualizer_PreampRight].value = rv;
        Local_CompDrawFast(ComponentIDBase + PanelEqualizer_PreampRight,0);

        bv = (((((float)rv) - 50.0f)) / 50.0f) * 12.0f;

        if(msg == CompMsg_ScrollL)
        {
            SetDecibelPreampValue(1, bv);
            settings.player.equalizer_last_bands[1] = bv;
        }
        SetDecibelPreampValue(0, bv);
        settings.player.equalizer_last_bands[0] = bv;
        break;

    case PanelEqualizer_PreampRight:
        if(ndata < 55 && ndata > 45)
        {
            rv = 50;
        }else{
            rv = ndata;
        }

        Components[compid].value = rv;
        if(msg == CompMsg_ScrollL)Components[ComponentIDBase + PanelEqualizer_PreampLeft].value = rv;
        Local_CompDrawFast(ComponentIDBase + PanelEqualizer_PreampLeft,0);

        bv = (((((float)rv) - 50.0f)) / 50.0f) * 12.0f;

        if(msg == CompMsg_ScrollL)
        {
            SetDecibelPreampValue(0, bv);
            settings.player.equalizer_last_bands[0] = bv;
        }
        SetDecibelPreampValue(1, bv);
        settings.player.equalizer_last_bands[1] = bv;
        break;


    case PanelEqualizer_BandLeft1:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(0,0,bv);
        settings.player.equalizer_last_bands[2] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight1].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight1,0);
            SetDecibelBandValue(0,1,bv);
            settings.player.equalizer_last_bands[12] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft2:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(1,0,bv);
        settings.player.equalizer_last_bands[3] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight2].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight2,0);
            SetDecibelBandValue(1,1,bv);
            settings.player.equalizer_last_bands[13] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft3:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(2,0,bv);
        settings.player.equalizer_last_bands[4] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight3].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight3,0);
            SetDecibelBandValue(2,1,bv);
            settings.player.equalizer_last_bands[14] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft4:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(3,0,bv);
        settings.player.equalizer_last_bands[5] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight4].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight4,0);
            SetDecibelBandValue(3,1,bv);
            settings.player.equalizer_last_bands[15] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft5:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(4,0,bv);
        settings.player.equalizer_last_bands[6] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight5].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight5,0);
            SetDecibelBandValue(4,1,bv);
            settings.player.equalizer_last_bands[16] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft6:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(5,0,bv);
        settings.player.equalizer_last_bands[7] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight6].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight6,0);
            SetDecibelBandValue(5,1,bv);
            settings.player.equalizer_last_bands[17] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft7:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(6,0,bv);
        settings.player.equalizer_last_bands[8] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight7].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight7,0);
            SetDecibelBandValue(6,1,bv);
            settings.player.equalizer_last_bands[18] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft8:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(7,0,bv);
        settings.player.equalizer_last_bands[9] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight8].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight8,0);
            SetDecibelBandValue(7,1,bv);
            settings.player.equalizer_last_bands[19] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft9:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(8,0,bv);
        settings.player.equalizer_last_bands[10] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight9].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight9,0);
            SetDecibelBandValue(8,1,bv);
            settings.player.equalizer_last_bands[20] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandLeft10:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(9,0,bv);
        settings.player.equalizer_last_bands[11] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandRight10].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandRight10,0);
            SetDecibelBandValue(9,1,bv);
            settings.player.equalizer_last_bands[21] = bv;
        }
        Components[compid].value = ndata;
        break;


    /* right channel */

    case PanelEqualizer_BandRight1:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(0,1,bv);
        settings.player.equalizer_last_bands[12] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft1].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft1,0);
            SetDecibelBandValue(0,0,bv);
            settings.player.equalizer_last_bands[2] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight2:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(1,1,bv);
        settings.player.equalizer_last_bands[13] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft2].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft2,0);
            SetDecibelBandValue(1,0,bv);
            settings.player.equalizer_last_bands[3] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight3:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(2,1,bv);
        settings.player.equalizer_last_bands[14] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft3].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft3,0);
            SetDecibelBandValue(2,0,bv);
            settings.player.equalizer_last_bands[4] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight4:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(3,1,bv);
        settings.player.equalizer_last_bands[15] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft4].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft4,0);
            SetDecibelBandValue(3,0,bv);
            settings.player.equalizer_last_bands[5] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight5:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(4,1,bv);
        settings.player.equalizer_last_bands[16] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft5].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft5,0);
            SetDecibelBandValue(4,0,bv);
            settings.player.equalizer_last_bands[6] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight6:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(5,1,bv);
        settings.player.equalizer_last_bands[17] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft6].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft6,0);
            SetDecibelBandValue(5,0,bv);
            settings.player.equalizer_last_bands[7] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight7:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(6,1,bv);
        settings.player.equalizer_last_bands[18] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft7].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft7,0);
            SetDecibelBandValue(6,0,bv);
            settings.player.equalizer_last_bands[8] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight8:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(7,1,bv);
        settings.player.equalizer_last_bands[19] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft8].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft8,0);
            SetDecibelBandValue(7,0,bv);
            settings.player.equalizer_last_bands[9] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight9:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(8,1,bv);
        settings.player.equalizer_last_bands[20] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft9].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft9,0);
            SetDecibelBandValue(8,0,bv);
            settings.player.equalizer_last_bands[10] = bv;
        }
        Components[compid].value = ndata;
        break;
    case PanelEqualizer_BandRight10:
        bv = ConvertBPercentToNum(ndata,12);
        SetDecibelBandValue(9,1,bv);
        settings.player.equalizer_last_bands[21] = bv;
        if(msg == CompMsg_ScrollL)
        {
            Components[ComponentIDBase + PanelEqualizer_BandLeft10].value = ndata;
            Local_CompDrawFast(ComponentIDBase + PanelEqualizer_BandLeft10,0);
            SetDecibelBandValue(9,0,bv);
            settings.player.equalizer_last_bands[11] = bv;
        }
        Components[compid].value = ndata;
        break;


    }

    return 1;
}

int Local_PanelEqualizer_Close(void)
{
    Local_ComponentClear();
    if(PanelEqualizer_PresetMain) DestroyMenu(PanelEqualizer_PresetMain);
    if(PanelEqualizer_PresetSelect) DestroyMenu(PanelEqualizer_PresetSelect);
    Display_Erase();
    return 1;
}

/*
 * Panel settings.skin color (theme) ..........................................
 */

int Local_PanelColor_Show(void)
{
    unsigned char r,g,b;
	if(CurrentPanel == PanelID_Color)return 1;

    Display_Erase();

    if(CurrentPanel != PanelID_Color)
    {
        ComponentIDBase = (short)
        Local_ComponentCreate(Comp_ScrollH,25,10,200,0,0,0,tip_text_color_hue,Local_PanelColor_Proc);
        Local_ComponentCreate(Comp_ScrollH,25,30,200,0,0,0,tip_text_color_saturation,Local_PanelColor_Proc);
        Local_ComponentCreate(Comp_ScrollH,25,50,200,0,0,0,tip_text_color_lightness,Local_PanelColor_Proc);
        Local_ComponentCreate(Comp_Button,180,60,50,10,CompData_ButtonPush,"Apply",0,Local_PanelColor_Proc);
		switch(PanelColor_Selected)
		{
		case PanelColor_Skin:
			Local_ComponentCreate(Comp_Button,125,60,50,10,CompData_ButtonPush,"Skin",0,Local_PanelColor_Proc);
			break;

		case PanelColor_Display:
			Local_ComponentCreate(Comp_Button,125,60,50,10,CompData_ButtonPush,"Display",0,Local_PanelColor_Proc);
			break;
		}
    }

	switch(PanelColor_Selected)
	{
	case PanelColor_Skin:
		Components[ComponentIDBase + PanelColor_ScrollH].value = settings.player.themecolor_h;
		Components[ComponentIDBase + PanelColor_ScrollS].value = settings.player.themecolor_s;
		Components[ComponentIDBase + PanelColor_ScrollL].value = settings.player.themecolor_l;

		color_hsv_2_rgb_fullint((unsigned short)(settings.player.themecolor_h * 3.5), (unsigned char)(settings.player.themecolor_s * 2.5), (unsigned char)(settings.player.themecolor_l * 2.5), &r, &g, &b);
		Local_DrawRect(MemoryDisplay, 20, 60, 98, 16, RGB(r, g, b));
		break;

	case PanelColor_Display:
		Components[ComponentIDBase + PanelColor_ScrollH].value = settings.player.displaycolor_h;
		Components[ComponentIDBase + PanelColor_ScrollS].value = settings.player.displaycolor_s;
		Components[ComponentIDBase + PanelColor_ScrollL].value = settings.player.displaycolor_l;

		color_hsv_2_rgb_fullint((unsigned short)(settings.player.displaycolor_h * 3.5), (unsigned char)(settings.player.displaycolor_s * 2.5), (unsigned char)(settings.player.displaycolor_l * 2.5), &r, &g, &b);
		Local_DrawRect(MemoryDisplay, 20, 60, 98, 16, RGB(r, g, b));
		break;
	}

    return 1;
}

int __stdcall  Local_PanelColor_Proc(int compid,unsigned long msg,unsigned long ndata,void* vdata)
{
    switch(compid - ComponentIDBase)
    {
    case PanelColor_ScrollH:
    case PanelColor_ScrollS:
    case PanelColor_ScrollL:
        if(msg == CompMsg_Scroll)
        {
			unsigned char r,g,b;
            Components[compid].value = (int)ConvertNumToPercent(ndata, Components[compid].ndata);
			
			color_hsv_2_rgb_fullint((unsigned short)(Components[ComponentIDBase + PanelColor_ScrollH].value * 3.5), (unsigned char)(Components[ComponentIDBase + PanelColor_ScrollS].value * 2.5), (unsigned char)(Components[ComponentIDBase + PanelColor_ScrollL].value * 2.5), &r, &g, &b);
			
			Local_DrawRect(window_main_dc, Coord_DisplayX + 20, Coord_DisplayY + 60, 98, 16, RGB(r, g, b));
			Local_DrawRect(MemoryDisplay, 20, 60, 98, 16, RGB(r, g, b));

			// goto ColorApply;
        }
        break;

	case PanelColor_Switch:
		if(msg == CompMsg_MouseUp)
		{
			unsigned char r,g,b;

			switch(PanelColor_Selected)
			{
			case PanelColor_Skin:
				PanelColor_Selected = PanelColor_Display;
				Components[ComponentIDBase + PanelColor_Switch].cap = "Display";
				break;

			case PanelColor_Display:
				PanelColor_Selected = PanelColor_Skin;
				Components[ComponentIDBase + PanelColor_Switch].cap = "Skin";
				break;
			}

			switch(PanelColor_Selected)
			{
			case PanelColor_Skin:
				Components[ComponentIDBase + PanelColor_ScrollH].value = settings.player.themecolor_h;
				Components[ComponentIDBase + PanelColor_ScrollS].value = settings.player.themecolor_s;
				Components[ComponentIDBase + PanelColor_ScrollL].value = settings.player.themecolor_l;
			
				Local_CompDrawFast(ComponentIDBase + PanelColor_ScrollH,0);
				Local_CompDrawFast(ComponentIDBase + PanelColor_ScrollS,0);
				Local_CompDrawFast(ComponentIDBase + PanelColor_ScrollL,0);

				color_hsv_2_rgb_fullint((unsigned short)(settings.player.themecolor_h * 3.5), (unsigned char)(settings.player.themecolor_s * 2.5), (unsigned char)(settings.player.themecolor_l * 2.5), &r, &g, &b);
				Local_DrawRect(window_main_dc, Coord_DisplayX + 20, Coord_DisplayY + 60, 98, 16, RGB(r, g, b));
				Local_DrawRect(MemoryDisplay, 20, 60, 98, 16, RGB(r, g, b));
				break;

			case PanelColor_Display:
				Components[ComponentIDBase + PanelColor_ScrollH].value = settings.player.displaycolor_h;
				Components[ComponentIDBase + PanelColor_ScrollS].value = settings.player.displaycolor_s;
				Components[ComponentIDBase + PanelColor_ScrollL].value = settings.player.displaycolor_l;
				
				Local_CompDrawFast(ComponentIDBase + PanelColor_ScrollH,0);
				Local_CompDrawFast(ComponentIDBase + PanelColor_ScrollS,0);
				Local_CompDrawFast(ComponentIDBase + PanelColor_ScrollL,0);

				color_hsv_2_rgb_fullint((unsigned short)(settings.player.displaycolor_h * 3.5), (unsigned char)(settings.player.displaycolor_s * 2.5), (unsigned char)(settings.player.displaycolor_l * 2.5), &r, &g, &b);
				Local_DrawRect(window_main_dc, Coord_DisplayX + 20, Coord_DisplayY + 60, 98, 16, RGB(r, g, b));
				Local_DrawRect(MemoryDisplay, 20, 60, 98, 16, RGB(r, g, b));

				break;
			}

		}
		break;

    case PanelColor_Apply:
        if(msg == CompMsg_MouseUp)
        {
//ColorApply:	

			if(PanelColor_Selected == PanelColor_Skin)
			{
				settings.player.themecolor_h = (unsigned short)Components[ComponentIDBase + PanelColor_ScrollH].value;
				settings.player.themecolor_s = (unsigned short)Components[ComponentIDBase + PanelColor_ScrollS].value;
				settings.player.themecolor_l = (unsigned short)Components[ComponentIDBase + PanelColor_ScrollL].value;
	            
				BaseSkin_SetColor(
				Components[ComponentIDBase + PanelColor_ScrollH].value,
				Components[ComponentIDBase + PanelColor_ScrollS].value,
				Components[ComponentIDBase + PanelColor_ScrollL].value);

			}else if(PanelColor_Selected == PanelColor_Display){
				HBITMAP vbitmap;

				settings.player.displaycolor_h = (unsigned short)Components[ComponentIDBase + PanelColor_ScrollH].value;
				settings.player.displaycolor_s = (unsigned short)Components[ComponentIDBase + PanelColor_ScrollS].value;
				settings.player.displaycolor_l = (unsigned short)Components[ComponentIDBase + PanelColor_ScrollL].value;
	            
				

				vbitmap = png_res_load_winbmp(instance_fennec, uni("png"), uni("display.actions"));
				AdjustColors(vbitmap, 0, Components[ComponentIDBase + PanelColor_ScrollH].value,
										 Components[ComponentIDBase + PanelColor_ScrollS].value,
										 Components[ComponentIDBase + PanelColor_ScrollL].value);
				
				SelectObject(ActionDisplay, vbitmap);
				DeleteObject(vbitmap);

				/*Display_Refresh();*/
				fennec_refresh(fennec_v_refresh_force_not);

			}


        }
        break;
    }

    return 1;
}

int Local_PanelColor_Close(void)
{
    Local_ComponentClear();
    Display_Erase();
    return 1;
}

void Display_Debug(int v)
{

}

/* --------------------------------------------------------------------------*/

int __stdcall Local_PanelVisualization_Buttons(int compid,unsigned long msg,unsigned long ndata,void* vdata)
{
	if(msg == Display_Mouse_Down && compid == -1) /* mouse down */
	{
		PanelVisualization_Mode += 1;
		if(PanelVisualization_Mode >= PanelVisualization_Modes)PanelVisualization_Mode = 0;
	}
	return 1;
}

int Local_PanelVisualization_Show(void)
{
	unsigned long i;

    if(CurrentPanel == PanelID_Visualization)return 1;

	Display_CurrentGeneralProc = Local_PanelVisualization_Buttons;

	if(PanelVisualization_AudioBuffer)
	{
		sys_mem_free(PanelVisualization_AudioBuffer);
		PanelVisualization_AudioBuffer = 0;
	}

	if(PanelVisualization_Audio)
	{
		sys_mem_free(PanelVisualization_Audio);
		PanelVisualization_Audio = 0;
	}

	if(PanelVisualization_FFTOut)
	{
		sys_mem_free(PanelVisualization_FFTOut);
		PanelVisualization_FFTOut = 0;
	}

	if(PanelVisualization_Peaks)
	{
		sys_mem_free(PanelVisualization_Peaks);
		PanelVisualization_Peaks = 0;
	}

	if(PanelVisualization_Falloff)
	{
		sys_mem_free(PanelVisualization_Falloff);
		PanelVisualization_Falloff = 0;
	}

	PanelVisualization_FFTSize   = 1024;
	PanelVisualization_Audio       = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float));
	PanelVisualization_FFTOut      = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float));
	PanelVisualization_Peaks       = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float) / 2);
	PanelVisualization_Falloff     = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float) / 2);

	PanelVisualization_fft_i = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float));
	PanelVisualization_fft_r = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float));
	PanelVisualization_PeaksMax = (float*) sys_mem_alloc(PanelVisualization_FFTSize * sizeof(float));

	for(i=0; i<PanelVisualization_FFTSize; i++)
	{
		unsigned int j;
		
		double mr = 0.0f;
		double mi = 0.0f;

		for(j=0; j<PanelVisualization_FFTSize; j++)
		{
			mi += 32768.0f * fabs(sin((6.2831853071795f * ((double)j) * ((double)i)) / ((double)(PanelVisualization_FFTSize))));
			mr += 32768.0f * fabs(cos((6.2831853071795f * ((double)j) * ((double)i)) / ((double)(PanelVisualization_FFTSize))));
		}

		mi /= 10.0f * ((double)(i + 40) * 0.02f);
		mr /= 10.0f * ((double)(i + 40) * 0.02f);

		PanelVisualization_PeaksMax[i] = (float) sqrt((mr * mr) - (mi + mi));
	}

	for(i=0; i<PanelVisualization_FFTSize/2; i++)
	{
		PanelVisualization_Peaks[i] = 0.0f;
		PanelVisualization_Falloff[i] = 0.0f;
	}
    Display_Erase();

	return 1;
}

void Local_Effect_BlurMem(void)
{
	int x,y, r, g, b;
	COLORREF col;

	for(x=0; x<Coord_DisplayW; x++)
	{
		for(y=0; y<Coord_DisplayH; y++)
		{
			col = GetPixel(MemoryDisplay, x, y);
			r = GetRValue(col);
			g = GetGValue(col);
			b = GetBValue(col);

			col = GetPixel(MemoryDisplay, x + 1, y);
			r += GetRValue(col);
			g += GetGValue(col);
			b += GetBValue(col);

			col = GetPixel(MemoryDisplay, x - 1, y);
			r += GetRValue(col);
			g += GetGValue(col);
			b += GetBValue(col);

			r /= 3;
			g /= 3;
			b /= 3;
		}
	}
}

void Local_Effect_BlurFast(void)
{
	int x,y, r, g, b;
	COLORREF col;

	for(x=Coord_DisplayX; x<Coord_DisplayW+Coord_DisplayX; x++)
	{
		for(y=Coord_DisplayY; y<Coord_DisplayH+Coord_DisplayY; y++)
		{
			col = GetPixel(window_main_dc, x, y);
			r = GetRValue(col);
			g = GetGValue(col);
			b = GetBValue(col);

			col = GetPixel(window_main_dc, x + 1, y);
			r += GetRValue(col);
			g += GetGValue(col);
			b += GetBValue(col);

			col = GetPixel(window_main_dc, x - 1, y);
			r += GetRValue(col);
			g += GetGValue(col);
			b += GetBValue(col);

			r /= 3;
			g /= 3;
			b /= 3;

			SetPixelV(window_main_dc, x, y, RGB(r, g, b));
		}
	}
}

int Local_PanelVisualization_Timer(void)
{
	unsigned long i;
	static int   lastmode = 0;
	static float fallcoeff[1024];
	static float lps[1];
	static float hps[300];
	static float sf[1024];
	float        tmp;
	int          rbb = 0; /* real buffer band */
	int          freq = 0;

	#define      coeff 10
	#define      coeffgravity 25.0f
	#define      coeffgravityvinc 0.2f

	POINT   fpt[512];

#define voiceprint 1

if(PanelVisualization_Mode == 0)
{
	/* spectrum analyzer */

	audio_getfloatbuffer(PanelVisualization_Audio, PanelVisualization_FFTSize, 1);
	fft_float(1024, 0, PanelVisualization_Audio, 0, PanelVisualization_fft_r, PanelVisualization_fft_i);

	Display_Erase_MemoryDC();
	
	audio_getdata(audio_v_data_frequency, &freq);

	for(i=0; i<(unsigned long)Coord_DisplayW/coeff; i++)
	{

		/* peak summation */

		{
			int   j;
			int   mx;
			float sumfreq = 0.0f;

			if(freq > 20000)
				mx = (int)((512.0f * (20000.0f / (double)freq)) / ((float)Coord_DisplayW / (float)coeff));
			else
				mx = (int)(512.0f / ((float)Coord_DisplayW / (float)coeff));

			for(j=0; j<=mx; j++)
			{
				rbb = (i * mx) + j;

				tmp = (float)sqrt((PanelVisualization_fft_r[rbb] * PanelVisualization_fft_r[rbb]) + (PanelVisualization_fft_i[rbb] * PanelVisualization_fft_i[rbb]));
			
				tmp = ((tmp * (float)Coord_DisplayH) / PanelVisualization_PeaksMax[rbb]);
			
				if(tmp < 0.0f)tmp = -tmp;

				sumfreq += tmp / (float)mx;
			}
			
			tmp = sumfreq;
		}
		if(PanelVisualization_Peaks[rbb] > (float)Coord_DisplayH)PanelVisualization_Peaks[rbb] = (float)Coord_DisplayH;

		/* compare peaks */

		if(PanelVisualization_Peaks[rbb] <= tmp)
		{
			PanelVisualization_Peaks[rbb]   = tmp;
		}else{
			PanelVisualization_Peaks[rbb]   -= 8.0f;
			if(PanelVisualization_Peaks[rbb] < 0.0f)PanelVisualization_Peaks[rbb] = 0.0f;
		}

		/* calculate falloff */
		
		if(PanelVisualization_Falloff[rbb] <= tmp)
		{
			fallcoeff[rbb] = (tmp - PanelVisualization_Peaks[rbb]) / (float)Coord_DisplayH / coeffgravity;
			PanelVisualization_Falloff[rbb] = tmp;
		}else{
			PanelVisualization_Falloff[rbb] -= fallcoeff[rbb];
			fallcoeff[rbb]                  += coeffgravityvinc;
		}

		if(PanelVisualization_Falloff[rbb] > Coord_DisplayH)PanelVisualization_Falloff[rbb] = (float)Coord_DisplayH;
		
		/* draw */
		
		Local_DisplayFastRect_Vis(MemoryDisplay, 0x00CC00, (i * coeff) + 2, Coord_DisplayH - (int)PanelVisualization_Falloff[rbb], coeff-1, 1);
		Local_DisplayFastRect_Vis(MemoryDisplay, 0x00CC00, (i * coeff) + 2, Coord_DisplayH - (int)PanelVisualization_Peaks[rbb], coeff-1, (int)PanelVisualization_Peaks[rbb]);
	}

	BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);

	return 0;

}else if(PanelVisualization_Mode == 1){

	audio_getfloatbuffer(PanelVisualization_Audio, PanelVisualization_FFTSize, 1);
	fft_float(1024, 0, PanelVisualization_Audio, 0, PanelVisualization_fft_r, PanelVisualization_fft_i);

	for(i=0; i<(unsigned long)Coord_DisplayH; i++)
	{

		/* peak summation */

		{
			int   j;
			int   mx;
			float sumfreq = 0.0f;
			
			mx = (int)(512.0f / ((float)Coord_DisplayH));

			for(j=0; j<=mx; j++)
			{
				rbb = (i * mx) + j;
				if(rbb > 512)rbb = 512;

				tmp = (float)sqrt((PanelVisualization_fft_r[rbb] * PanelVisualization_fft_r[rbb]) + (PanelVisualization_fft_i[rbb] * PanelVisualization_fft_i[rbb]));
				tmp = ((tmp * (float)720.0f) / PanelVisualization_PeaksMax[rbb]);
				
				if(tmp < 0.0f)tmp = -tmp;

				sumfreq += tmp / (float)mx;
			}
			
			tmp = sumfreq;
		}
		{
			float fs, ps, rs, ts;

			ps = tmp;

			if(ps > 512.0f)ps = 512.0f;

			ps = ps;

			ts = (float)((int)ps);
			if(ps - ((float)ts) >= 0.5)ts = (float)((int)ps) + 1;

			rs = (float)(((int)ts) - 255);
			if(rs < 0)rs = 0;
			fs = min(ts, 255);

			SetPixelV(MemoryDisplay, Coord_DisplayW - 1, Coord_DisplayH - 1 - i, RGB(fs, rs, 0));
		
			lps[0] = ps;
			hps[i] = ps;
		}

	
	}

	BitBlt(MemoryDisplay, 0, 0, Coord_DisplayW - 1, Coord_DisplayH, MemoryDisplay, 1, 0, SRCCOPY);
	BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);


}else if(PanelVisualization_Mode == 2){

	if(audio_getfloatbuffer(PanelVisualization_Audio, Coord_DisplayW, 1) == 1)
	{
		int  a;
		HPEN hp, hop;
	
		//Local_DisplayFastRect(MemoryDisplay, 0, 0, 0, Coord_DisplayW, Coord_DisplayH);
		Display_Erase();

		for(i=0; i<(unsigned long)Coord_DisplayW; i++)
		{
			if(i==0)
			{
				a=0;
			}else{
				a=i-1;
			}

			fpt[i].x = i;
			fpt[i].y = (long)(PanelVisualization_Audio[i] / 32768.0f * (float)(Coord_DisplayH / 2)) + (Coord_DisplayH / 2);

			//MoveToEx(MemoryDisplay, a, PanelVisualization_Audio[a] / 32768.0f * (float)(Coord_DisplayH / 2) + (Coord_DisplayH / 2), 0);
			//LineTo(MemoryDisplay, i, (PanelVisualization_Audio[i] / 32768.0f * (float)(Coord_DisplayH / 2)) + (Coord_DisplayH / 2));
		}

		hp  = CreatePen(PS_SOLID, 1, 0x00DD00);
		hop = (HPEN)SelectObject(MemoryDisplay, hp);

		Polyline(MemoryDisplay, fpt, Coord_DisplayW);
	
		SelectObject(MemoryDisplay, hop);
		DeleteObject(hp);

		BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);

	}

}else if(PanelVisualization_Mode == 3){

	if(audio_getfloatbuffer(PanelVisualization_Audio, Coord_DisplayW, 1) == 1)
	{
		int  a;
		HPEN hp, hop;
	
		Display_Erase();
		//Local_DisplayFastRect(MemoryDisplay, 0, 0, 0, Coord_DisplayW, Coord_DisplayH);

		for(i=0; i<(unsigned long)Coord_DisplayW; i++)
		{
			if(i==0)
			{
				a=0;
			}else{
				a=i-1;
			}

			fpt[i].x = i;
			fpt[i].y = (long)(PanelVisualization_Audio[i] / 32768.0f * (float)(Coord_DisplayH / 2)) + (Coord_DisplayH / 2);

			//MoveToEx(MemoryDisplay, a, PanelVisualization_Audio[a] / 32768.0f * (float)(Coord_DisplayH / 2) + (Coord_DisplayH / 2), 0);
			//LineTo(MemoryDisplay, i, (PanelVisualization_Audio[i] / 32768.0f * (float)(Coord_DisplayH / 2)) + (Coord_DisplayH / 2));
		}

		hp  = CreatePen(PS_SOLID, 1, 0x00DD00);
		hop = (HPEN)SelectObject(MemoryDisplay, hp);

		Polyline(MemoryDisplay, fpt, Coord_DisplayW);
	
		SelectObject(MemoryDisplay, hop);
		DeleteObject(hp);

		BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);

	}

	if(audio_getfloatbuffer(PanelVisualization_Audio, Coord_DisplayW, 2) == 1)
	{
		int  a;
		HPEN hp, hop;
	
		for(i=0; i<(unsigned long)Coord_DisplayW; i++)
		{
			if(i==0)
			{
				a=0;
			}else{
				a=i-1;
			}

			fpt[i].x = i;
			fpt[i].y = (long)(PanelVisualization_Audio[i] / 32768.0f * (float)(Coord_DisplayH / 2)) + (Coord_DisplayH / 2);

			//MoveToEx(MemoryDisplay, a, PanelVisualization_Audio[a] / 32768.0f * (float)(Coord_DisplayH / 2) + (Coord_DisplayH / 2), 0);
			//LineTo(MemoryDisplay, i, (PanelVisualization_Audio[i] / 32768.0f * (float)(Coord_DisplayH / 2)) + (Coord_DisplayH / 2));
		}

		hp  = CreatePen(PS_SOLID, 1, 0x00DDDD);
		hop = (HPEN)SelectObject(MemoryDisplay, hp);

		Polyline(MemoryDisplay, fpt, Coord_DisplayW);
	
		SelectObject(MemoryDisplay, hop);
		DeleteObject(hp);

		BitBlt(window_main_dc, Coord_DisplayX, Coord_DisplayY, Coord_DisplayW, Coord_DisplayH, MemoryDisplay, 0, 0, SRCCOPY);

	}
}

	return 1;
}

int Local_PanelVisualization_Close(void)
{

	sys_mem_free(PanelVisualization_AudioBuffer);
	PanelVisualization_AudioBuffer = 0;

	sys_mem_free(PanelVisualization_Audio);
	PanelVisualization_Audio = 0;

	sys_mem_free(PanelVisualization_FFTOut);
	PanelVisualization_FFTOut = 0;

	sys_mem_free(PanelVisualization_Peaks);
	PanelVisualization_Peaks = 0;

	sys_mem_free(PanelVisualization_Falloff);
	PanelVisualization_Falloff = 0;

    Display_Erase();
    return 1;
}

/* callback functions */

void __stdcall TimerProc_Display(HWND hwnd,UINT uMsg,UINT_PTR idEvent,unsigned long dwTime)
{
	if(fennec_power_state & fennec_power_state_hidden)return;

    switch(CurrentPanel)
    {
    case PanelID_Main:
        Local_PanelMain_Timer();
        break;

    case PanelID_Credits:
        Local_PanelCredits_Timer();
        break;

	case PanelID_Visualization:
		Local_PanelVisualization_Timer();
		break;

    default:
        return;
    }

    return;
}

int Local_DrawElectronicDigits_Time(HDC dc,int x,int y,int cmin,int csec)
{
    int sec1,sec2,min1,min2;

    /* +1 space for every digit */

    if(cmin > 99 || csec >= 60)return 0;

    sec1 = csec % 10; sec2 = csec / 10;
    min1 = cmin % 10; min2 = cmin / 10;

    BitBlt(dc,x,y,Coord_ENumberW,Coord_ENumberH,ActionDisplay,Coord_ENumberX + (min2 * Coord_ENumberW),Coord_ENumberY,SRCCOPY);
    BitBlt(dc,x + Coord_ENumberW + 1,y,Coord_ENumberW,Coord_ENumberH,ActionDisplay,Coord_ENumberX + (min1 * Coord_ENumberW),Coord_ENumberY,SRCCOPY);
    BitBlt(dc,x + ((Coord_ENumberW + 1) * 2),y,Coord_ENumberW,Coord_ENumberH,ActionDisplay,Coord_ENumberX + (11 * Coord_ENumberW),Coord_ENumberY,SRCCOPY);
    BitBlt(dc,x + ((Coord_ENumberW + 1) * 3),y,Coord_ENumberW,Coord_ENumberH,ActionDisplay,Coord_ENumberX + (sec2 * Coord_ENumberW),Coord_ENumberY,SRCCOPY);
    BitBlt(dc,x + ((Coord_ENumberW + 1) * 4),y,Coord_ENumberW,Coord_ENumberH,ActionDisplay,Coord_ENumberX + (sec1 * Coord_ENumberW),Coord_ENumberY,SRCCOPY);
    return 1;
}


/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/