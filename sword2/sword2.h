/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	_SWORD2
#define	_SWORD2

//#include "src\driver96.h"


#ifdef _PCF76	// Bodge for PCF76 version so that their demo CD can be labelled "PCF76" rather than "RBSII1"
	#define CD1_LABEL	"PCF76"
#else
	#define CD1_LABEL	"RBSII1"
#endif


#define	CD2_LABEL	"RBSII2"

void	Close_game();	//Tony11Oct96

void PauseGame(void);		// James17jun97
void UnpauseGame(void);		// James17jun97

#define HEAD_LEN 8

extern uint8 version_string[];		// for displaying from the console
extern uint8 unencoded_name[];

enum BSGameId {
	GID_BS2 = GID_BS2_FIRST,
	GID_BS2_DEMO
};

// TODO move stuff into class
class BS2State : public Engine {
		void errorString(const char *buf_input, char *buf_output);
	public:
		BS2State(GameDetector *detector, OSystem *syst);
		void go(void);
		void parseEvents(void);
		void Start_game(void);
		int32 InitialiseGame(void);
		OSystem *_syst;
		GameDetector *_detector;
		uint32 _features;
		byte _gameId;
	private:
		bool _quit;
};

extern BS2State *g_bs2;

#endif
