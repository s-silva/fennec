/* headers ------------------------------------------------------------------*/

#include "main.h"
#include "audio cd - windows.h"

/* CD audio definitions -----------------------------------------------------*/

#define cd_sector_size_raw    2352
#define cd_sector_size        2048
#define maximum_tracks         100
#define read_sectors            10
#define blocks_per_second       75
#define maximum_drives          26

/* structs ------------------------------------------------------------------*/

struct loaded_drive
{
	HANDLE         drive;
	unsigned short ntracks;

	struct
	{
		unsigned long start;
		unsigned long length;
		unsigned char loaded;
	}track[maximum_tracks];
};

/* declarations -------------------------------------------------------------*/

unsigned long audiocd_address_to_sectors(unsigned char addr[4]);

/* data ---------------------------------------------------------------------*/

struct loaded_drive  loaded_drives[maximum_drives];
unsigned char        engine_initialized = 0;

/* functions ----------------------------------------------------------------*/

/*
 * initialize engine.
 */
int audiocd_initialize(void)
{
	unsigned int i, j;

	if(engine_initialized)return 1; /* info: already initialized */

	for(i=0; i<maximum_drives; i++)
	{
		loaded_drives[i].drive = 0;
		for(j=0; j<maximum_tracks; j++)
		{
			loaded_drives[i].track[j].loaded = 0;
		}
	}

	engine_initialized = 1;

	return 0;
}

/*
 * initialize CD drive.
 * report: -3/1
 */
int audiocd_load(char driveid)
{
	CDROM_TOC     trackstable;
	HANDLE        temphandle;
	unsigned long nbr = 0;
	unsigned int  realindex;
	letter        windows_path[8] = {uni('\\'), uni('\\'), uni('.'), uni('\\'), (letter)driveid, uni(':'), uni('\0')};
	unsigned int  i, j;

	if(!engine_initialized)
	{
		audiocd_initialize();
	}

	if(driveid < 'A' || driveid > 'Z')return -1; /* error: invalid drive id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';
	
	/* already initialized? */
	/* if(loaded_drives[realindex].drive)return 1;*/
	if(loaded_drives[realindex].drive) CloseHandle(loaded_drives[realindex].drive); /* info: already initialized */

	/* load drive */
	temphandle = CreateFile(windows_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	
	if(temphandle == INVALID_HANDLE_VALUE)return -2; /* error: cannot load driver 'driveid' */

	if(!DeviceIoControl(temphandle, IOCTL_CDROM_READ_TOC, 0, 0, &trackstable, sizeof(trackstable), &nbr, 0))
	{
		CloseHandle(temphandle);
		return -3; /* error: cannot read TOC */
	}

	loaded_drives[realindex].drive   = temphandle;
	loaded_drives[realindex].ntracks = (trackstable.LastTrack - trackstable.FirstTrack) + 1;

	for(i=0; i<loaded_drives[realindex].ntracks; i++)
	{
		j = trackstable.FirstTrack + i - 1;
		loaded_drives[realindex].track[i].start  = audiocd_address_to_sectors(trackstable.TrackData[j].Address);
		loaded_drives[realindex].track[i].length = audiocd_address_to_sectors(trackstable.TrackData[j + 1].Address) - loaded_drives[realindex].track[i].start;
	}

	return 0;
}
/*
 * mark track as loaded.
 */
int audiocd_load_track(char driveid, unsigned int trackid)
{
	unsigned int  realindex;

	if(!engine_initialized)return -1; /* error: initialize the engine first */
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */
	if(trackid >= 100)return -3;                 /* error: invalid track id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(!loaded_drives[realindex].drive)
	{
		if(audiocd_load(driveid) < 0)return -4; /* error: drive initialization failed */
	}

	if(loaded_drives[realindex].ntracks < trackid)return -3;

	if(loaded_drives[realindex].track[trackid].loaded)return 1; /*info: already loaded */

	loaded_drives[realindex].track[trackid].loaded = 1;
	return 0;
}

/*
 * uninitialize track by track.
 */
int audiocd_unload(char driveid, unsigned int trackid)
{
	unsigned int  realindex;
	unsigned int  i, fnd = 0;

	if(!engine_initialized)return -1;            /* error: initialize the engine first */
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */
	if(trackid >= 100)return -3;                 /* error: invalid track id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(!loaded_drives[realindex].drive)return -4; /* error: uninitialized drive */
	if(loaded_drives[realindex].ntracks < trackid)return -3;

	if(!loaded_drives[realindex].track[trackid].loaded)return 1; /*info: already unloaded */

	loaded_drives[realindex].track[trackid].loaded = 0;

	for(i=0; i<maximum_tracks; i++)
	{
		if(loaded_drives[realindex].track[i].loaded)
		{
			fnd = 1;
			break;
		}
	}

	if(!fnd)
	{
		/* uninitialize drive */
		CloseHandle(loaded_drives[realindex].drive);
		loaded_drives[realindex].drive = 0;
	}
	return 0;
}

/*
 * get number of tracks.
 */
long audiocd_gettrackscount(char driveid)
{
	unsigned int  realindex;

	if(!engine_initialized)audiocd_initialize(); /* error: initialize the engine first */
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(!loaded_drives[realindex].drive)audiocd_load(driveid);

	return loaded_drives[realindex].ntracks;
}

/*
 * get track data size (bytes).
 */
