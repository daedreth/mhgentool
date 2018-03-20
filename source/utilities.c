#include <3ds.h>
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
