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

#include "fullpipe/fullpipe.h"

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"
#include "common/memstream.h"

#include "graphics/thumbnail.h"

#include "fullpipe/objects.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"

#include "fullpipe/constants.h"

namespace Fullpipe {

void GameLoader::readSavegame(const char *fname) {
	SaveHeader header;
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(fname);

	if (!saveFile) {
		warning("Cannot open save %s for loading", fname);
		return;
	}

	header.version = saveFile->readUint32LE();
	saveFile->read(header.magic, 32);
	header.updateCounter = saveFile->readUint32LE();
	header.unkField = saveFile->readUint32LE();
	header.encSize = saveFile->readUint32LE();

	debugC(3, kDebugLoading, "version: %d magic: %s updateCounter: %d unkField: %d encSize: %d, pos: %d",
			header.version, header.magic, header.updateCounter, header.unkField, header.encSize, saveFile->pos());

	if (header.version != 48)
		return;

	_updateCounter = header.updateCounter;

	byte *data = (byte *)malloc(header.encSize);
	saveFile->read(data, header.encSize);

	byte *map = (byte *)malloc(800);
	saveFile->read(map, 800);

	MfcArchive temp(new Common::MemoryReadStream(map, 800));

	if (_savegameCallback)
		_savegameCallback(&temp, false);

	delete saveFile;

	// Deobfuscate the data
	for (uint i = 0; i < header.encSize; i++)
		data[i] -= i & 0x7f;

	MfcArchive *archive = new MfcArchive(new Common::MemoryReadStream(data, header.encSize));

	GameVar *var = (GameVar *)archive->readClass();

	GameVar *v = _gameVar->getSubVarByName("OBJSTATES");

	if (!v) {
		v = _gameVar->addSubVarAsInt("OBJSTATES", 0);

		if (!v) {
			warning("No state to save");
			delete archive;
			return;
		}
	}

	addVar(var, v);

	getGameLoaderInventory()->loadPartial(*archive);

	int32 arrSize = archive->readUint32LE();

	debugC(3, kDebugLoading, "Reading %d infos", arrSize);

	for (uint i = 0; i < arrSize; i++) {
		_sc2array[i]._picAniInfosCount = archive->readUint32LE();

		if (_sc2array[i]._picAniInfosCount)
			debugC(3, kDebugLoading, "Count %d: %d", i, _sc2array[i]._picAniInfosCount);

		free(_sc2array[i]._picAniInfos);
		_sc2array[i]._picAniInfos = (PicAniInfo **)malloc(sizeof(PicAniInfo *) * _sc2array[i]._picAniInfosCount);

		for (uint j = 0; j < _sc2array[i]._picAniInfosCount; j++) {
			_sc2array[i]._picAniInfos[j] = new PicAniInfo();
			_sc2array[i]._picAniInfos[j]->load(*archive);
		}
	}

	int num = 2;
	for (int k = 0; k < _sc2array[num]._picAniInfosCount; k++) {
		debugC(4, kDebugLoading, "num: %d", k);
		debugC(4, kDebugLoading, "type: %d id: %d", _sc2array[num]._picAniInfos[k]->type, _sc2array[num]._picAniInfos[k]->objectId);
		debugC(4, kDebugLoading, "staticsId: %d movid: %d movphase: %d", _sc2array[num]._picAniInfos[k]->staticsId,
				_sc2array[num]._picAniInfos[k]->movementId, _sc2array[num]._picAniInfos[k]->dynamicPhaseIndex);
	}

	delete archive;

	getGameLoaderInventory()->rebuildItemRects();

	PreloadItem preloadItem;

	v = _gameVar->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");

	if (v) {
		if (g_fp->_currentScene)
			preloadItem.preloadId1 = g_fp->_currentScene->_sceneId & 0xffff;
		else
			preloadItem.preloadId1 = 0;

		preloadItem.param = v->getSubVarAsInt("Entrance");
		preloadItem.preloadId2 = 0;
		preloadItem.sceneId = v->getSubVarAsInt("Scene");

		if (_preloadCallback) {
			if (!_preloadCallback(preloadItem, 0))
				return;
		}

		clearGlobalMessageQueueList1();

		if (g_fp->_currentScene)
			unloadScene(g_fp->_currentScene->_sceneId);

		g_fp->_currentScene = 0;

		if (_preloadCallback)
			_preloadCallback(preloadItem, 50);

		loadScene(preloadItem.sceneId);

		ExCommand *ex = new ExCommand(preloadItem.sceneId, 17, 62, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags = 2;
		ex->_param = preloadItem.param;

		if (_preloadCallback)
			_preloadCallback(preloadItem, 100);

		ex->postMessage();
	}
}

void parseSavegameHeader(Fullpipe::FullpipeSavegameHeader &header, SaveStateDescriptor &desc) {
	int day = (header.date >> 24) & 0xFF;
	int month = (header.date >> 16) & 0xFF;
	int year = header.date & 0xFFFF;
	desc.setSaveDate(year, month, day);
	int hour = (header.time >> 8) & 0xFF;
	int minutes = header.time & 0xFF;
	desc.setSaveTime(hour, minutes);
	desc.setPlayTime(header.playtime * 1000);

	desc.setDescription(header.saveName);
}

void fillDummyHeader(Fullpipe::FullpipeSavegameHeader &header) {
	// This is wrong header, perhaps it is original savegame. Thus fill out dummy values
	header.date = (20 << 24) | (9 << 16) | 2016;
	header.time = (9 << 8) | 56;
	header.playtime = 1000;
}

bool readSavegameHeader(Common::InSaveFile *in, FullpipeSavegameHeader &header) {
	header.thumbnail = NULL;

	uint oldPos = in->pos();

	in->seek(-4, SEEK_END);

	uint headerOffset = in->readUint32LE();

	// Sanity check
	if (headerOffset >= in->pos() || headerOffset == 0) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	in->seek(headerOffset, SEEK_SET);

	in->read(header.id, 6);

	// Validate the header Id
	if (strcmp(header.id, "SVMCR")) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	header.version = in->readByte();
	if (header.version != FULLPIPE_SAVEGAME_VERSION) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	header.date = in->readUint32LE();
	header.time = in->readUint16LE();
	header.playtime = in->readUint32LE();

	// Generate savename
	SaveStateDescriptor desc;

	parseSavegameHeader(header, desc);
	header.saveName = Common::String::format("%s %s", desc.getSaveDate().c_str(), desc.getSaveTime().c_str());

	// Get the thumbnail
	header.thumbnail = Graphics::loadThumbnail(*in);

	in->seek(oldPos, SEEK_SET); // Rewind the file

	if (!header.thumbnail)
		return false;

	return true;
}

void GameLoader::addVar(GameVar *var, GameVar *subvar) {
	if (var && subvar) {
		int type = var->_varType;
		if (type == subvar->_varType && (!type || type == 1))
			subvar->_value.intValue = var->_value.intValue;

		for (GameVar *v = var->_subVars; v; v = v->_nextVarObj) {
			GameVar *nv = subvar->getSubVarByName(v->_varName);
			if (!nv) {
				nv = new GameVar;
				nv->_varName = (char *)calloc(strlen(v->_varName) + 1, 1);
				strcpy(nv->_varName, v->_varName);
				nv->_varType = v->_varType;

				subvar->addSubVar(nv);
			}

			addVar(v, nv);
		}
	}
}

void gameLoaderSavegameCallback(MfcArchive *archive, bool mode) {
	if (mode)
		for (int i = 0; i < 200; i++)
			archive->writeUint32LE(g_fp->_mapTable[i]);
	else
		for (int i = 0; i < 200; i++)
			g_fp->_mapTable[i] = archive->readUint32LE();
}

bool FullpipeEngine::loadGam(const char *fname, int scene) {
	_gameLoader = new GameLoader();

	if (!_gameLoader->loadFile(fname))
		return false;

	_currSoundListCount = 0;
	initObjectStates();
	// set_g_messageQueueCallback1(messageQueueCallback1); // substituted with direct call

	addMessageHandlerByIndex(global_messageHandler1, 0, 4);

	_inventory = getGameLoaderInventory();
	_inventory->setItemFlags(ANI_INV_MAP, 0x10003);
	_inventory->addItem(ANI_INV_MAP, 1);

	_inventory->rebuildItemRects();

	for (uint i = 0; i < _inventory->getScene()->_picObjList.size(); i++)
		((MemoryObject *)_inventory->getScene()->_picObjList[i]->_picture)->load();

	// _sceneSwitcher = sceneSwitcher; // substituted with direct call
	_gameLoader->_preloadCallback = preloadCallback;
	_gameLoader->_savegameCallback = gameLoaderSavegameCallback;

	_aniMan = accessScene(SC_COMMON)->getAniMan();
	_scene2 = 0;

	_movTable = _aniMan->countMovements();

	_aniMan->setSpeed(1);

	PictureObject *pic = accessScene(SC_INV)->getPictureObjectById(PIC_INV_MENU, 0);

	pic->setFlags(pic->_flags & 0xFFFB);

	// Not used in full game
	//_evalVersionPic = accessScene(SC_COMMON)->getPictureObjectById(PIC_CMN_EVAL, 0);

	initMap();
	initCursors();

	setMusicAllowed(_gameLoader->_gameVar->getSubVarAsInt("MUSIC_ALLOWED"));

	if (scene) {
		_gameLoader->loadScene(726);
		_gameLoader->gotoScene(726, TrubaLeft);

		if (scene != 726)
			_gameLoader->preloadScene(726, getSceneEntrance(scene));
	} else {
		if (_flgPlayIntro) {
			_gameLoader->loadScene(SC_INTRO1);
			_gameLoader->gotoScene(SC_INTRO1, TrubaUp);
		} else {
			_gameLoader->loadScene(SC_1);
			_gameLoader->gotoScene(SC_1, TrubaLeft);
		}
	}

	if (!_currentScene)
		return false;

	return true;
}

GameProject::GameProject() {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	_sceneTagList = 0;
}

bool GameProject::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "GameProject::load()");

	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	g_fp->_gameProjectVersion = file.readUint32LE();
	g_fp->_pictureScale = file.readUint16LE();
	g_fp->_scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	debugC(1, kDebugLoading, "_gameProjectVersion = %d", g_fp->_gameProjectVersion);
	debugC(1, kDebugLoading, "_pictureScale = %d", g_fp->_pictureScale);
	debugC(1, kDebugLoading, "_scrollSpeed = %d", g_fp->_scrollSpeed);
	debugC(1, kDebugLoading, "_headerFilename = %s", _headerFilename);

