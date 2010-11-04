#include <windows.h>

#define Function_SetVolumePercent 0x0001
#define Function_Mute             0x0002
#define Function_OpenFileDialog   0x0003
#define Function_Play             0x0004
#define Function_Pause            0x0005
#define Function_Stop             0x0006
#define Function_Previous         0x0007
#define Function_Next             0x0008
#define Function_Rewind           0x0009
#define Function_Forward          0x000a
#define Function_SelectThemePanel 0x000b
#define Function_MainPanel        0x000c
#define Function_EqualizerPanel   0x000d
#define Function_ShowCredits      0x000e
#define Function_RefreshDisplay   0x000f
#define Function_SelectPlayer     0x0010
#define Function_NewPlayerLoadDialog 0x0011
#define FGLOBAL_PLAY 0
#define FGLOBAL_PAUSE 1
#define FGLOBAL_STOP 2
#define FGLOBAL_NEXT 3
#define FGLOBAL_PREVIOUS 4
#define FGLOBAL_FORWARD 5
#define FGLOBAL_REWIND 6
#define FGLOBAL_OPEN 7
#define FGLOBAL_EJECT 8
#define FGLOBAL_SELECT 9
#define FGLOBAL_EXIT 10
#define FGLOBAL_POWEROFF 11
#define FGLOBAL_POWEROFF_WITHSAVE 12
#define FGLOBAL_POWEROFF_WITHOUTSAVE 13
#define FGLOBAL_POWEROFF_SLEEP 14
#define FGLOBAL_MINIMIZE 15
#define FGLOBAL_REFRESH 16
#define FGLOBAL_SHOW_DJ 17
#define FGLOBAL_SHOW_PIANO 18
#define FGLOBAL_SHOW_PLAYLIST 19
#define FGLOBAL_SHOW_MIXLIST 20
#define FGLOBAL_SHOW_VIDEO 21
#define FGLOBAL_SHOW_AMPLIFIER 22
#define FGLOBAL_SHOW_RECORDER 23
#define FGLOBAL_SHOW_SETTINGS 24
#define FGLOBAL_SHOW_ABOUT 25
#define FGLOBAL_RESTORE 26
#define FGLOBAL_HIDEWINDOW 27
#define FGLOBAL_SPLASHSCREEN 28
#define FGLOBAL_OPEN_NEW 29
#define FGLOBAL_SELECT_DUEL 30
#define FGLOBAL_SELECT_LIST 31


unsigned long Fennec_GlobalFunction(unsigned long fnc);
