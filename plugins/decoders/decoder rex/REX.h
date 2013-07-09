#ifndef REX_H_
#define REX_H_

/*
	REX by Propellerhead, (C) Copyright Propellerhead Software AB.
	REX and Propellerhead are trademarks of Propellerhead Software
	AB. email: rex@propellerheads.se
*/

#if defined(_MSC_VER) && !defined(__MWERKS__) 
	#pragma pack(push,8)
#elif (OSX && (__MWERKS__ >= 0x2405)) || (!OSX)
	#pragma options align = native
#else
	#error "REX not compiling with non-supported compiler"
#endif

#ifdef __cplusplus
namespace REX {
	extern "C" {
#endif


#define kREXPPQ 15360
#define kREXStringSize 255


typedef enum {
	/* Not really errors */
	kREXError_NoError					= 1,
	kREXError_OperationAbortedByUser	= 2,
	kREXError_NoCreatorInfoAvailable	= 3,

	/* Run-time errors */
	kREXError_NotEnoughMemoryForDLL		= 100,
	kREXError_UnableToLoadDLL			= 101,
	kREXError_DLLTooOld					= 102,
	kREXError_DLLNotFound				= 103,
	kREXError_APITooOld					= 104,
	kREXError_OutOfMemory				= 105,
	kREXError_FileCorrupt				= 106,
	kREXError_REX2FileTooNew			= 107,
	kREXError_FileHasZeroLoopLength		= 108,
	kREXError_OSVersionNotSupported		= 109,

	/* Implementation errors */
	kREXImplError_DLLNotLoaded			= 200,
	kREXImplError_DLLAlreadyLoaded		= 201,
	kREXImplError_InvalidHandle			= 202,
	kREXImplError_InvalidSize			= 203,
	kREXImplError_InvalidArgument		= 204,
	kREXImplError_InvalidSlice			= 205,
	kREXImplError_InvalidSampleRate		= 206,
	kREXImplError_BufferTooSmall		= 207,
	kREXImplError_IsBeingPreviewed		= 208,
	kREXImplError_NotBeingPreviewed		= 209,
	kREXImplError_InvalidTempo			= 210,

	/* DLL error - call the cops! */
	kREXError_Undefined					= 666
} REXError;

typedef unsigned long REXHandle;

typedef struct {
	long fChannels;
	long fSampleRate;
	long fSliceCount;			/* Number of slices */
	long fTempo;				/* Tempo set when exported from ReCycle, 123.456 BPM stored as 123456L etc. */
	long fOriginalTempo;		/* Original tempo of loop, as calculated by ReCycle from the locator positions and bars/beats/sign settings. */
	long fPPQLength;			/* Length of loop */
	long fTimeSignNom;
	long fTimeSignDenom;
	long fBitDepth;				/* Number of bits per sample in original data */
} REXInfo;


typedef struct {
	long fPPQPos;				/* Position of slice in loop */
	long fSampleLength;			/* Length of rendered slice, at its original sample rate. */
} REXSliceInfo;


typedef struct {
	char fName[kREXStringSize + 1];
	char fCopyright[kREXStringSize + 1];
	char fURL[kREXStringSize + 1];
	char fEmail[kREXStringSize + 1];
	char fFreeText[kREXStringSize + 1];
} REXCreatorInfo;


typedef enum {
	kREXCallback_Abort = 1,
	kREXCallback_Continue = 2
} REXCallbackResult;


typedef REXCallbackResult (*REXCreateCallback)(long percentFinished, void* userData);



extern REXError REXLoadDLL();

extern void		REXUnloadDLL();

extern REXError	REXCreate(				REXHandle* handle, 
										const char buffer[], 
										long size,
										REXCreateCallback callbackFunc,
										void* userData);

extern void		REXDelete(				REXHandle* handle);

extern REXError REXGetInfo(				REXHandle handle,
										long infoSize,
										REXInfo* info);

extern REXError REXGetInfoFromBuffer(	long bufferSize,
										const char buffer[],
										long infoSize,
										REXInfo* info);

extern REXError REXGetCreatorInfo(		REXHandle handle,
										long creatorInfoSize,
										REXCreatorInfo* creatorInfo);

extern REXError	REXGetSliceInfo(		REXHandle handle, 
										long sliceIndex, 
										long sliceInfoSize,
										REXSliceInfo* sliceInfo);

extern REXError REXSetOutputSampleRate(	REXHandle handle,
										long outputSampleRate);

extern REXError	REXRenderSlice(			REXHandle handle, 
										long sliceIndex, 
										long bufferFrameLength,
										float* outputBuffers[2]);

extern REXError	REXStartPreview(		REXHandle handle);

extern REXError	REXStopPreview(			REXHandle handle);

extern REXError	REXRenderPreviewBatch(	REXHandle handle, 
										long framesToRender, 
										float* outputBuffers[2]);

extern REXError	REXSetPreviewTempo(		REXHandle handle, 
										long tempo);




/* Internal stuff */

/*	Format is aaabbbccc in decimal. aaa=major,bbb=minor,ccc=revision. That
	is: versionLong=major * 1000000L + minor * 1000 + revision.
*/

#define REX_BUILD_VERSION(major,minor,revision)	((major) * 1000000L + (minor) * 1000 + (revision))
#define REX_API_VERSION		REX_BUILD_VERSION(1,1,1)

#ifdef __cplusplus
	}
}
#endif

#if defined(_MSC_VER) && !defined(__MWERKS__) 
	#pragma pack(pop)
#elif (OSX && (__MWERKS__ >= 0x2405)) || (!OSX)
	#pragma options align = reset
#else
	#error "REX not compiling with non-supported compiler"
#endif

#endif /* REX_H_ */