	_sceneTagList = new SceneTagList();

	_sceneTagList->load(file);

	if (g_fp->_gameProjectVersion >= 3)
		_field_4 = file.readUint32LE();

	if (g_fp->_gameProjectVersion >= 5) {
		file.readUint32LE();
		file.readUint32LE();
	}

	return true;
}

GameProject::~GameProject() {
	free(_headerFilename);

	delete _sceneTagList;
}

GameVar::GameVar() {
	_subVars = 0;
	_parentVarObj = 0;
	_nextVarObj = 0;
	_prevVarObj = 0;
	_field_14 = 0;
	_varType = 0;
	_value.floatValue = 0;
	_varName = 0;

	_objtype = kObjTypeGameVar;
}

GameVar::~GameVar() {
	if (_varType == 2)
		free(_value.stringValue);

	if (_parentVarObj && !_prevVarObj ) {
		if (_parentVarObj->_subVars == this) {
			_parentVarObj->_subVars = _nextVarObj;
		} else if (_parentVarObj->_field_14 == this) {
			_parentVarObj->_field_14 = _nextVarObj;
		} else {
			_parentVarObj = 0;
		}
	}

	if (_prevVarObj)
		_prevVarObj->_nextVarObj = _nextVarObj;

	if (_nextVarObj)
		_nextVarObj->_prevVarObj = _prevVarObj;

	_prevVarObj = 0;
	_nextVarObj = 0;

	GameVar *s = _subVars;

	while (s) {
		delete s;
		s = _subVars;
	}

	s = _field_14;

	while (s) {
		delete s;
		s = _field_14;
	}

	free(_varName);
}

