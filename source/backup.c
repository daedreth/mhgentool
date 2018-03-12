#include <3ds.h>
#include "pp2d.h"
#include "color.h"
#include <stdlib.h>

void createMhgenDir()
{
  FS_Archive ArchiveSD;
  Result res = 0;
  res = FSUSER_OpenArchive(&ArchiveSD, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
  FSUSER_CreateDirectory(ArchiveSD, fsMakePath(PATH_ASCII, "/mhgentool"), FS_ATTRIBUTE_DIRECTORY);
  FSUSER_CloseArchive(ArchiveSD);
}

Result copySingleSave(u32 archiveID, char* path, char* dest, u32 saveSize)
{
  FS_Archive sdcardArchive;
  FS_Archive extdataArchive;

  Handle saveFileHandle;
  Handle backupFileHandle;

  char *buffer;

  Result res = 0;

  const u32 pathArray[3] = { MEDIATYPE_SD, archiveID, 0 };
  res = FSUSER_OpenArchive(&extdataArchive, ARCHIVE_EXTDATA, (FS_Path){ PATH_BINARY, 12, pathArray });
  if(R_FAILED(res)) return res;
  res = FSUSER_OpenFile(&saveFileHandle, extdataArchive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
  if(R_FAILED(res)) return res;
  
  buffer = (char*) malloc (sizeof(char) * saveSize);

  FSFILE_Read(saveFileHandle, NULL, 0, buffer, saveSize);
  if(R_FAILED(res)) return res;
  
  res = FSUSER_OpenArchive(&sdcardArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
  if(R_FAILED(res)) return res;
  
  FSUSER_DeleteFile(sdcardArchive, fsMakePath(PATH_ASCII, dest));
  res = FSUSER_CreateFile(sdcardArchive, fsMakePath(PATH_ASCII, dest), 0, (u64)saveSize);
  if(R_FAILED(res)) return res;
  
  res = FSUSER_OpenFile(&backupFileHandle, sdcardArchive, fsMakePath(PATH_ASCII, dest), FS_OPEN_WRITE, 0);
  if(R_FAILED(res)) return res;
  
  res = FSFILE_Write(backupFileHandle, NULL, 0, buffer, saveSize, FS_WRITE_FLUSH);
  if(R_FAILED(res)) return res;
  
  free(buffer);

  FSFILE_Close(saveFileHandle);
  FSFILE_Close(backupFileHandle);
  FSUSER_CloseArchive(extdataArchive);
  FSUSER_CloseArchive(sdcardArchive);
  
  return 0;
}

Result dumpSaveAndBackup()
{
  u32 archiveCode;
  Result res = 0;

  u8 regionCode;
  CFGU_SecureInfoGetRegion(&regionCode);
  switch(regionCode)
    {
    case 0:
      archiveCode = 0x00001554;
      break;
    case 1:
      archiveCode = 0x00001870;
      break;
    case 2:
      archiveCode = 0x0000185b;
      break;
    default:
      archiveCode = 0x00;
    }

  res = copySingleSave(archiveCode, "/system", "/mhgentool/system", 4000815);
  if(R_FAILED(res)) return res;
  res = copySingleSave(archiveCode, "/system_backup", "/mhgentool/system_backup", 4000815);
  if(R_FAILED(res)) return res;

  return 0;
}

Result importSingleSave(u32 archiveID, char* path, char* dest, u32 saveSize)
{
  FS_Archive sdcardArchive;
  FS_Archive extdataArchive;

  Handle saveFileHandle;
  Handle backupFileHandle;

  char *buffer;

  Result res = 0;

  const u32 pathArray[3] = { MEDIATYPE_SD, archiveID, 0 };
  res = FSUSER_OpenArchive(&sdcardArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
  if(R_FAILED(res)) return res;
  res = FSUSER_OpenFile(&backupFileHandle, sdcardArchive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
  if(R_FAILED(res)) return res;
  
  buffer = (char*) malloc (sizeof(char) * saveSize);

  FSFILE_Read(backupFileHandle, NULL, 0, buffer, saveSize);
  if(R_FAILED(res)) return res;
  
  res = FSUSER_OpenArchive(&extdataArchive, ARCHIVE_EXTDATA, (FS_Path){ PATH_BINARY, 12, pathArray });
  if(R_FAILED(res)) return res;
  
  FSUSER_DeleteFile(extdataArchive, fsMakePath(PATH_ASCII, dest));
  res = FSUSER_CreateFile(extdataArchive, fsMakePath(PATH_ASCII, dest), 0, (u64)saveSize);
  if(R_FAILED(res)) return res;  
  
  res = FSUSER_OpenFile(&saveFileHandle, extdataArchive, fsMakePath(PATH_ASCII, dest), FS_OPEN_WRITE, 0);
  if(R_FAILED(res)) return res;
  
  res = FSFILE_Write(saveFileHandle, NULL, 0, buffer, saveSize, FS_WRITE_FLUSH);
  if(R_FAILED(res)) return res;

  
  free(buffer);

  FSFILE_Close(saveFileHandle);
  FSFILE_Close(backupFileHandle);
  FSUSER_CloseArchive(extdataArchive);
  FSUSER_CloseArchive(sdcardArchive);
  
  return 0;
}

Result importSaveAndBackup()
{
  u32 archiveCode;
  Result res = 0;

  u8 regionCode;
  CFGU_SecureInfoGetRegion(&regionCode);
  switch(regionCode)
    {
    case 0:
      archiveCode = 0x00001554;
      break;
    case 1:
      archiveCode = 0x00001870;
      break;
    case 2:
      archiveCode = 0x0000185b;
      break;
    default:
      archiveCode = 0x00;
    }

  res = importSingleSave(archiveCode, "/mhgentool/system", "/system", 4000815);
  if(R_FAILED(res)) return res;
  res = importSingleSave(archiveCode, "/mhgentool/system_backup", "/system_backup", 4000815);
  if(R_FAILED(res)) return res;

  return 0;
}

Result deleteSaveAndBackup()
{
  u64 archiveCode;
  Result res = 0;

  u8 regionCode;
  CFGU_SecureInfoGetRegion(&regionCode);
  switch(regionCode)
    {
    case 0:
      archiveCode = 0x00001554;
      break;
    case 1:
      archiveCode = 0x00001870;
      break;
    case 2:
      archiveCode = 0x0000185b;
      break;
    default:
      archiveCode = 0x00;
    }

  FS_ExtSaveDataInfo save;
  save.mediaType = 1;
  save.saveId = archiveCode;
  res = FSUSER_DeleteExtSaveData(save);
  if(R_FAILED(res)) return res;
 
  return 0;
}
