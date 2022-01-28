/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:fat.c                                                               *
* Description: FAT relative routines                                       *
* Author: Ted Hsu 			                                               *
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SoFlexible.h"
//#include "filectrl.h"
#include "fat.h"


#define SET_LONG_FILE_NAME(ptr, i, name)	ptr[i] = *name;if(*name == '\0') break; else name++;
#define GET_LONG_FILE_NAME(ptr, i, name)	*name = ptr[i];if(*name == '\0') break; else name++;
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))



void fLib_ParseFATPartitionTable(PartitionTableStruct *PartTable, UINT8 *Buf)
{
}

void fLib_MakeFATPartitionTable(PartitionTableStruct *PartTable, UINT8 *Buf, int PTableID)
{
	memcpy(Buf + PARTITION_TABLE0_OFFSET + (PTableID * sizeof(PartitionTableStruct)),
		PartTable, sizeof(PartitionTableStruct)); 
}

void fLib_ParseFATPartitionBootSector(ExtendFDCDescriptorStruct *FDCDesc, UINT8 *Buf)
{
}

void fLib_MakeFATPartitionBootSector(ExtendFDCDescriptorStruct *FDCDesc, UINT8 *Buf)
{
	memcpy(Buf, FDCDesc->JumpCommand, 3);
	memcpy(Buf + 3, FDCDesc->SystemIdentifier, 8);
	memcpy(Buf + 11, &FDCDesc->SectorSize, 2);
	Buf[13] = FDCDesc->SectorPerCluster;
	memcpy(Buf + 14, &FDCDesc->ReservedSectorCount, 2);
	Buf[16] = FDCDesc->FATNumber;
	memcpy(Buf + 17, &FDCDesc->RootDirEntryNumber, 2);
	memcpy(Buf + 19, &FDCDesc->TotalSector, 2);
	Buf[21] = FDCDesc->MediumIdentifier;
	memcpy(Buf + 22, &FDCDesc->SectorPerFAT, 2);
	memcpy(Buf + 24, &FDCDesc->SectorPerTrack, 2);
	memcpy(Buf + 26, &FDCDesc->SlideNumber, 2);
	memcpy(Buf + 28, &FDCDesc->HiddenSectorNumber, 4);
	memcpy(Buf + 32, &FDCDesc->TotalSector2, 4);
	Buf[36] = FDCDesc->PhyDiskNumber;
	Buf[37] = FDCDesc->Reserved0;
	Buf[38] = FDCDesc->ExtendSignature;
	memcpy(Buf + 39, &FDCDesc->VolumeID, 4);
	memcpy(Buf + 43, FDCDesc->VolumeLable, 11);
	memcpy(Buf + 54, FDCDesc->FileSystemType, 8);
	memcpy(Buf + 510, &FDCDesc->SignatureWord, 2);

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void fLib_InitFATFileSystemRoutines(FileSysStruct *FileSys, void *CardPtr, INIT_CARD InitCardPtr, READ_SECTOR ReadSectorPtr, WRITE_SECTOR WriteSectorPtr, ERASE_SECTOR EraseSectorPtr)
{
	FileSys->CardPtr = CardPtr;		
	FileSys->InitCard = InitCardPtr;
	FileSys->ReadSector = ReadSectorPtr;
	FileSys->WriteSector = WriteSectorPtr;
	FileSys->EraseSector = EraseSectorPtr;		
}

int fLib_InitFATFileSystem(FileSysStruct *FileSys)
{

	if(!FileSys->InitCard(FileSys->CardPtr))
		return FALSE;
	//printf("SDC INIT OK\n");
#if 1
    // For NANDC test only
    // Skip ReadFATPartitionTable and ReadFATPartitionBootSector parts
	FileSys->init = TRUE;
	return TRUE;
#endif
	
	/* make current directory is ROOT */
	strcpy(FileSys->CurrDir.DirName, "\0");
	FileSys->CurrDir.Cluster = 0;
	
	FileSys->FAT.FATTable = NULL;
	FileSys->CurrDir.Entry = NULL;
	
	/**********************************************/
	/* read partition table form SD memory card */
	/**********************************************/
	if(!ReadFATPartitionTable(FileSys))
		return FALSE;

	/**********************************************/
	/* read partition boot sector */
	/**********************************************/
	if(!ReadFATPartitionBootSector(FileSys))
		return FALSE;
		
	FileSys->ClusterSize = FileSys->FDCDesc.SectorSize * FileSys->FDCDesc.SectorPerCluster;

	/**********************************************/
	/* read FAT */
	/**********************************************/
	FileSys->FAT.FATTable = (UINT8 *)malloc(FileSys->FDCDesc.SectorSize * FileSys->FDCDesc.SectorPerFAT);		

	if(!ReadFAT(FileSys))
		return FALSE;
		
	/**********************************************/
	/* read root directory */
	/**********************************************/	
	FileSys->RootDirCluster = 0;		
	FileSys->RootDirSector = FileSys->FAT.FATSector + (FileSys->FDCDesc.FATNumber * FileSys->FDCDesc.SectorPerFAT);		
	FileSys->RootDirSectorCount = (FileSys->FDCDesc.RootDirEntryNumber * sizeof(DirEntryStruct)) / FileSys->FDCDesc.SectorSize;
	FileSys->FirstClusterSector = FileSys->RootDirSector + FileSys->RootDirSectorCount;
	
	if(!ReadFATDirectoryEntry(FileSys, &FileSys->CurrDir, FileSys->RootDirCluster, FileSys->RootDirSectorCount, (INT8 *)"\\"))
		return FALSE;
	
	FileSys->init = TRUE;
	
	return TRUE;
}

int fLib_FreeFATFileSystem(FileSysStruct *FileSys)
{
	if(FileSys->FAT.FATTable != NULL)
		free(FileSys->FAT.FATTable);
	FileSys->FAT.FATTable = NULL;
	
	FreeFATDirectoryEntry(&FileSys->CurrDir);
	return TRUE;
}


/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////


INT8 *GetDirectory(INT8 *str, INT8 *Dir)
{
	INT8 *pstr;

	while(1)
	{
		pstr = (INT8 *)strchr(str,'\\');
		
		if(pstr == NULL)
		{
			strcpy(Dir, str);
			return pstr;
		}
		else
		{
			if(pstr != str)
			{
				memcpy(Dir, str, pstr - str);
				Dir[pstr - str] = '\0';
				return pstr + 1;
			}
			str = pstr + 1;
		}
	}
}

void UpdateDirectoryName(INT8 *FullDir, INT8 *Dir)
{
	INT8 *pstr, *str;
	
	if(!strcmp(Dir, "."))
		return;
			
	if(!strcmp(Dir, ".."))
	{
		str = FullDir;
		while(1)
		{
			if((pstr = (INT8 *)strchr(str,'\\')) == NULL)
				break;
			if(*(pstr+1) == '\0')
				break;
			str = pstr + 1;
		}
		*str = '\0';
		return;		
	}
	strcat(FullDir, Dir);
	strcat(FullDir, "\\");
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
int ReadFATPartitionTable(FileSysStruct *FileSys)
{
	UINT8 TempBuf[512];
	extern INT32 fLib_printf(const char *f, ...);
	
	if(!FileSys->ReadSector(FileSys->CardPtr, PARTITION_AREA_SECTOR, 1, 512, TempBuf))
		return FALSE;
		
	memcpy(&(FileSys->PartitionTable), TempBuf + PARTITION_TABLE0_OFFSET, sizeof(PartitionTableStruct));
	
	if ((FileSys->PartitionTable.BootIndicator != 0) && (FileSys->PartitionTable.BootIndicator != 0x80)) 
	{
		FileSys->PartitionTable.RelativeSector = 0;
		fLib_printf("Partition Table not found\n");//modify by KMC in circulation procrss
		return TRUE;
	}
	 
	if ((FileSys->PartitionTable.SystemID != 0x1) && (FileSys->PartitionTable.SystemID != 0x04) && (FileSys->PartitionTable.SystemID != 0x06))
	{
		FileSys->PartitionTable.RelativeSector = 0;
		fLib_printf("Partition Table not found\n");//modify by KMC in circulation procrss
	}
			
	return TRUE;
} 

int ReadFATPartitionBootSector(FileSysStruct *FileSys)
{
	UINT8 TempBuf[512];
	ExtendFDCDescriptorStruct *FDCDesc;
	UINT32 SectorNumber;
	extern INT32 fLib_printf(const char *f, ...);
	
	SectorNumber = FileSys->PartitionTable.RelativeSector;
	
	if(!FileSys->ReadSector(FileSys->CardPtr, SectorNumber, 1, 512, TempBuf))
		return FALSE;
		
	/* because some fields of this table are not halfword/word alignment */
	FDCDesc = &(FileSys->FDCDesc);
	memcpy(FDCDesc->JumpCommand, TempBuf, 3);
	//add by silas for checking the Boot record
	if (FDCDesc->JumpCommand[0] != 0xE9)
	 	if((FDCDesc->JumpCommand[0] != 0xEB)&& (FDCDesc->JumpCommand[2] != 0x90)) 
		{
			fLib_printf("Master Boot Record not found\n");//modify by KMC in circulation procrss
			return FALSE;
		}

	memcpy(FDCDesc->SystemIdentifier, TempBuf + 3, 8);
	memcpy(&FDCDesc->SectorSize, TempBuf + 11, 2);
	FDCDesc->SectorPerCluster = TempBuf[13];
	memcpy(&FDCDesc->ReservedSectorCount, TempBuf + 14, 2);
	FDCDesc->FATNumber = TempBuf[16];
	memcpy(&FDCDesc->RootDirEntryNumber, TempBuf + 17, 2);
	memcpy(&FDCDesc->TotalSector, TempBuf + 19, 2);
	FDCDesc->MediumIdentifier = TempBuf[21];
	memcpy(&FDCDesc->SectorPerFAT, TempBuf + 22, 2);
	memcpy(&FDCDesc->SectorPerTrack, TempBuf + 24, 2);
	memcpy(&FDCDesc->SlideNumber, TempBuf + 26, 2);
	memcpy(&FDCDesc->HiddenSectorNumber, TempBuf + 28, 4);
	memcpy(&FDCDesc->TotalSector2, TempBuf + 32, 4);
	FDCDesc->PhyDiskNumber = TempBuf[36];
	FDCDesc->Reserved0 = TempBuf[37];
	FDCDesc->ExtendSignature = TempBuf[38];
	memcpy(&FDCDesc->VolumeID, TempBuf + 39, 4);
	memcpy(FDCDesc->VolumeLable, TempBuf + 43, 11);
	memcpy(FDCDesc->FileSystemType, TempBuf + 54, 8);
	memcpy(&FDCDesc->SignatureWord, TempBuf + 510, 2);
	//add by silas for checking the Boot record
 	if(FDCDesc->SignatureWord != 0xAA55 ) 
	{
		fLib_printf("Master Boot Record not found\n");//modify by KMC in circulation procrss
		return FALSE;
	}
	
	return TRUE;
}

int ReadFATDirectoryEntry(FileSysStruct *FileSys, DirectoryStruct *Directory, UINT32 Cluster, UINT32 SectorCount, INT8 *DirName)
{
	UINT8 *Buf;
	UINT32 Sector;
	
	/* alloc temp buffer, the buffer size should include one dump directory entry. */
	Buf = (UINT8 *)malloc((SectorCount * FileSys->FDCDesc.SectorSize) + sizeof(DirEntryStruct));
	/* let dump directory is ZERO */
//	memset(Buf + (SectorCount * FileSys->FDCDesc.SectorSize), 0, sizeof(DirEntryStruct)); 
	
	Directory->Entry = (DirEntryStruct *)Buf;
			
	Sector = GetFATDirSectorNumber(FileSys, Cluster);

	if(!FileSys->ReadSector(FileSys->CardPtr, Sector, SectorCount, FileSys->FDCDesc.SectorSize, (UINT8 *)Directory->Entry))
	{
		free(Directory->Entry);
		Directory->Entry = NULL;
		return FALSE;
	}
	Directory->Cluster = Cluster;
	Directory->Sector = Sector;
	Directory->SectorCount = SectorCount;
	
	strcpy(Directory->DirName, DirName);
	return TRUE;
}

int WriteFATDirectoryEntry(FileSysStruct *FileSys, DirectoryStruct *Directory)
{
	if(!FileSys->WriteSector(FileSys->CardPtr, Directory->Sector, Directory->SectorCount, FileSys->FDCDesc.SectorSize, (UINT8 *)Directory->Entry))
		return FALSE;		
		
	return TRUE;
}

void FreeFATDirectoryEntry(DirectoryStruct *Directory)
{
	if(Directory->Entry != NULL)
		free(Directory->Entry);
	Directory->Entry = NULL;
	Directory->DirName[0] = '\0';
}

int ReadFAT(FileSysStruct *FileSys)
{
#if (LBA_MODE)
	FileSys->FAT.FATSector =  FileSys->PartitionTable.RelativeSector + FileSys->FDCDesc.ReservedSectorCount;
#else
	FileSys->FAT.FATSector = (FileSys->PartitionTable.StartingCylinder * FileSys->FDCDesc.SlideNumber * FileSys->FDCDesc.SectorPerTrack) +
		(FileSys->PartitionTable.StartingHead * FileSys->FDCDesc.SectorPerTrack) + FileSys->PartitionTable.StartingSector;
#endif		
		
	if(!FileSys->ReadSector(FileSys->CardPtr, FileSys->FAT.FATSector, FileSys->FDCDesc.SectorPerFAT, FileSys->FDCDesc.SectorSize, FileSys->FAT.FATTable))
		return FALSE;
		
	if((FileSys->FAT.FATType = GetFATType(FileSys->FDCDesc.FileSystemType)) == FAT_TYPE_NOT_SUPPORT)
		return FALSE;
	
	FileSys->FAT.MaxClusterNumber = GetFATMaxClusterNumber(FileSys->FDCDesc.SectorPerFAT * FileSys->FDCDesc.SectorSize, FileSys->FAT.FATType);
	return TRUE;
}

int WriteFAT(FileSysStruct *FileSys)
{
	if(FileSys->WriteSector(FileSys->CardPtr, FileSys->FAT.FATSector, FileSys->FDCDesc.SectorPerFAT, FileSys->FDCDesc.SectorSize, FileSys->FAT.FATTable))
		return TRUE;
	
	/* if write FAT fail, read old FAT back */
	ReadFAT(FileSys);
	return FALSE;
}


UINT32 GetFATDirSectorNumber(FileSysStruct *FileSys, int ClusterNumber)
{
	if(ClusterNumber < 2)
		return FileSys->RootDirSector;
	else
		return FileSys->FirstClusterSector + ((ClusterNumber - 2) * FileSys->FDCDesc.SectorPerCluster);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
int GetFATType(UINT8 *FileTypeStr)
{
	if(!strncmp(FileTypeStr, "FAT12", 5))
	{
		return FAT_TYPE_FAT12;
	}
	if(!strncmp(FileTypeStr, "FAT16", 5))
	{
		return FAT_TYPE_FAT16;
	}
#if 0		
	if(!strncmp(FileTypeStr, "FAT32", 5))
		return FAT_TYPE_FAT32;
#endif
	return FAT_TYPE_NOT_SUPPORT;		
}

UINT32 GetFATMaxClusterNumber(UINT32 FATSize, int FATType)
{
	UINT32 MaxClusterNumber;
	
	switch(FATType)
	{
		case FAT_TYPE_FAT12:
			/* MaxClusterNumber = FATSize / 1.5 */
			MaxClusterNumber = (FATSize * 2)  / 3;
			break;
		case FAT_TYPE_FAT16:			
			/* MaxClusterNumber = FATSize / 2 */
			MaxClusterNumber = FATSize / 2 ;
			break;
		default:		
			MaxClusterNumber = 0 ;
			break;
	}
	return MaxClusterNumber;
}

int GetClusterStatus(FATStruct *FAT, UINT32 EntryValue)
{
	switch(FAT->FATType)
	{
		case FAT_TYPE_FAT12:
			if(EntryValue == 0x0000)
				return FAT_CLUSTER_NOT_USED;
			if((EntryValue >= 2) && (EntryValue <= FAT->MaxClusterNumber))
				return FAT_CLUSTER_ALLOCATED;
			if((EntryValue >= 0xFF8) && (EntryValue <= 0xFFF))
				return FAT_CLUSTER_FINAL;
			if(EntryValue == 0xFF7)
				return FAT_CLUSTER_DEFECTIVE;
			break;
		case FAT_TYPE_FAT16:
			if(EntryValue == 0x0000)
				return FAT_CLUSTER_NOT_USED;
			if((EntryValue >= 2) && (EntryValue <= FAT->MaxClusterNumber))
				return FAT_CLUSTER_ALLOCATED;
			if((EntryValue >= 0xFFF8) && (EntryValue <= 0xFFFF))
				return FAT_CLUSTER_FINAL;
			if(EntryValue == 0xFFF7)
				return FAT_CLUSTER_DEFECTIVE;
			break;
	}
	return FAT_CLUSTER_RESERVED;
}

UINT32 GetFATNextCluster(FATStruct *FAT, UINT32 NCluster)
{
	UINT16 FATOffset;
	UINT32 NextCluster;
	
	switch(FAT->FATType)
	{
		case FAT_TYPE_FAT12:
			FATOffset = NCluster + (NCluster / 2);
			if(NCluster & 0x01)
			{
				NextCluster = FAT->FATTable[FATOffset] >> 4;
				NextCluster += FAT->FATTable[FATOffset + 1] << 4;
			}
			else
			{
				NextCluster = FAT->FATTable[FATOffset];
				NextCluster += (FAT->FATTable[FATOffset + 1] & 0x0F) << 8;
			}
			break;
		case FAT_TYPE_FAT16:			
			NextCluster = ((UINT16 *)(FAT->FATTable))[NCluster];
			break;
		default:		
			NextCluster = 0;
			break;
	}	
	return NextCluster;
}

void SetFATNextCluster(FATStruct *FAT, UINT32 NCluster, UINT16 Value)
{
	UINT16 FATOffset;
	
	switch(FAT->FATType)
	{
		case FAT_TYPE_FAT12:
			FATOffset = NCluster + (NCluster / 2);
			if(NCluster & 0x01)
			{
				FAT->FATTable[FATOffset] &= 0x0F;				
				FAT->FATTable[FATOffset] += (UINT8)((Value << 4) & 0x00F0);
				
				FAT->FATTable[FATOffset + 1] = (UINT8)((Value >> 4) & 0x00FF);
			}
			else
			{
				FAT->FATTable[FATOffset] = (UINT8)Value;
				FAT->FATTable[FATOffset + 1] &= 0xF0;
				FAT->FATTable[FATOffset + 1] += (UINT8)((Value >> 8) & 0x000F);
			}
			break;
		case FAT_TYPE_FAT16:			
			((UINT16 *)(FAT->FATTable))[NCluster] = Value;
			break;
	}	
}

int FreeFATCluster(FATStruct *FAT, UINT32 NCluster)
{
	UINT32 NextCluster, ClusterStatus;
	
	while(1)
	{
		NextCluster = GetFATNextCluster(FAT, NCluster);	
		
		ClusterStatus =	GetClusterStatus(FAT, NextCluster);
		
		switch(ClusterStatus)
		{
			case FAT_CLUSTER_NOT_USED:
				return TRUE;
			case FAT_CLUSTER_ALLOCATED:
				SetFATNextCluster(FAT, NCluster, 0x0000);
				NCluster = NextCluster;
				break;
			case FAT_CLUSTER_FINAL:
				SetFATNextCluster(FAT, NCluster, 0x0000);
				return TRUE;
			case FAT_CLUSTER_RESERVED:
			case FAT_CLUSTER_DEFECTIVE:
				return FALSE;
		}
	}
	return FALSE;
}

UINT32 GetFATFirstFreeCluster(FATStruct *FAT)
{
	UINT32 EntryValue, ClusterNum = 2;
	
	while((EntryValue = GetFATNextCluster(FAT, ClusterNum)) != 0x0000)
		ClusterNum++;
	
	SetFATNextCluster(FAT, ClusterNum, 0xFFFF);
	return ClusterNum;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
DirEntryStruct *GetFirstDirEntry(DirEntryStruct *DirEntry)
{
	if(DirEntry->Attrib & ATTRIB_VOLUME_ID)
		return GetNextDirEntry(DirEntry);

	switch(DirEntry->Name[0])
	{
		case 0xE5:
			return GetNextDirEntry(DirEntry);
		case 0x00:
		case 0xff:
			return NULL;
		default:
			return DirEntry;
	}
}

DirEntryStruct *GetNextDirEntry(DirEntryStruct *DirEntry)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
		
	DirEntry++;
		
	while(1)
	{
		if(DirEntry->Attrib & ATTRIB_VOLUME_ID)
			DirEntry++;
		else
		{	
			switch(DirEntry->Name[0])
			{
				case 0xE5:
					DirEntry++;
					break;
				case 0x00:
				case 0xff:
					return NULL;
				default:
					return DirEntry;
			}
		}
	}
}

void FreeDirEntry(DirEntryStruct *DirEntry)
{
	DirEntryStruct *FirstDirEntry;
	
	FirstDirEntry = DirEntry;
	
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
				
	while(FirstDirEntry != DirEntry)
	{
		DirEntry->Name[0] = 0xE5;
		DirEntry--;
	}
	DirEntry->Name[0] = 0xE5;
}

DirEntryStruct *GetFirstFreeDirEntry(DirEntryStruct *DirEntry, int EntryCount, UINT8 Attrib)
{
	int FreeCount = 0, i;
	DirEntryStruct *FreeEntry;
	
	while(1)
	{
		if((DirEntry->Name[0]) == 0xE5)
		{
			if(FreeCount == 0)
				FreeEntry = DirEntry;
			FreeCount++;
			if(FreeCount == EntryCount)
				break;
		}
		else if((DirEntry->Name[0]) == 0x00)
		{
			FreeEntry = DirEntry;
			break;
		}
		else
		{
			FreeCount = 0;
			DirEntry++;
		}
	}
	
	DirEntry = FreeEntry;
	
	/* if EntryCount > 1, these entries are used to save long file name. */
	for(i = 0; i < (EntryCount - 1); i++, DirEntry++)
		DirEntry->Attrib = ATTRIB_LONG_NAME;

	/* set attrib of the first entry to desired Attrib */
	DirEntry->Attrib = Attrib;

	return FreeEntry;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void GetDirEntryInfo(DirEntryStruct *DirEntry, DirEntryInfoStruct *DirEntryInfo)
{
	int i;
	INT8 *FileName, *LongFileName, *Ptr;
	DirEntryStruct *FirstDirEntry;
	
	/* store original directory entry to FirstDirEntry */
	FirstDirEntry = DirEntry;
	
	/* let DirEntry is First entry */
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	
	/* get file name */
	FileName = DirEntryInfo->Name;	
	for(i = 0; i < 8; i++, FileName++)
	{
		if(DirEntry->Name[i] == ' ')
			break;
		*FileName = DirEntry->Name[i];
	}
	
	for(i = 0; i < 3; i++, FileName++)
	{
		if(DirEntry->Extension[i] == ' ')
			break;
		if(i == 0)
		{
			*FileName = '.';
			FileName++;
		}
		*FileName = DirEntry->Extension[i];
	}	
	*FileName = '\0';

	DirEntryInfo->Attrib = DirEntry->Attrib;
	DirEntryInfo->StartCluster = DirEntry->StartCluster;
	DirEntryInfo->Length = DirEntry->FileLength;
	DirEntryInfo->Date = *(DirEntryDateStruct *)&(DirEntry->Date);
	DirEntryInfo->Time = *(DirEntryTimeStruct *)&(DirEntry->Time);
	
	/* Get Long File Name */
	LongFileName = DirEntryInfo->LongName;
	if(FirstDirEntry == DirEntry)
	{
		strcpy(LongFileName, DirEntryInfo->Name);
	}
	else
	{		
		while(DirEntry != FirstDirEntry)
		{
			DirEntry--;
			Ptr = (INT8 *)DirEntry;
			/* so for, we only support one-byte character */		
	 		GET_LONG_FILE_NAME(Ptr, 1, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 3, LongFileName);
	 		GET_LONG_FILE_NAME(Ptr, 5, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 7, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 9, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 14, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 16, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 18, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 20, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 22, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 24, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 28, LongFileName);
 			GET_LONG_FILE_NAME(Ptr, 30, LongFileName);
		}
		*LongFileName = '\0';
	}
}

void SetDirEntryInfo(DirEntryStruct *DirEntry, DirEntryInfoStruct *DirEntryInfo)
{
	INT8 *LongFileName, *Ptr;
	DirEntryStruct *FirstDirEntry;
	
	/* store original directory entry to FirstDirEntry */
	FirstDirEntry = DirEntry;
		
	/* let DirEntry is First entry */
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	
	/* set file name */
	SetDirEntryFileName(DirEntry, DirEntryInfo->Name);

	DirEntry->Attrib = DirEntryInfo->Attrib;
	DirEntry->StartCluster = DirEntryInfo->StartCluster;
	DirEntry->FileLength = DirEntryInfo->Length;
	*(DirEntryDateStruct *)&(DirEntry->Date) = DirEntryInfo->Date;
	*(DirEntryTimeStruct *)&(DirEntry->Time) = DirEntryInfo->Time;
	
	/* Set Long File Name */
	LongFileName = DirEntryInfo->LongName;
	while(DirEntry != FirstDirEntry)
	{
		DirEntry--;
		Ptr = (INT8 *)DirEntry;
		/* so for, we only support one-byte character */		
 		SET_LONG_FILE_NAME(Ptr, 1, LongFileName);
		SET_LONG_FILE_NAME(Ptr, 3, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 5, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 7, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 9, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 14, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 16, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 18, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 20, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 22, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 24, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 28, LongFileName);
 		SET_LONG_FILE_NAME(Ptr, 30, LongFileName);
 	}
}

void GetDirEntryFileName(DirEntryStruct *DirEntry, INT8 *FileName)
{
	int i ;
	
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	
	for(i = 0; i < 8; i++, FileName++)
	{
		if(DirEntry->Name[i] == ' ')
			break;
		*FileName = DirEntry->Name[i];
	}
	
	for(i = 0; i < 3; i++, FileName++)
	{
		if(DirEntry->Extension[i] == ' ')
			break;
		if(i == 0)
		{
			*FileName = '.';
			FileName++;
		}
		*FileName = DirEntry->Extension[i];
	}
	
	*FileName = '\0';
}

void SetDirEntryFileName(DirEntryStruct *DirEntry, INT8 *FileName)
{
	int i ;
	
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;

	if(!strcmp(FileName, "."))
	{
		memcpy(DirEntry->Name, ".       ", 8);
		memcpy(DirEntry->Extension, "   ", 3);
		return;
	}
	
	if(!strcmp(FileName, ".."))
	{
		memcpy(DirEntry->Name, "..      ", 8);
		memcpy(DirEntry->Extension, "   ", 3);
		return;
	}
	
		
	for(i = 0; i < 8; i++)
	{
		if((*FileName == '.') || (*FileName == '\0'))
		{
			DirEntry->Name[i] = ' ';
		}
		else
		{
			DirEntry->Name[i] = *FileName;
			FileName++;
		}
	}
	
	/* if *FileName is not '\0', it must be '.'. we must omit this '.' */
	if(*FileName != '\0')
		FileName++;
		
	for(i = 0; i < 3; i++)
	{
		if(*FileName == '\0')
		{
			DirEntry->Extension[i] = ' ';
		}
		else
		{
			DirEntry->Extension[i] = *FileName;
			FileName++;
		}
	}
}

UINT8 GetDirEntryAttrib(DirEntryStruct *DirEntry)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	return 	DirEntry->Attrib;
}

void SetDirEntryAttrib(DirEntryStruct *DirEntry, UINT8 Attrib)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	DirEntry->Attrib = Attrib;
}

UINT16 GetDirEntryStartCluster(DirEntryStruct *DirEntry)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	return DirEntry->StartCluster;
}

void SetDirEntryStartCluster(DirEntryStruct *DirEntry, UINT16 StartCluster)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	DirEntry->StartCluster = StartCluster;
}

