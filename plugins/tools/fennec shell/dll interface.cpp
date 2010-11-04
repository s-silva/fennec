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

long       refcount = 0;
HINSTANCE  dllinstance;

/* {2625DEE6-B52B-4b51-AC1B-3DCA7A50B080} */
const GUID CLSID_FennecPlayer = {0x2625DEE6, 0xB52B, 0x4b51, {0xAC, 0x1B, 0x3D, 0xCA, 0x7A, 0x50, 0xB0, 0x80} };



/* shellext -----------------------------------------------------------------*/

class class_shellext: 
	public IClassFactory, 
	public class_unknown
{
public:

	 class_shellext() { InterlockedIncrement(&refcount); }
	~class_shellext() { InterlockedDecrement(&refcount); }

	/* <unknown interface> */

	STDMETHOD(QueryInterface) (REFGUID iid, void **outObject)
	{
		if(iid == IID_IUnknown)
		{
			*outObject = (void *)(IUnknown *)(IClassFactory *)this;
			AddRef();
			return S_OK;
		}

		if(iid == IID_IClassFactory)
		{
			*outObject = (void *)(IClassFactory *)this;
			AddRef();
			return S_OK;
		}


		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)()
	{
		return ++__m_RefCount;
	}

	STDMETHOD_(ULONG, Release)()
	{
		if (--__m_RefCount != 0)
			return __m_RefCount;
		
		delete this;
		return 0;
	}

	/* </unknown interface> */
  
	STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, void**);
	STDMETHODIMP LockServer(BOOL);
};




STDMETHODIMP class_shellext::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **ppvObj)
{
	*ppvObj = NULL;
	if(pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	
	fennec_context *shellExt;
	try
	{
		shellExt = new fennec_context();
	}
	catch(...) { return E_OUTOFMEMORY; }

	if (shellExt == NULL)
		return E_OUTOFMEMORY;
	  
	HRESULT res = shellExt->QueryInterface(riid, ppvObj);
	if (res != S_OK)
		delete shellExt;

	return res;

	return S_OK;
}


STDMETHODIMP class_shellext::LockServer(BOOL /* fLock */)
{
  return S_OK;
}


/*---------------------------------------------------------------------------*/


extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
  if(dwReason == DLL_PROCESS_ATTACH)
  {
	  dllinstance = hInstance;

  }else if (dwReason == DLL_PROCESS_DETACH){

  }
  return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
  return (refcount == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
  *ppv = NULL;

  if(IsEqualIID(rclsid, CLSID_FennecPlayer))
  {
    class_shellext  *cf;
    try
    {
      cf = new class_shellext;
    }
    catch(...) { return E_OUTOFMEMORY; }

    if (cf == 0)
      return E_OUTOFMEMORY;

    HRESULT res = cf->QueryInterface(riid, ppv);

    if (res != S_OK)
      delete cf;

    return res;
  }

  return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer(void)
{
  return S_OK;
}

STDAPI DllUnregisterServer(void)
{
  return S_OK;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

