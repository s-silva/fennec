/*
 * Copyright (C) 2001 Billy Biggs <vektor@dumbterm.net>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_types.h>
#include <dvdread/ifo_read.h>
#include <dvdread/dvd_udf.h>
#include <dvdread/nav_read.h>
#include <dvdread/nav_print.h>

#include "../../plugin.h"


struct dvinfo
{
	int               titleid, chapid, pgc_id, len, start_cell, cur_cell;
    unsigned int      cur_pack;
    int               angle, ttn, pgn, next_cell;
    dvd_reader_t     *dvd;
    dvd_file_t       *title;
    ifo_handle_t     *vmg_file;
    tt_srpt_t        *tt_srpt;
    ifo_handle_t     *vts_file;
    vts_ptt_srpt_t   *vts_ptt_srpt;
    pgc_t            *cur_pgc;
};


/**
 * Returns true if the pack is a NAV pack.  This check is clearly insufficient,
 * and sometimes we incorrectly think that valid other packs are NAV packs.  I
 * need to make this stronger.
 */
int is_nav_pack( unsigned char *buffer )
{
    return ( buffer[ 41 ] == 0xbf && buffer[ 1027 ] == 0xbf );
}


int dvddec_open(int driveid, void **info)
{
	struct dvinfo  *o;// = (struct dvinfo*)info;
   
	o = sys_mem_alloc(sizeof(struct dvinfo));
	*info = (void*)o;
	
	o->titleid = 0;
    o->chapid = 0;
    o->angle = 0;

	o->dvd = DVDOpen( "G:\\");

    if(!o->dvd ) return -1;

    o->vmg_file = ifoOpen(o->dvd, 0);
    if(!o->vmg_file)
	{
        DVDClose(o->dvd);
        return -1;
    }

    o->tt_srpt = o->vmg_file->tt_srpt;

    if(o->titleid < 0) o->titleid = 0;
	else if(o->titleid >= o->tt_srpt->nr_of_srpts) o->titleid = o->tt_srpt->nr_of_srpts - 1;


    if(o->chapid < 0) o->chapid = 0;
	else if(o->chapid >= o->tt_srpt->title[o->titleid].nr_of_ptts) o->chapid = o->tt_srpt->title[o->titleid].nr_of_ptts - 1;


    if(o->angle < 0) o->angle = 0;
	else if(o->angle >= o->tt_srpt->title[o->titleid].nr_of_angles)o->angle = o->tt_srpt->title[o->titleid].nr_of_angles - 1;

    o->vts_file = ifoOpen(o->dvd, o->tt_srpt->title[o->titleid].title_set_nr);
   
	if(!o->vts_file)
	{
        ifoClose(o->vmg_file);
        DVDClose(o->dvd);
        return -1;
    }


    /*
     * Determine which program chain we want to watch.  This is based on the
     * chapter number.
     */

    o->ttn          = o->tt_srpt->title[o->titleid].vts_ttn;
    o->vts_ptt_srpt = o->vts_file->vts_ptt_srpt;
    o->pgc_id       = o->vts_ptt_srpt->title[o->ttn - 1].ptt[o->chapid].pgcn;
    o->pgn          = o->vts_ptt_srpt->title[o->ttn - 1].ptt[o->chapid].pgn;
    o->cur_pgc      = o->vts_file->vts_pgcit->pgci_srp[o->pgc_id - 1].pgc;
    o->start_cell   = o->cur_pgc->program_map[o->pgn - 1] - 1;


    /*
     * We've got enough info, time to open the title set data.
     */
    o->title = DVDOpenFile(o->dvd, o->tt_srpt->title[o->titleid].title_set_nr, DVD_READ_TITLE_VOBS);
    if(!o->title)
	{
        ifoClose(o->vts_file);
        ifoClose(o->vmg_file);
        DVDClose(o->dvd);
        return -1;
    }

    o->next_cell = o->start_cell;

	{
		o->cur_cell = o->start_cell;

        /* Check if we're entering an angle block. */
        if(o->cur_pgc->cell_playback[o->cur_cell ].block_type == BLOCK_TYPE_ANGLE_BLOCK)
		{
            int i;

            o->cur_cell += o->angle;
            for( i = 0;; ++i )
			{
                if(o->cur_pgc->cell_playback[o->cur_cell + i ].block_mode == BLOCK_MODE_LAST_CELL )
				{
                    o->next_cell = o->cur_cell + i + 1;
                    break;
                }
            }
        }else{
            o->next_cell = o->cur_cell + 1;
        }

		o->cur_pack = o->cur_pgc->cell_playback[o->cur_cell ].first_sector;

	}
	return 0;
}

