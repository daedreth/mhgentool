#include <3ds.h>
#include <stdio.h>
#include "pp2d.h"
#include "backup.h"
#include "character.h"
#include "color.h"

//#define DEBUG(...) fprintf(stderr, __VA_ARGS__)

#define MENU_SEPARATOR 15

Result drawCharacterMenu()
{
  u32 counter = 0;
  u32 cursorPosition = 0;
  while (aptMainLoop())
    {
      // read inputs
      hidScanInput();

      char* mainMenuOptions[3] = {"Export Character(soon)", "Import Character(soon)", "Delete Character(soon)"};

      //begin a frame. this needs to be called once per frame, not once per screen
      pp2d_frame_begin(GFX_TOP, GFX_LEFT);

      
      pp2d_draw_rectangle(0, 0, PP2D_SCREEN_TOP_WIDTH, PP2D_SCREEN_HEIGHT, COLOR_BLACK);
      for (u8 i = 0; i < 3; ++i) {
	pp2d_draw_rectangle(10, 10 + (i * 80), 380, 60, COLOR_ROSE);
      }		  

      pp2d_draw_textf(37, 30, 0.5f, 0.5f, COLOR_BLACK, "tesss");
      
      if((hidKeysDown() & KEY_UP) && (cursorPosition > 0))  cursorPosition--;      
      if((hidKeysDown() & KEY_DOWN) && (cursorPosition < ((sizeof mainMenuOptions / sizeof *mainMenuOptions) - 1))) cursorPosition++;

      pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);

      for (counter = 0; counter < (sizeof mainMenuOptions / sizeof *mainMenuOptions); ++counter)
	{
	  pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * counter), 0.5f, 0.5f, COLOR_WHITE, "%s", mainMenuOptions[counter]);
	}

      pp2d_draw_text(15, 50 + (MENU_SEPARATOR * cursorPosition), 0.5f, 0.5f, COLOR_WHITE, "->");


      if(hidKeysDown() & KEY_B) return 0;
      if(hidKeysDown() & KEY_START) return 100;
      pp2d_frame_end();
    }
  return 0;
}


Result drawSaveMenu()
{
  u32 counter = 0;
  u32 cursorPosition = 0;
  u32 opSuccess = 0;
  
  while (aptMainLoop())
    {
      // read inputs
      hidScanInput();

      char* backupMenuOptions[3] = {"Backup Save", "Import Save", "Delete Save"};

      //begin a frame. this needs to be called once per frame, not once per screen
      pp2d_frame_begin(GFX_TOP, GFX_LEFT);
      pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);
      if(opSuccess == 2)
	{
	  pp2d_draw_text(37, 50, 0.5f, 0.5f, COLOR_WHITE, "Are you sure?");
	  pp2d_draw_text(37, 65, 0.5f, 0.5f, COLOR_WHITE, "A - Confirm");
	  pp2d_draw_text(37, 80, 0.5f, 0.5f, COLOR_WHITE, "B - Cancel");
	  if(hidKeysDown() & KEY_B) {opSuccess = 0; break;}
	  if(hidKeysDown() & KEY_A)
	    {
	      Result op = deleteSaveAndBackup();
	      if(R_FAILED(op))
		{
		  pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * counter), 0.5f, 0.5f, COLOR_WHITE, "%8lx", op);
		}else{
		opSuccess = 1;
		continue;
	      }
	    }
	}
      if(opSuccess == 1)
	{
	  pp2d_draw_text(37, 50, 0.5f, 0.5f, COLOR_WHITE, "Done!");
	  pp2d_draw_text(37, 65, 0.5f, 0.5f, COLOR_WHITE, "Press B to exit.");
	  if(hidKeysDown() & KEY_B) opSuccess = 0;
	}else if (opSuccess == 0){
	if((hidKeysDown() & KEY_UP) && (cursorPosition > 0))  cursorPosition--;      
	if((hidKeysDown() & KEY_DOWN) && (cursorPosition < ((sizeof backupMenuOptions / sizeof *backupMenuOptions) - 1))) cursorPosition++;

	for (counter = 0; counter < (sizeof backupMenuOptions / sizeof *backupMenuOptions); ++counter)
	  {
	    pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * counter), 0.5f, 0.5f, COLOR_WHITE, "%s", backupMenuOptions[counter]);
	  }

	pp2d_draw_text(15, 50 + (MENU_SEPARATOR * cursorPosition), 0.5f, 0.5f, COLOR_WHITE, "->");

	if((hidKeysDown() & KEY_A) && (cursorPosition == 0))
	  {
	    Result op = dumpSaveAndBackup();
	    if(R_FAILED(op))
	      {
		pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * counter), 0.5f, 0.5f, COLOR_WHITE, "%8lx", op);
	      }else{
	      opSuccess = 1;
	    }
	  }

	if((hidKeysDown() & KEY_A) && (cursorPosition == 1))
	  {
	    Result op = importSaveAndBackup();
	    if(R_FAILED(op))
	      {
		pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * counter), 0.5f, 0.5f, COLOR_WHITE, "%8lx", op);
	      }else{
	      opSuccess = 1;
	    }
	  }
	
	if((hidKeysDown() & KEY_A) && (cursorPosition == 2))
	  {
	    opSuccess = 2;
	    continue;
	  }

	if(hidKeysDown() & KEY_B) return 0;
	if(hidKeysDown() & KEY_START) return 100;
      }
      pp2d_frame_end();
    }
  return 0;
}

