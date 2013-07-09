/*
	REX by Propellerhead, (C) Copyright Propellerhead Software AB.
	REX and Propellerhead are trademarks of Propellerhead Software
	AB. email: rex@propellerheads.se
*/
	/* Is this windows platform? */
#ifndef WINDOWS
	#ifdef _WINDOWS
		#define WINDOWS 1
	#endif /* _WINDOWS */
#endif /* WINDOWS */
#ifndef WINDOWS
	#ifdef WIN32
		#define WINDOWS 1
	#endif /* WIN32 */
#endif /* WINDOWS */
#ifndef WINDOWS
	#ifdef _WINDOWS
		#define WINDOWS 1
	#endif /* _WINDOWS */
#endif /* WINDOWS */
#ifndef WINDOWS
	#ifdef PC
		#define WINDOWS 1
	#endif /* _WINDOWS */
#endif /* WINDOWS */
#ifndef WINDOWS
	#ifdef _WIN32
		#define WINDOWS 1
	#endif /* _WIN32 */
#endif /* WINDOWS */

#ifndef WINDOWS
	/* Is this macintosh platform? */
	#ifndef MAC
		#ifdef _MAC
			#define MAC 1
		#endif /* _MAC */
	#endif /* MAC */
	#ifndef MAC
		#ifdef TARGET_OS_MAC
			#define MAC 1
		#endif /* TARGET_OS_MAC */
	#endif /* MAC */
	#ifndef MAC
		#ifdef OSX
			#define MAC 1
		#endif  /* OSX */
	#endif /* MAC */
#else
	#ifdef MAC
		~"Can not determine platform, please set preprocessor define WINDOWS, MAC or OSX to 1"
	#endif /* MAC */
#endif /* WINDOWS */

#ifndef WINDOWS
	#ifndef MAC
		~"Can not determine platform, please set preprocessor define WINDOWS, MAC or OSX to 1"
	#endif /* MAC */
#endif /* WINDOWS */

#ifndef DEBUG
	#ifdef _DEBUG
		#define DEBUG 1
	#endif /* _DEBUG */
#endif /* DEBUG */
#if DEBUG!=0
	#ifdef NDEBUG
		~"Can not determine debug/release mode, please set preprocessor define DEBUG 1 or *dont* set it at all"
	#endif /* NDEBUG */
#endif /* DEBUG */
#ifndef DEBUG
	#ifndef NDEBUG
		#define NDEBUG
	#endif
#endif


#include "REX.h"
#include <assert.h>

/*
 *
 * Local assertion macro.
 *
 */
#if !__MACH__
#include <assert.h>
#define REX_ASSERT(e) assert(e)
#else
#include <stdio.h>
#include <stdlib.h>
static void RexAssertionFailed__(const char* file, int line)
{
	fprintf(stderr, "Assertion failed at %d, %s\n", line, file);
	abort();
}
#define REX_ASSERT(e) ((e) ? ((void) 0) : RexAssertionFailed__(__FILE__, __LINE__))
#endif

#if WINDOWS
	#define NOGDICAPMASKS
	#define NOVIRTUALKEYCODES
	#define NOWINMESSAGES
	#define NOWINSTYLES
	#define NOSYSMETRICS
	#define NOMENUS
	#define NOICONS
	#define NOKEYSTATES
	#define NOSYSCOMMANDS
	#define NORASTEROPS
	#define NOSHOWWINDOW
	#define OEMRESOURCE
	#define NOATOM
	#define NOCLIPBOARD
	#define NOCOLOR
	#define NOCTLMGR
	#define NODRAWTEXT
	#define NOGDI
	#define NOUSER
	#define NONLS
	#define NOMB
	#define NOMEMMGR
	#define NOMETAFILE
	#define NOMINMAX
	#define NOMSG
	#define NOOPENFILE
	#define NOSCROLL
	#define NOSERVICE
	#define NOSOUND
	#define NOTEXTMETRIC
	#define NOWH
	#define NOWINOFFSETS
	#define NOCOMM
	#define NOKANJI
	#define NOHELP
	#define NOPROFILER
	#define NODEFERWINDOWPOS
	#define NOMCX
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <stdlib.h>
#endif /* WINDOWS */
#if MAC
	#include <Aliases.h>
	#include <CodeFragments.h>
	#include <Folders.h>
	#include <Resources.h>
	#include <Gestalt.h>
	#include <string.h>
	#include <Processes.h>
