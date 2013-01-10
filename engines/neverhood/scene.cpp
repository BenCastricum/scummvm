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

#include "neverhood/scene.h"
#include "neverhood/collisionman.h"

namespace Neverhood {

Scene::Scene(NeverhoodEngine *vm, Module *parentModule, bool clearHitRects)
	: Entity(vm, 0), _parentModule(parentModule), _dataResource(vm) {
	
	_isKlaymanBusy = false;
	_doConvertMessages = false;
	_messageList = NULL;
	_rectType = 0;
	_mouseClickPos.x = 0;
	_mouseClickPos.y = 0;
	_mouseClicked = false;
	_rectList = NULL;
	_klayman = NULL;
	_mouseCursor = NULL;
	_palette = NULL;
	_background = NULL;
	if (clearHitRects) {
		_vm->_collisionMan->clearHitRects();
		_vm->_collisionMan->clearCollisionSprites();
	}
	_vm->_screen->setFps(24);
	_vm->_screen->setSmackerDecoder(NULL);
	_canAcceptInput = true;
	_messageList2 = NULL;
	_smackerPlayer = NULL;
	_isMessageListBusy = false;
	_messageValue = -1;
	
	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene::handleMessage);
	
	_vm->_screen->clearRenderQueue();
}

Scene::~Scene() {

	if (_palette) {
		removeEntity(_palette);
		delete _palette;
	}

	// Delete all entities
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
		delete *iter;

	// Don't delete surfaces since they always belong to an entity
	
	// Purge the resources after each scene
	_vm->_res->purgeResources();

}

void Scene::draw() {
	if (_smackerPlayer) {
		if (_smackerPlayer->getSurface())
			_smackerPlayer->getSurface()->draw();
	} else {
		for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++)
			(*iter)->draw();
	}	
}

void Scene::addEntity(Entity *entity) {
	int index = 0, insertIndex = -1;
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++) {
		if ((*iter)->getPriority() > entity->getPriority()) {
			insertIndex = index;
			break;
		}
		index++;
	}
	if (insertIndex >= 0)
		_entities.insert_at(insertIndex, entity);
	else
		_entities.push_back(entity);		
}

bool Scene::removeEntity(Entity *entity) {
	for (uint index = 0; index < _entities.size(); index++)
		if (_entities[index] == entity) {
			_entities.remove_at(index);
			return true;
		}
	return false; 
}

void Scene::addSurface(BaseSurface *surface) {
	if (surface) {
		int index = 0, insertIndex = -1;
		for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++) {
			if ((*iter)->getPriority() > surface->getPriority()) {
				insertIndex = index;
				break;
			}
			index++;
		}
		if (insertIndex >= 0)
			_surfaces.insert_at(insertIndex, surface);
		else
			_surfaces.push_back(surface);		
	}
}

bool Scene::removeSurface(BaseSurface *surface) {
	for (uint index = 0; index < _surfaces.size(); index++) {
		if (_surfaces[index] == surface) {
			_surfaces.remove_at(index);
			return true;
		}
	}
	return false; 
}

Sprite *Scene::addSprite(Sprite *sprite) {
	addEntity(sprite);
	addSurface(sprite->getSurface());
	return sprite;
}

void Scene::removeSprite(Sprite *sprite) {
	removeSurface(sprite->getSurface());
	removeEntity(sprite);
}

void Scene::setSurfacePriority(BaseSurface *surface, int priority) {
	surface->setPriority(priority);
	if (removeSurface(surface))
		addSurface(surface);
}

void Scene::setSpriteSurfacePriority(Sprite *sprite, int priority) {
	if (sprite)
		setSurfacePriority(sprite->getSurface(), priority);
}

void Scene::deleteSprite(Sprite **sprite) {
	_vm->_collisionMan->removeCollisionSprite(*sprite);
	removeSurface((*sprite)->getSurface());
	removeEntity(*sprite);
	delete *sprite;
	*sprite = NULL;
}

Background *Scene::addBackground(Background *background) {
	addEntity(background);
	addSurface(background->getSurface());
	return background;
}