UINT32 GetDirEntryFileLength(DirEntryStruct *DirEntry)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	return DirEntry->FileLength;
}

void SetDirEntryFileLength(DirEntryStruct *DirEntry, UINT32 FileLength)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	DirEntry->FileLength = FileLength;
}

DirEntryDateStruct GetDirEntryDate(DirEntryStruct *DirEntry)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;		
	return *(DirEntryDateStruct *)&(DirEntry->Date);
}

void SetDirEntryDate(DirEntryStruct *DirEntry, DirEntryDateStruct Date)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;		
	*(DirEntryDateStruct *)&(DirEntry->Date) = Date;
}

DirEntryTimeStruct GetDirEntryTime(DirEntryStruct *DirEntry)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	return *(DirEntryTimeStruct *)&(DirEntry->Time);
}

void SetDirEntryTime(DirEntryStruct *DirEntry, DirEntryTimeStruct Time)
{
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	*(DirEntryTimeStruct *)&(DirEntry->Time) = Time;
}

void GetDirEntryLongFileName(DirEntryStruct *DirEntry, INT8 *LongFileName)
{
	DirEntryStruct *FirstDirEntry;
	UINT8 *Ptr;
	
	FirstDirEntry = DirEntry;
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	
	if(FirstDirEntry == DirEntry)
	{
		GetDirEntryFileName(DirEntry, LongFileName);
		return;
	}
		
	while(DirEntry != FirstDirEntry)
	{
		DirEntry--;
		Ptr = (UINT8 *)DirEntry;
		/* so for, we only support one-byte character */		
 		GET_LONG_FILE_NAME(Ptr, 1, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 3, LongFileName);
 		GET_LONG_FILE_NAME(Ptr, 5, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 7, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 9, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 14, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 16, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 18, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 20, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 22, LongFileName);
 		GET_LONG_FILE_NAME(Ptr, 24, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 28, LongFileName);
		GET_LONG_FILE_NAME(Ptr, 30, LongFileName);
	}
	*LongFileName = '\0';
}

