/**----------------------------------------------------------------------------

 Fennec 7.1.1 Player 1.1
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

#include "main.h"

#define TOPCMD_TOP      0
#define TOPCMD_NEXT     1
#define TOPCMD_MAX      2
#define Fennex_Commands_MAX 2

const struct COMMANDINFO {
  LPCSTR  pszNameA;
  LPCWSTR pszNameW;
  LPCSTR  pszHelpA;
  LPCWSTR pszHelpW;
} c_rgciTop[] = {
  { "Open files",  L"Open files", "Open and play file(s) with Fennec Player.",  L"Open and play file(s) with Fennec Player.", }, // TOPCMD_TOP
  { "Add to playlist", L"Add to playlist", "Add file(s) to the playlist.", L"Add file(s) to the playlist.", },// TOPCMD_NEXT
};



fennec_context::fennec_context()
{
	wfilescount = 0;
	menupopup   = 0;
	InterlockedIncrement(&refcount);
}

fennec_context::~fennec_context()
{
	if(wfilescount)free(wfiles);
	if(menupopup)DestroyMenu(menupopup);

	InterlockedDecrement(&refcount);
}

STDMETHODIMP fennec_context::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT dataObject, HKEY /* hkeyProgID */)
{
	UINT i;


	if (dataObject == NULL)
		return E_INVALIDARG;

	STGMEDIUM  medium;
    FORMATETC  fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	HRESULT hr = dataObject->GetData(&fe, &medium);

	if (FAILED(hr))
		return E_INVALIDARG;

	if(wfilescount)free(wfiles);

	wfilescount = DragQueryFile(reinterpret_cast<HDROP>(medium.hGlobal), (UINT)-1, 0, 0);
	
	if(wfilescount)
	{
		wfiles = (LPTSTR) malloc(wfilescount * (sizeof(wfiles[0]) * MAX_PATH));

		for(i=0; i<wfilescount; i++)
		{
			DragQueryFile(reinterpret_cast<HDROP>(medium.hGlobal), i, wfiles + (i * MAX_PATH), MAX_PATH);
		}
	}

	ReleaseStgMedium(&medium);

	return S_OK;
}


STDMETHODIMP fennec_context::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT commandIDFirst, UINT commandIDLast, UINT flags)
{
	UINT cid = commandIDFirst; 
	
	if ((flags & 0x000F) != CMF_NORMAL && (flags & CMF_VERBSONLY) == 0 && (flags & CMF_EXPLORE) == 0) 
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, cid); 


	if (!(CMF_DEFAULTONLY & flags))
	{
		menupopup = CreatePopupMenu();

		InsertMenu(menupopup, indexMenu++, MF_STRING | MF_BYPOSITION, cid++, L"Open files");
		InsertMenu(menupopup, indexMenu++, MF_STRING | MF_BYPOSITION, cid++, L"Add to playlist");
		
		InsertMenu(hmenu, indexMenu++, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT_PTR)menupopup, L"Fennec Player");
		cid++;

		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, cid - commandIDFirst);
	}
	
	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, USHORT(0));
}

int strcmpiA_invariant(LPCSTR psz1, LPCSTR psz2)
{
	return CompareStringA(LOCALE_INVARIANT, NORM_IGNORECASE, psz1, -1, psz2, -1) - CSTR_EQUAL;
}

int strcmpiW_invariant(LPCWSTR psz1, LPCWSTR psz2)
{
	return CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, psz1, -1, psz2, -1) - CSTR_EQUAL;
}

HRESULT fennec_context::ValidateCommand(UINT_PTR idCmd, BOOL fUnicode, UINT *puOffset)
{
	UINT i = (UINT)idCmd;
	if(!IS_INTRESOURCE(idCmd))
	{
		if(fUnicode)
		{
			for(i = 0; i < TOPCMD_MAX; i++)
			{
				if(strcmpiW_invariant((LPCWSTR)idCmd, c_rgciTop[i].pszNameW) == 0)
					break;
			}
		}else{
			for(i = 0; i < TOPCMD_MAX; i++)
			{
				if(strcmpiA_invariant((LPCSTR)idCmd, c_rgciTop[i].pszNameA) == 0)
					break;
			}
		}
	}

	if(i < 2)
	{
		*puOffset = (UINT)i;
		return S_OK;
	}

	return E_INVALIDARG;
}