bool GameVar::load(MfcArchive &file) {
	_varName = file.readPascalString();
	_varType = file.readUint32LE();

	debugCN(6, kDebugLoading, "[%03d] ", file.getLevel());
	for (int i = 0; i < file.getLevel(); i++)
		debugCN(6, kDebugLoading, " ");

	debugCN(6, kDebugLoading, "<%s>: ", transCyrillic((byte *)_varName));

	switch (_varType) {
	case 0:
		_value.intValue = file.readUint32LE();
		debugC(6, kDebugLoading, "d --> %d", _value.intValue);
		break;
	case 1:
		_value.intValue = file.readUint32LE(); // FIXME
		debugC(6, kDebugLoading, "f --> %f", _value.floatValue);
		break;
	case 2:
		_value.stringValue = file.readPascalString();
		debugC(6, kDebugLoading, "s --> %s", _value.stringValue);
		break;
	default:
		error("Unknown var type: %d (0x%x)", _varType, _varType);
	}

	file.incLevel();
	_parentVarObj = (GameVar *)file.readClass();
	_prevVarObj = (GameVar *)file.readClass();
	_nextVarObj = (GameVar *)file.readClass();
	_field_14 = (GameVar *)file.readClass();
	_subVars = (GameVar *)file.readClass();
	file.decLevel();

	return true;
}