void SetDirEntryLongFileName(DirEntryStruct *DirEntry, INT8 *LongFileName)
{
	DirEntryStruct *FirstDirEntry;
	UINT8 *Ptr;
	
	FirstDirEntry = DirEntry;
	while(DirEntry->Attrib == ATTRIB_LONG_NAME)
		DirEntry++;
	
	if(FirstDirEntry != DirEntry)
	{
		while(DirEntry != FirstDirEntry)
		{
			DirEntry--;
			Ptr = (UINT8 *)DirEntry;
			/* so for, we only support one-byte character */		
	 		SET_LONG_FILE_NAME(Ptr, 1, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 3, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 5, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 7, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 9, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 14, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 16, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 18, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 20, LongFileName);
	 		SET_LONG_FILE_NAME(Ptr, 22, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 24, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 28, LongFileName);
 			SET_LONG_FILE_NAME(Ptr, 30, LongFileName);
 		}
 	}
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void GetCurrentDirectory(FileSysStruct *FileSys, INT8 *DirStr)
{
	strcpy(DirStr, FileSys->CurrDir.DirName);
}

int DisplayFATInfo(FileSysStruct *FileSys, INT8 *InfoStr)
{
	UINT8 VolumeLable[12];
	
	memcpy(VolumeLable, FileSys->FDCDesc.VolumeLable, 11);
	VolumeLable[11] = '\0';
	
	sprintf(InfoStr, "Volume Lable: %s, Volume Serial Number: %0.8X", VolumeLable, FileSys->FDCDesc.VolumeID);
	return TRUE;
}

int DisplayDirEntryInfo(FileSysStruct *FileSys, DirEntryStruct *DirEntry, INT8 *InfoStr)
{
	DirEntryInfoStruct DirInfo;
	
	GetDirEntryInfo(DirEntry, &DirInfo);	
	if(DirInfo.Attrib & ATTRIB_DIRECTORY)
	{
		sprintf(InfoStr, "%12s    %4d %6d <<<<DIR>>>> %0.2d-%0.2d-%0.4d %0.2d:%0.2d:%0.2d %s", DirInfo.Name, DirInfo.StartCluster, GetFATDirSectorNumber(FileSys, DirInfo.StartCluster),
			FAT_GET_MONTH(DirInfo.Date), FAT_GET_DAY(DirInfo.Date), FAT_GET_YEAR(DirInfo.Date)+1980, 
			FAT_GET_HOUR(DirInfo.Time), FAT_GET_MIN(DirInfo.Time), FAT_GET_SEC(DirInfo.Time), 
			DirInfo.LongName);
	}
	else
	{
		sprintf(InfoStr, "%12s    %4d %6d 0x%0.2X %6d %0.2d-%0.2d-%0.4d %0.2d:%0.2d:%0.2d %s", DirInfo.Name, DirInfo.StartCluster, GetFATDirSectorNumber(FileSys, DirInfo.StartCluster),
			DirInfo.Attrib, DirInfo.Length, 
			FAT_GET_MONTH(DirInfo.Date), FAT_GET_DAY(DirInfo.Date), FAT_GET_YEAR(DirInfo.Date)+1980, 
			FAT_GET_HOUR(DirInfo.Time), FAT_GET_MIN(DirInfo.Time), FAT_GET_SEC(DirInfo.Time), 
			DirInfo.LongName);
	}
	return TRUE;
}

DirEntryStruct *IsFileExistInDirectory(DirEntryStruct *DirEntry, INT8 *FileName)
{
	INT8 EntryName[0x10];
	
	DirEntry = GetFirstDirEntry(DirEntry);
	while(DirEntry != NULL)
	{		
		if(GetDirEntryAttrib(DirEntry) != ATTRIB_DIRECTORY)
		{
			GetDirEntryFileName(DirEntry, EntryName);
			//if(!stricmp(EntryName, FileName))
			if(!strcmp(EntryName, FileName))
				return DirEntry;	
		}
		DirEntry = GetNextDirEntry(DirEntry);
	}
	return NULL;
}

DirEntryStruct *IsDirectoryExistInDirectory(DirEntryStruct *DirEntry, INT8 *DirName)
{
	INT8 EntryName[0x10];
	
	DirEntry = GetFirstDirEntry(DirEntry);
	while(DirEntry != NULL)
	{		
		if(GetDirEntryAttrib(DirEntry) & ATTRIB_DIRECTORY)
		{
			GetDirEntryFileName(DirEntry, EntryName);
			//if(!stricmp(EntryName, DirName))
			if(!strcmp(EntryName, DirName))
				return DirEntry;	
		}
		DirEntry = GetNextDirEntry(DirEntry);
	}
	return NULL;
}

int ReadFATDirectory(FileSysStruct *FileSys, DirectoryStruct *CurrDirectory, DirectoryStruct *Directory, INT8 *DirStr)
{
	INT8 NextDirStr[MAX_DIRECTORY_PATH_SIZE], FullDirName[MAX_DIRECTORY_PATH_SIZE];
	DirEntryStruct *DirEntry;

	if(IsFullPath(DirStr))
	{
		if(!ReadFATDirectoryEntry(FileSys, Directory, FileSys->RootDirCluster, FileSys->RootDirSectorCount, (INT8 *)"\\"))
			return FALSE;
	}
	else
	{
		if(!ReadFATDirectoryEntry(FileSys, Directory, CurrDirectory->Cluster, CurrDirectory->SectorCount, CurrDirectory->DirName))
			return FALSE;
	}

	do
	{
		DirStr = GetDirectory(DirStr, NextDirStr);
		
		if(*NextDirStr == '\0')
			break;
		
		if((DirEntry = IsDirectoryExistInDirectory(Directory->Entry, NextDirStr)) == NULL)
		{
			FreeFATDirectoryEntry(Directory);
			return FALSE;
		}
		
		strcpy(FullDirName, Directory->DirName);
		UpdateDirectoryName(FullDirName, NextDirStr);
			
		FreeFATDirectoryEntry(Directory);
						
		/* Read next directory */
		if(!ReadFATDirectoryEntry(FileSys, Directory, GetDirEntryStartCluster(DirEntry), FileSys->FDCDesc.SectorPerCluster, FullDirName))
			return FALSE;				
				
	}while(DirStr != NULL);		
	
	return TRUE;
}
 
void UpdateFATCurrentDirectory(FileSysStruct *FileSys)
{
	DirectoryStruct Directory;
	
	if(ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FileSys->CurrDir.DirName))
	{
		FreeFATDirectoryEntry(&FileSys->CurrDir);
		memcpy(&FileSys->CurrDir, &Directory, sizeof(DirectoryStruct));
	}
	else if(ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, (INT8 *)"\\"))
	{
		FreeFATDirectoryEntry(&FileSys->CurrDir);
		memcpy(&FileSys->CurrDir, &Directory, sizeof(DirectoryStruct));
	}
}

