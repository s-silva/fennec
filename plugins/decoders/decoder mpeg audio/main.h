/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (MPEG).
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

#include "id3 editor.h"

#include "libmad/mad.h"

/* MAD stuff ----------------------------------------------------------------*/

#define SAMPLE_DEPTH	16
#define scale(x, y)	    dither((x), (y))
#define XING_MAGIC	    (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')

struct xing {
    long flags;
    unsigned long frames;
    unsigned long bytes;
    unsigned char toc[100];
    long scale;
};

enum {
    XING_FRAMES = 0x00000001L,
    XING_BYTES  = 0x00000002L,
    XING_TOC    = 0x00000004L,
    XING_SCALE  = 0x00000008L
};

struct dither {
    mad_fixed_t error[3];
    mad_fixed_t random;
};

int                    parse_xing(struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen);
int                    scan_header(unsigned long id, struct mad_header *header, struct xing *xing);
__inline signed int    dither(mad_fixed_t sample, struct dither *dither);
__inline unsigned long prng(unsigned long state);
void                   pack_pcm(unsigned char **pcm, unsigned int nsamples, mad_fixed_t const *ch1, mad_fixed_t const *ch2);
void                   finish(unsigned long id);

/*---------------------------------------------------------------------------*/


struct decoder_data
{
	unsigned int   lengthsec;
    unsigned int   bitrate;
    unsigned int   frequency;
    unsigned int   channels;
    unsigned int   bits;
    unsigned int   buflen;
    unsigned int   samplecount;
    unsigned long  rate;
    unsigned long  frames;
	unsigned long  size;
    unsigned char* buffer;
	struct         xing xing;
    struct         mad_stream stream;
    struct         mad_frame frame;
    struct         mad_synth synth;
	mad_timer_t    timer;
    mad_timer_t    length;
};

struct plugin_stream
{
	unsigned char       initialized;
	t_sys_file_handle   hstream;
	struct decoder_data decdata;
	unsigned char       sourcefile;
};

extern struct plugin_stream *pstreams;
extern unsigned long         pstreams_count;

void          decoder_initialize(unsigned long id);
void          decoder_uninitialize(unsigned long id);
int           decoder_openfile(unsigned long id, const string fname);
int           decoder_readdata(unsigned long id, void *block, unsigned long *size);
void          decoder_seek(unsigned long id, double fpos);
unsigned long decoder_getduration_ms(unsigned long id);
int           decoder_score(const string fname, int foptions);


int           tagread(const string fname, struct fennec_audiotag* rtag);
int           tagwrite(const string fname,  struct fennec_audiotag *wtag);

unsigned long callc fennec_initialize_input(struct general_input_data* gin);
int           callc fennec_plugin_initialize(void);
int           callc fennec_plugin_getformat(unsigned long id, unsigned long* ftype, void* fdata);
int           callc fennec_plugin_getposition(unsigned long id, unsigned long* ptype, void* pos);
int           callc fennec_plugin_setposition(unsigned long id, unsigned long ptype, void* pos);
int           callc fennec_plugin_getduration(unsigned long id, unsigned long* ptype, void* pos);
int           callc fennec_plugin_getstatus(unsigned long id, unsigned long* stype, void* stt);
int           callc fennec_plugin_read(unsigned long id, unsigned long dsize, unsigned long* dread, void* rdata);
int           callc fennec_plugin_about(void* pdata);
int           callc fennec_plugin_settings(void* pdata);
int           callc fennec_plugin_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize);
int           callc fennec_plugin_get_extension(int id, string out_ext, string out_desc, int* newdsc);
int           callc fennec_plugin_tagread (const string fname, struct fennec_audiotag* rtag);
int           callc fennec_plugin_close(unsigned long id);
int           callc fennec_plugin_uninitialize(void);
int           callc fennec_plugin_tagwrite (const string fname, struct fennec_audiotag* rtag);
unsigned long callc fennec_plugin_open(unsigned long otype, unsigned long osize, const string odata);
int           callc fennec_plugin_score(const string fname, int foptions);

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