int dvddec_close(void *info)
{
	struct dvinfo  *o = (struct dvinfo*)info;

	if(!o->dvd)return 0;

	ifoClose(o->vts_file);
    ifoClose(o->vmg_file);
    DVDCloseFile(o->title);
    DVDClose(o->dvd);

	o->dvd = 0;

	sys_mem_free(info);
	return 1;
}

int dvddec_seek(void *info, float pos)
{
	int             i, j, blockcount = 0, locblock, cblock = 0, cpos = 0;
	struct dvinfo  *o = (struct dvinfo*)info;

	blockcount = o->cur_pgc->cell_playback[o->cur_pgc->nr_of_cells - 1].last_sector;

	locblock = (int)((float)blockcount * pos);

	for(i=0; i < o->cur_pgc->nr_of_cells; i++)
	{
		if((locblock > o->cur_pgc->cell_playback[i].first_sector) && (locblock < o->cur_pgc->cell_playback[i].last_sector))
		{
			o->cur_cell = i;
			o->next_cell = i + 1;
			cpos = o->cur_pgc->cell_playback[i].first_sector;//locblock;
			break;
		}
	}

	o->cur_pack = cpos;

	
	return 1;
}

int dvddec_read(void *info, void *buf)
{
	int             len;
	dsi_t           dsi_pack;
	unsigned char  *data = (unsigned char*)buf;
    unsigned int    next_vobu, next_ilvu_start, cur_output_size;
	struct dvinfo  *o = (struct dvinfo*)info;
	int             read_size = 0;
	int             dtt;

	if(!buf)return 0;
	if(!o->dvd)return 0;

    len = DVDReadBlocks(o->title, (int)o->cur_pack, 1, buf);

    if( len != 1 )
	{
        ifoClose(o->vts_file);
        ifoClose(o->vmg_file);
        DVDCloseFile(o->title);
        DVDClose(o->dvd);
		o->dvd = 0;
        return 0;
    }
    assert(is_nav_pack(buf));

    navRead_DSI( &dsi_pack, &(data[ DSI_START_BYTE ]) );
    assert(o->cur_pack == dsi_pack.dsi_gi.nv_pck_lbn );

    next_ilvu_start = o->cur_pack + dsi_pack.sml_agli.data[o->angle].address;
    cur_output_size = dsi_pack.dsi_gi.vobu_ea;


    if(dsi_pack.vobu_sri.next_vobu != SRI_END_OF_CELL )
	{
        next_vobu = o->cur_pack + ( dsi_pack.vobu_sri.next_vobu & 0x7fffffff );
    } else {
        next_vobu = o->cur_pack + cur_output_size + 1;
    }

    assert( cur_output_size < 1024 );
    o->cur_pack++;

    /**
        * Read in and output cursize packs.
        */
    len = DVDReadBlocks(o->title, (int)o->cur_pack, cur_output_size, buf);

    if(len != (int) cur_output_size)
	{
        ifoClose(o->vts_file);
        ifoClose(o->vmg_file);
        DVDCloseFile(o->title);
        DVDClose(o->dvd);
		o->dvd = 0;
        return 0;
    }

    /* fwrite( data, cur_output_size, DVD_VIDEO_LB_LEN, outf ); */
	read_size = cur_output_size * DVD_VIDEO_LB_LEN;
    o->cur_pack = next_vobu;

	dtt = o->cur_pgc->cell_playback[o->cur_cell].playback_time.second;


	if(o->cur_pack >= o->cur_pgc->cell_playback[o->cur_cell].last_sector)
	{
		o->cur_cell = o->next_cell;

        /* Check if we're entering an angle block. */
        if(o->cur_pgc->cell_playback[o->cur_cell ].block_type == BLOCK_TYPE_ANGLE_BLOCK)
		{
            int i;

            o->cur_cell += o->angle;
            for( i = 0;; ++i )
			{
                if(o->cur_pgc->cell_playback[o->cur_cell + i ].block_mode == BLOCK_MODE_LAST_CELL )
				{
                    o->next_cell = o->cur_cell + i + 1;
                    break;
                }
            }
        }else{
            o->next_cell = o->cur_cell + 1;
        }

		o->cur_pack = o->cur_pgc->cell_playback[o->cur_cell ].first_sector;
	}

	if(o->next_cell >= o->cur_pgc->nr_of_cells) return 0;

	return read_size;
}