/***************************************************************************/
/***************************************************************************/
DirEntryStruct *OpenFATDirectory(FileSysStruct *FileSys, INT8 *DirStr, DirectoryStruct *Directory)
{
	if(strlen(DirStr) == 0)
	{
		if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, Directory, FileSys->CurrDir.DirName))
		{
			FreeFATDirectoryEntry(Directory);
			return NULL;
		}
	}	
	else
	{
		if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, Directory, DirStr))
		{
			FreeFATDirectoryEntry(Directory);
			return NULL;
		}
	}	
	return GetFirstDirEntry(Directory->Entry);
}

void CloseFATDirectory(FileSysStruct *FileSys, DirectoryStruct *Directory)
{
	FreeFATDirectoryEntry(Directory);
}
/***************************************************************************/
/***************************************************************************/
int ChangeFATDirectory(FileSysStruct *FileSys, INT8 *DirStr)
{
	DirectoryStruct Directory;
		
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, DirStr))
		return FALSE;
		
	FreeFATDirectoryEntry(&FileSys->CurrDir);
	memcpy(&FileSys->CurrDir, &Directory, sizeof(DirectoryStruct));
	return TRUE;
}

/***************************************************************************/
/***************************************************************************/
int RenameFATDirectoryFile(FileSysStruct *FileSys, INT8 *OldName, INT8 *NewName)
{
	DirectoryStruct Directory;
	DirEntryStruct *DirEntry;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], NameStr[MAX_DIRECTORY_PATH_SIZE];
	INT8 NewFatherDirName[MAX_DIRECTORY_PATH_SIZE], NewNameStr[MAX_DIRECTORY_PATH_SIZE];

	SplitPath(OldName, FatherDirName, NameStr);
	SplitPath(NewName, NewFatherDirName, NewNameStr);

	/* change to the father directory of new directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
		
	/* if it is directory */
	if((DirEntry = IsDirectoryExistInDirectory(Directory.Entry, NameStr)) != NULL)
	{
		/* if the father directory is the same */
		//if((!stricmp(FatherDirName, "")) || (!stricmp(FatherDirName, NewFatherDirName)))
		if((!strcmp(FatherDirName, "")) || (!strcmp(FatherDirName, NewFatherDirName)))
		{
			SetDirEntryFileName(DirEntry, NewNameStr);
			if(WriteFATDirectoryEntry(FileSys, &Directory))
			{
				FreeFATDirectoryEntry(&FileSys->CurrDir);
				UpdateFATCurrentDirectory(FileSys);
				return TRUE;
			}
		}	
	}
	/* if it is file */
	else if((DirEntry = IsFileExistInDirectory(Directory.Entry, NameStr)) != NULL)
	{
		/* if have the same directory name */
		//if((!stricmp(FatherDirName, "")) || (!stricmp(FatherDirName, NewFatherDirName)))
		if((!strcmp(FatherDirName, "")) || (!strcmp(FatherDirName, NewFatherDirName)))
		{
			SetDirEntryFileName(DirEntry, NewNameStr);
			if(WriteFATDirectoryEntry(FileSys, &Directory))
			{
				FreeFATDirectoryEntry(&FileSys->CurrDir);
				UpdateFATCurrentDirectory(FileSys);
				return TRUE;
			}
		}
		else
		{
			FreeDirEntry(DirEntry);	
#if 0				
			/* free the new one */
			FreeFATCluster(&(FileSys->FAT), GetDirEntryStartCluster(DirEntry));
			if(WriteFAT(FileSys))
			{
				/* save */
				if(WriteFATDirectoryEntry(FileSys, &Directory))
				{
					FreeFATDirectoryEntry(&Directory);
					UpdateFATCurrentDirectory(FileSys);
					return TRUE;
				}
			}
#endif			
		}
	}
					
	FreeFATDirectoryEntry(&FileSys->CurrDir);
	return FALSE;
}
/***************************************************************************/
/***************************************************************************/
int MakeFATDirectory(FileSysStruct *FileSys, INT8 *FullDirName)
{
	DirectoryStruct Directory;
	DirEntryInfoStruct DirEntryInfo;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], DirName[MAX_DIRECTORY_PATH_SIZE];

	SplitPath(FullDirName, FatherDirName, DirName);

	/* change to the father directory of new directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
		
	if(IsDirectoryExistInDirectory(Directory.Entry, DirName) != NULL)
		return FALSE;
				
	if(IsFileExistInDirectory(Directory.Entry, DirName) != NULL)
		return FALSE;
		
	strcpy(DirEntryInfo.Name, DirName);
	
	/* create this directory */
	if(MakeFATNextDirectory(FileSys, &Directory, &DirEntryInfo))
	{
		FreeFATDirectoryEntry(&Directory);
		UpdateFATCurrentDirectory(FileSys);
		return TRUE;
	}		
			
	FreeFATDirectoryEntry(&Directory);
	return FALSE;
}

