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

#ifndef TITANIC_MAITRED_SCRIPT_DE_H
#define TITANIC_MAITRED_SCRIPT_DE_H

#include "titanic/true_talk/MaitreD_script.h"

namespace Titanic {
namespace German {

class MaitreDScript : public ::Titanic::MaitreDScript {
public:
	MaitreDScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		::Titanic::MaitreDScript(val1, charClass, v2, charName, v3, val2) {}
};

} // End of namespace German
} // End of namespace Titanic

#endif /* TITANIC_MAITRED_SCRIPT_DE_H */