long audiocd_gettracksize(char driveid, unsigned int trackid)
{
	unsigned int  realindex;

	if(!engine_initialized)return -1;            /* error: initialize the engine first */
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */
	if(trackid >= 100)return -3;                 /* error: invalid track id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(!loaded_drives[realindex].drive)return -4; /* error: uninitialized drive */
	if(loaded_drives[realindex].ntracks < trackid)return -3;

	return loaded_drives[realindex].track[trackid].length * cd_sector_size_raw;
}

/*
 * read data.
 * return: blocks read.
 */
long audiocd_read(char driveid, unsigned int trackid, unsigned long startblock, unsigned long *rbytes, void* buffer)
{
	unsigned long nbr = 0;
	unsigned int  realindex;
	RAW_READ_INFO rri;

	if(!engine_initialized)return -1;            /* error: initialize the engine first */
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */
	if(trackid >= 100)return -3;                 /* error: invalid track id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(!loaded_drives[realindex].drive)return -4; /* error: uninitialized drive */
	if(loaded_drives[realindex].ntracks < trackid)return -3;

	if(!DeviceIoControl(loaded_drives[realindex].drive, IOCTL_STORAGE_CHECK_VERIFY2, 0, 0, 0, 0, &nbr, 0))return 0;
	
	rri.TrackMode           = CDDA;
	rri.SectorCount         = read_sectors;

	rri.DiskOffset.QuadPart = (loaded_drives[realindex].track[trackid].start + (startblock * read_sectors)) * cd_sector_size;
	
	if(DeviceIoControl(loaded_drives[realindex].drive, IOCTL_CDROM_RAW_READ, &rri, sizeof(rri), buffer, cd_sector_size_raw * read_sectors, rbytes, 0))
	
		return read_sectors;
	else
		return 0;
}

int audiocd_check(char driveid)
{
	unsigned long nbr = 0;
	unsigned int  realindex;

	if(!engine_initialized)return -1;            /* error: initialize the engine first */
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(!loaded_drives[realindex].drive)return -4; /* error: uninitialized drive */
	return DeviceIoControl(loaded_drives[realindex].drive, IOCTL_STORAGE_CHECK_VERIFY2, 0, 0, 0, 0, &nbr, 0);
}

long audiocd_getblocksize(void)
{
	return cd_sector_size_raw * read_sectors;
}

/*
 * load disc;
 */
int audiocd_tray_load(char driveid)
{
	HANDLE        hdrive;
	unsigned long nbr = 0;
	unsigned int  realindex;
	int           res;

	if(!engine_initialized)audiocd_initialize();
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(loaded_drives[realindex].drive)
	{
		hdrive = loaded_drives[realindex].drive;
	}else{
		letter windows_path[8] = {uni('\\'), uni('\\'), uni('.'), uni('\\'), (letter)driveid, uni(':'), uni('\0')};
		hdrive = CreateFile(windows_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if(hdrive == INVALID_HANDLE_VALUE)return -2; /* error: cannot load driver 'driveid' */
	}

	res = DeviceIoControl(hdrive, IOCTL_STORAGE_LOAD_MEDIA, 0, 0, 0, 0, &nbr, 0);

	if(!loaded_drives[realindex].drive)
	{
		CloseHandle(hdrive);
	}
	return (!res);
}

/*
 * eject disc;
 */
int audiocd_tray_eject(char driveid)
{
	HANDLE        hdrive;
	ATOM          atm;
	unsigned long nbr = 0;
	unsigned int  realindex;
	int           res;

	if(!engine_initialized)audiocd_initialize();
	if(driveid < 'A' || driveid > 'Z')return -2; /* error: invalid drive id */

	/* real index: 0 to 25 */
	realindex = driveid - 'A';

	if(loaded_drives[realindex].drive)
	{
		hdrive = loaded_drives[realindex].drive;
	}else{
		letter windows_path[8] = {uni('\\'), uni('\\'), uni('.'), uni('\\'), (letter)driveid, uni(':'), uni('\0')};
		hdrive = CreateFile(windows_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if(hdrive == INVALID_HANDLE_VALUE)return -2; /* error: cannot load driver 'driveid' */
	}
	
	atm = GlobalFindAtom(uni("fennec.plugin.inputredbook.1.trayejected"));

	if(atm)
	{
		GlobalDeleteAtom(atm);
		res = DeviceIoControl(hdrive, IOCTL_STORAGE_LOAD_MEDIA, 0, 0, 0, 0, &nbr, 0);
	}else{
		GlobalAddAtom(uni("fennec.plugin.inputredbook.1.trayejected"));
		res = DeviceIoControl(hdrive, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &nbr, 0);
	}
	
	if(!loaded_drives[realindex].drive)
	{
		CloseHandle(hdrive);
	}
	return (!res);
}

/*
 * uninitialize whole engine.
 */
int audiocd_uninitialize(void)
{
	unsigned int i;

	if(!engine_initialized)return 1; /* info: already initialized */

	for(i=0; i<maximum_drives; i++)
	{
		if(loaded_drives[i].drive)
		{
			CloseHandle(loaded_drives[i].drive);
		}
	}

	engine_initialized = 0;

	return 0;
}

/* local functions ----------------------------------------------------------*/

unsigned long audiocd_address_to_sectors(unsigned char addr[4])
{
	unsigned long sectors = (addr[1] * (blocks_per_second * 60)) + (addr[2] * blocks_per_second) + addr[3];
	return sectors - 150;
}

