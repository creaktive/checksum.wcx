/****************************************************************************
	This file is part of MD5/SHA1 checksum generator/checker plugin for
	Total Commander.
	Copyright (C) 2003  Stanislaw Y. Pusep

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	E-Mail:	stanis@linuxmail.org
	Site:	http://sysdlabs.hypermart.net/
****************************************************************************/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <direct.h>
#include "wcxhead.h"
#include "md5.h"
#include "sha1.h"
#include "parser.h"


#define SUM_BUFSIZE 32768
#define VERSION "v0.2"


typedef struct
{
	char archive[MAX_PATH];
	char cwd[MAX_PATH];
	char file[MAX_PATH];
	char sum[64];
	unsigned int size;
	sum_node *ptr;
	sum_node *list;
} sum_handle;


tProcessDataProc progress;
int aborted = 1;


void fix_path(char *p, char from, char to)
{
	for (; *p != '\0'; p++)
		if (*p == from)
			*p = to;

}


BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID reserved)
{
	return TRUE;
}


HANDLE __stdcall OpenArchive(tOpenArchiveData *ArchiveData)
{
	sum_handle *sumH;
	char *p;

	sumH = (sum_handle *) malloc(sizeof(sum_handle));
	strncpy(sumH->archive, ArchiveData->ArcName, MAX_PATH);

	if ((ArchiveData->ArcName[1] == ':') ||
		((ArchiveData->ArcName[0] == '\\') && (ArchiveData->ArcName[1] == '\\')))
	{
		strncpy(sumH->cwd, ArchiveData->ArcName, MAX_PATH);
		for (p = sumH->cwd + strlen(sumH->cwd); p >= sumH->cwd; p--)
			if (*p == '\\' || *p == '/')
			{
				*p = '\0';
				break;
			}
	}
	else
		_getcwd(sumH->cwd, MAX_PATH);

	sumH->ptr = NULL;

	if ((sumH->list = sum_parse(sumH->archive)) != NULL)
		return sumH;
	else
	{
		ArchiveData->OpenResult = E_EOPEN;
		return NULL;
	}
}


int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
	sum_handle *sumH = hArcData;
	WIN32_FILE_ATTRIBUTE_DATA stat;
	FILETIME local;
	WORD date, time;

	if (sumH->ptr == NULL)
		sumH->ptr = sumH->list;
	else
		sumH->ptr = sumH->ptr->next;

	if (sumH->ptr == NULL)
	{
		sumH->ptr = sumH->list;
		return E_END_ARCHIVE;
	}

	memset(sumH->file, '\0', MAX_PATH);
	strncpy(sumH->file, sumH->cwd, MAX_PATH);
	strncat(sumH->file, "\\", MAX_PATH);
	strncat(sumH->file, sumH->ptr->filename, MAX_PATH);
	fix_path(sumH->file, '/', '\\');

	memset(sumH->sum, '\0', sizeof(sumH->sum));
	strncpy(sumH->sum, sumH->ptr->checksum, sizeof(sumH->sum));

	memset(HeaderData, '\0', sizeof(HeaderData));
	strncpy(HeaderData->ArcName, sumH->archive, MAX_PATH);
	strncpy(HeaderData->FileName, sumH->ptr->filename, MAX_PATH);
	fix_path(HeaderData->FileName, '/', '\\');

	if (GetFileAttributesEx(sumH->file, GetFileExInfoStandard, &stat))
	{
		FileTimeToLocalFileTime(&stat.ftLastWriteTime, &local);
		FileTimeToDosDateTime(&local, &date, &time);
		sumH->size = stat.nFileSizeLow;
		HeaderData->UnpSize		= stat.nFileSizeLow;
		HeaderData->FileTime	= (date << 16) | time;
		HeaderData->FileAttr	= stat.dwFileAttributes;
	}
	else
	{
		sumH->size = -1;
		HeaderData->UnpSize		= -1;
		HeaderData->FileTime	= -1;
	}

	return 0;
}


