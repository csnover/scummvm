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

#ifndef SCI_S2_PANORAMA_IMAGE_H
#define SCI_S2_PANORAMA_IMAGE_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "sci/s2/system/globject.h"
#include "sci/s2/system/types.h"

namespace Sci {

class ResourceManager;

class S2PanoramaSprite;

class S2PanoramaImage : public GLObject {
public:
	S2PanoramaImage(const Common::Rect &rect);
	S2PanoramaImage(const uint16 resourceNo);

	static void init(ResourceManager *resourceManager) { _resourceManager = resourceManager; }

	int16 getWidth() const { return _width; }
	int16 getHeight() const { return _height; }

	byte *getPixels() const { return _pixels; }
	void setPixels(byte *pixels) {
		assert(!_ownedPixels.size());
		_pixels = pixels;
	}

	bool hasSprites() const { return !_sprites.empty(); }

	void loadImage(const uint16 panoramaNo);
	virtual void doIt() override {}

	void draw(S2PanoramaSprite &sprite);
	void erase(S2PanoramaSprite &sprite);

	const GLPoint &getPosition() const { return _position; }

	bool getIsDrawn() const { return _isDrawn; }
	void setIsDrawn(const bool isDrawn) { _isDrawn = isDrawn; }

	Common::List<S2PanoramaSprite *> &getSprites() { return _sprites; }

protected:
	bool _isDrawn;
	bool _isSprite;
	GLPoint _position;

private:
	void calculateDimensions(S2PanoramaSprite &source, int16 &targetX, int16 &targetY, int &skipX, int &paddingX) const;

	static ResourceManager *_resourceManager;
	int16 _width;
	int16 _height;
	byte *_pixels;
	Common::Array<byte> _ownedPixels;
	Common::List<S2PanoramaSprite *> _sprites;
};

} // End of namespace Sci

#endif