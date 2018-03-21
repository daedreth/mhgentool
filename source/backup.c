#include <3ds.h>
#include <backup.h>
#include "pp2d.h"
#include "colors.h"
#include <stdlib.h>

Result MHGEN_BackupSingleSave(u32 archiveID, char* path, char* dest, u32 saveSize)
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

Result MHGEN_BackupSaves()
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

  res = MHGEN_BackupSingleSave(archiveCode, "/system", "/mhgentool/system", 4000815);
  if(R_FAILED(res)) return res;
  res = MHGEN_BackupSingleSave(archiveCode, "/system_backup", "/mhgentool/system_backup", 4000815);
  if(R_FAILED(res)) return res;

  return 0;
}

Result MHGEN_drawBackupMenu()
{
    u8 cursorPosition = 1;
    Result res = 0;
    
  while (aptMainLoop())
    {
      hidScanInput();
      
      pp2d_frame_begin(GFX_TOP, GFX_LEFT);

      pp2d_frame_draw_on(GFX_TOP, GFX_LEFT);
      
      // top screen background
      pp2d_texture_select_part(0, 0, 0, 0, 240, 400, 240);
      pp2d_texture_queue();

      // logo
      pp2d_texture_select_part(0, 0, 0, 0, 480, 400, 240);
      pp2d_texture_queue();

      pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);
      
      // bottom screen background
      pp2d_texture_select_part(0, 0, 0, 0, 0, 320, 240);
      pp2d_texture_queue();

      // selection sprite
      pp2d_texture_select_part(0, 12, 20 + ((cursorPosition - 1) * 65), 321, 58, 207, 56);
      pp2d_texture_queue();

      // button sprites
      for (u8 menuCounter = 0; menuCounter < 3; ++menuCounter)
	{
	  pp2d_texture_select_part(0, 12, 20 + (menuCounter * 65), 321, 1, 207, 56);
	  pp2d_texture_queue();
	}

      // button text
      for (u8 menuCounter = 0; menuCounter < 3; ++menuCounter)
	{
	  pp2d_texture_select_part(0, 17, 20 + (menuCounter * 65), 737, (menuCounter * 56) + (menuCounter + 1), 207, 56);
	  pp2d_texture_queue();
	}

      // help option
      pp2d_texture_select_part(0, 230, 120 , 401, 240, 79, 84);
      pp2d_texture_queue();


      if(hidKeysDown() & KEY_A)
	{
	  if(cursorPosition == 1)
	    {
	      res = MHGEN_BackupSaves();
	      if(R_FAILED(res)) return res;
	    }
	}
      
      // move cursor up and down
      if((hidKeysDown() & KEY_UP) && (cursorPosition > 1)) cursorPosition--;
      if((hidKeysDown() & KEY_DOWN) && (cursorPosition < 3)) cursorPosition++;

      if(hidKeysDown() & KEY_B) return 0;
      
      pp2d_frame_end();
    }
  return 0;
}
/*
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
*/