void Scene::setBackground(uint32 fileHash) {
	_background = addBackground(new Background(_vm, fileHash, 0, 0));
}

void Scene::changeBackground(uint32 fileHash) {
	_background->load(fileHash);
}

void Scene::setPalette(uint32 fileHash) {
	_palette = fileHash ? new Palette(_vm, fileHash) : new Palette(_vm);
	_palette->usePalette();
}

void Scene::setHitRects(uint32 id) {
	_vm->_collisionMan->setHitRects(id);
}

Sprite *Scene::insertStaticSprite(uint32 fileHash, int surfacePriority) {
	return addSprite(new StaticSprite(_vm, fileHash, surfacePriority));
}

void Scene::insertScreenMouse(uint32 fileHash, const NRect *mouseRect) {
	NRect rect(-1, -1, -1, -1);
	if (mouseRect)
		rect = *mouseRect;
	_mouseCursor = new Mouse(_vm, fileHash, rect);
	addEntity(_mouseCursor);
}

void Scene::insertPuzzleMouse(uint32 fileHash, int16 x1, int16 x2) {
	_mouseCursor = new Mouse(_vm, fileHash, x1, x2);
	addEntity(_mouseCursor);
}

void Scene::insertNavigationMouse(uint32 fileHash, int type) {
	_mouseCursor = new Mouse(_vm, fileHash, type);
	addEntity(_mouseCursor);
}

void Scene::showMouse(bool visible) {
	_mouseCursor->getSurface()->setVisible(visible);
}

void Scene::changeMouseCursor(uint32 fileHash) {
	_mouseCursor->load(fileHash);
	_mouseCursor->updateCursor();
}

SmackerPlayer *Scene::addSmackerPlayer(SmackerPlayer *smackerPlayer) {
	addEntity(smackerPlayer);
	addSurface(smackerPlayer->getSurface());
	return smackerPlayer;
}

void Scene::update() {

	if (_mouseClicked) {
		if (_klayman) {
			if (_canAcceptInput &&
				_klayman->hasMessageHandler() && 
				sendMessage(_klayman, 0x1008, 0) != 0 &&
				queryPositionSprite(_mouseClickPos.x, _mouseClickPos.y)) {
				_mouseClicked = false;
			} else if (_canAcceptInput &&
				_klayman->hasMessageHandler() && 
				sendMessage(_klayman, 0x1008, 0) != 0) {
				_mouseClicked = !queryPositionRectList(_mouseClickPos.x, _mouseClickPos.y);
			}
		} else if (queryPositionSprite(_mouseClickPos.x, _mouseClickPos.y)) {
			_mouseClicked = false;
		}
	}

	processMessageList();

	// Update all entities		
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
		(*iter)->handleUpdate();

}

void Scene::leaveScene(uint32 result) {
	sendMessage(_parentModule, 0x1009, result);
}

uint32 Scene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0: // mouse moved
		if (_mouseCursor && _mouseCursor->hasMessageHandler())
			sendMessage(_mouseCursor, 0x4002, param);
		break;
	case 1: // mouse clicked
		_mouseClicked = true;
		_mouseClickPos = param.asPoint();
		break;
	case 6:
		sendMessage(_parentModule, 0x1009, param);		
		break;
	case 0x1006:
		// Sent by Klayman when its animation sequence has finished
		if (_isKlaymanBusy) {
			_isKlaymanBusy = false;
			if (_messageListIndex == _messageListCount) {
				// If the current message list was processed completely,
				// sent Klayman into the idle state.
				sendMessage(_klayman, 0x4004, 0);
			} else {
				// Else continue with the next message in the current message list
				processMessageList();
			}
		}
		break;
	case 0x1007:
		// This isn't sent by any code, check if it's in a message list
		// This cancels the current message list and sets Klayman into the idle state.
		if (_isKlaymanBusy) {
			_isKlaymanBusy = false;
			_messageList = NULL;
			sendMessage(_klayman, 0x4004, 0);
		}
		break;
	case 0x101D:
		// Hide the mouse cursor
		if (_mouseCursor) {
			_mouseCursorWasVisible = _mouseCursor->getSurface()->getVisible();
			_mouseCursor->getSurface()->setVisible(false);
		}
		break;
	case 0x101E:
		// Show the mouse cursor
		if (_mouseCursorWasVisible && _mouseCursor) {
			_mouseCursor->getSurface()->setVisible(true);//CHECKME?!?
			// TODO sendMessage(_mouseCursor, 0x4002, g_Screen->_mousePos);
		}
		break;
	case 0x1022:
		// Set the sender's surface priority
		setSurfacePriority(((Sprite*)sender)->getSurface(), param.asInteger());
		break;
	}
	return 0;
}

