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

#ifndef SCI_RESOURCE_SOURCES_MACRESFORK_H
#define SCI_RESOURCE_SOURCES_MACRESFORK_H

#include "common/macresman.h"
#include "sci/resource/resource.h"
#include "sci/resource/source.h"

namespace Common {
class SeekableReadStream;
class String;
}

namespace Sci {

class ResourceManager;

/**
 * Reads SCI1.1+ resources from a Mac resource fork.
 */
class MacResourceForkResourceSource final : public ResourceSource {
public:
	MacResourceForkResourceSource(const Common::String &name, int volNum) :
		ResourceSource(kSourceMacResourceFork, name, volNum) {}

	virtual bool scanSource(ResourceManager *resMan) override;

	virtual void loadResource(const ResourceManager *resMan, Resource *res) const override;

private:
	Common::MacResManager _macResMan;

	bool isCompressableResource(ResourceType type) const;
	void decompressResource(const ResourceManager *resMan, Common::SeekableReadStream *stream, Resource *resource) const;
};

} // End of namespace Sci

#endif