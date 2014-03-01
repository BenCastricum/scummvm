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
 */

#ifndef MADS_SOUND_NEBULAR_H
#define MADS_SOUND_NEBULAR_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"

namespace MADS {

class SoundManager;

namespace Nebular {

/**
 * Represents the data for a channel on the Adlib
 */
class AdlibChannel {
public:
	int _activeCount;
	int _field1;
	int _field2;
	int _field3;
	int _field4;
	int _sampleIndex;
	int _volume;
	int _field7;
	int _field8;
	int _field9;
	int _fieldA;
	uint8 _fieldB;
	int _fieldC;
	int _fieldD;
	int _fieldE;
	byte *_ptr1;
	byte *_pSrc;
	byte *_ptr3;
	byte *_ptr4;
	int _field17;
	int _field19;
	byte *_soundData;
	int _field1D;
	int _field1E;
	int _field1F;
public:
	AdlibChannel();

	void reset();
	void enable(int flag);
	void setPtr2(byte *pData);
	void load(byte *pData);
	void check(byte *nullPtr);
};

class AdlibChannelData {
public:
	int _field0;
	int _freqMask;
	int _freqBase;
	int _field6;
};

class AdlibSample {
public:
	int _attackRate;
	int _decayRate;
	int _sustainLevel;
	int _releaseRate;
	bool _egTyp;
	bool _ksr;
	int _totalLevel;
	int _scalingLevel;
	int _waveformSelect;
	int _freqMultiple;
	int _feedback;
	bool _ampMod;
	int _vib;
	int _alg;
	int _fieldE;
	int _freqMask;
	int _freqBase;
	int _field14;

	AdlibSample() {}
	AdlibSample(Common::SeekableReadStream &s);
};

struct RegisterValue {
	uint8 _regNum;
	uint8 _value;

	RegisterValue(int regNum, int value) {
		_regNum = regNum; _value = value;
	}
};

#define ADLIB_CHANNEL_COUNT 9
#define ADLIB_CHANNEL_MIDWAY 5
#define CALLBACKS_PER_SECOND 60

/**
 * Base class for the sound player resource files
 */
class ASound: public Audio::AudioStream {
private:
	struct CachedDataEntry {
		int _offset;
		byte *_data;
	};
	Common::List<CachedDataEntry> _dataCache;
	uint16 _randomSeed;

	/**
	 * Does the initial Adlib initialisation
	 */
	void adlibInit();

	/**
	 * Does on-going processing for the Adlib sounds being played
	 */
	void update();

	/**
	 * Polls each of the channels for updates
	 */
	void pollChannels();

	/**
	 * Checks the status of the channels
	 */
	void checkChannels();

	/**
	 * Polls the currently active channel
	 */
	void pollActiveChannel();

	/**
	 * Updates the octave of the currently active channel
	 */
	void updateOctave();

	void updateChannelState();
	void updateActiveChannel();
	
	/**
	 * Loads up the specified sample
	 */
	void loadSample(int sampleIndex);

	/**
	 * Writes out the data of the selected sample to the Adlib
	 */
	void processSample();

	void updateFNumber();
protected:
	/**
	 * Queue a byte for an Adlib register
	 */
	void write(int reg, int val);

	/**
	 * Queue a byte for an Adlib register, and store it in the _ports array
	 */
	int write2(int state, int reg, int val);

	/**
	 * Flush any pending Adlib register values to the OPL driver
	 */
	void flush();

	/**
	 * Turn a channel on
	 */
	void channelOn(int reg, int volume);

	/**
	 * Turn a channel off
	 */
	void channelOff(int reg);

	/**
	 * Checks for whether a poll result needs to be set
	 */
	void resultCheck();

	/**
	 * Loads a data block from the sound file, caching the result for any future
	 * calls for the same data
	 */
	byte *loadData(int offset, int size);

	/**
	 * Play the specified sound
	 * @param offset	Offset of sound data within sound player data segment
	 * @param size		Size of sound data block
	 */
	void playSound(int offset, int size);

	/**
	 * Play the specified raw sound data
	 * @param pData		Pointer to data block containing sound data
	 * @param startingChannel	Channel to start scan from
	 */
	void playSoundData(byte *pData, int startingChannel = ADLIB_CHANNEL_MIDWAY);

	/**
	 * Checks to see whether the given block of data is already loaded into a channel.
	 */
	bool isSoundActive(byte *pData);

	/**
	 * Sets the frequency for a given channel.
	 */
	void setFrequency(int channel, int freq);

	/**
	 * Returns a 16-bit random number
	 */
	int getRandomNumber();

	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();
public:
	Audio::Mixer *_mixer;
	FM_OPL *_opl;
	Audio::SoundHandle _soundHandle;
	AdlibChannel _channels[ADLIB_CHANNEL_COUNT];
	AdlibChannel *_activeChannelPtr;
	AdlibChannelData _channelData[11];
	Common::Array<AdlibSample> _samples;
	AdlibSample *_samplePtr;
	Common::File _soundFile;
	Common::Queue<RegisterValue> _queue;
	Common::Mutex _driverMutex;
	int _dataOffset;
	int _frameCounter;
	bool _isDisabled;
	int _v1;
	int _v2;
	int _activeChannelNumber;
	int _freqMask1;
	int _freqMask2;
	int _freqBase1;
	int _freqBase2;
	int _channelNum1, _channelNum2;
	int _v7;
	int _v8;
	int _v9;
	int _v10;
	int _pollResult;
	int _resultFlag;
	byte _nullData[2];
	int _ports[256];
	bool _stateFlag;
	int _activeChannelReg;
	int _v11;
	bool _amDep, _vibDep, _splitPoint;
	int _samplesPerCallback;
	int _samplesPerCallbackRemainder;
	int _samplesTillCallback;
	int _samplesTillCallbackRemainder;
public:
	/**
	 * Constructor
	 * @param filename		Specifies the adlib sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 */
	ASound(Audio::Mixer *mixer, const Common::String &filename, int dataOffset);

	/**
	 * Destructor
	 */
	virtual ~ASound();

	/**
	 * Execute a player command. Most commands represent sounds to play, but some
	 * low number commands also provide control operations.
	 * @param commandId		Player ommand to execute.
	 * @param param			Optional parameter used by a few commands
	 */
	virtual int command(int commandId, int param) = 0;

	/**
	 * Stop all currently playing sounds
	 */
	int stop();

	/**
	 * Main poll method to allow sounds to progress
	 */
	int poll();

	/**
	 * General noise/note output
	 */
	void noise();

	/**
	 * Return the current frame counter
	 */
	int getFrameCounter() { return _frameCounter; }

	// AudioStream interface
	/**
	 * Main buffer read
	 */
	virtual int readBuffer(int16 *buffer, const int numSamples);
	
	/**
	 * Mono sound only
	 */
	virtual bool isStereo() const { return false; }
	
	/**
	 * Data is continuously pushed, so definitive end
	 */
	virtual bool endOfData() const { return false; }

	/**
	 * Return sample rate
	 */
	virtual int getRate() const { return 11025; }
};

class ASound1: public ASound {
private:
	typedef int (ASound1::*CommandPtr)();
	static const CommandPtr _commandList[42];
	bool _cmd23Toggle;
	int _commandParam;

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();

	void command111213();
	int command2627293032();
public:
	ASound1(Audio::Mixer *mixer);

	virtual int command(int commandId, int param);
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_SOUND_NEBULAR_H */
