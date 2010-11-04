/*
* This source code is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*       
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* File Name: testac3dec.c							
*
* Reference:
*
* Author: Li Feng,  fli_linux@yahoo.com.cn                                                 
*
* Description:
*
* 	
* 
* History:
* 02/23/2005  Li Feng    Created
*  
*
*CodeReview Log:
* 
*/
#include <stdio.h>
#include <string.h>
#include "libac3dec.h"

typedef signed char         INT8;
typedef signed short        INT16;
typedef signed int          INT32;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned long        UINT32;

#define PCM_FRAME_SIZE 2048*4

#define WAVE_FORMAT_PCM 1

#define SWAP32(val) (UINT32)((((UINT32)(val)) & 0x000000FF)<<24|	\
	(((UINT32)(val)) & 0x0000FF00)<<8 |	\
	(((UINT32)(val)) & 0x00FF0000)>>8 |	\
	(((UINT32)(val)) & 0xFF000000)>>24)	


#pragma pack(push,1)

typedef struct _riffchunk
{
	UINT32 fcc;
	UINT32  cb;
} RIFFCHUNK, *LPRIFFCHUNK;

typedef struct _rifflist 
{
	UINT32 fcc;
	UINT32  cb;
	UINT32 fccListType;
} RIFFLIST , *LPRIFFLIST;

#define RIFFROUND(cb) ((cb) + ((cb)&1))

#define RIFFNEXT(pChunk) (LPRIFFCHUNK)((LPBYTE)(pChunk) \
	+ sizeof(RIFFCHUNK) \
+ RIFFROUND(((LPRIFFCHUNK)pChunk)->cb))
typedef struct 
{
	UINT32 fcc;
	UINT32 cb;
	UINT16  wFormatTag; 
	UINT16  nChannels; 
	UINT32 nSamplesPerSec; 
	UINT32 nAvgBytesPerSec; 
	UINT16  nBlockAlign; 
	UINT16  wBitsPerSample; 
}WAVEFORM;

#pragma pack(pop)

const UINT32 WAVE_HEAD_LEN=44;
static UINT8 head[44];	

void SetWaveHead(UINT32 dwDataLen, UINT32 nSampleRate, UINT32 nChannel)
{
	RIFFCHUNK *pch;
	RIFFLIST  *priff;
	WAVEFORM *pwave;
	
	memset(&head,0x00,WAVE_HEAD_LEN);
	priff=(RIFFLIST*)head;
	priff->fcc=0x46464952;//SWAP32('RIFF');
	if(dwDataLen)
		priff->cb=dwDataLen+WAVE_HEAD_LEN-sizeof(RIFFCHUNK);
	priff->fccListType=0x45564157;//SWAP32('WAVE');
	pwave=(WAVEFORM*)(priff+1);
	pwave->fcc=0x20746d66;//SWAP32('fmt ');
	pwave->cb=sizeof(WAVEFORM)-sizeof(RIFFCHUNK);
	pwave->wFormatTag=WAVE_FORMAT_PCM;
	pwave->nChannels=nChannel;
	pwave->nSamplesPerSec=nSampleRate;
	pwave->nAvgBytesPerSec=pwave->nSamplesPerSec*nChannel*2;
	pwave->nBlockAlign=nChannel*2;
	pwave->wBitsPerSample=16;
	pch=(RIFFCHUNK*)(pwave+1);
	pch->fcc=0x61746164;//SWAP32('data');
	if(dwDataLen)
	{
		pch->cb = dwDataLen;
	}
}

#define BUF_SIZE 1024

int SearchSyncWord(FILE *fp)
{
	unsigned char syncWord[2];
	int i=0;
	if(fread(syncWord, 1, 2, fp)!=2)
		return 0;
	do 
	{
		if(syncWord[0] == 0x0B && syncWord[1] == 0x77)
		{
			return 1;
		}
		i++;
		syncWord[0] = syncWord[1];
	} 
	while(fread(syncWord+1,1,1, fp));
	return 1;
}

int main(int argc, char **argv)
{
	FILE *fpInput;
	FILE *fpOutput;
	signed short data[PCM_FRAME_SIZE];
	unsigned char mpa_data[1792];
	int nRead = 0;
	UINT32 nDataLen;
	UINT32 nFileLen = 0;
	UINT32 nFrameSize;
	UINT32 nFrameLen;
	UINT32 nSampleRate;
	UINT32 n = 0;
	
	if(argc<3)
		return 0;
	
	fpInput = fopen(argv[1], "rb");
	fpOutput= fopen(argv[2], "wb");
	if(fpInput==NULL ||fpOutput==NULL)
	{
		printf("open file error\n");
		goto err;
	}
	
	fwrite(head, 1, WAVE_HEAD_LEN, fpOutput);
	if(fread(mpa_data, 1, 5, fpInput)!=5)
	{
		goto err;
	}
	if(AC3_GetAudioInfo(&nFrameSize, &nSampleRate, mpa_data)==0)
	{
		printf("AC3_GetAudioInfo Error1\n");
		goto err;
	}
	printf("nFrameSize=%d, nSampleRate = %d \n",nFrameSize,nSampleRate );

	while((fread(mpa_data+5, 1, nFrameSize-5, fpInput))==nFrameSize-5)
	{
		nFrameLen = AC3_SampleConvert(data, &nDataLen, mpa_data, nFrameSize);
		if(nDataLen>0)
		{
			fwrite(data, 1, nDataLen, fpOutput);
			nFileLen += nDataLen;
		}
		if(SearchSyncWord(fpInput)==0)
			break;
		memset(mpa_data, 0, 1792);
		mpa_data[0] = 0x0B;
		mpa_data[1] = 0x77;
		if(fread(mpa_data+2, 1, 3, fpInput)!=3)
		{
			goto err;
		}
		
		if(AC3_GetAudioInfo(&nFrameSize, &nSampleRate, mpa_data)==0)
		{
			printf("AC3_GetAudioInfo Error\n");
			break;
		}
		printf("nFrameSize=%d, nSampleRate = %d \n",nFrameSize,nSampleRate );
		
		n++;
		printf("decode %d frame\n", n);
	}
	printf("nSampleRate = %d\n", nSampleRate);
	SetWaveHead(nFileLen, nSampleRate, 2);
	fseek(fpOutput, 0, SEEK_SET);
	fwrite(head, 1, WAVE_HEAD_LEN, fpOutput);
	
err:
	if(fpInput)
	{
		fclose(fpInput);
	}
	if(fpOutput)
	{
		fclose(fpOutput);
	}
	return 0;
}


