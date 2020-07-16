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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_MOVIE_H
#define DIRECTOR_MOVIE_H

namespace Common {
class ReadStreamEndian;
class SeekableSubReadStreamEndian;
}

namespace Director {

class Archive;
class Cast;
struct CastMemberInfo;
class CastMember;
class DirectorEngine;
class Lingo;
struct LingoArchive;
class ScriptContext;
class Stage;
struct Symbol;

struct InfoEntry {
	uint32 len;
	byte *data;

	InfoEntry() { len = 0; data = nullptr; }

	~InfoEntry() {
		free(data);
	}

	Common::String readString(bool pascal = true) {
		Common::String res;

		if (len == 0)
			return res;

		int start = pascal ? 1 : 0;

		for (uint i = start; i < len; i++)
			if (data[i] == '\r')
				res += '\n';
			else if (data[i] >= 0x20)
				res += data[i];

		return res;
	}
};

struct InfoEntries {
	uint32 unk1;
	uint32 unk2;
	uint32 flags;
	uint32 scriptId;
	Common::Array<InfoEntry> strings;

	InfoEntries() : unk1(0), unk2(0), flags(0), scriptId(0) {}
};

class Movie {
public:
	Movie(Stage *stage);
	~Movie();

	static Common::Rect readRect(Common::ReadStreamEndian &stream);
	static InfoEntries loadInfoEntries(Common::SeekableSubReadStreamEndian &stream);

	bool loadArchive();
	void setArchive(Archive *archive);
	Archive *getArchive() const { return _movieArchive; };
	Common::String getMacName() const { return _macName; }
	Stage *getCurrentStage() const { return _stage; }
	DirectorEngine *getVM() const { return _vm; }
	Cast *getCast() const { return _cast; }
	Cast *getSharedCast() const { return _sharedCast; }
	Score *getScore() const { return _score; }

	void clearSharedCast();
	void loadSharedCastsFrom(Common::String filename);

	CastMember *getCastMember(int castId);
	CastMember *getCastMemberByName(const Common::String &name);
	CastMemberInfo *getCastMemberInfo(int castId);
	const Stxt *getStxt(int castId);

	LingoArchive *getMainLingoArch();
	LingoArchive *getSharedLingoArch();
	ScriptContext *getScriptContext(ScriptType type, uint16 id);
	Symbol getHandler(const Common::String &name);

private:
	void loadFileInfo(Common::SeekableSubReadStreamEndian &stream);

public:
	Archive *_movieArchive;
	Common::Rect _movieRect;
	uint16 _currentMouseDownSpriteId;
	uint16 _currentClickOnSpriteId;
	uint32 _lastEventTime;
	uint32 _lastRollTime;
	uint32 _lastClickTime;
	Common::Point _lastClickPos;
	uint32 _lastKeyTime;
	uint32 _lastTimerReset;
	uint16 _stageColor;
	Cast *_sharedCast;
	bool _allowOutdatedLingo;

private:
	Stage *_stage;
	DirectorEngine *_vm;
	Lingo *_lingo;
	Cast *_cast;
	Score *_score;

	uint32 _flags;

	Common::String _macName;
	Common::String _createdBy;
	Common::String _changedBy;
	Common::String _script;
	Common::String _directory;
};

} // End of namespace Director

#endif
