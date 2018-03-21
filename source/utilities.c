#include <3ds.h>
#include "pp2d.h"
#include "utilities.h"

Result MHGEN_CreateToolDirectories()
{
  FS_Archive sdCardArchive;
  Result res = 0;
  char* directories[4] = {"/mhgentool", "/mhgentool/backups", "/mhgentool/characters", "/mhgentool/palicos"};
  res = FSUSER_OpenArchive(&sdCardArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
  if(R_FAILED(res)) return res;
  
  for (u8 dirCounter = 0; dirCounter < (sizeof directories / sizeof *directories); ++dirCounter)
    FSUSER_CreateDirectory(sdCardArchive, fsMakePath(PATH_ASCII, directories[dirCounter]), FS_ATTRIBUTE_DIRECTORY);

  FSUSER_CloseArchive(sdCardArchive);
  return 0;
}

Result MHGEN_NoExtdataError()
{
  u32 archiveCode;
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
  
  FS_Archive extdataArchive;
  Result res = 0;

  const u32 pathArray[3] = { MEDIATYPE_SD, archiveCode, 0 };
  res = FSUSER_OpenArchive(&extdataArchive, ARCHIVE_EXTDATA, (FS_Path){ PATH_BINARY, 12, pathArray });
  if(R_FAILED(res)){
    while(aptMainLoop())
      {
	hidScanInput();      

	// restart frame
	pp2d_frame_end();
	pp2d_frame_begin(GFX_TOP, GFX_LEFT);
    
	// top screen background
	pp2d_texture_select_part(0, 0, 0, 0, 240, 400, 240);
	pp2d_texture_queue();

	// top screen error texture
	pp2d_texture_select_part(0, 95, 80, 321, 172, 207, 67);
	pp2d_texture_queue();

	pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);

	// error texture
	pp2d_texture_select_part(0, 55, 80, 529, 172, 207, 67);
	pp2d_texture_queue();

	if(hidKeysDown() & KEY_START) return 100;
      }
  }
  FSUSER_CloseArchive(extdataArchive);
  return 0;
}