STDMETHODIMP fennec_context::InvokeCommand(LPCMINVOKECOMMANDINFO commandInfo)
{
	UINT     i, j;
	TCHAR    modfennec[MAX_PATH];
	HWND     owindow;

	CMINVOKECOMMANDINFOEX* lpicix = (CMINVOKECOMMANDINFOEX*)commandInfo;

	BOOL fUnicode = commandInfo->cbSize >= sizeof(CMINVOKECOMMANDINFOEX) && (commandInfo->fMask & CMIC_MASK_UNICODE);
	UINT idCmd;
	HRESULT hr = ValidateCommand(fUnicode ? (UINT_PTR)lpicix->lpVerbW : (UINT_PTR)commandInfo->lpVerb, fUnicode, &idCmd);
	
	
	if(SUCCEEDED(hr))
	{
		if(idCmd == 0 || idCmd == 1)
		{

			GetModuleFileName(dllinstance, modfennec, MAX_PATH);
			j = (UINT)wcslen(modfennec);

			for(;;)
			{
				if(!j || modfennec[j] == '/' || modfennec[j] == '\\')
				{
					modfennec[j] = L'\0';
					break;
				}
				j--;
			}

			wcscat(modfennec, L"\\fennec player.exe");

			

			{
				owindow = FindWindow(L"fennec 7.1 player 1.10000 main", 0);
				int  k = 0;

				if(!owindow)
					ShellExecute(0, 0, modfennec, 0, 0, SW_SHOWNORMAL);


				while(!owindow)
				{
					
					owindow = FindWindow(L"fennec 7.1 player 1.10000 main", 0);
					
					if(!owindow)
					{
						Sleep(10);
						k++;

						if(k > 400)return S_OK;
					}
				}

			}
			
			{
				COPYDATASTRUCT   cd;

				if(owindow && commandInfo->lpVerb == 0) /* open */ 
				{
					cd.cbData = 0;
					cd.lpData = 0;

					SendMessage(owindow, WM_COPYDATA, 3 /* clear list */, (LPARAM)&cd);
				}
			}

			for(i=0; i<wfilescount; i++)
			{
				COPYDATASTRUCT   cd;
				LPTSTR           cfp = wfiles + (i * MAX_PATH);
				HANDLE           fh;
				WIN32_FIND_DATA  fd;

				if(!owindow)return S_OK;

				cd.cbData = sizeof(TCHAR) * MAX_PATH;
				cd.lpData = cfp;

				fh = FindFirstFile(cfp, &fd);

				if(fh == INVALID_HANDLE_VALUE)continue;

				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY || fd.dwFileAttributes & FILE_ATTRIBUTE_DEVICE)
				{
					SendMessage(owindow, WM_COPYDATA, 6 /* add directory */, (LPARAM)&cd);
				}else{
					SendMessage(owindow, WM_COPYDATA, 2 /* add files */, (LPARAM)&cd);
				}

				FindClose(fh);
			}

			{
				COPYDATASTRUCT   cd;

				if(owindow && commandInfo->lpVerb == 0) /* open */ 
				{
					cd.dwData = 0;
					cd.cbData = 0;
					cd.lpData = 0;

					SendMessage(owindow, WM_COPYDATA, 4 /* switch into */, (LPARAM)&cd);
				}
			}
			
		}else{
			hr = E_INVALIDARG;
		}
	}

	return hr;
}


STDMETHODIMP fennec_context::GetCommandString(UINT_PTR commandOffset, UINT uType, UINT * pwReserved, LPSTR pszName, UINT cchMax)
{
	UINT id;
	HRESULT hr = ValidateCommand(commandOffset, uType & GCS_UNICODE, &id);

	if(FAILED(hr))
	{
		if (uType == GCS_VALIDATEA || uType == GCS_VALIDATEW)
		{
			hr = S_FALSE;
		}
		return hr;
	}

	switch (uType)
	{
	case GCS_VERBA:
		lstrcpynA(pszName, c_rgciTop[id].pszNameA, cchMax);
		return S_OK;

	case GCS_VERBW:
		lstrcpynW((LPWSTR)pszName, c_rgciTop[id].pszNameW, cchMax);
		return S_OK;

	case GCS_HELPTEXTA:
		lstrcpynA(pszName, c_rgciTop[id].pszHelpA, cchMax);
		return S_OK;

	case GCS_HELPTEXTW:
		lstrcpynW((LPWSTR)pszName, c_rgciTop[id].pszHelpW, cchMax);
		return S_OK;

	case GCS_VALIDATEA:
		case GCS_VALIDATEW:
		return S_OK;
	}

	return E_NOTIMPL;

	/*
	if(uType & GCS_HELPTEXT)
	{
		switch(commandOffset)
		{
		case 0:
			if (uType & GCS_VERBA)
				lstrcpynA(pszName,  "Open and play file(s) with Fennec Player.", cchMax);
			else
				lstrcpynW((LPWSTR)pszName, L"Open and play file(s) with Fennec Player.", cchMax);
			break;


		case 1:
			if (uType & GCS_VERBA)
				lstrcpynA(pszName, "Add file(s) to the playlist.", cchMax);
			else
				lstrcpynW((LPWSTR)pszName, L"Add file(s) to the playlist.", cchMax);
			break;


		default:
			break;

		}
	}
	return S_OK; */
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

