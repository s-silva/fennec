#include <windows.h>
#include "resource.h"

#define rt_error       0x1
#define rt_warning     0x2
#define rt_stepping    0x4
#define rt_info        0x8
#define rt_suberror    0x10



int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


HANDLE  rmutex;
void   *rmem;
int     rlength, rcur, rline;
int     rmode = 0xFFFFFF;
int     rfollow = 1, rpause = 0;


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG  msg;


	DialogBox(hInstance, (LPCTSTR)dialog_report, 0, (DLGPROC)DialogProc);


	while(GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);	
		DispatchMessage(&msg);
	}

	return 1;
}


int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case button_cancel:
		case button_ok:
			ReleaseMutex(rmutex);
			EndDialog(hwnd, 0);
			PostQuitMessage(0);
			break;

		case button_clear:
			rcur  = 0;
			rline = 0;
			memset(rmem, 0, min(32, rlength));
			SendDlgItemMessage(hwnd, text, WM_SETTEXT, 0, (LPARAM)"");
			break;

		case button_pause:
			rpause ^= 1;
			break;

		default:
			if(IsDlgButtonChecked(hwnd, check_error) == BST_CHECKED)
				rmode |= rt_error;
			else
				rmode &= ~rt_error;

			if(IsDlgButtonChecked(hwnd, check_suberrors) == BST_CHECKED)
				rmode |= rt_suberror;
			else
				rmode &= ~rt_suberror;

			if(IsDlgButtonChecked(hwnd, check_warnings) == BST_CHECKED)
				rmode |= rt_warning;
			else
				rmode &= ~rt_warning;

			if(IsDlgButtonChecked(hwnd, check_information) == BST_CHECKED)
				rmode |= rt_info;
			else
				rmode &= ~rt_info;

			if(IsDlgButtonChecked(hwnd, check_stepping) == BST_CHECKED)
				rmode |= rt_stepping;
			else
				rmode &= ~rt_stepping;

			if(IsDlgButtonChecked(hwnd, check_follow) == BST_CHECKED)
				rfollow = 1;
			else
				rfollow = 0;

			break;
		}
		break;

	case WM_COPYDATA:
		{
			char             mbs[4096];
			COPYDATASTRUCT  *cd;
			BOOL             tr = 1;
			
			cd = (COPYDATASTRUCT*) lParam;

			if(rpause) break;

			if(!cd->lpData || !cd->cbData) break;

			if(!(rmode & cd->dwData)) break;

			mbs[0] = 0;

			if(wParam > sizeof(char))
				WideCharToMultiByte(CP_ACP, 0, cd->lpData, -1, mbs, sizeof(mbs), "?", &tr);
			else
				strcpy(mbs, cd->lpData);

			if(rcur + (int)strlen(mbs) >= rlength)
			{
				rlength += 0x10000;
				rmem     = realloc(rmem, rlength);
				memset((char*)rmem + rcur, 0, rlength - rcur);
			}

			strcat(rmem, mbs);
			strcat(rmem, "\r\n");
			rline++;
	
			SendDlgItemMessage(hwnd, text, WM_SETTEXT, 0, (LPARAM)rmem);
			rcur += (int)strlen(mbs) + 2;


			if(rfollow)
				SendDlgItemMessage(hwnd, text, EM_LINESCROLL, 0, (LPARAM)rline);
		}
		break;

	case WM_INITDIALOG:
		rmutex = CreateMutex(0, 0, "fennec debug - reporting");

		rmem    = malloc(0x10000);
		rlength = 0x10000;
		rcur    = 0;
		rline   = 0;
		memset(rmem, 0, rlength);

		CheckDlgButton(hwnd, check_error, BST_CHECKED);
		CheckDlgButton(hwnd, check_suberrors, BST_CHECKED);
		CheckDlgButton(hwnd, check_warnings, BST_CHECKED);
		CheckDlgButton(hwnd, check_information, BST_CHECKED);
		CheckDlgButton(hwnd, check_stepping, BST_CHECKED);
		CheckDlgButton(hwnd, check_follow, BST_CHECKED);
		break;
	}


	return 0;
}		


