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

#include "sci/s2/room.h"

namespace Sci {

constexpr GLPoint S2Room::absTop;
constexpr GLPoint S2Room::absBottom;
constexpr GLPoint S2Room::roomTop;
constexpr GLPoint S2Room::roomBottom;

void S2Room::dispose(const int roomNo) {
	if (_activeSubRoom) {
		_activeSubRoom->dispose(roomNo);
		_activeSubRoom.reset();
	}
	clear();
}

void S2Room::doIt() {
	if (_activeSubRoom) {
		_activeSubRoom->doIt();
	}
}

bool S2Room::handleEvent(GLEvent &event) {
	if (_activeSubRoom) {
		return _activeSubRoom->handleEvent(event);
	}
	return false;
}

void S2Room::initSubRoom(const int roomNo) {
	_activeSubRoom->init(roomNo);
}

} // End of namespace Sci