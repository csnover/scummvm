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

#include "sci/s2/system/glsound.h"
#include "sci/s2/game.h"
#include "sci/sound/audio32.h"

namespace Sci {

S2Game *GLSoundTrack::_game = nullptr;
Audio32 *GLSoundTrack::_mixer = nullptr;

void GLSoundTrack::addWaitNode(const int minTicks, const int maxTicks) {
	_nodes.emplace_back(new GLWaitNode(minTicks, maxTicks));
}

void GLSoundTrack::addSoundNode(const uint16 soundNo, const int16 volume, const bool loop, const int16 pan, const int somePercent) {
	_nodes.emplace_back(new GLSound(soundNo, loop ? GLSound::State::PlayingForever : GLSound::State::PlayingOnce, volume, nullptr, pan, somePercent));
}

void GLSoundTrack::addPRSNode(const uint16 soundNo, const int somePercent, const int16 volume, const int16 pan) {
	GLPRSNode *node;
	if (!_nodes.empty() && _nodes.back()->getType() == GLNode::Type::PRS) {
		node = static_cast<GLPRSNode *>(_nodes.back().get());
	} else {
		node = new GLPRSNode();
		_nodes.emplace_back(node);
	}

	node->addSound(soundNo, GLSound::State::PlayingForever, volume, nullptr, pan, somePercent);
}

void GLSoundTrack::play() {
	_isPaused = false;
	_isStopped = false;
	_currentNode = _nodes.begin();
	cue();
}

void GLSoundTrack::changeState(GLScript &script, const int state) {
	switch (state) {
	case 0:
		break;

	case 1: {
		auto iterator = ++_currentNode;
		if (iterator == _nodes.end()) {
			iterator = _currentNode = _nodes.begin();
		}
		auto *node = iterator->get();
		if (node->getType() == GLNode::Type::Header) {
			++iterator;
			++_currentNode;
			node = iterator->get();
		}
		switch (node->getType()) {
		case GLNode::Type::Header:
			warning("Two headers in a row"); // This was ignored in SSCI
			break;
		case GLNode::Type::Sound: {
			GLSound &sound = static_cast<GLSound &>(*node);
			_currentSoundNo = sound.getResourceNo();
			if (sound.getRandomness() != 100 && _game->getRandomNumber(0, 100) > sound.getRandomness()) {
				break;
			}

			if (sound.getPan() != 50) {
				_mixer->setPan(ResourceId(kResourceTypeAudio, sound.getResourceNo()), NULL_REG, sound.getPan());
			}
			_mixer->restart(ResourceId(kResourceTypeAudio, sound.getResourceNo()), false, false, sound.getVolume(), NULL_REG, false);

			break;
		}
		case GLNode::Type::Wait: {
			GLWaitNode &wait = static_cast<GLWaitNode &>(*node);
			// SSCI did some randomness check here but this function was never
			// used so it is omitted
			_currentSoundNo = 0;
			int numSecondsToWait;
			if (wait.getMaximum()) {
				numSecondsToWait = _game->getRandomNumber(wait.getMinimum(), wait.getMaximum());
			} else {
				numSecondsToWait = wait.getMinimum();
			}

			setSeconds(numSecondsToWait);
			break;
		}
		case GLNode::Type::PRS: {
			auto &prs = static_cast<GLPRSNode &>(*node);
			auto &sound = prs.getSound(_game->getRandomNumber(0, prs.size() - 1));
			_currentSoundNo = sound.getResourceNo();
			if (_game->getRandomNumber(0, 99) > sound.getRandomness()) {
				break;
			}

			auto resourceId = ResourceId(kResourceTypeAudio, _currentSoundNo);
			if (sound.getPan() != 50) {
				_mixer->setPan(resourceId, NULL_REG, sound.getPan());
			}
			_mixer->restart(resourceId, true, false, sound.getVolume(), NULL_REG, false);
			break;
		}
		}

		if ((*iterator)->getType() != GLNode::Type::Wait) {
			script.setCycles(1);
		}
		break;
	}

	case 2:
		if (_isStopped || _isPaused) {
			script.setState(-1);
		} else if (_mixer->getPosition({ kResourceTypeAudio, _currentSoundNo }) != -1) {
			script.setState(1);
		} else {
			script.setState(0);
		}

		// SSCI unlocked the resource here, we do not need to do that since our
		// resource locks are managed by the kernel
		script.setCycles(1);
		break;

	default:
		error("Invalid state %d in sound track", state);
	}
}

} // End of namespace Sci