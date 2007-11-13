/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifdef IPHONE_BACKEND

#include "graphics/surface.h"

// #include <AudioToolbox/AudioQueue.h>
// 
// /* Audio Resources */
// #define AUDIO_BUFFERS 3
// #define AUDIO_PRECACHE 4
// #define WAVE_BUFFER_SIZE 735
// #define WAVE_BUFFER_BANKS 25
// 
// typedef struct AQCallbackStruct {
//     AudioQueueRef queue;
//     UInt32 frameCount;
//     AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
//     AudioStreamBasicDescription mDataFormat;
// } AQCallbackStruct;

class OSystem_IPHONE : public OSystem {
protected:
	static const OSystem::GraphicsMode s_supportedGraphicsModes[];
	
	Common::SaveFileManager *_savefile;
	Audio::Mixer *_mixer;
	Common::TimerManager *_timer;

	Graphics::Surface _framebuffer;
	byte *_offscreen;
	OverlayColor  *_overlayBuffer;
	uint16 *_fullscreen;

	uint16  _palette[256];
	bool _overlayVisible;
	uint16 _screenWidth;
	uint16 _screenHeight;

	struct timeval _startTime;
	
	bool _mouseVisible;
	byte *_mouseBuf;
	byte _mouseKeyColour;
	uint _mouseWidth, _mouseHeight;
	int	_mouseX, _mouseY;
	int	_mouseHotspotX, _mouseHotspotY;
	long _lastMouseDown;
	long _lastMouseTap;
	Common::Event _queuedInputEvent;
	bool _secondaryTapped;
	long _lastSecondaryDown;
	long _lastSecondaryTap;
	int _gestureStartX, _gestureStartY;

public:

	OSystem_IPHONE();
	virtual ~OSystem_IPHONE();

	virtual void initBackend();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	bool setGraphicsMode(const char *name);
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();

	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int cursorTargetScale = 1);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	typedef void (*SoundProc)(void *param, byte *buf, int len);
	virtual bool setSoundCallback(SoundProc proc, void *param);
	virtual void clearSoundCallback();
	virtual int getOutputSampleRate() const;

	virtual void quit();

	virtual void setWindowCaption(const char *caption);

	virtual Common::SaveFileManager *getSavefileManager();
	virtual Audio::Mixer *getMixer();
	virtual Common::TimerManager *getTimerManager();
};

#endif
