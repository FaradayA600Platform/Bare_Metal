/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:FILECTRL.c                                                          *
* Description: file control routines                                       *
* Author: Ted Hsu 			                                               *
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../libc/include/ctype.h"
#include "SoFlexible.h"
//#include "flib.h"
#include "filectrl.h"
#include "fat.h"

#define TTY_FILE_NAME		":tt"

FileHandleStruct FileHandleTable[MAX_FILE_SUPPORT];

/* let drive from A to Z, and last one is standard output/input/error */
FileSysStruct *FileSysTable[MAX_FILE_SYS_SUPPORT + 1];

UINT32 CurrentDrive = 0;


/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////




void SplitPath(INT8 *path, INT8 *dir, INT8 *fname)
{
	INT8 *DirEnd, *DirStart;
	
	*dir = *fname = '\0';
	
	/* get dir */
	DirStart = DirEnd = path;
	while(( DirEnd = strchr(path, '\\')) != NULL )
		path = (DirEnd + 1);
		
	memcpy(dir, DirStart, path - DirStart);
	dir[path - DirStart] = '\0';
		
	/* get file name */
	while(*path != '\0')
		*fname++ = *path++;
	*fname = '\0';
}

void SplitDrivePath(INT8 *DrivePath, UINT32 *drive, INT8 *path)
{
 	*drive = CurrentDrive;
 	*path = '\0';
 	
 	if(DrivePath == NULL)
 		return;
 		
 	if(strlen(DrivePath) >= 2)
 	{
		/* get drive name */
		if(DrivePath[1] == ':')
		{
			if(isupper(DrivePath[0]))
				*drive = DrivePath[0] - 'A'; 
			else if(islower(DrivePath[0]))
				*drive = DrivePath[0] - 'a'; 
			
			DrivePath += 2;
		}
	}
	
	strcpy(path, DrivePath);		
}

int IsFullPath(INT8 *path)
{
	return (*path == '\\') ? TRUE : FALSE;
}


FileSysStruct *GetFileSystem(UINT32 Drive)
{
	if(Drive >= MAX_FILE_SYS_SUPPORT)
		return NULL;
		
	if	(FileSysTable[Drive]->init == FALSE)
	{
		if(!fLib_InitFATFileSystem(FileSysTable[Drive]))
			return NULL;
	}
			
	return FileSysTable[Drive];
}

int IsTTYFileOpen(UINT32 OpenMode)
{
	int i;
		
	/* find if this file is open or not */	
	for(i = 0; i < MAX_FILE_SUPPORT; i++)
		if((FileHandleTable[i].Attrib == OpenMode) && (strcmp(FileHandleTable[i].FileName, TTY_FILE_NAME) == 0))
			return i;
			
	return -1;	
}

int IsFileOpen(UINT32 Drive, INT8 *FileName)
{
	int i;
	FileSysStruct *FileSys;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];
	
	if(Drive >= MAX_FILE_SYS_SUPPORT)
		return NULL;
		
	if((FileSys = GetFileSystem(Drive)) == NULL)
		return FALSE;
		
	if(!IsFullPath(FileName))
	{
		GetCurrentDirectory(FileSys, Path);
		strcat(Path, FileName);
	}
	else
		strcpy(Path, FileName);
		
	/* find if this file is open or not */	
	for(i = 0; i < MAX_FILE_SUPPORT; i++)
		if((FileHandleTable[i].Drive == Drive) && (strcmp(FileHandleTable[i].FileName, Path) == 0))
			return TRUE;
			
	return FALSE;	
}

int CreateTTYFileHandle(UINT32 OpenMode)
{
	int i;
	
	for(i = 0; i < MAX_FILE_SUPPORT; i++)
	{
		if(FileHandleTable[i].FileName[0] == '\0')
		{
			FileHandleTable[i].FileSys = NULL;
			FileHandleTable[i].Drive = 0xFFFFFFFF;
			FileHandleTable[i].Attrib = OpenMode;
			FileHandleTable[i].IsTTY = TRUE;
			strcpy((INT8 *)FileHandleTable[i].FileName, ":tt"); 

			return i;
		}	
	}
	return -1;
}