GameVar *GameVar::getSubVarByName(const char *name) {
	GameVar *sv = 0;

	if (_subVars != 0) {
		sv = _subVars;
		for (;sv && scumm_stricmp(sv->_varName, name); sv = sv->_nextVarObj)
			;
	}
	return sv;
}

bool GameVar::setSubVarAsInt(const char *name, int value) {
	GameVar *var = getSubVarByName(name);

	if (var) {
		if (var->_varType == 0) {
			var->_value.intValue = value;

			return true;
		}
		return false;
	}

	var = new GameVar();
	var->_varType = 0;
	var->_value.intValue = value;
	var->_varName = (char *)calloc(strlen(name) + 1, 1);
	strcpy(var->_varName, name);

	return addSubVar(var);
}

int GameVar::getSubVarAsInt(const char *name) {
	GameVar *var = getSubVarByName(name);

	if (var)
		return var->_value.intValue;
	else
		return 0;
}

GameVar *GameVar::addSubVarAsInt(const char *name, int value) {
	if (getSubVarByName(name)) {
		return 0;
	} else {
		GameVar *var = new GameVar();

		var->_varType = 0;
		var->_value.intValue = value;

		var->_varName = (char *)calloc(strlen(name) + 1, 1);
		strcpy(var->_varName, name);

		return (addSubVar(var) != 0) ? var : 0;
	}
}

bool GameVar::addSubVar(GameVar *subvar) {
	GameVar *var = _subVars;

	if (var) {
		for (GameVar *i = var->_nextVarObj; i; i = i->_nextVarObj)
			var = i;

		var->_nextVarObj = subvar;
		subvar->_prevVarObj = var;
		subvar->_parentVarObj = this;

		return true;
	} else {
		_subVars = subvar;
		subvar->_parentVarObj = this;

		return true;
	}

	return false;
}

int GameVar::getSubVarsCount() {
	int res;
	GameVar *sub = _subVars;

	for (res = 0; sub; res++)
		sub = sub->_nextVarObj;

	return res;
}

GameVar *GameVar::getSubVarByIndex(int idx) {
	GameVar *sub = _subVars;

	while (idx--) {
		sub = sub->_nextVarObj;

		if (!sub)
			return 0;
	}

	return sub;
}

bool PicAniInfo::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "PicAniInfo::load()");

	type = file.readUint32LE();
	objectId = file.readUint16LE();
	field_6 = file.readUint16LE();
	field_8 = file.readUint32LE();
	sceneId = file.readUint16LE();
	field_E = file.readUint16LE();
	ox = file.readSint32LE();
	oy = file.readSint32LE();
	priority = file.readUint32LE();
	staticsId = file.readUint16LE();
	if (objectId == 334)
		warning("objid: %d stid: %d", objectId, staticsId);
	movementId = file.readUint16LE();
	dynamicPhaseIndex = file.readUint16LE();
	flags = file.readUint16LE();
	field_24 = file.readUint32LE();
	someDynamicPhaseIndex = file.readUint32LE();

	return true;
}

} // End of namespace Fullpipe
