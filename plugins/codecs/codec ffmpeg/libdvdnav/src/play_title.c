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


/**
 * Returns true if the pack is a NAV pack.  This check is clearly insufficient,
 * and sometimes we incorrectly think that valid other packs are NAV packs.  I
 * need to make this stronger.
 */
int is_nav_pack( unsigned char *buffer )
{
    return ( buffer[ 41 ] == 0xbf && buffer[ 1027 ] == 0xbf );
}


int main( int argc, char **argv )
{
    int titleid, chapid, pgc_id, len, start_cell, cur_cell;
    unsigned int cur_pack;
    int angle, ttn, pgn, next_cell;
    unsigned char *data;//[ 1024 * DVD_VIDEO_LB_LEN ];
    dvd_reader_t *dvd;
    dvd_file_t *title;
    ifo_handle_t *vmg_file;
    tt_srpt_t *tt_srpt;
    ifo_handle_t *vts_file;
    vts_ptt_srpt_t *vts_ptt_srpt;
    pgc_t *cur_pgc;
	FILE *outf;


    titleid = 1;
    chapid = 0;
    angle = 0;


	data = malloc(1024 * DVD_VIDEO_LB_LEN);
	outf = fopen("c:\\dv.mpg", "wb");
    /**
     * Open the disc.
     */
	dvd = DVDOpen( "G:\\");
    if( !dvd ) {
        fprintf( stderr, "Couldn't open DVD: %s\n", argv[ 1 ] );
        return -1;
    }


    /**
     * Load the video manager to find out the information about the titles on
     * this disc.
     */
    vmg_file = ifoOpen( dvd, 0 );
    if( !vmg_file ) {
        fprintf( stderr, "Can't open VMG info.\n" );
        DVDClose( dvd );
        return -1;
    }
    tt_srpt = vmg_file->tt_srpt;


    /**
     * Make sure our title number is valid.
     */
    fprintf( stderr, "There are %d titles on this DVD.\n",
             tt_srpt->nr_of_srpts );
    if( titleid < 0 || titleid >= tt_srpt->nr_of_srpts ) {
        fprintf( stderr, "Invalid title %d.\n", titleid + 1 );
        ifoClose( vmg_file );
        DVDClose( dvd );
        return -1;
    }


    /**
     * Make sure the chapter number is valid for this title.
     */
    fprintf( stderr, "There are %d chapters in this title.\n",
             tt_srpt->title[ titleid ].nr_of_ptts );

    if( chapid < 0 || chapid >= tt_srpt->title[ titleid ].nr_of_ptts ) {
        fprintf( stderr, "Invalid chapter %d\n", chapid + 1 );
        ifoClose( vmg_file );
        DVDClose( dvd );
        return -1;
    }


    /**
     * Make sure the angle number is valid for this title.
     */
    fprintf( stderr, "There are %d angles in this title.\n",
             tt_srpt->title[ titleid ].nr_of_angles );
    if( angle < 0 || angle >= tt_srpt->title[ titleid ].nr_of_angles ) {
        fprintf( stderr, "Invalid angle %d\n", angle + 1 );
        ifoClose( vmg_file );
        DVDClose( dvd );
        return -1;
    }


    /**
     * Load the VTS information for the title set our title is in.
     */
    vts_file = ifoOpen( dvd, tt_srpt->title[ titleid ].title_set_nr );
    if( !vts_file ) {
        fprintf( stderr, "Can't open the title %d info file.\n",
                 tt_srpt->title[ titleid ].title_set_nr );
        ifoClose( vmg_file );
        DVDClose( dvd );
        return -1;
    }


    /**
     * Determine which program chain we want to watch.  This is based on the
     * chapter number.
     */
    ttn = tt_srpt->title[ titleid ].vts_ttn;
    vts_ptt_srpt = vts_file->vts_ptt_srpt;
    pgc_id = vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ].pgcn;
    pgn = vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ].pgn;
    cur_pgc = vts_file->vts_pgcit->pgci_srp[ pgc_id - 1 ].pgc;
    start_cell = cur_pgc->program_map[ pgn - 1 ] - 1;


    /**
     * We've got enough info, time to open the title set data.
     */
    title = DVDOpenFile( dvd, tt_srpt->title[ titleid ].title_set_nr,
                         DVD_READ_TITLE_VOBS );
    if( !title ) {
        fprintf( stderr, "Can't open title VOBS (VTS_%02d_1.VOB).\n",
                 tt_srpt->title[ titleid ].title_set_nr );
        ifoClose( vts_file );
        ifoClose( vmg_file );
        DVDClose( dvd );
        return -1;
    }

    /**
     * Playback by cell in this pgc, starting at the cell for our chapter.
     */
    next_cell = start_cell;
    for( cur_cell = start_cell; next_cell < cur_pgc->nr_of_cells; ) {

        cur_cell = next_cell;

        /* Check if we're entering an angle block. */
        if( cur_pgc->cell_playback[ cur_cell ].block_type
                                        == BLOCK_TYPE_ANGLE_BLOCK ) {
            int i;

            cur_cell += angle;
            for( i = 0;; ++i ) {
                if( cur_pgc->cell_playback[ cur_cell + i ].block_mode
                                          == BLOCK_MODE_LAST_CELL ) {
                    next_cell = cur_cell + i + 1;
                    break;
                }
            }
        } else {
            next_cell = cur_cell + 1;
        }


        /**
         * We loop until we're out of this cell.
         */
        for( cur_pack = cur_pgc->cell_playback[ cur_cell ].first_sector;
             cur_pack < cur_pgc->cell_playback[ cur_cell ].last_sector; ) {

            dsi_t dsi_pack;
            unsigned int next_vobu, next_ilvu_start, cur_output_size;


            /**
             * Read NAV packet.
             */
            len = DVDReadBlocks( title, (int) cur_pack, 1, data );
            if( len != 1 ) {
                fprintf( stderr, "Read failed for block %d\n", cur_pack );
                ifoClose( vts_file );
                ifoClose( vmg_file );
                DVDCloseFile( title );
                DVDClose( dvd );
                return -1;
            }
            assert( is_nav_pack( data ) );


            /**
             * Parse the contained dsi packet.
             */
            navRead_DSI( &dsi_pack, &(data[ DSI_START_BYTE ]) );
            assert( cur_pack == dsi_pack.dsi_gi.nv_pck_lbn );


            /**
             * Determine where we go next.  These values are the ones we mostly
             * care about.
             */
            next_ilvu_start = cur_pack
                              + dsi_pack.sml_agli.data[ angle ].address;
            cur_output_size = dsi_pack.dsi_gi.vobu_ea;


            /**
             * If we're not at the end of this cell, we can determine the next
             * VOBU to display using the VOBU_SRI information section of the
             * DSI.  Using this value correctly follows the current angle,
             * avoiding the doubled scenes in The Matrix, and makes our life
             * really happy.
             *
             * Otherwise, we set our next address past the end of this cell to
             * force the code above to go to the next cell in the program.
             */
            if( dsi_pack.vobu_sri.next_vobu != SRI_END_OF_CELL ) {
                next_vobu = cur_pack
                            + ( dsi_pack.vobu_sri.next_vobu & 0x7fffffff );
            } else {
                next_vobu = cur_pack + cur_output_size + 1;
            }

            assert( cur_output_size < 1024 );
            cur_pack++;

            /**
             * Read in and output cursize packs.
             */
            len = DVDReadBlocks( title, (int)cur_pack, cur_output_size, data );
            if( len != (int) cur_output_size ) {
                fprintf( stderr, "Read failed for %d blocks at %d\n",
                         cur_output_size, cur_pack );
                ifoClose( vts_file );
                ifoClose( vmg_file );
                DVDCloseFile( title );
                DVDClose( dvd );
                return -1;
            }

            fwrite( data, cur_output_size, DVD_VIDEO_LB_LEN, outf );
            cur_pack = next_vobu;
        }
    }

	fclose(outf);
    ifoClose( vts_file );
    ifoClose( vmg_file );
    DVDCloseFile( title );
    DVDClose( dvd );
    return 0;
}