#if OSX
	#include <CFURL.h>
	#include <CFBundle.h>
	#include <CFNumber.h>
	#include <CFPreferences.h>
	#include <CFString.h>
	#include <stdio.h>
#endif
	#ifndef TRUE
		#define TRUE	1
		#define FALSE	0
	#endif
#endif /* MAC */


#ifdef __cplusplus
namespace REX {
#endif


static const short kREXCompatibleMajor=(short)(REX_API_VERSION/1000000L);


static signed long gREXLoadCount=0;

#if WINDOWS
	static HINSTANCE gREXDLLInstance=0;
	static const char kWinREXDebugDLLPathName[]="REX Shared Library Debug.dll";
	static const char kWinREXBetaDLLPathName[]="REX Shared Library Beta.dll";
	static const char kWinREXReleaseDLLPathName[]="REX Shared Library.dll";
#endif /* WINDOWS */

#if MAC
	static CFragConnectionID gREXDLLInstance=0;
	typedef short TMacVolumeID;
	typedef long TMacDirectoryID;
	typedef unsigned char TMacPascalChar;
	const long kMacPascalStringSize=255;

	static const char kMacREXDebugDLLPathName[]="REX Shared Library Debug";
	static const char kMacREXBetaDLLPathName[]="REX Shared Library Beta";
	static const char kMacREXReleaseDLLPathName[]="REX Shared Library";
#endif /* MAC */




#if WINDOWS
typedef int (_cdecl *REXPROC)();
#endif /* WINDOWS */
#if MAC
typedef int (*REXPROC)();
#endif /* MAC */





/*
	Win32 platform code
*/

#if WINDOWS

static char WinIsThisWindowsVersionSupported()
{
	char result=0;
	OSVERSIONINFO osVersionInfo;
	int bIsWindows98orLater = 0;
	int bIsNT4orLater = 0;

	memset(&osVersionInfo, 0, sizeof(OSVERSIONINFO));
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	result = GetVersionEx(&osVersionInfo);
	if (result==0) {
		return FALSE;
	}
	bIsWindows98orLater = (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
						   ((osVersionInfo.dwMajorVersion > 4) || ((osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion > 0)));
	bIsNT4orLater =	(osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osVersionInfo.dwMajorVersion >= 4);
	if (bIsWindows98orLater || bIsNT4orLater) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static char WinGetFileVersion(const char* pathName,short* outRevision, short* outVersion, short* outBuildHi, short* outBuildLo){
	/*DWORD lpdwHandle=0;
	DWORD lSize;
	UINT lOldErrorMode;
	char result=FALSE;

	lOldErrorMode=SetErrorMode(SEM_NOOPENFILEERRORBOX);

	lSize=GetFileVersionInfoSizeA((char*)pathName,&lpdwHandle);
	if(0!=lSize){
		BOOL resultFlag;
		char* buffer=0;

		buffer=(char*)malloc(lSize);
		if(0!=buffer){
			resultFlag=GetFileVersionInfoA((char*)pathName,0,lSize,(LPVOID)buffer);
			if(0!=resultFlag){
				VS_FIXEDFILEINFO* lFInfo=0;
				UINT lFLength;

				resultFlag=VerQueryValueA((LPVOID)buffer,"\\",(VOID FAR* FAR*)&lFInfo,&lFLength);
				if(0!=resultFlag){
					*outRevision=HIWORD(lFInfo->dwFileVersionMS);
					*outVersion=LOWORD(lFInfo->dwFileVersionMS);
					*outBuildHi=HIWORD(lFInfo->dwFileVersionLS);
					*outBuildLo=LOWORD(lFInfo->dwFileVersionLS);
					result=TRUE;
				}
			}
			free(buffer);
			buffer=0;
		}
	}
	SetErrorMode(lOldErrorMode);
	return result; */
}

static REXError VerifyREXDLLVersion(){
	/*char completeFileName[256+1];
	DWORD result=GetModuleFileNameA(gREXDLLInstance,completeFileName,256);
	if(result==0){
		return kREXError_UnableToLoadDLL;
	}
	{
		short revision;
		short version;
		short buildHi;
		short buildLo;
		WinGetFileVersion(completeFileName,&revision, &version, &buildHi, &buildLo);
		if(revision!=kREXCompatibleMajor){
			if (revision > kREXCompatibleMajor) {
				return kREXError_APITooOld;
			} else {
				return kREXError_DLLTooOld;
			}
		}
		{
			long apiVersion = REX_API_VERSION;
			long gotVersion = REX_BUILD_VERSION(revision, version, buildHi);
			if (apiVersion > gotVersion) {
				return kREXError_DLLTooOld;
			}
		}
	}*/
	return kREXError_NoError;
}

static REXError LoadREXDLLByName(const char dllName[]){
	gREXDLLInstance=LoadLibraryA(dllName);
	if(NULL==gREXDLLInstance){
		DWORD error=GetLastError();
		switch(error){
			case ERROR_NOT_ENOUGH_MEMORY:
			case ERROR_OUTOFMEMORY:
			case ERROR_TOO_MANY_OPEN_FILES:
				return kREXError_NotEnoughMemoryForDLL;
			case ERROR_MOD_NOT_FOUND:
				return kREXError_DLLNotFound;
			default:
				return kREXError_UnableToLoadDLL;
		}
	}else{
		return kREXError_NoError;
	}
}

REXError LoadREXDLL(){
	if (FALSE==WinIsThisWindowsVersionSupported()) {
		return kREXError_OSVersionNotSupported;
	}
	else{
		REXError result=LoadREXDLLByName(kWinREXDebugDLLPathName);
		if(result!=kREXError_NoError){
			result=LoadREXDLLByName(kWinREXBetaDLLPathName);
		}
		if(result!=kREXError_NoError){
			result=LoadREXDLLByName(kWinREXReleaseDLLPathName);
		}
		return result;
	}
}

static REXPROC FindREXDLLFunction(const char functionName[]){
	REXPROC result=(REXPROC)GetProcAddress(gREXDLLInstance,functionName);
	return result;
}

static void UnloadREXDLL(){
	if(0!=gREXDLLInstance){
		FreeLibrary(gREXDLLInstance);
		gREXDLLInstance=0;
	}
}

char IsREXLoadedByThisInstance(){
	if(0==gREXDLLInstance){
		return FALSE;
	}else{
		return TRUE;
	}
}

#endif /* WINDOWS */







/*
	MacOS platform code
*/

#if MAC

static long MacGetSystemVersion(void)
{
	long response = 0x0000;
	OSErr err;

	err = Gestalt(gestaltSystemVersion, &response);
	REX_ASSERT(err == noErr);

	return response;
}



static void MacCToPascal(const char c[], TMacPascalChar p[]){
	long size=(long)strlen(c);

	memmove(p + 1,c,(unsigned long)size);
	p[0]=(unsigned char)size;
}

static char MacResolve(FSSpec* macFSSpec){
	Boolean targetIsFolderFlag;
	Boolean wasAliasedFlag;
	OSErr err;

	err=ResolveAliasFile(macFSSpec,TRUE,&targetIsFolderFlag,&wasAliasedFlag);
	if(err == noErr){
		return TRUE;
	}else{
		return FALSE;
	}
}

static char MacGetDirID(const FSSpec* macFSSpec,TMacDirectoryID* directoryID){
	CInfoPBRec pb;
	OSErr err;

	pb.hFileInfo.ioNamePtr = (TMacPascalChar*)macFSSpec->name;
	pb.hFileInfo.ioVRefNum = macFSSpec->vRefNum;
	pb.hFileInfo.ioDirID = macFSSpec->parID;
	pb.hFileInfo.ioFDirIndex = 0;	/* use ioNamePtr and ioDirID */
	err = PBGetCatInfoSync(&pb);
	if(err == noErr){
		char isDirectoryFlag;
		if(pb.hFileInfo.ioFlAttrib & 0x10){
			isDirectoryFlag=TRUE;
		}
		else{
			isDirectoryFlag=FALSE;
		}
		REX_ASSERT(isDirectoryFlag);
		*directoryID=pb.hFileInfo.ioDirID;
		return TRUE;
	}
	return FALSE;
}

static char MacGetNamedTargetInFolder(FSSpec* macFSSpec, const FSSpec* folderFSSpec, const char name[])
{
	TMacPascalChar 	pascalFileName[kMacPascalStringSize + 1];
	OSErr 			err;
	
	REX_ASSERT(macFSSpec != 0);
	REX_ASSERT(folderFSSpec != 0);
	REX_ASSERT(name != 0);

	if (MacGetDirID(folderFSSpec, &macFSSpec->parID)) {
		macFSSpec->vRefNum = folderFSSpec->vRefNum;
		MacCToPascal(name, pascalFileName);
		err = FSMakeFSSpec(folderFSSpec->vRefNum, macFSSpec->parID, pascalFileName, macFSSpec);
		if (err == noErr) {
			if (MacResolve(macFSSpec)) {
				return TRUE;
			}
		}
	}
		
	return FALSE;
}

static char MacGetApplicationFolder(FSSpec* appFolderFSSpec)
{
	OSErr err;
	ProcessSerialNumber psn;
	ProcessInfoRec 		processInfo;
	
	REX_ASSERT(appFolderFSSpec != 0);
	
	err = GetCurrentProcess(&psn);
	if (err == noErr) {
		memset(&processInfo, 0, sizeof(processInfo));
		processInfo.processInfoLength = sizeof(processInfo);
		processInfo.processAppSpec = appFolderFSSpec;
		err = GetProcessInformation(&psn, &processInfo);
		if (err == noErr) {
			err = FSMakeFSSpec(
							appFolderFSSpec->vRefNum,
							appFolderFSSpec->parID,
							0,
							appFolderFSSpec);
			if (err == noErr) {
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

static char MacGetSpecialFolder(FSSpec* extFolderSpec, short domain, long dirType)
{
	OSErr err;
	short volumeID;
	long dirID;

	REX_ASSERT(extFolderSpec != 0);

	err = FindFolder(
				domain,
				dirType,
				kDontCreateFolder,
				&volumeID,
				&dirID);
	
	if (err == noErr) {
		err = FSMakeFSSpec(volumeID, dirID, 0, extFolderSpec);
		if (err == noErr) {
			return TRUE;
		}
	}
	
	return FALSE;
}

static char MacGetMacFSSpecForREXDLLByName(FSSpec* macFSSpec,const char dllName[])
{
	char 				result = FALSE;
	FSSpec 				folderSpec;
	int					i;
	short				domains[] = {kUserDomain, kLocalDomain, kNetworkDomain, kSystemDomain};
	
	REX_ASSERT(macFSSpec != 0);
	REX_ASSERT(dllName != 0);
	
	/* Next to application on both platforms. */
	if (MacGetApplicationFolder(&folderSpec))
	{
		if (MacGetNamedTargetInFolder(macFSSpec, &folderSpec, dllName))
		{
			return TRUE;
		}
	}
	
	if (MacGetSystemVersion() < 0x1000)
	{
		/* Mac OS 9 search: extensions folder. */
		if (MacGetSpecialFolder(&folderSpec, kOnSystemDisk, kExtensionFolderType))
		{
			if (MacGetNamedTargetInFolder(macFSSpec, &folderSpec, dllName))
			{
				return TRUE;
			}
		}
	}
	else
	{
		/* Mac OS X search: application support folder in all domains */
		for (i = 0; i < sizeof(domains)/sizeof(short); i ++) {
			if (MacGetSpecialFolder(&folderSpec, domains[i], kApplicationSupportFolderType))
			{
				if (MacGetNamedTargetInFolder(macFSSpec, &folderSpec, dllName))
				{
					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}

static char MacGetMacFSSpecForREXDLL(FSSpec* macFSSpec){
	if(MacGetMacFSSpecForREXDLLByName(macFSSpec,kMacREXDebugDLLPathName)){
		return TRUE;
	}
	if(MacGetMacFSSpecForREXDLLByName(macFSSpec,kMacREXBetaDLLPathName)){
		return TRUE;
	}
	return MacGetMacFSSpecForREXDLLByName(macFSSpec,kMacREXReleaseDLLPathName);
}

static char MacFindResourceRevision(FSSpec* macFSSpec,short* outRevision){
	char result=FALSE;
	OSErr err=noErr;
	short resRefNum=-1;
	short prevResRefNum=CurResFile();

	/*	Open resource fork. */
	resRefNum=FSpOpenResFile(macFSSpec,fsRdPerm);
	if(resRefNum!=-1){
		UseResFile(resRefNum);
		/*	Load MacOS "vers" resource with ID 1 from DLL's resource fork into a VersRec structure. */
		{
			Handle versHandle=0;
			versHandle=Get1Resource('vers',1);
			if(versHandle!=0){

				HLock(versHandle);
				{
					VersRec* versionResource=(VersRec*)*versHandle;
					*outRevision=versionResource->numericVersion.majorRev;
				}
				HUnlock(versHandle);
				ReleaseResource(versHandle);
				versHandle=0;
				err=ResError();
				if(err == noErr){
					result=TRUE;
				}
			}
		}
		/*	Close resource fork. */
		CloseResFile(resRefNum);
		err=ResError();
		REX_ASSERT(err==noErr);
	}
	UseResFile(prevResRefNum);
	return result;
}

static REXError VerifyREXDLLVersion(){
	FSSpec macFSSpec;
	REXError result=kREXError_UnableToLoadDLL;

	if(MacGetMacFSSpecForREXDLL(&macFSSpec)){
		short revision;
		if(MacFindResourceRevision(&macFSSpec,&revision)){
			if(revision==kREXCompatibleMajor){
				result=kREXError_NoError;
			}
		}
		// ??DE: More error codes?
	}
	return result;
}

static REXError LoadREXDLL(){
	OSErr err;
	Ptr mainAddr;
	Str255 errName;
	FSSpec macFSSpec;
	REXError result=kREXError_UnableToLoadDLL;

	if(MacGetMacFSSpecForREXDLL(&macFSSpec)){

		err=GetDiskFragment(&macFSSpec,0,kCFragGoesToEOF,"\p",kLoadCFrag,&gREXDLLInstance,&mainAddr,errName);
		switch(err){
			case noErr:
				result=kREXError_NoError;
				break;
			/*	Programming error. */
			case paramErr:
				REX_ASSERT(0);
				result=kREXError_UnableToLoadDLL;
				break;
			/*	Specified fragment not found. */
			case cfragNoLibraryErr:
			/*	Loaded fragment has unacceptable unresolved symbols. */
			case cfragUnresolvedErr:
			/*	Order error during user initialization function. */
			case cfragInitOrderErr:
			/*	Import library is too old / new. */
			case cfragImportTooOldErr:
			case cfragImportTooNewErr:
			/*	Circularity in required initialization order. */
			case cfragInitLoopErr:
			/*	Error connecting to fragment. */
			case cfragLibConnErr:
			/*	Initialization procedure did not return noErr. */
			case cfragInitFunctionErr:
				result=kREXError_UnableToLoadDLL;
				break;
			/*	Not enough memory for internal bookkeeping. */
			case cfragNoPrivateMemErr:
			/*	Not enough memory in userÕs address. */
			case cfragNoClientMemErr:
				result=kREXError_OutOfMemory;
				break;
			default:
				result=kREXError_UnableToLoadDLL;
				break;
		}
	}
	return result;
}

static REXPROC FindREXDLLFunction(const char functionName[]){
	Ptr symbolAddress;
	OSErr err;
	CFragSymbolClass symClass;
	TMacPascalChar pascalString[1 + kMacPascalStringSize];

	MacCToPascal(functionName,pascalString);
	err=FindSymbol(gREXDLLInstance,(StringPtr)pascalString,&symbolAddress,&symClass);
	switch(err){
		case noErr:
			return (REXPROC)symbolAddress;

		/*	Connection ID is not valid. */
		case cfragConnectionIDErr:
			return 0;

		/*	Symbol was not found in connection. */
		case cfragNoSymbolErr:
			return 0;

		default:
			return 0;
	}
}

static void UnloadREXDLL(){
	if(0!=gREXDLLInstance){
		OSErr err=CloseConnection (&gREXDLLInstance);
		gREXDLLInstance=0;
	}
}

static char IsREXLoadedByThisInstance(){
	if(0==gREXDLLInstance){
		return FALSE;
	}else{
		return TRUE;
	}
}

#endif /* MAC */








/* 
	To support a new platform these functions need to be implemented:

		REXError LoadREXDLL();
		char IsREXLoadedByThisInstance();
		REXError VerifyREXDLLVersion();
		REXPROC FindREXDLLFunction(const char functionName[]){
		void UnloadREXDLL();
*/


/*
	Portable code
*/


typedef char (*TDLLOpenProc)();
typedef char (*TDLLCloseProc)();
typedef REXError (*TREXCreateProc)(REXHandle*, const char [], long, REXCreateCallback, void*);
typedef void (*TREXDeleteProc)(REXHandle*);
typedef REXError (*TREXGetInfoProc)(REXHandle, long, REXInfo*);
typedef REXError (*TREXGetInfoFromBufferProc)(long, const char [], long, REXInfo*);
typedef REXError (*TREXGetCreatorInfoProc)(REXHandle, long, REXCreatorInfo*);
typedef REXError (*TREXGetSliceInfoProc)(REXHandle, long, long, REXSliceInfo*);
typedef REXError (*TREXSetOutputSampleRateProc)(REXHandle, long);
typedef REXError (*TREXRenderSliceProc)(REXHandle, long, long, float* [2]);
typedef REXError (*TREXStartPreviewProc)(REXHandle);
typedef REXError (*TREXStopPreviewProc)(REXHandle);
typedef REXError (*TREXRenderPreviewBatchProc)(REXHandle, long, float* [2]);
typedef REXError (*TREXSetPreviewTempoProc)(REXHandle, long);

static const char kDLLOpenProcName[]="Open";
static const char kDLLCloseProcName[]="Close";
static const char kREXCreateProcName[]="REXCreate";
static const char kREXDeleteProcName[]="REXDelete";
static const char kREXGetInfoProcName[]="REXGetInfo";
static const char kREXGetInfoFromBufferProcName[]="REXGetInfoFromBuffer";
static const char kREXGetCreatorInfoProcName[]="REXGetCreatorInfo";
static const char kREXGetSliceInfoProcName[]="REXGetSliceInfo";
static const char kREXSetOutputSampleRateProcName[]="REXSetOutputSampleRate";
static const char kREXRenderSliceProcName[]="REXRenderSlice";
static const char kREXStartPreviewProcName[]="REXStartPreview";
static const char kREXStopPreviewProcName[]="REXStopPreview";
static const char kREXRenderPreviewBatchProcName[]="REXRenderPreviewBatch";
static const char kREXSetPreviewTempoProcName[]="REXSetPreviewTempo";

static TDLLOpenProc gDLLOpenProc=0;
static TDLLCloseProc gDLLCloseProc=0;
static TREXCreateProc gREXCreateProc=0;
static TREXDeleteProc gREXDeleteProc=0;
static TREXGetInfoProc gREXGetInfoProc=0;
static TREXGetInfoFromBufferProc gREXGetInfoFromBufferProc=0;
static TREXGetCreatorInfoProc gREXGetCreatorInfoProc=0;
static TREXGetSliceInfoProc gREXGetSliceInfoProc=0;
static TREXSetOutputSampleRateProc gREXSetOutputSampleRateProc=0;
static TREXRenderSliceProc gREXRenderSliceProc=0;
static TREXStartPreviewProc gREXStartPreviewProc=0;
static TREXStopPreviewProc gREXStopPreviewProc=0;
static TREXRenderPreviewBatchProc gREXRenderPreviewBatchProc=0;
static TREXSetPreviewTempoProc gREXSetPreviewTempoProc=0;


static void ClearProcPointers()
{
	gDLLOpenProc=0;
	gDLLCloseProc=0;
	gREXCreateProc=0;
	gREXDeleteProc=0;
	gREXGetInfoProc=0;
	gREXGetInfoFromBufferProc=0;
	gREXGetCreatorInfoProc=0;
	gREXGetSliceInfoProc=0;
	gREXSetOutputSampleRateProc=0;
	gREXRenderSliceProc=0;
	gREXStartPreviewProc=0;
	gREXStopPreviewProc=0;
	gREXRenderPreviewBatchProc=0;
	gREXSetPreviewTempoProc=0;
}


REXError REXCreate(REXHandle* handle, const char buffer[], long size, REXCreateCallback callbackFunc, void* userData)
{
	if(!IsREXLoadedByThisInstance() || gREXCreateProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXCreateProc(handle, buffer, size, callbackFunc, userData);
	}
}

void REXDelete(REXHandle* handle)
{
	if(!IsREXLoadedByThisInstance() || gREXDeleteProc == 0){
		REX_ASSERT(FALSE);	/* DLL has not been loaded! */
	} else {
		gREXDeleteProc(handle);
	}
}

REXError REXGetInfo(REXHandle handle, long infoSize, REXInfo* info)
{
	if(!IsREXLoadedByThisInstance() || gREXGetInfoProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXGetInfoProc(handle, infoSize, info);
	}
}

REXError REXGetInfoFromBuffer(long bufferSize, const char buffer[], long infoSize, REXInfo* info)
{
	if(!IsREXLoadedByThisInstance() || gREXGetInfoFromBufferProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXGetInfoFromBufferProc(bufferSize, buffer, infoSize, info);
	}
}

REXError REXGetCreatorInfo(REXHandle handle, long creatorInfoSize, REXCreatorInfo* creatorInfo)
{
	if(!IsREXLoadedByThisInstance() || gREXGetCreatorInfoProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXGetCreatorInfoProc(handle, creatorInfoSize, creatorInfo);
	}
}

REXError REXGetSliceInfo(REXHandle handle, long sliceIndex, long sliceInfoSize, REXSliceInfo* sliceInfo)
{
	if(!IsREXLoadedByThisInstance() || gREXGetSliceInfoProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXGetSliceInfoProc(handle, sliceIndex, sliceInfoSize, sliceInfo);
	}
}

REXError REXSetOutputSampleRate(REXHandle handle, long outputSampleRate)
{
	if(!IsREXLoadedByThisInstance() || gREXSetOutputSampleRateProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXSetOutputSampleRateProc(handle, outputSampleRate);
	}
}

REXError REXRenderSlice(REXHandle handle, long sliceIndex, long bufferFrameLength, float* outputBuffers[2])
{
	if(!IsREXLoadedByThisInstance() || gREXRenderSliceProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXRenderSliceProc(handle, sliceIndex, bufferFrameLength, outputBuffers);
	}
}

REXError REXStartPreview(REXHandle handle)
{
	if(!IsREXLoadedByThisInstance() || gREXStartPreviewProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXStartPreviewProc(handle);
	}
}

REXError REXStopPreview(REXHandle handle)
{
	if(!IsREXLoadedByThisInstance() || gREXStopPreviewProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXStopPreviewProc(handle);
	}
}

REXError REXRenderPreviewBatch(REXHandle handle, long framesToRender, float* outputBuffers[2])
{
	if(!IsREXLoadedByThisInstance() || gREXRenderPreviewBatchProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXRenderPreviewBatchProc(handle, framesToRender, outputBuffers);
	}
}

REXError REXSetPreviewTempo(REXHandle handle, long tempo)
{
	if(!IsREXLoadedByThisInstance() || gREXSetPreviewTempoProc == 0){
		return kREXImplError_DLLNotLoaded;
	} else {
		return gREXSetPreviewTempoProc(handle, tempo);
	}
}





REXError REXLoadDLL()
{
	if(IsREXLoadedByThisInstance()){
		//REX_ASSERT(FALSE);		/* REXLoadDLL() should only be called once before it is unloaded by calling REXUnloadDLL()! */
		gREXLoadCount++;
		return kREXImplError_DLLAlreadyLoaded;
	}else{
		REX_ASSERT(gREXLoadCount==0);
		REX_ASSERT(gDLLOpenProc==0);
		REX_ASSERT(gDLLCloseProc==0);
		REX_ASSERT(gREXCreateProc==0);
		REX_ASSERT(gREXDeleteProc==0);
		REX_ASSERT(gREXGetInfoProc==0);
		REX_ASSERT(gREXGetInfoFromBufferProc==0);
		REX_ASSERT(gREXGetCreatorInfoProc==0);
		REX_ASSERT(gREXGetSliceInfoProc==0);
		REX_ASSERT(gREXSetOutputSampleRateProc==0);
		REX_ASSERT(gREXRenderSliceProc==0);
		REX_ASSERT(gREXStartPreviewProc==0);
		REX_ASSERT(gREXStopPreviewProc==0);
		REX_ASSERT(gREXRenderPreviewBatchProc==0);
		REX_ASSERT(gREXSetPreviewTempoProc==0);
		{
			REXError result=LoadREXDLL();

			if(result==kREXError_NoError){
				result=VerifyREXDLLVersion();
				if(result!=kREXError_NoError){
					UnloadREXDLL();
					return result;
				}else{
					gREXLoadCount=1;
					
					gDLLOpenProc=(TDLLOpenProc)FindREXDLLFunction(kDLLOpenProcName);
					gDLLCloseProc=(TDLLCloseProc)FindREXDLLFunction(kDLLCloseProcName);
					gREXCreateProc=(TREXCreateProc)FindREXDLLFunction(kREXCreateProcName);
					gREXDeleteProc=(TREXDeleteProc)FindREXDLLFunction(kREXDeleteProcName);
					gREXGetInfoProc=(TREXGetInfoProc)FindREXDLLFunction(kREXGetInfoProcName);
					gREXGetInfoFromBufferProc=(TREXGetInfoFromBufferProc)FindREXDLLFunction(kREXGetInfoFromBufferProcName);
					gREXGetCreatorInfoProc=(TREXGetCreatorInfoProc)FindREXDLLFunction(kREXGetCreatorInfoProcName);
					gREXGetSliceInfoProc=(TREXGetSliceInfoProc)FindREXDLLFunction(kREXGetSliceInfoProcName);
					gREXSetOutputSampleRateProc=(TREXSetOutputSampleRateProc)FindREXDLLFunction(kREXSetOutputSampleRateProcName);
					gREXRenderSliceProc=(TREXRenderSliceProc)FindREXDLLFunction(kREXRenderSliceProcName);
					gREXStartPreviewProc=(TREXStartPreviewProc)FindREXDLLFunction(kREXStartPreviewProcName);
					gREXStopPreviewProc=(TREXStopPreviewProc)FindREXDLLFunction(kREXStopPreviewProcName);
					gREXRenderPreviewBatchProc=(TREXRenderPreviewBatchProc)FindREXDLLFunction(kREXRenderPreviewBatchProcName);
					gREXSetPreviewTempoProc=(TREXSetPreviewTempoProc)FindREXDLLFunction(kREXSetPreviewTempoProcName);

					if ((gDLLOpenProc == 0) ||
						(gDLLCloseProc == 0) ||
						(gREXCreateProc == 0) ||
						(gREXDeleteProc == 0) ||
						(gREXGetInfoProc == 0) ||
						(gREXGetInfoFromBufferProc == 0) ||
						(gREXGetCreatorInfoProc == 0) ||
						(gREXGetSliceInfoProc == 0) ||
						(gREXSetOutputSampleRateProc == 0) ||
						(gREXRenderSliceProc == 0) ||
						(gREXStartPreviewProc == 0) ||
						(gREXStopPreviewProc == 0) ||
						(gREXRenderPreviewBatchProc == 0) ||
						(gREXSetPreviewTempoProc == 0)) 
					{
						ClearProcPointers();
						UnloadREXDLL();
						return kREXError_UnableToLoadDLL;
					} else {
						if (gDLLOpenProc() != 0) {
							return kREXError_NoError;
						} else {
							ClearProcPointers();
							UnloadREXDLL();
							return kREXError_UnableToLoadDLL;
						}
					}
				}
			}else{
				return result;
			}
		}
	}
}



void REXUnloadDLL(){
	/* REXLoadDLL() should only be called once, before REXUnloadDLL() is called! */
	/* REXUnloadDLL() should not be called unless a call to REXLoadDLL() has succeeded! */
	REX_ASSERT(gREXLoadCount==1);
	if (gREXLoadCount > 0) {
		gREXLoadCount--;
	}
	if (gREXLoadCount==0) {
		gDLLCloseProc();
		ClearProcPointers();
		UnloadREXDLL();
	}
}










#ifdef __cplusplus
}		/* End of namespace REX */
#endif