int MakeFATNextDirectory(FileSysStruct *FileSys, DirectoryStruct *Directory, DirEntryInfoStruct *DirEntryInfo)
{	
	UINT32 NewCluster;
	DirEntryStruct *NewDirEntry, *EmptyDirEntry;
	DirEntryInfoStruct NewDirEntryInfo;
		
	//strupr(DirEntryInfo->Name);
	
	/* find a free cluster */	
	NewCluster = GetFATFirstFreeCluster(&(FileSys->FAT));
	//FileSys->EraseSector(FileSys->CardPtr, GetFATDirSectorNumber(FileSys, NewCluster), FileSys->FDCDesc.SectorPerCluster, FileSys->FDCDesc.SectorSize);
			
	/* malloc a buf */
	NewDirEntry = (DirEntryStruct *)malloc(1 * FileSys->FDCDesc.SectorSize);
	memset(NewDirEntry, 0x0, FileSys->FDCDesc.SectorSize);
	
	/* copy DirEntryInfo to NewDirEntryInfo, NewDirEntryInfo will be used to create . and .. in this new directory */
	memcpy(&NewDirEntryInfo, DirEntryInfo, sizeof(DirEntryInfoStruct));
	
	/* get a free entry in this new directory */		
	if((EmptyDirEntry = GetFirstFreeDirEntry(NewDirEntry, 1, ATTRIB_DIRECTORY)) == NULL)
		return FALSE;
		
	/* create . directory entry, in this new directory */
	NewDirEntryInfo.StartCluster = NewCluster;
	strcpy(NewDirEntryInfo.Name, ".");
	NewDirEntryInfo.Attrib = ATTRIB_DIRECTORY;
	NewDirEntryInfo.Length = 0;
	SetDirEntryInfo(EmptyDirEntry, &NewDirEntryInfo);	
				
	/* get a free entry in this new irectory */		
	if((EmptyDirEntry = GetFirstFreeDirEntry(NewDirEntry, 1, ATTRIB_DIRECTORY)) == NULL)
		return FALSE;
		
	/* create .. directory entry, in this directory */
	NewDirEntryInfo.StartCluster = Directory->Cluster;
	strcpy(NewDirEntryInfo.Name, "..");
	NewDirEntryInfo.Attrib = ATTRIB_DIRECTORY;
	NewDirEntryInfo.Length = 0;
	SetDirEntryInfo(EmptyDirEntry, &NewDirEntryInfo);	
		
	/* save this new directory */
	if(FileSys->WriteSector(FileSys->CardPtr, GetFATDirSectorNumber(FileSys, NewCluster), 1, FileSys->FDCDesc.SectorSize, (UINT8 *)NewDirEntry))
	{
		/* now, update FAT table */
		if(WriteFAT(FileSys))
		{		
			/* create directory entry in this directory */		
			if((EmptyDirEntry = GetFirstFreeDirEntry(Directory->Entry, 1, ATTRIB_DIRECTORY)) != NULL)
			{	
				/* add this directory entry to current directory */
				DirEntryInfo->StartCluster = NewCluster;
				DirEntryInfo->Attrib = ATTRIB_DIRECTORY;
				DirEntryInfo->Length = 0;
				SetDirEntryInfo(EmptyDirEntry, DirEntryInfo);
				
				/* save */
				if(WriteFATDirectoryEntry(FileSys, Directory))
				{
					free(NewDirEntry);
					return TRUE;
				}
			}
		}
	}
	free(NewDirEntry);
	return FALSE;
}

