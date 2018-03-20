#include <3ds.h>
#include "colors.h"
#include "pp2d.h"
#include "menu.h"

Result MHGEN_drawMainMenu()
{
  u32 cursorPosition = 0;
  while (aptMainLoop())
    {
      hidScanInput();
      
      char* mainMenuOptions[3] = {"Character(soon)", "Save(soon)", "Palico(soon)"};

      pp2d_frame_begin(GFX_TOP, GFX_LEFT);

      pp2d_frame_draw_on(GFX_TOP, GFX_LEFT);
      pp2d_texture_select_part(0, 0, 0, 0, 240, 400, 240);
      pp2d_texture_queue();

      pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);
      pp2d_texture_select_part(0, 0, 0, 0, 0, 320, 240);
      pp2d_texture_queue();
      
      for (u8 menuCounter = 0; menuCounter < (sizeof mainMenuOptions / sizeof *mainMenuOptions); ++menuCounter)
	pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * menuCounter), 0.5f, 0.5f, COLOR_WHITE, "%s", mainMenuOptions[menuCounter]);

      pp2d_draw_text(15, 50 + (MENU_SEPARATOR * cursorPosition), 0.5f, 0.5f, COLOR_WHITE, "->");

      if(hidKeysDown() & KEY_START) return 100;
      pp2d_frame_end();
    }
  return 0;
}
