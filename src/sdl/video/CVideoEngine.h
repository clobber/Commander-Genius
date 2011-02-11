/*
 * CVideoEngine.h
 *
 *  Created on: 05.02.2011
 *      Author: gerhard
 *
 *  This is a base class which will process the video settings on your system.
 *  CVideoDriver calls that Engine through an overloaded Extra class like COpenGL or CSDLVideo
 */

#ifndef CVIDEOENGINE_H_
#define CVIDEOENGINE_H_

#include <SDL.h>
#include <string>

#include "scale2x/scalebit.h"
#include "sdl/CVidConfig.h"

class CVideoEngine {
public:

	CVideoEngine(const CVidConfig& VidConfig);

	virtual bool init();
	virtual void updateScreen() = 0;
	virtual void shutdown();

	void setScrollBuffer(Sint16 *pbufx, Sint16 *pbufy);
	SDL_Surface *createSurface( std::string name, bool alpha, int width, int height, int bpp, int mode, SDL_PixelFormat* format );
	virtual bool createSurfaces() = 0;
	unsigned char *fetchStartScreenPixelPtr();
	virtual void collectSurfaces() = 0;
	void blitScrollSurface();
	void stop();

	SDL_Surface *getBlitSurface() { return BlitSurface; }
	SDL_Surface *getScreenSurface() { return screen; }
	SDL_Surface *getFGLayerSurface() { return FGLayerSurface; }
	SDL_Surface *getScrollSurface() { return ScrollSurface; }
	SDL_Surface *getFXSurface() { return FXSurface; }

protected:

	void scale2xnofilter(char *dest, char *src, short bbp);
	void scale3xnofilter(char *dest, char *src, short bbp);
	void scale4xnofilter(char *dest, char *src, short bbp);

	SDL_Surface *BlitSurface;
	SDL_Surface *FGLayerSurface;       	// Scroll buffer for Messages
	SDL_Surface *ScrollSurface;       	// 512x512 scroll buffer
	SDL_Surface *FXSurface;

	bool m_blitsurface_alloc;

	const CVidConfig &m_VidConfig;

	Sint16 *mp_sbufferx, *mp_sbuffery;

	SDL_Surface *screen;                // the actual video memory/window

	// Those variables are used for the rendering process, so they don't need to be recalculated
	unsigned m_dst_slice, m_src_slice;

	unsigned int m_Mode;
};

#endif /* CVIDEOENGINE_H_ */