bool Scene::queryPositionSprite(int16 mouseX, int16 mouseY) {
	for (uint i = 0; i < _vm->_collisionMan->getCollisionSpritesCount(); i++) {
		Sprite *sprite = _vm->_collisionMan->getCollisionSprite(i);
		if (sprite->hasMessageHandler() && sprite->isPointInside(mouseX, mouseY) && 
			sendPointMessage(sprite, 0x1011, _mouseClickPos) != 0) {
			return true;
		}
	}
	return false;
}

bool Scene::queryPositionRectList(int16 mouseX, int16 mouseY) {
	int16 klaymanX = _klayman->getX();
	int16 klaymanY = _klayman->getY();
	if (_rectType == 1) {
		RectList &rectList = *_rectList;
		for (uint i = 0; i < rectList.size(); i++) {
			debug(2, "(%d, %d) ? (%d, %d, %d, %d)", klaymanX, klaymanY, rectList[i].rect.x1, rectList[i].rect.y1, rectList[i].rect.x2, rectList[i].rect.y2);
			if (rectList[i].rect.contains(klaymanX, klaymanY)) {
				for (uint j = 0; j < rectList[i].subRects.size(); j++) {
					debug(2, "  (%d, %d) ? (%d, %d, %d, %d)", mouseX, mouseY, rectList[i].subRects[j].rect.x1, rectList[i].subRects[j].rect.y1, rectList[i].subRects[j].rect.x2, rectList[i].subRects[j].rect.y2);
					if (rectList[i].subRects[j].rect.contains(mouseX, mouseY)) {
						return setMessageList2(rectList[i].subRects[j].messageListId);
					}
				}
			}
		}
	} else if (_rectType == 2) {
		MessageList *messageList = _dataResource.getMessageListAtPos(klaymanX, klaymanY, mouseX, mouseY);
		if (messageList && messageList->size())
			setMessageList2(messageList, true, true);
	}
	return true;
}

void Scene::setMessageList(uint32 id, bool canAcceptInput, bool doConvertMessages) {
	setMessageList(_vm->_staticData->getMessageList(id), canAcceptInput, doConvertMessages);
}

void Scene::setMessageList(MessageList *messageList, bool canAcceptInput, bool doConvertMessages) {
	_messageList = messageList;
	_messageListCount = _messageList ? _messageList->size() : 0;
	_messageListIndex = 0;
	_isKlaymanBusy = false;
	_canAcceptInput = canAcceptInput;
	_doConvertMessages = doConvertMessages;
	_messageListStatus = 1;
	sendMessage(_klayman, 0x101C, 0);
}

bool Scene::setMessageList2(uint32 id, bool canAcceptInput, bool doConvertMessages) {
	return setMessageList2(_vm->_staticData->getMessageList(id), canAcceptInput, doConvertMessages);
}

bool Scene::setMessageList2(MessageList *messageList, bool canAcceptInput, bool doConvertMessages) {
	if (_messageListStatus == 0 ||
		(_messageListStatus == 1 && messageList != _messageList2) ||
		(_messageListStatus == 2 && messageList == _messageList2)) {
		// NOTE Skipped unneeded resource preloading code
		_messageValue = -1;
		_messageList2 = messageList;
		setMessageList(messageList, canAcceptInput, doConvertMessages);
		return true;
	}
	return false;
}

