/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
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

#include "fennec_main.h"
#include <shlobj.h>
#include "winerror.h"

int get_default_file_icon(const string ext);


int fileassociation_set(const string ext,    /* extension (w\o leading period) */
						const string acname, /* action name */
						const string lacname,/* long action name */
						const string icpath, /* icon path */
						const string accmd,  /* action command */
						const string dsc,    /* type description */
						int          icidx)  /* icon index */
{

	letter        apext [256];
	letter        apcls [256];
	letter        kval  [256];
	letter        icdat [256];
	letter        tmpbuf[128];
	HKEY          rkey, extkey, clskey, tmpkey, tmpkey2, tmpkey3;
	long          vsize;
	const string  cls_name = uni("fennec.player.file.");
	const string  bkp_name = uni("fennec.player.backup");
	
	if(!ext)return 0;
	if(!acname)return 0;
	if(!str_len(ext))return 0;
	if(!str_len(acname))return 0;

	if(icidx == -1)icidx = get_default_file_icon(ext);

	apext[0] = uni('.');
	str_cpy(apext + 1, ext);

	/* create class name */

	str_cpy(apcls, cls_name);
	str_cat(apcls, ext); /* i.e. "fennec.player.file.ogg" */

	/* try opening keys */

	if(RegOpenKey(HKEY_CLASSES_ROOT, 0, &rkey) != ERROR_SUCCESS)return 0;


	RegCreateKey(rkey, apext, &extkey);

	/* get current value */

	memset(kval, 0, sizeof(kval));
	vsize = sizeof(kval);
	RegQueryValue(extkey, 0 /* (default) */, kval, &vsize);

	if(str_icmp(apcls, kval))
	{
		/* create a backup */
		RegSetValueEx(extkey, bkp_name, 0, REG_SZ, (CONST BYTE*)kval, (unsigned long)str_size(kval));
	}

	RegSetValue(extkey, 0, REG_SZ, apcls, (unsigned long)str_size(apcls));

	RegCloseKey(extkey);

	/* create the class */

	if(RegCreateKey(rkey, apcls, &clskey) != ERROR_SUCCESS)
	{
		RegCloseKey(rkey);
		return 0;
	}

	RegSetValue(clskey, 0, REG_SZ, dsc, (unsigned long)str_size(dsc));

	/* <icon creation> */

	RegCreateKey(clskey, uni("DefaultIcon"), &tmpkey);

	memset(tmpbuf, 0, sizeof(tmpbuf));
	str_itos(icidx, tmpbuf, 10);
	str_cpy(icdat, icpath);
	str_cat(icdat, uni(","));
	str_cat(icdat, tmpbuf);

	RegSetValue(tmpkey, 0, REG_SZ, icdat, (unsigned long)str_size(icdat));

	RegCloseKey(tmpkey);

	/* </icon creation> */

	/* <shell> */

	RegCreateKey(clskey, uni("shell"), &tmpkey);

	RegSetValue(tmpkey, 0, REG_SZ, acname, (unsigned long)str_size(acname));
	
	RegCreateKey(tmpkey, acname, &tmpkey2);

	RegSetValue(tmpkey2, 0, REG_SZ, lacname, (unsigned long)str_size(lacname));

	RegCreateKey(tmpkey2, uni("command"), &tmpkey3);

	memset(kval, 0, sizeof(kval));
	str_cpy(kval, uni("\""));
	str_cat(kval, accmd);
	str_cat(kval, uni("\" \"%1\"")); /* "%1" */

	RegSetValue(tmpkey3, 0, REG_SZ, kval, (unsigned long)str_size(kval));

	RegCloseKey(tmpkey3);
	RegCloseKey(tmpkey2);
	RegCloseKey(tmpkey);
	RegCloseKey(clskey);
	RegCloseKey(rkey);

	str_cpy(apcls, uni("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"));
	str_cat(apcls, apext);

	RegOpenKey(HKEY_CURRENT_USER, apcls, &rkey);
	RegDeleteValue(rkey, uni("Application"));
	RegDeleteValue(rkey, uni("ProgID"));
	RegCloseKey(rkey);

	/* </shell> */
	return 1;
}

int fileassociation_restore(const string ext)
{
	letter         apext[255];
	letter         apcls[255];
	letter         kvalue[255];
	unsigned long  kvlen;
	HKEY           extkey;
	const string   cls_name = uni("fennec.player.file.");
	const string   bkp_name = uni("fennec.player.backup");
	LONG a;

	apext[0] = uni('.');
	str_cpy(apext + 1, ext);

	str_cpy(apcls, cls_name);
	str_cat(apcls, ext); /* i.e. "fennec.player.file.ogg" */


	if(RegOpenKey(HKEY_CLASSES_ROOT, apext, &extkey) != ERROR_SUCCESS)return 0;

	memset(kvalue, 0, sizeof(kvalue));
	kvlen = sizeof(kvalue);
	a = RegQueryValueEx(extkey, bkp_name, 0, 0, (LPBYTE)kvalue, &kvlen);

	if(!str_len((const string)kvalue))
	{
		/* no previous association */
		RegCloseKey(extkey);
		RegDeleteKey(HKEY_CLASSES_ROOT, apext);
		//SHDeleteKey(rkey, apcls);
		return 1;
	}

	RegSetValue(extkey, 0, REG_SZ, (LPCTSTR)kvalue, (unsigned long)str_size(kvalue));
	RegDeleteValue(extkey, bkp_name);

	RegCloseKey(extkey);
	return 1;
}

