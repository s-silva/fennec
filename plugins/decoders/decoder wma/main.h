/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (WMA).
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

#include "../../../include/system.h"
#include "../../../include/fennec.h"
#include <wmsdk.h>

struct plugin_stream
{
	unsigned char     initialized;

	unsigned long     frequency;
	unsigned long     channels;
	unsigned long     bitspersample;
	unsigned long     bitrate;
	unsigned long     duration;
	
	unsigned long     wmaudioout;
	unsigned long     bufferpt;
	unsigned long     buffersize;
	unsigned long     bufferallocsize;
	
	char*             buffer;

	IWMSyncReader*    wmreader; 
};

extern struct plugin_stream* pstreams;

int decoder_load(unsigned long id, const string sname);
int decoder_close(unsigned long id);
int decoder_seek(unsigned long id, double pos);
unsigned long decoder_read(unsigned long id, void* adata, unsigned long dsize);

int tagwrite(const string fname,  struct fennec_audiotag *wtag);
int tagread(const string fname, struct fennec_audiotag *rtag);

unsigned long callc fennec_initialize_input(struct general_input_data* gin);

int callc fennec_plugin_initialize(void);
int callc fennec_plugin_getformat(unsigned long id, unsigned long* ftype, void* fdata);
int callc fennec_plugin_getposition(unsigned long id, unsigned long* ptype, void* pos);
int callc fennec_plugin_setposition(unsigned long id, unsigned long ptype, void* pos);
int callc fennec_plugin_getduration(unsigned long id, unsigned long* ptype, void* pos);
int callc fennec_plugin_getstatus(unsigned long id, unsigned long* stype, void* stt);
int callc fennec_plugin_read(unsigned long id, unsigned long dsize, unsigned long* dread, void* rdata);
int callc fennec_plugin_about(void* pdata);
int callc fennec_plugin_settings(void* pdata);
int callc fennec_plugin_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize);
int callc fennec_plugin_get_extension(int id, string out_ext, string out_desc, int* newdsc);
int callc fennec_plugin_tagread (const string fname, struct fennec_audiotag* rtag);
int callc fennec_plugin_close(unsigned long id);
int callc fennec_plugin_uninitialize(void);
int callc fennec_plugin_tagwrite (const string fname, struct fennec_audiotag* rtag);
unsigned long callc fennec_plugin_open(unsigned long otype, unsigned long osize, const string odata);