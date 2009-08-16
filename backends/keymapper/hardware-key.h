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
* $URL$
* $Id$
*
*/

#ifndef COMMON_HARDWARE_KEY_H
#define COMMON_HARDWARE_KEY_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/types.h"
#include "common/str.h"
#include "common/keyboard.h"
#include "common/list.h"
#include "common/util.h"

namespace Common {


#define HWKEY_ID_SIZE (30)


// Structure for describing specific key+modifier combos mapped to actions,
// to allow for modifiers to work properly without having to define the whole 
// hardware key set an additional time for each possible modifier combination
struct ActionKey {
	KeyCode keycode;
	byte flags;

	ActionKey () { 
		keycode = KEYCODE_INVALID;
		flags = 0;
	}

	ActionKey (const KeyState &key) { 
		keycode = key.keycode;
		flags = key.flags;
	}


	ActionKey (KeyCode ky, byte f) { 
		keycode = ky;
		flags = f;
	}

	bool operator ==(const ActionKey &x) const {
		return keycode == x.keycode && flags == x.flags;
	}

};

	
/**
* Describes an available hardware key
*/
struct HardwareKey {
	/** unique id used for saving/loading to config */
	char hwKeyId[HWKEY_ID_SIZE];

	/** Human readable description */
	String description;

	/**
	* The KeyState that is generated by the back-end
	* when this hardware key is pressed.
	*/
	ActionKey key;

	KeyType type;
	ActionType preferredAction;

	// Mask of modifiers that can possibly apply to this key.
	byte modMask;

	HardwareKey(const char *i, ActionKey ky = ActionKey(), String desc = "", byte mods = ~0,
				KeyType typ = kGenericKeyType, ActionType prefAct = kGenericActionType)
		: key(ky), description(desc), type(typ), preferredAction(prefAct), modMask(mods) {
		assert(i);
		strncpy(hwKeyId, i, HWKEY_ID_SIZE);
	}
};

/**
* Describes an available hardware modifier
*/
struct HardwareMod {
	/** unique id used for saving/loading to config */
	char hwModId[HWKEY_ID_SIZE];

	/** Human readable description */
	String description;

	/**
	* The modifier flags that are generated by the 
	* back-end when this modifier key is pressed.
	*/
	byte modFlags;

	HardwareMod(const char *i, byte mf, String desc = "")
		: modFlags(mf), description(desc) {
		assert(i);
		strncpy(hwModId, i, HWKEY_ID_SIZE);
	}
};


/**
 * Simple class to encapsulate a device's set of HardwareKeys.
 * Each device should instantiate this and call addHardwareKey a number of times
 * in its constructor to define the device's available keys.
 */
class HardwareKeySet {
public:

	virtual ~HardwareKeySet() {
		List<const HardwareKey*>::const_iterator it;

		for (it = _keys.begin(); it != _keys.end(); it++)
			delete *it;
	}

	void addHardwareMod(HardwareMod *mod) {
		checkForMod(mod);
		_mods.push_back(mod);
	}

	void addHardwareKey(HardwareKey *key) {
		checkForKey(key);
		_keys.push_back(key);
	}

	const HardwareKey *findHardwareKey(const char *id) const {
		List<const HardwareKey*>::const_iterator it;

		for (it = _keys.begin(); it != _keys.end(); it++) {
			if (strncmp((*it)->hwKeyId, id, HWKEY_ID_SIZE) == 0)
				return (*it);
		}
		return 0;
	}

	const HardwareKey *findHardwareKey(const ActionKey& keystate) const {
		List<const HardwareKey*>::const_iterator it;

		for (it = _keys.begin(); it != _keys.end(); it++) {
			if ((*it)->key.keycode == keystate.keycode)
				return (*it);
		}
		return 0;
	}

	const HardwareMod *findHardwareMod(const char *id) const {
		List<const HardwareMod*>::const_iterator it;

		for (it = _mods.begin(); it != _mods.end(); it++) {
			if (strncmp((*it)->hwModId, id, HWKEY_ID_SIZE) == 0)
				return (*it);
		}
		return 0;
	}

	const HardwareMod *findHardwareMod(const ActionKey& keystate) const {
		List<const HardwareMod*>::const_iterator it;

		for (it = _mods.begin(); it != _mods.end(); it++) {
			if ((*it)->modFlags == keystate.flags)
				return (*it);
		}
		return 0;
	}

	const List<const HardwareKey*> &getHardwareKeys() const {
		return _keys;
	}

	uint size() const {
		return _keys.size();
	}


private:

	void checkForKey(HardwareKey *key) {
		List<const HardwareKey*>::iterator it;

		for (it = _keys.begin(); it != _keys.end(); it++) {
			if (strncmp((*it)->hwKeyId, key->hwKeyId, HWKEY_ID_SIZE) == 0)
				error("Error adding HardwareKey '%s' - id of %s already in use!", key->description.c_str(), key->hwKeyId);
			else if ((*it)->key == key->key)
				error("Error adding HardwareKey '%s' - key already in use!", key->description.c_str());
		}
	}

	void checkForMod(HardwareMod *mod) {
		List<const HardwareMod*>::iterator it;

		for (it = _mods.begin(); it != _mods.end(); it++) {
			if (strncmp((*it)->hwModId, mod->hwModId, HWKEY_ID_SIZE) == 0)
				error("Error adding HardwareMod '%s' - id of %s already in use!", mod->description.c_str(), mod->hwModId);
			else if ((*it)->modFlags == mod->modFlags)
				error("Error adding HardwareMod '%s' - modFlags already in use!", mod->description.c_str());
		}
	}

	List<const HardwareKey*> _keys;
	List<const HardwareMod*> _mods;
};


} // end of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_HARDWARE_KEY_H