/***************************************************************************/
/***************************************************************************/
int DeleteFATDirectory(FileSysStruct *FileSys, UINT8 *FullDirName)
{	
	DirectoryStruct Directory;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], DirName[MAX_DIRECTORY_PATH_SIZE];

	SplitPath(FullDirName, FatherDirName, DirName);

	/* change to the father directory of new directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
		
	/* delete this directory */
	if(DeleteFATNextDirectory(FileSys, &Directory, DirName))
	{
		FreeFATDirectoryEntry(&Directory);
		UpdateFATCurrentDirectory(FileSys);
		return TRUE;
	}
			
	FreeFATDirectoryEntry(&Directory);
	return FALSE;
}

int DeleteFATNextDirectory(FileSysStruct *FileSys, DirectoryStruct *Directory, INT8 *DirName)
{
	UINT8 Attrib;
	INT8 NextDirName[0x10];
	DirEntryStruct *DeleteDirEntry, *DirEntry;
	DirectoryStruct NextDirectory;
	
	if((DeleteDirEntry = IsDirectoryExistInDirectory(Directory->Entry, DirName)) == NULL)
		return FALSE;
	
	if(!ReadFATDirectory(FileSys, Directory, &NextDirectory, DirName))
		return FALSE;
		
	DirEntry = GetFirstDirEntry(NextDirectory.Entry);
	while(DirEntry != NULL)
	{
		Attrib = GetDirEntryAttrib(DirEntry);
		GetDirEntryFileName(DirEntry, NextDirName);
		
		if((!strcmp(NextDirName, "..")) || (!strcmp(NextDirName, ".")))
		{
			DirEntry = GetNextDirEntry(DirEntry);
			continue;
		}
		
		if(Attrib & ATTRIB_DIRECTORY)
		{
			DeleteFATNextDirectory(FileSys, &NextDirectory, NextDirName);
		}

		/* free clusters of this entry and write back to memory card */
		FreeFATCluster(&(FileSys->FAT), GetDirEntryStartCluster(DirEntry));	
		if(!WriteFAT(FileSys))
			return FALSE;
			
		/* free this entry and write back to memory card */
		FreeDirEntry(DirEntry);		
		if(!WriteFATDirectoryEntry(FileSys, &NextDirectory))
			return FALSE;
		
		/* get next entry */
		DirEntry = GetNextDirEntry(DirEntry);
	}
				
	/* free clusters of this entry and write back to memory card */
	FreeFATCluster(&(FileSys->FAT), GetDirEntryStartCluster(DeleteDirEntry));	
	if(!WriteFAT(FileSys))
		return FALSE;
		
	/* free this entry and write back to memory card */
	FreeDirEntry(DeleteDirEntry);		
	if(!WriteFATDirectoryEntry(FileSys, Directory))
		return FALSE;
		
	return TRUE;
}

