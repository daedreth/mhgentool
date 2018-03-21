#include <3ds.h>
#include "colors.h"
#include "pp2d.h"
#include "menu.h"

Result MHGEN_drawMainMenu()
{
  u8 cursorPosition = 0;
  
  while (aptMainLoop())
    {
      hidScanInput();
      
      char* mainMenuOptions[3] = {"Character(soon)", "Save(soon)", "Palico(soon)"};

      pp2d_frame_begin(GFX_TOP, GFX_LEFT);

      // top screen background
      pp2d_frame_draw_on(GFX_TOP, GFX_LEFT);
      pp2d_texture_select_part(0, 0, 0, 0, 240, 400, 240);
      pp2d_texture_queue();

      // bottom screen background
      pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);
      pp2d_texture_select_part(0, 0, 0, 0, 0, 320, 240);
      pp2d_texture_queue();

      // selection sprite
      pp2d_texture_select_part(0, 15, 20 + (cursorPosition * 65), 321, 58, 207, 56);
      pp2d_texture_queue();

      // button sprites
      for (u8 menuCounter = 0; menuCounter < (sizeof mainMenuOptions / sizeof *mainMenuOptions); ++menuCounter)
	{
	  pp2d_texture_select_part(0, 15, 20 + (menuCounter * 65), 321, 1, 207, 56);
	  pp2d_texture_queue();
	}

      for (u8 menuCounter = 0; menuCounter < (sizeof mainMenuOptions / sizeof *mainMenuOptions); ++menuCounter)
	{
	  pp2d_texture_select_part(0, 20, 20 + (menuCounter * 65), 529, (menuCounter * 56) + (menuCounter + 1), 207, 56);
	  pp2d_texture_queue();
	}
      
      // move cursor up and down
      if((hidKeysDown() & KEY_UP) && (cursorPosition > 0)) cursorPosition--;
      if((hidKeysDown() & KEY_DOWN) && (cursorPosition < 2)) cursorPosition++;
      
      if(hidKeysDown() & KEY_START) return 100;
      pp2d_frame_end();
    }
  return 0;
}