void Scene::processMessageList() {
	debug(7, "Scene::processMessageList() _isMessageListBusy = %d; _isKlaymanBusy = %d", _isMessageListBusy, _isKlaymanBusy);

	if (_isMessageListBusy || _isKlaymanBusy)
		return;

	_isMessageListBusy = true;

	if (!_messageList) {
		_messageList2 = NULL;
		_messageListStatus = 0;
	}
	
	if (_messageList && _klayman) {
	
		while (_messageList && _messageListIndex < _messageListCount && !_isKlaymanBusy) {
			uint32 messageNum = (*_messageList)[_messageListIndex].messageNum;
			uint32 messageParam = (*_messageList)[_messageListIndex].messageValue;
			
			++_messageListIndex;
			if (_messageListIndex == _messageListCount)
				sendMessage(_klayman, 0x1021, 0);
			if (_doConvertMessages)
				messageNum = convertMessageNum(messageNum);
			if (messageNum == 0x1009 || messageNum == 0x1024) {
				sendMessage(_parentModule, messageNum, messageParam);
			} else if (messageNum == 0x100A) {
				_messageValue = messageParam;
				sendMessage(_parentModule, messageNum, messageParam);
			} else if (messageNum == 0x4001) {
				_isKlaymanBusy = true;
				sendPointMessage(_klayman, 0x4001, _mouseClickPos);
			} else if (messageNum == 0x100D) {
				if (this->hasMessageHandler() && sendMessage(this, 0x100D, messageParam) != 0)
					continue;
			} else if (messageNum == 0x101A) {
				_messageListStatus = 0;
			} else if (messageNum == 0x101B) {
				_messageListStatus = 2;
			} else if (messageNum == 0x1020) {
				_canAcceptInput = false;
			} else if (messageNum >= 0x2000 && messageNum <= 0x2FFF) {
				if (this->hasMessageHandler() && sendMessage(this, messageNum, messageParam) != 0) {
					_isMessageListBusy = false;
					return;
				}
			} else if (messageNum != 0x4003) {
				_isKlaymanBusy = true;
				if (_klayman->hasMessageHandler() && sendMessage(_klayman, messageNum, messageParam) != 0) {
					_isKlaymanBusy = false;
				}
			} 
			if (_messageListIndex == _messageListCount) {
				_canAcceptInput = true;
				_messageList = NULL;
			}
		}
	}

	_isMessageListBusy = false;
	
}

void Scene::cancelMessageList() {
	_isKlaymanBusy = false;
	_messageList = NULL;
	_canAcceptInput = true;
	sendMessage(_klayman, 0x4004, 0);
}

void Scene::setRectList(uint32 id) {
	setRectList(_vm->_staticData->getRectList(id));
}

void Scene::setRectList(RectList *rectList) {
	_rectList = rectList;
	_rectType = 1;
}

void Scene::clearRectList() {
	_rectList = NULL;
	_rectType = 0;
}

void Scene::loadHitRectList() {
	HitRectList *hitRectList = _dataResource.getHitRectList();
	if (hitRectList) {
		_hitRectList = *hitRectList;
		_vm->_collisionMan->setHitRects(&_hitRectList);
	}
}

void Scene::loadDataResource(uint32 fileHash) {
	_dataResource.load(fileHash);
	_rectType = 2;
	if (_klayman)
		_klayman->loadDataResource(fileHash);
}

uint16 Scene::convertMessageNum(uint32 messageNum) {
	switch (messageNum) {
	case 0x00004004:
		return 0x4001;
	case 0x00000083:
		return 0x100A;
	case 0x044001C8:
		return 0x481C;
	case 0x02420480:
		return 0x4818;
	case 0x08004025:
		return 0x100D;
	case 0x04404281:
		return 0x4824;
	case 0x08400880:
		return 0x4825;
	case 0x08209081:
		return 0x4823;
	case 0x24000060:
		return 0x1009;
	case 0x42002200:
		return 0x4004;
	case 0x428D4894:
		return 0x101A;	
	}
	return 0x1000;
}

} // End of namespace Neverhood
