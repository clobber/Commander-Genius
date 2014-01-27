/*
 * CGamePlayMode.cpp
 *
 *  Created on: 26.03.2011
 *      Author: gerstrong
 */

#include "CGamePlayMode.h"
//#include "engine/galaxy/CPlayGameGalaxy.h"
//#include "engine/vorticon/playgame/CPlayGameVorticon.h"
#include "graphics/effects/CColorMerge.h"
#include "sdl/music/CMusicPlayer.h"
#include <base/GsTimer.h>
#include <base/video/CVideoDriver.h>
#include <base/GsApp.h>
#include <memory>

CGamePlayMode::CGamePlayMode(const int Episode,
        const std::string& DataDirectory,
        const int startLevel) :
m_startLevel(startLevel),
m_Episode(Episode),
m_DataDirectory(DataDirectory)
{}

CGamePlayMode::CGamePlayMode( GMSwitchToPlayGameMode &gpmode ) :
m_startLevel(gpmode.m_startlevel),
m_Episode(gpmode.m_Episode),
m_DataDirectory(gpmode.m_DataDirectory)
{}

void CGamePlayMode::loadGame()
{
    mp_PlayGame->ponder(0);

    if(mp_PlayGame->loadXMLGameState())
        return;

    mp_PlayGame->loadGameState();
}

void CGamePlayMode::pumpEvent(const CEvent *evPtr)
{
    mp_PlayGame->pumpEvent(evPtr);
}

void CGamePlayMode::ponder(const float deltaT)
{
	// The player is playing the game. It also includes scenes like ending
    CEventContainer& EventContainer = gEventManager;

    mp_PlayGame->ponder(deltaT);
	
	if( EventContainer.occurredEvent<SaveGameEvent>() )
	{
        mp_PlayGame->saveXMLGameState();
        EventContainer.pop_Event();
	}


	if( mp_PlayGame->getEndGame() )
	{
		m_startLevel = 0;
        EventContainer.add( new GMSwitchToPassiveMode(/*m_DataDirectory, m_Episode*/) );
	}
	else if( mp_PlayGame->getStartGame() )
	{ // Start another new game
        EventContainer.add( new GMSwitchToPlayGameMode(m_Episode, g_pBehaviorEngine->mPlayers, m_DataDirectory) );
	}
	else if( mp_PlayGame->getExitEvent() )
	{
		EventContainer.add( new GMQuit() );
	}

}

void CGamePlayMode::render()
{
    mp_PlayGame->render();

    if(g_pBehaviorEngine->m_option[OPT_SHOWFPS].value)
    {
        SDL_Rect rect;
        rect.x = 5;
        rect.y = 5;
        rect.w = 150;
        rect.h = 10;

        if(!mpFPSSurface)
        {
            mpFPSSurface.reset(CG_CreateRGBSurface(rect), &SDL_FreeSurface);
        }

        std::string tempbuf = "FPS: " + ftoa(gTimer.LastFPS());
        SDL_FillRect(mpFPSSurface.get(),NULL,0x88888888);
        //gGraphics.getFont(1).drawFont(mpFPSSurface.get(), tempbuf, 1, 1, false);

        SDL_BlitSurface(mpFPSSurface.get(), NULL, gVideoDriver.getBlitSurface(), &rect);
    }
}