char *md5sum(char *filename)
{
	HANDLE *h;
	char *buf;
	struct MD5Context md5c;
	unsigned int read, i;
	unsigned char keybuf[16];
	char *checksum, *p;

	if ((h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ+FILE_SHARE_WRITE+FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		return NULL;

	buf = (char *) malloc(SUM_BUFSIZE);
	MD5Init(&md5c);

	while (ReadFile(h, buf, SUM_BUFSIZE, &read, NULL))
	{
		MD5Update(&md5c, buf, read);
		aborted = progress(filename, read);
		if ((read == 0) || (aborted == 0))
			break;
	}

	MD5Final(keybuf, &md5c);
	CloseHandle(h);
	free(buf);

	checksum = (char *) malloc(33);
	memset(checksum, '\0', 33);
	for (i = 0, p = checksum; i < sizeof(keybuf); i++, p += 2)
		sprintf(p, "%02x", keybuf[i]);

	return checksum;
}


char *sha1sum(char *filename)
{
	HANDLE *h;
	char *buf;
	SHA_State sha1s;
	unsigned int read, i;
	unsigned char keybuf[20];
	char *checksum, *p;

	if ((h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ+FILE_SHARE_WRITE+FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		return NULL;

	buf = (char *) malloc(SUM_BUFSIZE);
	SHA_Init(&sha1s);

	while (ReadFile(h, buf, SUM_BUFSIZE, &read, NULL))
	{
		SHA_Bytes(&sha1s, buf, read);
		aborted = progress(filename, read);
		if ((read == 0) || (aborted == 0))
			break;
	}

	SHA_Final(&sha1s, keybuf);
	CloseHandle(h);
	free(buf);

	checksum = (char *) malloc(41);
	memset(checksum, '\0', 41);
	for (i = 0, p = checksum; i < sizeof(keybuf); i++, p += 2)
		sprintf(p, "%02x", keybuf[i]);

	return checksum;
}


int __stdcall ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
	sum_handle *sumH = hArcData;
	char message[MAX_PATH], checksum[64], *sum, *type, fail[64];
	FILE *f;
	BOOL is_ok = FALSE;
	int typen;

	if (Operation != PK_SKIP)
	{
		if (sumH->size != -1)
		{
			typen = sumH->ptr->type;
			switch (typen)
			{
				case MD5:	type = "MD5";	sum = md5sum(sumH->file);	break;
				case SHA1:	type = "SHA1";	sum = sha1sum(sumH->file);	break;
				default: sum = NULL;
			}

			if (sum == NULL)
				return E_EREAD;

			memset(checksum, '\0', sizeof(checksum));
			strncpy(checksum, sum, sizeof(checksum));
			free(sum);

			if (aborted == 0)
				return E_EABORTED;

			is_ok = (strncmp(sumH->sum, checksum, sizeof(sumH->sum)) == 0) ? TRUE : FALSE;
		}
		else
		{
			switch (strlen(sumH->sum))
			{
				case MD5:	type = "MD5";	typen = MD5;	break;
				case SHA1:	type = "SHA1";	typen = SHA1;	break;
			}

			MessageBox(NULL, sumH->file, "File not found!", MB_OK|MB_ICONWARNING);
		}

		if (Operation == PK_TEST)
			return is_ok ? 0 : E_BAD_ARCHIVE;
		else
		{
			memset(message, '\0', MAX_PATH);
			if (DestPath != NULL)
			{
				strncpy(message, DestPath, MAX_PATH);
				if (message[strlen(message)] != '\\')
					strcat(message, "\\");
			}
			strncat(message, DestName, MAX_PATH);

			if ((f = fopen(message, "wt")) != NULL)
			{
				memset(fail, '?', sizeof(fail));
				fail[typen] = '\0';

				fprintf(f, "%s\n\n", sumH->file);
				fprintf(f, "expected:\t%s\n", sumH->sum);
				fprintf(f, "computed:\t%s\n", (sumH->size != -1) ? checksum : fail);
				fprintf(f, "\n%s checksum %s!\n", type, is_ok ? "OK" : "FAILED");
				fclose(f);
			}
			else
				return E_ECREATE;
		}
	}

	return 0;
}


int __stdcall CloseArchive(HANDLE *hArcData)
{
	sum_handle *sumH = (sum_handle *) hArcData;

	sum_free(sumH->list);
	free(sumH);
	
	return 0;
}


int __stdcall PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags)
{
	FILE *f;
	char *p;
	char disk[MAX_PATH], arch[MAX_PATH];
	char *checksum;
	int type;

	if (Flags == PK_PACK_MOVE_FILES)
		return E_NOT_SUPPORTED;

	strncpy(arch, SrcPath, MAX_PATH);
	for (p = arch; *p; p++)
		if (*p == '\\')
		{
			*(++p) = '\0';
			break;
		}

	if (GetDriveType(arch) == DRIVE_CDROM)
		strncpy(disk, PackedFile, MAX_PATH);
	else
	{
		strncpy(disk, SrcPath, MAX_PATH);
		for (p = PackedFile + strlen(PackedFile); p >= PackedFile; p--)
			if (*p == '\\')
			{
				p++;
				break;
			}
		strncat(disk, p, MAX_PATH);
	}

	p = PackedFile + strlen(PackedFile) - 4;
	if (!strnicmp(p, ".md5", 4))
		type = MD5;
	else if (!strnicmp(p, ".sha", 4))
		type = SHA1;
	else
	{
		MessageBox(NULL, "Extension must be .MD5 or .SHA!", NULL, MB_OK|MB_ICONSTOP);
		return E_ECREATE;
	}

	if ((f = fopen(disk, "wt")) != NULL)
	{
		for (p = AddList; *p != '\0'; p += strlen(p) + 1)
		{
			memset(disk, '\0', MAX_PATH);
			memset(arch, '\0', MAX_PATH);

			strncpy(disk, SrcPath, MAX_PATH);
			strncat(disk, p, MAX_PATH);

			if (SubPath != NULL)
				strncat(arch, SubPath, MAX_PATH);
			strncat(arch, p, MAX_PATH);
			fix_path(arch, '\\', '/');

			//fprintf(f, "[%s] [%s]\n", disk, arch);
			if (disk[strlen(disk) - 1] != '\\')
			{
				if (type == MD5)
					checksum = md5sum(disk);
				else if (type == SHA1)
					checksum = sha1sum(disk);
				else
					checksum = NULL;

				if (checksum == NULL)
				{
					fclose(f);
					return E_EOPEN;
				}

				if (aborted == 0)
				{
					fclose(f);
					return E_EABORTED;
				}

				fprintf(f, "%s *%s\n", checksum, arch);
				free(checksum);
			}
		}

		fclose(f);
	}
	else
		return E_ECREATE;

	return 0;
}


/*
int __stdcall DeleteFiles(char *PackedFile, char *DeleteList)
{
	return 0;
}
*/


int __stdcall GetPackerCaps(void)
{
	return PK_CAPS_NEW|PK_CAPS_MULTIPLE|PK_CAPS_OPTIONS;
}


void __stdcall ConfigurePacker(HWND Parent, HINSTANCE DllInstance)
{
	MessageBox(Parent,
		"Provides MD5/SHA1 checksum generator/checker\n"
		"from within Total Commander packer interface\n\n"

		"Copyright © 2003  Stanislaw Y. Pusep\n"
		"stanis@linuxmail.org\n"
		"http://sysdlabs.hypermart.net/proj/",
		
		"MD5/SHA1 checksum wrapper " VERSION,
		
		MB_OK|MB_ICONINFORMATION);

	return;
}


void __stdcall SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
	return;
}


void __stdcall SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
	progress = pProcessDataProc;
	return;
}
