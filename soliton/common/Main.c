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

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

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

#ifdef __SASC
__near LONG __stack = 20000;
#elif defined(__VBCC)
LONG _stack = 20000;
#endif

char *VERSION_STRING = "$VER: Soliton 2.1 (29.03.2002)",
     *VERSION_NUMBER = "2.1",
     *VERSION_DATE   = "2002-03-29",
     *APPNAME        = "Soliton";

/****************************************************************************************
  Init- / ExitLibs
****************************************************************************************/

struct Library *MUIMasterBase = NULL;
struct Library *DataTypesBase = NULL;
struct Library *LayersBase = NULL;
struct UtilityBase *UtilityBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

static BOOL InitAll(void)
{
  InitLocale();

  if((IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 38)))
  {
    if((MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 17)))
    {
      if((UtilityBase = (struct UtilityBase *) OpenLibrary("utility.library", 38)))
      {
        if((DataTypesBase = OpenLibrary("datatypes.library", 38)))
        {
          if((LayersBase = OpenLibrary("layers.library", 38)))
          {
            if((GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 38)))
            {
              if(NoneSlider_Init() &&
              Cardgame_Init() &&
              About_Init() &&
              Statistics_Init() &&
              BoardWindow_Init() &&
              Settings_Init() &&
              Soliton_Init() &&
              CButton_Init() &&
              CSolitaire_Init() &&
              ProfileManager_Init())
              {
                return TRUE;
              }
            }
            else
              ErrorReq(MSG_OPENLIB_GRAPHICS);
          }
          else
            ErrorReq(MSG_OPENLIB_LAYERS);
        }
        else
          ErrorReq(MSG_OPENLIB_DATATYPES);
      }
      else
        ErrorReq(MSG_OPENLIB_UTILITY);
    }
    else
      ErrorReq(MSG_OPENLIB_MUIMASTER);
  }
  else
    ErrorReq(MSG_OPENLIB_INTUITION);

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

  if(GfxBase)
    CloseLibrary((struct Library *) GfxBase);
  if(LayersBase)
    CloseLibrary(LayersBase);
  if(DataTypesBase)
    CloseLibrary(DataTypesBase);
  if(UtilityBase)
    CloseLibrary((struct Library *) UtilityBase);
  if(MUIMasterBase)
    CloseLibrary(MUIMasterBase);
  if(IntuitionBase)
    CloseLibrary((struct Library *) IntuitionBase);

  ExitLocale();
}

/****************************************************************************************
  main
****************************************************************************************/

void main(void)
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
}
