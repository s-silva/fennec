#include <windows.h>

#define IOCTL_CDROM_RAW_READ 0x2403E
#define IOCTL_CDROM_READ_TOC 0x24000

#define MAXIMUM_NUMBER_TRACKS 100

typedef struct _TRACK_DATA
{
	UCHAR Reserved;
	UCHAR Control : 4;
	UCHAR Adr : 4;
	UCHAR TrackNumber;
	UCHAR Reserved1;
	UCHAR Address[4];
} TRACK_DATA;

typedef struct _CDROM_TOC
{
	UCHAR Length[2];
	UCHAR FirstTrack;
	UCHAR LastTrack;
	TRACK_DATA TrackData[MAXIMUM_NUMBER_TRACKS];
} CDROM_TOC;

typedef enum _TRACK_MODE_TYPE
{
	YellowMode2,
	XAForm2,
	CDDA
} TRACK_MODE_TYPE, *PTRACK_MODE_TYPE;

typedef struct __RAW_READ_INFO
{
	LARGE_INTEGER  DiskOffset;
	ULONG  SectorCount;
	TRACK_MODE_TYPE  TrackMode;
} RAW_READ_INFO, *PRAW_READ_INFO;