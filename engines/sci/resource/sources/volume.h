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

#ifndef SCI_RESOURCE_SOURCES_VOLUME_H
#define SCI_RESOURCE_SOURCES_VOLUME_H

#include "sci/resource/resource.h"
#include "sci/resource/source.h"

namespace Sci {

class VolumeResourceSource : public DataOnlyResourceSource {
public:
	VolumeResourceSource(const Common::String &name, ResourceSource *map, int volNum, ResSourceType type = kSourceVolume) :
		DataOnlyResourceSource(type, name, volNum),
		_associatedMap(map) {}

	VolumeResourceSource(const Common::String &name, ResourceSource *map, int volNum, const Common::FSNode *resFile) :
		DataOnlyResourceSource(kSourceVolume, name, volNum, resFile),
		_associatedMap(map) {}

	bool isVolumeForMap(const ResourceSource *map, int volumeNo) const {
		return (_associatedMap == map && _volumeNumber == volumeNo);
	}

	virtual void loadResource(const ResourceManager *resMan, Resource *res) const override;

private:
	ResourceErrorCode decompress(const ResourceManager *resMan, Resource *res, Common::SeekableReadStream *file) const;

	ResourceSource *const _associatedMap;
};

} // End of namespace Sci

#endif
