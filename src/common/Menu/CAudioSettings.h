/*
 * CAudioSettings.h
 *
 *  Created on: 28.11.2009
 *      Author: gerstrong
 */

#ifndef CAUDIOSETTINGS_H_
#define CAUDIOSETTINGS_H_

#include "CBaseMenu.h"
#include "CVolumeSettings.h"
#include "fileio/CExeFile.h"
#include <SDL.h>
#include <string>

#include "gui/CGUIComboSelection.h"
#include "gui/CGUISwitch.h"
#include "gui/CGUINumberControl.h"


class CAudioSettings : public CBaseMenu
{
public:
	CAudioSettings(Uint8 dlg_theme);

	void init();

	void process();

	void release();


private:

	CGUIComboSelection	*mpRate;
	CGUISwitch			*mpStereo;
	CGUIComboSelection	*mpDepth;
	CGUIComboSelection	*mpSBToggle;
	CGUINumberControl	*mpSoundVolume;
	CGUINumberControl	*mpMusicVolume;

	SDL_AudioSpec	mAudioSpec;
	bool mSoundblaster;
};

#endif /* CAUDIOSETTINGS_H_ */
