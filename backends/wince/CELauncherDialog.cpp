/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "CELauncherDialog.h"

#include "base/engine.h"

#include "gui/browser.h"
#include "gui/message.h"

#include "common/config-manager.h"

using namespace GUI;
using namespace Common;


CELauncherDialog::CELauncherDialog(GameDetector &detector) : GUI::LauncherDialog(detector) {
}

void CELauncherDialog::addCandidate(String &path, DetectedGameList &candidates) {
	int idx = -1;
	DetectedGame result;

	if (candidates.isEmpty())
		return;

	if (candidates.size() == 1)
		idx = 0;
	else {
			char candidateName[100];
			int i;
			for (i=path.size() - 2; i && path[i] != '\\'; i--);
			strcpy(candidateName, &path[i + 1]);
			candidateName[strlen(candidateName) - 1] = '\0';
			for (i=0; i<candidates.size(); i++) {						
				if (scumm_stricmp(candidateName, candidates[i].name) == 0) {
					idx = i;
					break;
			}
		}		
	}

	if (idx < 0)
		return;

	// FIXME : factor that
	result = candidates[idx];

	// The auto detector or the user made a choice.
	// Pick a domain name which does not yet exist (after all, we
	// are *adding* a game to the config, not replacing).
	String domain(result.name);
	if (ConfMan.hasGameDomain(domain)) {
		char suffix = 'a';
		domain += suffix;
		while (ConfMan.hasGameDomain(domain)) {
			assert(suffix < 'z');
			domain.deleteLastChar();
			suffix++;
			domain += suffix;
		}
		ConfMan.set("gameid", result.name, domain);
		ConfMan.set("description", result.description, domain);
	}
	ConfMan.set("path", path, domain);

	// Set language if specified
	if (result.language != Common::UNK_LANG)
		ConfMan.set("language", Common::getLanguageCode(result.language), domain);

	// Set platform if specified
	if (result.platform != Common::kPlatformUnknown)
		ConfMan.set("platform", Common::getPlatformCode(result.platform), domain);
}

void CELauncherDialog::automaticScanDirectory(const FilesystemNode *node) {
	// First check if we have a recognized game in the current directory
	FSList *files = node->listDir(FilesystemNode::kListFilesOnly);
	DetectedGameList candidates(PluginManager::instance().detectGames(*files));
	addCandidate(node->path(), candidates);
	// Then recurse on the subdirectories
	FSList *dirs = node->listDir(FilesystemNode::kListDirectoriesOnly);
	for (FSList::ConstIterator currentDir = dirs->begin(); currentDir != dirs->end(); ++currentDir)
		automaticScanDirectory(&(*currentDir));

}

void CELauncherDialog::addGame() {
	MessageDialog alert("Do you want to perform an automatic scan ?", "Yes", "No");
	if (alert.runModal() == 1 && _browser->runModal()) {
			// Clear existing domains
			ConfigManager::DomainMap &domains = (ConfigManager::DomainMap&)ConfMan.getGameDomains();
			domains.clear();
			ConfMan.flushToDisk();
			automaticScanDirectory(_browser->getResult());
			ConfMan.flushToDisk();
			updateListing();
			draw();
	}
	else
		GUILauncherDialog::addGame();
}	