/* declarations */

int  audiocd_initialize(void);
int  audiocd_load(char driveid);
int  audiocd_load_track(char driveid, unsigned int trackid);
int  audiocd_unload(char driveid, unsigned int trackid);
long audiocd_gettrackscount(char driveid);
long audiocd_gettracksize(char driveid, unsigned int trackid);
long audiocd_read(char driveid, unsigned int trackid, unsigned long startblock, unsigned long *rbytes, void* buffer);
int  audiocd_check(char driveid);
int  audiocd_tray_eject(char driveid);
int  audiocd_tray_load(char driveid);
long audiocd_getblocksize(void);
int  audiocd_uninitialize(void);
