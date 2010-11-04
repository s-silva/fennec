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

#include <windows.h>
#include <shlobj.h>




/* data ---------------------------------------------------------------------*/

extern long       refcount;
extern HINSTANCE  dllinstance;





/* header -------------------------------------------------------------------*/

class class_unknown
{
public:
	ULONG __m_RefCount;

	class_unknown(): __m_RefCount(0) {}
};





class fennec_context: 
	public IContextMenu,
	public IShellExtInit,
	public class_unknown
{

public:


	/* <unknown interface> */

	STDMETHOD(QueryInterface) (REFGUID iid, void **outObject)
	{
		if(iid == IID_IUnknown)
		{
			*outObject = (void *)(IUnknown *)(IContextMenu *)this;
			AddRef();
			return S_OK;
		}
		
		if (iid == IID_IContextMenu)
		{
			*outObject = (void *)(IContextMenu *)this;
			AddRef();
			return S_OK;
		}

		if (iid == IID_IShellExtInit)
		{
			*outObject = (void *)(IShellExtInit *)this;
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)()
	{
		InterlockedIncrement((LONG *)&__m_RefCount);
		return __m_RefCount;
	}

	STDMETHOD_(ULONG, Release)()
	{
		InterlockedDecrement((LONG *)&__m_RefCount);
		if(__m_RefCount != 0)
			return __m_RefCount;
		
		delete this;
		return 0;
	}

	/* </unknown interface> */


	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, LPDATAOBJECT dataObject, HKEY hkeyProgID);

	STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax);

private:
	LPTSTR  wfiles;
	UINT    wfilescount;
	HMENU   menupopup;

	HRESULT ValidateCommand(UINT_PTR idCmd, BOOL fUnicode,UINT *puOffset);

public:
	fennec_context();
	~fennec_context();
};

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

