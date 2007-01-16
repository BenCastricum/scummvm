/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef AGI_LOGIC_H
#define AGI_LOGIC_H

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Agi {

/**
 * AGI logic resource structure.
 */
struct AgiLogic {
	uint8 *data;		/**< raw resource data */
	int size;		/**< size of data */
	int sIP;		/**< saved IP */
	int cIP;		/**< current IP */
	int numTexts;		/**< number of messages */
	const char **texts;		/**< message list */
};

}                             // End of namespace Agi

#endif				/* AGI_LOGIC_H */
