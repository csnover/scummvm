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

#ifndef SCI_S2_PANORAMA_SPRITE_H
#define SCI_S2_PANORAMA_SPRITE_H

#undef abort
#undef FILE
#undef time
#undef mktime
#undef localtime
#undef gmtime
#undef getdate
#undef asctime
#undef difftime
#undef ctime
#undef clock
#include <functional>
#include "sci/s2/panorama_image.h"
#include "sci/s2/system/glcycler.h"
#include "sci/s2/system/glmover.h"
#include "sci/s2/system/types.h"

namespace Sci {

class S2PanoramaSprite;

using S2PanoramaCycler = AbsGLCycler<S2PanoramaSprite>;
using S2PanoramaEndForwardCycler = AbsGLEndForwardCycler<S2PanoramaSprite>;
using S2PanoramaEndBackCycler = AbsGLEndBackCycler<S2PanoramaSprite>;
using S2PanoramaEndForwardBackwardCycler = AbsGLEndForwardBackwardCycler<S2PanoramaSprite>;
using S2PanoramaStartResetCycler = AbsGLStartResetCycler<S2PanoramaSprite>;
using S2PanoramaEndResetCycler = AbsGLEndResetCycler<S2PanoramaSprite>;
using S2PanoramaCycleToCycler = AbsGLCycleToCycler<S2PanoramaSprite>;
using S2PanoramaMover = AbsGLMover<S2PanoramaSprite, GLBresen>;
using S2PanoramaJump = AbsGLMover<S2PanoramaSprite, GLArc>;

class S2PanoramaSprite : public S2PanoramaImage {
public:
	using EventHandler = std::function<void(S2PanoramaSprite &)>;

	S2PanoramaSprite(const uint16 resourceNo, const GLPoint &position, const int16 celNo = 0, const int16 numCels = 1, const bool transparent = false, const bool visible = true);

	const EventHandler &getMouseDownHandler() const { return _mouseDownHandler; }
	template <typename T, typename U>
	EventHandler setMouseDownHandler(T object, U fn) {
		using namespace std::placeholders;
		return std::bind(object, fn, _1);
	}
	void setMouseDownHandler(const EventHandler &handler) { _mouseDownHandler = handler; }

	const Common::Rect &getRect() { return _bounds; }
	void setBounds(const Common::Rect &bounds);

	void setPosition(const GLPoint &position, const bool);

	int getCycleSpeed() const { return _cycleSpeed; }
	void setCycleSpeed(const int cycleSpeed) { _cycleSpeed = cycleSpeed; }

	int getMoveSpeed() const { return _moveSpeed; }
	void setMoveSpeed(const int moveSpeed) { _moveSpeed = moveSpeed; }

	const GLPoint &getStepSize() const { return _stepSize; }

	int16 getCel() const { return _celNo; }
	void setCel(const int16 celNo, const bool = false) { _celNo = celNo; }
	int16 getLastCel() const { return _numCels - 1; }

	int16 getCelWidth() const { return _celWidth; }
	int16 getCelHeight() const { return _celHeight; }

	bool getHasTransparency() const { return _hasTransparency; }
	bool getIsVisible() const { return _isVisible; }

	Common::Array<byte> &getSavedPixels() { return _savedPixels; }

	void hide() { _isVisible = false; }
	void show() { _isVisible = true; }
	void update();

private:
	Common::Rect _bounds;
	GLPoint _nextPosition;
	EventHandler _mouseDownHandler;

	Common::Array<byte> _savedPixels;
	int16 _celNo;
	int16 _numCels;
	int16 _celWidth;
	int16 _celHeight;
	int16 _scaleInfo;
	int _cycleSpeed;
	int _moveSpeed;
	GLPoint _stepSize;
	bool _isVisible;
	bool _hasTransparency;
};

} // End of namespace Sci

#endif