int CreateFileHandle(FileSysStruct *FileSys, UINT32 Drive, INT8 *FileName, UINT32 OpenMode)
{
	int i;
	
	for(i = 0; i < MAX_FILE_SUPPORT; i++)
	{
		if(FileHandleTable[i].FileName[0] == '\0')
		{
			FileHandleTable[i].FileSys = FileSys;
			FileHandleTable[i].Drive = Drive;
			FileHandleTable[i].Attrib = OpenMode;
			FileHandleTable[i].IsTTY = FALSE;
			if(!IsFullPath(FileName))
			{
				GetCurrentDirectory(FileSys, FileHandleTable[i].FileName);
				strcat(FileHandleTable[i].FileName, FileName);
			}
			else
				strcpy(FileHandleTable[i].FileName, FileName);
			return i;
		}
	}	
	return -1;
}

void FreeFileHandle(int Handle)
{
	FileHandleTable[Handle].FileSys = NULL;
	FileHandleTable[Handle].Drive = 0xFFFFFFFF;
	FileHandleTable[Handle].Attrib = 0;
	FileHandleTable[Handle].IsTTY = FALSE;
	strcpy(FileHandleTable[Handle].FileName, "\0");			
}

int IsTTYFileName(INT8 *FileName)
{
	//if(!stricmp(FileName, ":tt"))
	if(!strcmp(FileName, ":tt"))
		return 1;
	return 0;
}

/////////////////////////////////////////////////////
//
//	file control routines 
//
/////////////////////////////////////////////////////


void fLib_InitFileSystem()
{
	UINT32 i;
	
	for(i = 0; i < MAX_FILE_SYS_SUPPORT; i++)
		FileSysTable[i] = NULL;
	
	for(i = 0; i < MAX_FILE_SUPPORT; i++)
	{
		FileHandleTable[i].FileSys = NULL;
		FileHandleTable[i].Drive = 0xFFFF;
		FileHandleTable[i].FileName[0] = '\0';
		FileHandleTable[i].IsTTY = FALSE;	
		
		FileHandleTable[i].StartCluster = 0;
		FileHandleTable[i].Attrib = 0;
		FileHandleTable[i].FileTotalSize = 0;
		
		FileHandleTable[i].CurrCluster = NULL;
		FileHandleTable[i].ClusterBuf = NULL;
		FileHandleTable[i].ClusterBufSize = 0;
		FileHandleTable[i].ClusterBufOffset = 0;
		
		FileHandleTable[i].Dirty = FALSE;
	}

#if 1	
	/* stdin */	
	strcpy((INT8 *)FileHandleTable[Monitor_STDIN].FileName, ":tt"); 
	FileHandleTable[Monitor_STDIN].Attrib = ATTRIB_READ_ONLY;
	FileHandleTable[Monitor_STDIN].IsTTY = TRUE;
	
	/* stdout */	
	strcpy((INT8 *)FileHandleTable[Monitor_STDOUT].FileName, ":tt"); 
	FileHandleTable[Monitor_STDOUT].Attrib = 0;
	FileHandleTable[Monitor_STDOUT].IsTTY = TRUE;
	
	/* stdout */	
	strcpy((INT8 *)FileHandleTable[Monitor_STDERR].FileName, ":tt"); 
	FileHandleTable[Monitor_STDOUT].Attrib = 0;
	FileHandleTable[Monitor_STDOUT].IsTTY = TRUE;
#endif
}

int fLib_InsertFileSystem(UINT32 Drive, FileSysStruct *FileSys)
{
	Drive -= 'A';
	
	if(Drive >= MAX_FILE_SYS_SUPPORT)
		return FALSE;
		
	if(FileSysTable[Drive] != NULL)
		return FALSE;
		
	FileSysTable[Drive] = FileSys;
	
	CurrentDrive = Drive;
	
	FileSys->init = FALSE;
	
	return TRUE;
}