/***************************************************************************/
/***************************************************************************/
int DeleteFATFile(FileSysStruct *FileSys, INT8 *FullFileName)
{
	DirectoryStruct Directory;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], FileName[MAX_DIRECTORY_PATH_SIZE];
	DirEntryStruct *DirEntry;

	SplitPath(FullFileName, FatherDirName, FileName);

	/* change to the father directory of new directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
		
	if((DirEntry = IsFileExistInDirectory(Directory.Entry, FileName)) != NULL)
	{
		FreeFATCluster(&(FileSys->FAT), GetDirEntryStartCluster(DirEntry));
		if(WriteFAT(FileSys))
		{
			FreeDirEntry(DirEntry);		
			/* save */
			if(WriteFATDirectoryEntry(FileSys, &Directory))
			{
				FreeFATDirectoryEntry(&Directory);
				UpdateFATCurrentDirectory(FileSys);
				return TRUE;
			}
		}
	}
			
	FreeFATDirectoryEntry(&Directory);
	return FALSE;
}

/***************************************************************************/
/***************************************************************************/
int CreateFATEmptyFile(FileSysStruct *FileSys, INT8 *FullFileName)
{
	DirectoryStruct Directory;
	DirEntryInfoStruct DirEntryInfo;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], FileName[MAX_DIRECTORY_PATH_SIZE];

	SplitPath(FullFileName, FatherDirName, FileName);

	/* change to the father directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
				
	if(IsDirectoryExistInDirectory(Directory.Entry, FileName) != NULL)
		return FALSE;	
		
	if(IsFileExistInDirectory(Directory.Entry, FileName) != NULL)
		return FALSE;	
				
	strcpy(DirEntryInfo.Name, FileName);
		
	/* create this file */
	if(CreateFATNextEmptyFile(FileSys, &Directory, &DirEntryInfo))
	{
		FreeFATDirectoryEntry(&Directory);
		UpdateFATCurrentDirectory(FileSys);
		return TRUE;
	}
			
	FreeFATDirectoryEntry(&Directory);
	return FALSE;
	
}

int CreateFATNextEmptyFile(FileSysStruct *FileSys, DirectoryStruct *Directory, DirEntryInfoStruct *DirEntryInfo)
{
	UINT32 NewCluster;
	DirEntryStruct *DirEntry;
		
	//strupr(DirEntryInfo->Name);
	
	/* find a free cluster */	
	NewCluster = GetFATFirstFreeCluster(&(FileSys->FAT));		
	/* now, update FAT table */
	if(!WriteFAT(FileSys))
		return FALSE;
		
	/* create directory entry in this directory */		
	if((DirEntry = GetFirstFreeDirEntry(Directory->Entry, 1, DirEntryInfo->Attrib)) == NULL)
		return FALSE;
	
	/* add this directory entry to current directory */
	DirEntryInfo->StartCluster = NewCluster;
	DirEntryInfo->Length = 0;
	DirEntryInfo->Attrib = ATTRIB_ARCHIVE;
	SetDirEntryInfo(DirEntry, DirEntryInfo);
		
	/* save */
	if(!WriteFATDirectoryEntry(FileSys, Directory))
		return FALSE;

	return TRUE;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
int IsFileExist(FileSysStruct *FileSys, UINT8 *FullFileName)
{
	DirectoryStruct Directory;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], FileName[MAX_DIRECTORY_PATH_SIZE];

	SplitPath(FullFileName, FatherDirName, FileName);
		
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
		
	if(IsFileExistInDirectory(Directory.Entry, FileName) != NULL)
	{
		FreeFATDirectoryEntry(&Directory);
		return TRUE;	
	}
	FreeFATDirectoryEntry(&Directory);
	return FALSE;
}

/***************************************************************************/
/***************************************************************************/
int OpenFATEmptyFile(FileSysStruct *FileSys, FileHandleStruct *fd, INT8 *FullFileName)
{
	DirectoryStruct Directory;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], FileName[MAX_DIRECTORY_PATH_SIZE];
	DirEntryStruct *DirEntry;
	UINT32 NewCluster;
	DirEntryInfoStruct DirEntryInfo;

	SplitPath(FullFileName, FatherDirName, FileName);

	/* change to the father directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;	

	if(IsDirectoryExistInDirectory(Directory.Entry, FileName) != NULL)
		return FALSE;	
		
	if((DirEntry = IsFileExistInDirectory(Directory.Entry, FileName)) == NULL)
	{
		strcpy(DirEntryInfo.Name, FileName);
			
		/* create this file */
		if(CreateFATNextEmptyFile(FileSys, &Directory, &DirEntryInfo))
		{
			DirEntry = IsFileExistInDirectory(Directory.Entry, FileName);
			fd->Attrib = GetDirEntryAttrib(DirEntry);
			fd->FileTotalSize = 0;
				
			fd->StartCluster = fd->CurrCluster = GetDirEntryStartCluster(DirEntry);
			fd->ClusterBuf = malloc(FileSys->ClusterSize);
			fd->ClusterBufSize = FileSys->ClusterSize;
			fd->ClusterBufOffset = 0;
				
			fd->Pos = 0;			
			fd->Dirty = FALSE;
	
			FreeFATDirectoryEntry(&Directory);
			UpdateFATCurrentDirectory(FileSys);
			return TRUE;
		}
		FreeFATDirectoryEntry(&Directory);
		return FALSE;
	}

	FreeFATCluster(&(FileSys->FAT), GetDirEntryStartCluster(DirEntry));
	NewCluster = GetFATFirstFreeCluster(&(FileSys->FAT));
	if(WriteFAT(FileSys))
	{
		SetDirEntryStartCluster(DirEntry, NewCluster);
		SetDirEntryFileLength(DirEntry, 0);			
		/* save */
		if(WriteFATDirectoryEntry(FileSys, &Directory))
		{
			
			fd->Attrib = GetDirEntryAttrib(DirEntry);
			fd->FileTotalSize = 0;
			
			fd->StartCluster = fd->CurrCluster = NewCluster;
			fd->ClusterBuf = malloc(FileSys->ClusterSize);
			fd->ClusterBufSize = FileSys->ClusterSize;
			fd->ClusterBufOffset = 0;
				
			fd->Pos = 0;			
			fd->Dirty = FALSE;
			
			FreeFATDirectoryEntry(&Directory);
			UpdateFATCurrentDirectory(FileSys);
			return TRUE;
		}
	}			
	FreeFATDirectoryEntry(&Directory);
	return FALSE;
}