int fileassociation_selected(const string ext)
{
	letter         apext[255];
	letter         apcls[255];
	letter         rvalue[260];
	unsigned long  rvsize = sizeof(rvalue);
	const string   cls_name = uni("fennec.player.file.");

	apext[0] = uni('.');
	str_cpy(apext + 1, ext);

	str_cpy(apcls, cls_name);
	str_cat(apcls, ext); /* i.e. "fennec.player.file.ogg" */

	memset(rvalue, 0, sizeof(rvalue));
	
	RegQueryValue(HKEY_CLASSES_ROOT, apext, rvalue, (PLONG)&rvsize);

	if(rvsize)
	{
		if(str_icmp(apcls, rvalue))return 0;
	}
	return 1;
}

unsigned int fileassociation_geticonid(const string ext)
{
	letter         apext[255];
	letter         apcls[255];
	letter         rvalue[260];
	unsigned long  rvsize = sizeof(rvalue);
	const string   cls_name = uni("fennec.player.file.");
	int            iconid = 0;
	string         pt;

	apext[0] = uni('.');
	str_cpy(apext + 1, ext);

	str_cpy(apcls, cls_name);
	str_cat(apcls, ext); /* i.e. "fennec.player.file.ogg" */
	str_cat(apcls, uni("\\DefaultIcon"));

	if(RegQueryValue(HKEY_CLASSES_ROOT, apcls, rvalue, (PLONG)&rvsize) != ERROR_SUCCESS) goto pt_retdefaultid;
	if(!rvsize) goto pt_retdefaultid;

	pt = str_rchr(rvalue, uni(','));
	if(!pt) goto pt_retdefaultid;
	pt++; /* ',' */

	while(*pt == uni(' '))pt++;

	iconid = str_stoi(pt);
	return iconid;

pt_retdefaultid:

	/* default icon ids */

	return get_default_file_icon(ext);
}