void fLib_RemoveFileSystem(UINT32 Drive)
{
	Drive -= 'A';
	
	if(Drive >= MAX_FILE_SYS_SUPPORT)
		return;
		
	FileSysTable[Drive] = NULL;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/
/********************************************************************/
UINT32 fLib_DirDirectory(INT8 *Name, INT8 *Buf)
{
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];
	DirectoryStruct Directory;
	DirEntryStruct *DirEntry;
	INT8 MsgStr[0x100]; 
	
	SplitDrivePath(Name, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return FALSE;
		
	DisplayFATInfo(FileSysTable[Drive], Buf);
//	strcat(Buf, "\n");
	printf("\n");
	printf("fjafhjasdhffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
	if((DirEntry = OpenFATDirectory(FileSysTable[Drive], Path, &Directory)) != NULL)
	{
		//for large number files issue, message too much, buffer overflow 
//		sprintf(MsgStr, "Directory of %s, Cluster:%d, Sector:%d\n", Directory.DirName, Directory.Cluster, Directory.Sector);
//		strcat(Buf, MsgStr);
		printf("Directory of %s, Cluster:%d, Sector:%d\n", Directory.DirName, Directory.Cluster, Directory.Sector);		
		
		while(DirEntry != NULL)
		{
			DisplayDirEntryInfo(FileSysTable[Drive], DirEntry, MsgStr);			
						
//			strcat(Buf, MsgStr);
//			strcat(Buf, "\n");
			printf("%s\n",MsgStr);	
					
			DirEntry = GetNextDirEntry(DirEntry);
		}
	}
	else
	{
//		strcat(Buf, "Path Not Found!!!\n");		
		printf("Path Not Found!!!\n");		
	}
	
	CloseFATDirectory(FileSysTable[Drive], &Directory);

	return TRUE;
}


UINT32 fLib_DirDirectoryEx(INT8 *Name, INT8 *Buf, UINT32 u32BufSize, UINT32 u32DirEntryPtr, UINT32 *FileCnt)
{
	UINT32 Drive, u32BufOfSet = 0;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];
	static DirectoryStruct Directory;
	DirEntryStruct *DirEntry;
	UINT8* MsgStr;
	static UINT32 u32FileCnt = 0;
	
	MsgStr = (UINT8*) malloc(0x200);	
	SplitDrivePath(Name, &Drive, Path);

	if(GetFileSystem(Drive) == NULL)
		return 0xFFFFFFFF;

	if(u32DirEntryPtr == NULL)	
	{
		DisplayFATInfo(FileSysTable[Drive], Buf);
		strcat(Buf, "\n");
		
		if((DirEntry = OpenFATDirectory(FileSysTable[Drive], Path, &Directory)) != NULL)
		{
			sprintf(MsgStr, "Directory of %s, Cluster:%d, Sector:%d\n", 
					Directory.DirName, Directory.Cluster, Directory.Sector);
			strcat(Buf, MsgStr);
			u32BufOfSet = strlen(Buf);
			u32FileCnt = 0;
			
			while(DirEntry != NULL)
			{
				DisplayDirEntryInfo(FileSysTable[Drive], DirEntry, MsgStr);
				u32BufOfSet = u32BufOfSet + (strlen(MsgStr) + 1);
				if(u32BufOfSet > u32BufSize)
				{
					free(MsgStr);
					return DirEntry;
				}
				else
				{
					strcat(Buf, MsgStr);
					strcat(Buf, "\n");
					u32FileCnt++;
					FileCnt[0] = u32FileCnt;
					DirEntry = GetNextDirEntry(DirEntry);
				}
			}
		}
		else
		{
			strcat(Buf, "Path Not Found!!!\n");		
		}
	}
	else
	{
		DirEntry = u32DirEntryPtr;
		while(DirEntry != NULL)
		{
			DisplayDirEntryInfo(FileSysTable[Drive], DirEntry, MsgStr);
			u32BufOfSet = u32BufOfSet + (strlen(MsgStr) + 1);
			if(u32BufOfSet > u32BufSize)
			{
				free(MsgStr);
				return DirEntry;
			}
			else
			{
				strcat(Buf, MsgStr);
				strcat(Buf, "\n");
				u32FileCnt++;
				FileCnt[0] = u32FileCnt;
				DirEntry = GetNextDirEntry(DirEntry);
			}
		}
	}
	CloseFATDirectory(FileSysTable[Drive], &Directory);	
	free(MsgStr);
	return DirEntry;
}

UINT32 fLib_ChangeDirectory(INT8 *Name)
{
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];

	SplitDrivePath(Name, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return FALSE;
		
		
	if(ChangeFATDirectory(FileSysTable[Drive], Path))
	{
		CurrentDrive = Drive;
		return TRUE;
	}
	return FALSE;
}

UINT32 fLib_MakeDirectory(INT8 *Name)
{
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];

	SplitDrivePath(Name, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return FALSE;
		
	return MakeFATDirectory(FileSysTable[Drive], Path);	
}

UINT32 fLib_MakeEmptyFile(INT8 *Name)
{
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];

	SplitDrivePath(Name, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return FALSE;
		
	return CreateFATEmptyFile(FileSysTable[Drive], Path);	
}

UINT32 fLib_DeleteDirectory(INT8 *Name)
{
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];

	SplitDrivePath(Name, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return FALSE;
		
	return DeleteFATDirectory(FileSysTable[Drive], Path);	
}

/********************************************************************/
/********************************************************************/
/********************************************************************/
/********************************************************************/
UINT32 fLib_OpenFile(INT8 *FileName, UINT32 OpenMode, UINT32 FileNameLen)
{
	int FreeHandleIndex, Index;
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];

	/* if it is TTY file */
	if(IsTTYFileName(FileName))
	{
		if((Index = IsTTYFileOpen(OpenMode)) != -1)
			return Index;
		
		if((FreeHandleIndex = CreateTTYFileHandle(OpenMode)) == -1)
			return 0xFFFFFFFF;
			
		return FreeHandleIndex;
	}
	
	/* now, it is NOT TTY file */
	SplitDrivePath(FileName, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return 0xFFFFFFFF;
			
	if(IsFileOpen(Drive, Path))
		return 0xFFFFFFFF;

	if((FreeHandleIndex = CreateFileHandle(FileSysTable[Drive], Drive, Path, OpenMode)) == -1)
		return 0xFFFFFFFF;
		
	FileHandleTable[FreeHandleIndex].Mode = OpenMode;
	
	/* read : Mode 0 = "r", Mode 1 = "rb" Mode 2 = "r+", Mode 3 = "r+b" */	
	if((OpenMode == 0) || (OpenMode == 1) || (OpenMode == 2) || (OpenMode == 3) || 
		(OpenMode == 0x10) || (OpenMode == 0x12))
	{
		if(OpenFATFile(FileHandleTable[FreeHandleIndex].FileSys, &FileHandleTable[FreeHandleIndex], Path))
			return FreeHandleIndex;
	}
	/* write : Mode 4 = "w", Mode 5 = "wb" Mode 6 = "w+", Mode 7 = "w+b" */			
	if((OpenMode == 4) || (OpenMode == 5) || (OpenMode == 6) || (OpenMode == 7) ||
	(OpenMode == 0x14) || (OpenMode == 0x16))
	{
		if(OpenFATEmptyFile(FileHandleTable[FreeHandleIndex].FileSys, &FileHandleTable[FreeHandleIndex], Path))
			return FreeHandleIndex;
	}
	/* append : Mode 8 = "a", Mode 9 = "ab" Mode 10 = "a+", Mode 11 = "a+b" */
	if((OpenMode == 8) || (OpenMode == 9) || (OpenMode == 0xA) || (OpenMode == 0xB) ||
	(OpenMode == 0x18) || (OpenMode == 0x1A))
	{
		if(OpenFATFile(FileHandleTable[FreeHandleIndex].FileSys, &FileHandleTable[FreeHandleIndex], Path))
			return FreeHandleIndex;
		if(OpenFATEmptyFile(FileHandleTable[FreeHandleIndex].FileSys, &FileHandleTable[FreeHandleIndex], Path))
			return FreeHandleIndex;
	}
		
	FreeFileHandle(FreeHandleIndex);
	return 0xFFFFFFFF;
}

UINT32 fLib_CloseFile(UINT32 handle)
{
	if(FileHandleTable[handle].IsTTY) 
	{
//		FileHandleTable[handle].Attrib = 0;
		return 0;
	}
	
	if((FileHandleTable[handle].Mode == 0) || (FileHandleTable[handle].Mode == 1))
	{
		FreeFileHandle(handle);
		return 0;	
	}
		
	if(CloseFATFile(FileHandleTable[handle].FileSys, &FileHandleTable[handle]))
	{
		FreeFileHandle(handle);
		return 0;
	}
	else
	{
		FreeFileHandle(handle);
		return 0xFFFFFFFF;
	}
}

UINT32 fLib_DeleteFile(INT8 *FileName, UINT32 FileNameLen)
{
	UINT32 Drive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE];

	SplitDrivePath(FileName, &Drive, Path);
	
	if(GetFileSystem(Drive) == NULL)
		return 0xFFFFFFFF;
	
	if(IsFileOpen(Drive, Path))
		return 0xFFFFFFFF;
			
	if(DeleteFATFile(FileSysTable[Drive], Path))
		return 0;
		
	return 0xFFFFFFFF;
}