/***************************************************************************/
/***************************************************************************/
int OpenFATFile(FileSysStruct *FileSys, FileHandleStruct *fd, INT8 *FullFileName)
{
	DirectoryStruct Directory;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], FileName[MAX_DIRECTORY_PATH_SIZE];
	DirEntryStruct *DirEntry;

	SplitPath(FullFileName, FatherDirName, FileName);

	/* change to the father directory */	
	if(!ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
		return FALSE;
		
	if((DirEntry = IsFileExistInDirectory(Directory.Entry, FileName)) == NULL)
		return FALSE;
		
	fd->Attrib = GetDirEntryAttrib(DirEntry);
	fd->FileTotalSize = GetDirEntryFileLength(DirEntry);

	fd->StartCluster = fd->CurrCluster = GetDirEntryStartCluster(DirEntry);
	fd->ClusterBuf = malloc(FileSys->ClusterSize);
	fd->ClusterBufSize = FileSys->ClusterSize;
	fd->ClusterBufOffset = 0;
				
	fd->Pos = 0;			
	fd->Dirty = FALSE;
	return TRUE;
}

int CloseFATFile(FileSysStruct *FileSys, FileHandleStruct *fd)
{
	DirectoryStruct Directory;
	INT8 FatherDirName[MAX_DIRECTORY_PATH_SIZE], FileName[MAX_DIRECTORY_PATH_SIZE];
	DirEntryStruct *DirEntry;
		
	SplitPath(fd->FileName, FatherDirName, FileName);

	/* change to the father directory */	
	if(ReadFATDirectory(FileSys, &FileSys->CurrDir, &Directory, FatherDirName))
	{
		if((DirEntry = IsFileExistInDirectory(Directory.Entry, FileName)) != NULL)
		{
			if(fd->Dirty)
			{
				if(FileSys->WriteSector(FileSys->CardPtr, GetFATDirSectorNumber(FileSys, fd->CurrCluster), FileSys->FDCDesc.SectorPerCluster, FileSys->FDCDesc.SectorSize, fd->ClusterBuf))		
				{
					SetDirEntryFileLength(DirEntry, fd->FileTotalSize);
					if(WriteFATDirectoryEntry(FileSys, &Directory))
					{
						free(fd->ClusterBuf);
						UpdateFATCurrentDirectory(FileSys);
						if(!WriteFAT(FileSys))
							return FALSE;
						return TRUE;
					}
				}
			}
			else
			{
				free(fd->ClusterBuf);
				if(!WriteFAT(FileSys))
					return FALSE;
				return TRUE;
			}
		}
	
	}
	 
	free(fd->ClusterBuf);
	return FALSE;
}

/***************************************************************************/
/***************************************************************************/
int ReadFATFile(FileSysStruct *FileSys, FileHandleStruct *fd, UINT32 Len, UINT8 *Buf)
{
	UINT32 WLen;
	
	
	while(Len > 0)
	{	
		/* if no any data in ClusterBuf */
		if(fd->ClusterBufOffset == 0)
		{
			/* if current cluster is not allocated */
			if(GetClusterStatus(&(FileSys->FAT), fd->CurrCluster) != FAT_CLUSTER_ALLOCATED)
				return Len;
				
			/* read one cluster */
			if(!FileSys->ReadSector(FileSys->CardPtr, GetFATDirSectorNumber(FileSys, fd->CurrCluster), FileSys->FDCDesc.SectorPerCluster, FileSys->FDCDesc.SectorSize, fd->ClusterBuf))
				return Len;
				
			/* move CurrCluster to next cluster */
			fd->CurrCluster = GetFATNextCluster(&(FileSys->FAT), fd->CurrCluster);
		}
		
		/* WLen is the minimum of Len or data left in ClusterBuf */
		WLen = min(Len, fd->ClusterBufSize - fd->ClusterBufOffset);
		
		/* if over the file size */
		if((fd->Pos + WLen) > fd->FileTotalSize)
			WLen = fd->FileTotalSize - fd->Pos;
			
		/* copy to buf */
		memcpy(Buf, fd->ClusterBuf + fd->ClusterBufOffset, WLen);		
		
		Len -= WLen;
		fd->Pos += WLen;
		Buf += WLen;
		
		if((fd->ClusterBufOffset = fd->ClusterBufOffset + WLen) == fd->ClusterBufSize)
			fd->ClusterBufOffset = 0;		
			
		if(fd->FileTotalSize == fd->Pos)
			break;
	}	
	return Len;
}

/***************************************************************************/
/***************************************************************************/
int WriteFATFile(FileSysStruct *FileSys, FileHandleStruct *fd, UINT32 Len, UINT8 *Buf)
{
	UINT32 WLen, NewCluster;
	
	while(Len > 0)
	{
		WLen = min(Len, fd->ClusterBufSize - fd->ClusterBufOffset);
		Len -= WLen;
	
		memcpy(fd->ClusterBuf + fd->ClusterBufOffset, Buf, WLen);
		fd->ClusterBufOffset += WLen;
		fd->Dirty = TRUE;
		fd->FileTotalSize += WLen;
		fd->Pos += WLen;
		
		if(fd->ClusterBufOffset == fd->ClusterBufSize)
		{
			if(!FileSys->WriteSector(FileSys->CardPtr, GetFATDirSectorNumber(FileSys, fd->CurrCluster), FileSys->FDCDesc.SectorPerCluster, FileSys->FDCDesc.SectorSize, fd->ClusterBuf))
				return Len;
				
			NewCluster = GetFATFirstFreeCluster(&(FileSys->FAT));
			SetFATNextCluster(&(FileSys->FAT), fd->CurrCluster, NewCluster);
//			if(!WriteFAT(FileSys))
//				return Len;
				
			fd->CurrCluster = NewCluster;
			fd->ClusterBufOffset = 0;
		}
	}
	return Len;
}

/***************************************************************************/
/***************************************************************************/
int SeekFATFile(FileSysStruct *FileSys, FileHandleStruct *fd, UINT32 Pos)
{
	UINT32 WLen;
	
#if 0	
	if(Pos > fd->FileTotalSize)
		return FALSE;
#else	
	while(Pos > fd->FileTotalSize)
		Pos -= fd->FileTotalSize;
#endif
		
	fd->CurrCluster = fd->StartCluster;
	fd->Pos = 0;
	fd->ClusterBufOffset = 0;
	while(Pos > 0)
	{
		/* if current cluster is not allocated */
		if(GetClusterStatus(&(FileSys->FAT), fd->CurrCluster) != FAT_CLUSTER_ALLOCATED)
			return FALSE;
			
		/* read one cluster */
		if(!FileSys->ReadSector(FileSys->CardPtr, GetFATDirSectorNumber(FileSys, fd->CurrCluster), FileSys->FDCDesc.SectorPerCluster, FileSys->FDCDesc.SectorSize, fd->ClusterBuf))
			return FALSE;
			
		/* move CurrCluster to next cluster */
		fd->CurrCluster = GetFATNextCluster(&(FileSys->FAT), fd->CurrCluster);
		
		WLen = min(Pos, fd->ClusterBufSize);
		Pos -= WLen;
		if((fd->ClusterBufOffset = fd->ClusterBufOffset + WLen) == fd->ClusterBufSize)
			fd->ClusterBufOffset = 0;		
		fd->Pos += WLen;
	}	
	return TRUE;	
}