int get_default_file_icon(const string ext)
{
	if(!ext) return 0;
	if(!ext[0]) return 0;

	/*if(!str_icmp(ext, uni("mp4")))    return -1 + 1; */ /* for video */
	if(!str_icmp(ext, uni("m2a")))    return -1 + 1;
	if(!str_icmp(ext, uni("m4a")))    return -1 + 1;
	if(!str_icmp(ext, uni("aac")))    return -1 + 1;
	if(!str_icmp(ext, uni("amr")))    return -1 + 2;
	if(!str_icmp(ext, uni("mpc")))    return -1 + 5;
	if(!str_icmp(ext, uni("raw")))    return -1 + 2;
	if(!str_icmp(ext, uni("aif")))    return -1 + 7;
	if(!str_icmp(ext, uni("aiff")))   return -1 + 7;
	if(!str_icmp(ext, uni("wav")))    return -1 + 7;
	if(!str_icmp(ext, uni("wave")))   return -1 + 7;
	if(!str_icmp(ext, uni("au")))     return -1 + 7;
	if(!str_icmp(ext, uni("caf")))    return -1 + 7;
	if(!str_icmp(ext, uni("snd")))    return -1 + 7;
	if(!str_icmp(ext, uni("svx")))    return -1 + 7;
	if(!str_icmp(ext, uni("paf")))    return -1 + 7;
	if(!str_icmp(ext, uni("fap")))    return -1 + 7;
	if(!str_icmp(ext, uni("gsm")))    return -1 + 7;
	if(!str_icmp(ext, uni("nist")))   return -1 + 7;
	if(!str_icmp(ext, uni("ircam")))  return -1 + 7;
	if(!str_icmp(ext, uni("sf")))     return -1 + 7;
	if(!str_icmp(ext, uni("voc")))    return -1 + 7;
	if(!str_icmp(ext, uni("w64")))    return -1 + 7;
	if(!str_icmp(ext, uni("mat4")))   return -1 + 7;
	if(!str_icmp(ext, uni("mat5")))   return -1 + 7;
	if(!str_icmp(ext, uni("mat")))    return -1 + 7;
	if(!str_icmp(ext, uni("xi")))     return -1 + 7;
	if(!str_icmp(ext, uni("pvf")))    return -1 + 7;
	if(!str_icmp(ext, uni("sds")))    return -1 + 7;
	if(!str_icmp(ext, uni("sd2")))    return -1 + 7;
	if(!str_icmp(ext, uni("vox")))    return -1 + 7;
	if(!str_icmp(ext, uni("ogg")))    return -1 + 6;
/*  if(!str_icmp(ext, uni("linein"))) return -1 + 1; */
	if(!str_icmp(ext, uni("mod")))    return -1 + 3;
	if(!str_icmp(ext, uni("nst")))    return -1 + 3;
	if(!str_icmp(ext, uni("mdz")))    return -1 + 3;
	if(!str_icmp(ext, uni("mdr")))    return -1 + 3;
	if(!str_icmp(ext, uni("m15")))    return -1 + 3;
	if(!str_icmp(ext, uni("s3m")))    return -1 + 3;
	if(!str_icmp(ext, uni("stm")))    return -1 + 3;
	if(!str_icmp(ext, uni("s3z")))    return -1 + 3;
	if(!str_icmp(ext, uni("xm")))     return -1 + 3;
	if(!str_icmp(ext, uni("xmz")))    return -1 + 3;
	if(!str_icmp(ext, uni("it")))     return -1 + 3;
	if(!str_icmp(ext, uni("itz")))    return -1 + 3;
	if(!str_icmp(ext, uni("mtm")))    return -1 + 3;
	if(!str_icmp(ext, uni("669")))    return -1 + 3;
	if(!str_icmp(ext, uni("ult")))    return -1 + 3;
	if(!str_icmp(ext, uni("wow")))    return -1 + 3;
	if(!str_icmp(ext, uni("far")))    return -1 + 3;
	if(!str_icmp(ext, uni("mdl")))    return -1 + 3;
	if(!str_icmp(ext, uni("okt")))    return -1 + 3;
	if(!str_icmp(ext, uni("dmf")))    return -1 + 3;
	if(!str_icmp(ext, uni("ptm")))    return -1 + 3;
	if(!str_icmp(ext, uni("med")))    return -1 + 3;
	if(!str_icmp(ext, uni("ams")))    return -1 + 3;
	if(!str_icmp(ext, uni("dbm")))    return -1 + 3;
	if(!str_icmp(ext, uni("dsm")))    return -1 + 3;
	if(!str_icmp(ext, uni("umx")))    return -1 + 3;
	if(!str_icmp(ext, uni("amf")))    return -1 + 3;
	if(!str_icmp(ext, uni("psm")))    return -1 + 3;
	if(!str_icmp(ext, uni("mt2")))    return -1 + 3;
	if(!str_icmp(ext, uni("mid")))    return -1 + 3;
	if(!str_icmp(ext, uni("midi")))   return -1 + 3;
	if(!str_icmp(ext, uni("rmi")))    return -1 + 3;
	if(!str_icmp(ext, uni("smf")))    return -1 + 3;
	if(!str_icmp(ext, uni("mp1")))    return -1 + 2;
	if(!str_icmp(ext, uni("mp2")))    return -1 + 2;
	if(!str_icmp(ext, uni("mp3")))    return -1 + 4;
	if(!str_icmp(ext, uni("cda")))    return -1 + 2;
	if(!str_icmp(ext, uni("wma")))    return -1 + 8;

	if(!str_icmp(ext, uni("ape")))    return -1 + 9;
	if(!str_icmp(ext, uni("flac")))   return -1 + 10;
	if(!str_icmp(ext, uni("wv")))     return -1 + 13;
	if(!str_icmp(ext, uni("speex")))  return -1 + 14;

	if(!str_icmp(ext, uni("mpg")))    return -1 + 15;
	if(!str_icmp(ext, uni("mpeg")))   return -1 + 15;
	if(!str_icmp(ext, uni("avi")))    return -1 + 15;
	if(!str_icmp(ext, uni("vob")))    return -1 + 15;
	if(!str_icmp(ext, uni("flv")))    return -1 + 15;
	if(!str_icmp(ext, uni("3gp")))    return -1 + 15;
	if(!str_icmp(ext, uni("mov")))    return -1 + 15;
	if(!str_icmp(ext, uni("qt")))     return -1 + 15;
	if(!str_icmp(ext, uni("asf")))    return -1 + 15;
	if(!str_icmp(ext, uni("wmv")))    return -1 + 15;
	if(!str_icmp(ext, uni("divx")))   return -1 + 15;
	if(!str_icmp(ext, uni("mkv")))    return -1 + 15;
	if(!str_icmp(ext, uni("mp4")))    return -1 + 15;
	if(!str_icmp(ext, uni("ogm")))    return -1 + 15;
	if(!str_icmp(ext, uni("rm")))     return -1 + 15;
	if(!str_icmp(ext, uni("rmvb")))   return -1 + 15;
	if(!str_icmp(ext, uni("swf")))    return -1 + 15;

	return -1 + 2; /* audio */
}