UINT32 fLib_RenameFile(INT8 *OldFileName, UINT32 OldFileNameLen, INT8 *NewFileName, UINT32 NewFileNameLen)
{	
	UINT32 Drive, NewDrive;
	INT8 Path[MAX_DIRECTORY_PATH_SIZE], NewPath[MAX_DIRECTORY_PATH_SIZE];

	SplitDrivePath(OldFileName, &Drive, Path);
	SplitDrivePath(NewFileName, &NewDrive, NewPath);
	
	if(GetFileSystem(Drive) == NULL)
		return 0xFFFFFFFF;
		
	if(Drive != NewDrive)
		return 0xFFFFFFFF;
		
	if(IsFileOpen(Drive, Path))
		return 0xFFFFFFFF;
			
	if(IsFileOpen(NewDrive, NewPath))
		return 0xFFFFFFFF;
		
//	if(IsFullPath(NewFileName))
//		return 0xFFFFFFFF;
		
	if(RenameFATDirectoryFile(FileSysTable[Drive], Path, NewPath))
		return 0;
		
	return 0xFFFFFFFF;
}

UINT32 fLib_FileLen(UINT32 handle)
{
	return FileHandleTable[handle].FileTotalSize;
}

UINT32 fLib_WriteFile(UINT32 handle, UINT8 *Buf, UINT32 Len)
{
	int i;
	
	if(FileHandleTable[handle].IsTTY) 
	{
	    for(i = 0; i < Len ; i++, Buf++)	
    		fLib_DebugPrintChar(*Buf);
 
		return 0;
	}
	else
		return WriteFATFile(FileHandleTable[handle].FileSys, &FileHandleTable[handle], Len, Buf);
}

UINT32 fLib_ReadFile(UINT32 handle, UINT8 *Buf, UINT32 Len)
{	
	if(FileHandleTable[handle].IsTTY) 
	{
		return (Len - fLib_DebugGetUserCommand(Buf, Len));
	}
	else
		return ReadFATFile(FileHandleTable[handle].FileSys, &FileHandleTable[handle], Len, Buf);
}

UINT32 fLib_SeekFile(UINT32 handle, UINT32 Pos)
{
	if(SeekFATFile(FileHandleTable[handle].FileSys, &FileHandleTable[handle], Pos))
		return 0;
		
	return 0xFFFFFFFF;
}

UINT32 fLib_IsTTYFile(UINT32 handle)
{
	return FileHandleTable[handle].IsTTY;
}

UINT32 SerachAvailableDrive(void){
	UINT32 Drive;
	
	for(Drive=2;Drive<MAX_FILE_SYS_SUPPORT;Drive++){
		if(FileSysTable[Drive] == NULL){
			return (Drive+'A');
			
		}
	}
	return 0;
}
