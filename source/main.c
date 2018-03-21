#include <3ds.h>
#include "pp2d.h"
#include "utilities.h"
#include "menu.h"

int main()
{
  romfsInit();
  cfguInit();
  pp2d_init();


  MHGEN_CreateToolDirectories(); // utilities.h
  
  pp2d_set_screen_color(GFX_TOP, ABGR8(255, 10, 10, 10));
  pp2d_set_screen_color(GFX_BOTTOM, ABGR8(255, 10, 10, 10));
  pp2d_load_texture_png(0, "romfs:/spritesheet.png");
    Result res = 0;
  res = MHGEN_NoExtdataError();


  while(true){
    switch(res){
    case 0:
      res = MHGEN_drawMainMenu(); // menu.h
      break;
    default:
      return 0;
    }
  }

  pp2d_free_texture(0);
  // exit pp2d environment
  pp2d_exit();
    
  return 0;
}
