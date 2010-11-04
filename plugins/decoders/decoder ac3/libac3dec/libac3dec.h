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
* File Name: libac3dec.h							
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
#ifndef __LIB_AC3_DEC_H__
#define __LIB_AC3_DEC_H__

#ifdef _WIN32
#ifdef LIBAC3DEC_EXPORTS
#define AC3DEC __declspec(dllexport)
#else
#define AC3DEC __declspec(dllimport)
#endif
#else
#define AC3DEC	
#endif

int AC3_GetAudioInfo(unsigned long* pnFrameSize, unsigned long *pnSampleRate,unsigned char *pAc3Header);

int AC3_SampleConvert(signed short *pPcmData, unsigned long *pnPcmDataLen, unsigned char* pAc3Buf, unsigned long nAc3DataLen);

#endif 