Result drawMainMenu()
{
  u32 counter = 0;
  u32 cursorPosition = 0;
  while (aptMainLoop())
    {
      hidScanInput();
      
      char* mainMenuOptions[3] = {"Character(soon)", "Save", "Palico(soon)"};

      //begin a frame. this needs to be called once per frame, not once per screen
      pp2d_frame_begin(GFX_TOP, GFX_LEFT);
      
      if((hidKeysDown() & KEY_UP) && (cursorPosition > 0))  cursorPosition--;      
      if((hidKeysDown() & KEY_DOWN) && (cursorPosition < ((sizeof mainMenuOptions / sizeof *mainMenuOptions) - 1))) cursorPosition++;

      pp2d_frame_draw_on(GFX_BOTTOM, GFX_LEFT);
      pp2d_texture_select_part(0, 0, 0, 0, 0, 320, 240);
      //      pp2d_texture_blend(RGBA8(0xFE, 0xFE, 0xFE, 0xFF));
      pp2d_texture_queue();


      for (counter = 0; counter < (sizeof mainMenuOptions / sizeof *mainMenuOptions); ++counter)
	{
	  pp2d_draw_textf(37, 50 + (MENU_SEPARATOR * counter), 0.5f, 0.5f, COLOR_WHITE, "%s", mainMenuOptions[counter]);
	}

      pp2d_draw_text(15, 50 + (MENU_SEPARATOR * cursorPosition), 0.5f, 0.5f, COLOR_WHITE, "->");

      if((hidKeysDown() & KEY_A) && (cursorPosition == 0)) return 1;
      if((hidKeysDown() & KEY_A) && (cursorPosition == 1)) return 2;

      if(hidKeysDown() & KEY_START) return 100;
      pp2d_frame_end();
    }
  return 0;
}

int main()
{

  romfsInit();
  createMhgenDir();
  cfguInit();
  pp2d_init();

  //  consoleDebugInit(debugDevice_SVC);


    pp2d_set_screen_color(GFX_TOP, ABGR8(255, 10, 10, 10));
  pp2d_set_screen_color(GFX_BOTTOM, ABGR8(255, 10, 10, 10));
  pp2d_load_texture_png(0, "romfs:/botMenu.png");
  
  Result res = 0;
  
  // set the screen background color



  
  while (true)
    {
      if (res == 0) res = drawMainMenu();
      if (res == 1) res = drawCharacterMenu();
      if (res == 2) res = drawSaveMenu();
      if (res == 100) return 0;
    }

  pp2d_free_texture(0);
  // exit pp2d environment
  pp2d_exit();
    
  return 0;
}
