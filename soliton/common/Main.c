/* Soliton cardgames for Amiga computers
 * Copyright (C) 1997-2002 Kai Nickel and other authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include <clib/alib_protos.h>
#include <exec/types.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/muimaster.h>

/* Libraries which are declared here */
#define __NOLIBBASE__
#include <proto/intuition.h>

#include "About.h"
#include "BoardWindow.h"
#include "Cardgame.h"
#include "CButton.h"
#include "CSolitaire.h"
#include "Locales.h"
#include "MUITools.h"
#include "ProfileManager.h"
#include "Settings.h"
#include "Soliton.h"
#include "Statistics.h"

#include "SDI_compiler.h"
#include "SDI_iface.h"

#if defined(__MORPHOS__)
unsigned long __stack = 20000*2;
#elif defined(__SASC)
__near LONG __stack = 20000;
#elif defined(__VBCC)
LONG _stack = 20000;
#endif

char *APPNAME = "Soliton";

/****************************************************************************************
  Init- / ExitLibs
****************************************************************************************/

struct Library *MUIMasterBase = NULL;
struct Library *DataTypesBase = NULL;
struct Library *LayersBase = NULL;
struct UtilityBase *UtilityBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

EXEC_INTERFACE_DECLARE(struct IntuitionIFace *IIntuition);
EXEC_INTERFACE_DECLARE(struct GraphicsIFace *IGraphics);
EXEC_INTERFACE_DECLARE(struct UtilityIFace *IUtility);
EXEC_INTERFACE_DECLARE(struct LayersIFace *ILayers);
EXEC_INTERFACE_DECLARE(struct MUIMasterIFace *IMUIMaster);
EXEC_INTERFACE_DECLARE(struct DataTypesIFace *IDataTypes);

static BOOL InitAll(void)
{
	BOOL rc;

  rc = FALSE;

  InitLocale();

  if (!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 38)))
  {
		ErrorReq(MSG_OPENLIB_INTUITION);
  	return FALSE;
  }

	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 17)))
	{
		ErrorReq(MSG_OPENLIB_MUIMASTER);
		return FALSE;
	}
	
	if (!(UtilityBase = (struct UtilityBase *) OpenLibrary("utility.library", 38)))
	{
		ErrorReq(MSG_OPENLIB_UTILITY);
		return FALSE;
	}
	
	if (!(DataTypesBase = OpenLibrary("datatypes.library", 38)))
	{
		ErrorReq(MSG_OPENLIB_DATATYPES);
		return FALSE;
	}
	
	if (!(LayersBase = OpenLibrary("layers.library", 38)))
	{
		ErrorReq(MSG_OPENLIB_LAYERS);
		return FALSE;
	}

	if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 38)))
	{
		ErrorReq(MSG_OPENLIB_GRAPHICS);
		return FALSE;
	}

  if (!EXEC_INTERFACE_GET_MAIN(IIntuition,IntuitionBase) ||
      !EXEC_INTERFACE_GET_MAIN(IMUIMaster,MUIMasterBase) ||
      !EXEC_INTERFACE_GET_MAIN(IUtility,UtilityBase) ||
      !EXEC_INTERFACE_GET_MAIN(IDataTypes,DataTypesBase) ||
      !EXEC_INTERFACE_GET_MAIN(ILayers,LayersBase) ||
      !EXEC_INTERFACE_GET_MAIN(IGraphics,GfxBase))
 		return FALSE;

  if (NoneSlider_Init() && Cardgame_Init() && About_Init() && Statistics_Init() &&
      BoardWindow_Init() && Settings_Init() && Soliton_Init() && CButton_Init() &&
      CSolitaire_Init() && ProfileManager_Init())
  {
		return TRUE;
  }

  return FALSE;
}

static void ExitAll(void)
{
  ProfileManager_Exit();
  CSolitaire_Exit();
  CButton_Exit();
  Soliton_Exit();
  Settings_Exit();
  BoardWindow_Exit();
  Statistics_Exit();
  About_Exit();
  Cardgame_Exit();
  NoneSlider_Exit();

  EXEC_INTERFACE_DROP(IGraphics);
  EXEC_INTERFACE_DROP(ILayers);
  EXEC_INTERFACE_DROP(IDataTypes);
  EXEC_INTERFACE_DROP(IUtility);
  EXEC_INTERFACE_DROP(IMUIMaster);
  EXEC_INTERFACE_DROP(IIntuition);

  CloseLibrary((struct Library *) GfxBase);
  CloseLibrary(LayersBase);
  CloseLibrary(DataTypesBase);
  CloseLibrary((struct Library *) UtilityBase);
  CloseLibrary(MUIMasterBase);
  CloseLibrary((struct Library *) IntuitionBase);

  ExitLocale();
}

/****************************************************************************************
  main
****************************************************************************************/

#if defined(__STORMGCC__) || defined(__STORM__)
void wbmain(struct WBStartup *wbmsg)
{
	char *args[2], game[256];
	args[0] = wbmsg->sm_ArgList[0].wa_Name;
	args[1] = game;

	CurrentDir(wbmsg->sm_ArgList[0].wa_Lock);

	main(2,args);
}
#endif

int main(int argc, char **argv)
{
  if(InitAll())
  {
    Object* soliton = (Object*)NewObject(CL_Soliton->mcc_Class, NULL, TAG_DONE);
    if(soliton)
    {
      ULONG sigs = 0;
      while(DoMethod(soliton, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
      {
        if(sigs)
        {
          sigs = Wait(sigs | SIGBREAKF_CTRL_C);
          if(sigs & SIGBREAKF_CTRL_C)
            break;
        }
      }
      MUI_DisposeObject(soliton);
    }
    else
      ErrorReq(MSG_CREATE_APPLICATION);
  }
  ExitAll();
  return 0;
}
