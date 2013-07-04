#include "skin.h"
#include "skin settings.h"
#include <shlobj.h>
#include <userenv.h>
#include "wininet.h"


#define  covers_max    10

int  covers_queue_count = 0;
int  covers_nointernet = 0;
int  covers_initialized = 0;
int  downloader_t_terminate = 0;
static HINTERNET  hin;

static letter     thumbs_albums_dir[1024];
static letter     thumbs_artists_dir[1024];
static int        thumbs_albums_dir_slen = 0;
static int        thumbs_artists_dir_slen = 0;

CRITICAL_SECTION   coverdown_cs;

DWORD downloader_thread(LPVOID lp);

struct cover
{
	letter artist[1024];
	letter album[1024];
};


struct cover covers_data[covers_max];



int  covers_init(void)
{
	if(covers_initialized) return 0;

	covers_queue_count = 0;

	if(covers_nointernet) return 0;


	memset(thumbs_albums_dir, 0, sizeof(thumbs_albums_dir));
	SHGetSpecialFolderPath(0, thumbs_albums_dir, CSIDL_PERSONAL, FALSE);
	str_cat(thumbs_albums_dir, uni("\\Fennec\\Thumbnails\\Albums\\"));
	thumbs_albums_dir_slen = str_len(thumbs_albums_dir);

	memset(thumbs_artists_dir, 0, sizeof(thumbs_artists_dir));
	SHGetSpecialFolderPath(0, thumbs_artists_dir, CSIDL_PERSONAL, FALSE);
	str_cat(thumbs_artists_dir, uni("\\Fennec\\Thumbnails\\Artists\\"));
	thumbs_artists_dir_slen = str_len(thumbs_artists_dir);


	InitializeCriticalSection(&coverdown_cs);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)downloader_thread, 0, 0, 0);


	hin = InternetOpen(uni("fennec 1.3"), INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);

	if(!hin) /* no internet */
	{
		covers_nointernet = 1;
		covers_uninit();
		return 0;
	}

	covers_initialized = 1;

	//cover_download_discog(uni("Opeth"), uni("Blackwater Park"));

	return 1;
}

int  covers_uninit(void)
{
	if(!covers_initialized) return 0;

	EnterCriticalSection(&coverdown_cs);
	downloader_t_terminate = 1;
	LeaveCriticalSection(&coverdown_cs);

	while(downloader_t_terminate)
	{
		Sleep(0);
	}

	DeleteCriticalSection(&coverdown_cs);

	
	if(hin)
		InternetCloseHandle(hin);

	covers_initialized = 0;

	return 1;
}

int  covers_addtoqueue(const string artist, const string album)
{
	if(covers_queue_count + 1 >= covers_max) return 0;

	str_cpy(covers_data[covers_queue_count].artist, artist);
	if(album)
		str_cpy(covers_data[covers_queue_count].album, album);
	else
		covers_data[covers_queue_count].album[0] = 0;

	covers_queue_count++;

	return 1;
}


void  search_andterminate_once(string s, letter t)
{
	for(;;s++)
	{
		if(*s == 0) return;
		if(*s == t){ *s = 0; return;};
	}
}

void  search_replace_letters(string s, letter o, letter n)
{
	for(;;s++)
	{
		if(*s == 0) return;
		if(*s == o) *s = n;
	}
}


string  search_instr(string s, string t)
{
	string   mterm = t;
	int      matchstarted = 0;

	for(;;s++)
	{
		if(*s == 0) return 0;

		if(*s == *mterm)
		{
			matchstarted = 1;
			mterm++;

			if(*mterm == 0) return s + 1;

		}else{
			if(matchstarted)
			{
				mterm = t;
				matchstarted = 0;
			}
		}
	} 
}


int  cover_download_discog(const string artist, const string album)
{
	HINTERNET  huri;
	//string     search_url = uni("http://api.discogs.com/search?q=");
	string     search_url = uni("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=990b6cf53fc136b05379d28f10659046&artist=");
	string     search_url_artist = uni("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&api_key=990b6cf53fc136b05379d28f10659046&artist=");
	letter     search_str[1024];
	INTERNET_BUFFERS   ibuf;
	char       bytebuffer[4096];
	letter     strbuffer[4096];
	BOOL       readresult;
	int        mode = 0;
	string     imgpath;
	int        fmode = 0; /* jpeg or png */
	letter     fname[1024];

	if(!artist) return 0;
	if(artist[0] == 0) return 0;


	if(album)
		str_cpy(search_str, search_url);
	else
		str_cpy(search_str, search_url_artist);

	str_cat(search_str, artist);

	if(album)
	{
		if(mode == 0) /* last.fm */
		{
			str_cat(search_str, uni("&album="));
		}else{
			str_cat(search_str, uni("+"));
		}

		str_cat(search_str, album);
	}

	search_replace_letters(search_str, uni(' '), uni('+'));

	
	
	huri = InternetOpenUrl(hin, search_str, 0, 0, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE, 0);

	if(!huri) return 0;

	memset(&ibuf, 0, sizeof(ibuf));
	memset(bytebuffer, 0, sizeof(bytebuffer));
	memset(strbuffer, 0, sizeof(strbuffer));

	ibuf.dwStructSize   = sizeof(ibuf);
    ibuf.Next           = 0;
    ibuf.lpcszHeader    = 0;
    ibuf.lpvBuffer      = bytebuffer;
    ibuf.dwBufferLength = 4096;

	readresult = InternetReadFileEx(huri, &ibuf, IRF_NO_WAIT, 0);

	if(!readresult)
	{
		InternetCloseHandle(huri);
		return 0;
	}

	MultiByteToWideChar(CP_UTF8, 0, bytebuffer, ibuf.dwBufferLength, strbuffer, 4096);

	imgpath = search_instr(strbuffer, uni("<image size=\"large\">"));

	if(!imgpath)
	{
		InternetCloseHandle(huri);
		return 0;
	}

	search_andterminate_once(imgpath, uni('<'));

	if(!imgpath[0])
	{
		InternetCloseHandle(huri);
		return 0;
	}


	/* generate file name */

	if(album)
		str_cpy(fname, thumbs_albums_dir);
	else
		str_cpy(fname, thumbs_artists_dir);

	str_cat(fname, artist);

	if(album)
	{
		str_cat(fname, uni(" - "));
		str_cat(fname, album);
	}

	{
		int k = str_len(imgpath);

		if(k > 2)
		{
			if(imgpath[k - 2] == uni('n'))
				fmode = 1; /* png */
		}
	}


	if(fmode == 1) /* png */
	{
		str_cat(fname, uni(".png"));
	}else{        /* jpg */
		str_cat(fname, uni(".jpg"));
	}

	InternetCloseHandle(huri);




	URLDownloadToFile(0, imgpath, fname, 0, 0);

	return 1;
}




DWORD downloader_thread(LPVOID lp)
{
	int sleepy_time = 1000;

point_start:

	EnterCriticalSection(&coverdown_cs);


	if(downloader_t_terminate)
	{
		downloader_t_terminate = 0;
		LeaveCriticalSection(&coverdown_cs);
		return 0;
	}


	if(covers_queue_count == 0) sleepy_time = 5000;
	else sleepy_time = 1000;


	if(covers_queue_count)
	{
		if(covers_data[covers_queue_count].album[0])
			cover_download_discog(covers_data[covers_queue_count].artist, covers_data[covers_queue_count].album);
		else
			cover_download_discog(covers_data[covers_queue_count].artist, 0);

		covers_queue_count--;
	}
	
	LeaveCriticalSection(&coverdown_cs);
	Sleep(1000);

	goto point_start;
}