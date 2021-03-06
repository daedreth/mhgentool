#include <3ds.h>
#include "colors.h"
#include "pp2d.h"
#include "menu.h"

Result MHGEN_drawMainMenu()
{
  u8 cursorPosition = 1;
  
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
	  pp2d_texture_select_part(0, 17, 20 + (menuCounter * 65), 529, (menuCounter * 56) + (menuCounter + 1), 207, 56);
	  pp2d_texture_queue();
	}

      // strikethrough, these get removed as options are added
      pp2d_texture_select_part(0, 12, 20 , 321, 114, 207, 56);
      pp2d_texture_queue();
      pp2d_texture_select_part(0, 12, 150, 321, 114, 207, 56);
      pp2d_texture_queue();

      // help option
      pp2d_texture_select_part(0, 230, 120 , 401, 240, 79, 84);
      pp2d_texture_queue();
      
      // move cursor up and down
      if((hidKeysDown() & KEY_UP) && (cursorPosition > 1)) cursorPosition--;
      if((hidKeysDown() & KEY_DOWN) && (cursorPosition < 3)) cursorPosition++;

      if(hidKeysDown() & KEY_A)
	{
	  if(cursorPosition == 2)
	    {
	      return 2;
	    }
	}
      
      if(hidKeysDown() & KEY_START) return 100;
      pp2d_frame_end();
    }
  return 0;
}
