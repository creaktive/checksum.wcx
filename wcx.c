/****************************************************************************
	This file is part of MD5 checksum generator/checker plugin for
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


#include <windows.h>
#include <direct.h>
#include "wcxhead.h"
#include "md5.h"
#include "parser.h"


#define MD5_BUFSIZE 32768
#define VERSION "v0.1"


typedef struct
{
	char archive[MAX_PATH];
	char cwd[MAX_PATH];
	char file[MAX_PATH];
	char sum[33];
	unsigned int size;
	md5_node *ptr;
	md5_node *list;
} md5_handle;


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
	md5_handle *md5H;
	char *p;

	md5H = (md5_handle *) malloc(sizeof(md5_handle));
	strncpy(md5H->archive, ArchiveData->ArcName, MAX_PATH);

	if ((ArchiveData->ArcName[1] == ':') ||
		((ArchiveData->ArcName[0] == '\\') && (ArchiveData->ArcName[1] == '\\')))
	{
		strncpy(md5H->cwd, ArchiveData->ArcName, MAX_PATH);
		for (p = md5H->cwd + strlen(md5H->cwd); p >= md5H->cwd; p--)
			if (*p == '\\' || *p == '/')
			{
				*p = '\0';
				break;
			}
	}
	else
		_getcwd(md5H->cwd, MAX_PATH);

	md5H->ptr = NULL;

	if ((md5H->list = md5_parse(md5H->archive)) != NULL)
		return md5H;
	else
	{
		ArchiveData->OpenResult = E_EOPEN;
		return NULL;
	}
}


int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
	md5_handle *md5H = hArcData;
	WIN32_FILE_ATTRIBUTE_DATA stat;
	FILETIME local;
	WORD date, time;

	if (md5H->ptr == NULL)
		md5H->ptr = md5H->list;
	else
		md5H->ptr = md5H->ptr->next;

	if (md5H->ptr == NULL)
	{
		md5H->ptr = md5H->list;
		return E_END_ARCHIVE;
	}

	memset(md5H->file, '\0', MAX_PATH);
	strncpy(md5H->file, md5H->cwd, MAX_PATH);
	strncat(md5H->file, "\\", MAX_PATH);
	strncat(md5H->file, md5H->ptr->filename, MAX_PATH);
	fix_path(md5H->file, '/', '\\');

	memset(md5H->sum, '\0', 33);
	strncpy(md5H->sum, md5H->ptr->checksum, 32);

	memset(HeaderData, '\0', sizeof(HeaderData));
	strncpy(HeaderData->ArcName, md5H->archive, MAX_PATH);
	strncpy(HeaderData->FileName, md5H->ptr->filename, MAX_PATH);
	fix_path(HeaderData->FileName, '/', '\\');

	if (GetFileAttributesEx(md5H->file, GetFileExInfoStandard, &stat))
	{
		FileTimeToLocalFileTime(&stat.ftLastWriteTime, &local);
		FileTimeToDosDateTime(&local, &date, &time);
		md5H->size = stat.nFileSizeLow;
		HeaderData->UnpSize		= stat.nFileSizeLow;
		HeaderData->FileTime	= (date << 16) | time;
		HeaderData->FileAttr	= stat.dwFileAttributes;
	}
	else
	{
		md5H->size = -1;
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

	checksum = (char *) malloc(64);
	memset(checksum, '\0', 64);

	if ((h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ+FILE_SHARE_WRITE+FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		return NULL;

	buf = (char *) malloc(MD5_BUFSIZE);
	MD5Init(&md5c);

	while (ReadFile(h, buf, MD5_BUFSIZE, &read, NULL))
	{
		MD5Update(&md5c, buf, read);
		aborted = progress(filename, read);
		if ((read == 0) || (aborted == 0))
			break;
	}

	MD5Final(keybuf, &md5c);
	CloseHandle(h);
	free(buf);

	for (i = 0, p = checksum; i < sizeof(keybuf); i++, p += 2)
		sprintf(p, "%02x", keybuf[i]);

	return checksum;
}


int __stdcall ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
	md5_handle *md5H = hArcData;
	char message[MAX_PATH], checksum[33], *sum;
	FILE *f;
	BOOL is_ok = FALSE;

	if (Operation != PK_SKIP)
	{
		if (
				(md5H->size != -1) &&
				((sum = md5sum(md5H->file)) != NULL)
			)
		{
			memset(checksum, '\0', sizeof(checksum));
			strncpy(checksum, sum, sizeof(checksum));
			free(sum);

			if (aborted == 0)
				return E_EABORTED;

			is_ok = (strncmp(md5H->sum, checksum, 32) == 0) ? TRUE : FALSE;
		}
		else
		{
			md5H->size = -1;
			MessageBox(NULL, md5H->file, "File not found!", MB_OK+MB_ICONWARNING);
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
				fprintf(f, "%s\n\n", md5H->file);
				fprintf(f, "expected:\t%s\n", md5H->sum);
				fprintf(f, "computed:\t%s\n", (md5H->size != -1) ? checksum : "????????????????????????????????");
				fprintf(f, "\nMD5 checksum %s!\n", is_ok ? "OK" : "FAILED");
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
	md5_handle *md5H = (md5_handle *) hArcData;

	md5_free(md5H->list);
	free(md5H);
	
	return 0;
}


int __stdcall PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags)
{
	FILE *f;
	char *p;
	char disk[MAX_PATH], arch[MAX_PATH];
	char *checksum;

	if (Flags == PK_PACK_MOVE_FILES)
		return E_NOT_SUPPORTED;

	strncpy(disk, SrcPath, MAX_PATH);
	for (p = PackedFile + strlen(PackedFile); p >= PackedFile; p--)
		if (*p == '\\')
		{
			p++;
			break;
		}
	strncat(disk, p, MAX_PATH);

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
				if ((checksum = md5sum(disk)) == NULL)
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
		"Provides MD5 checksum generator/checker\n"
		"from within Total Commander packer interface\n\n"

		"Copyright © 2003  Stanislaw Y. Pusep\n"
		"stanis@linuxmail.org\n"
		"http://sysdlabs.hypermart.net/proj/",
		
		"MD5 checksum wrapper " VERSION,
		
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
