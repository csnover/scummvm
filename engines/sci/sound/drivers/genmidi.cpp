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

#include "audio/mididrv.h"
#include "sci/sound/drivers/genmidi.h"

namespace Sci {

Sci1GeneralMidiDriver::Sci1GeneralMidiDriver(ResourceManager &resMan, SciVersion version) :
	Sci1SoundDriver(resMan, version),
	_reverbMode(0),
	_masterVolume(15) {

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_PREFER_GM);
	_device.reset(MidiDriver::createMidi(dev));
	assert(_device);
	const int errNo = _device->open();
	if (errNo != 0) {
		error("Failure opening General MIDI device: %s", _device->getErrorName(errNo));
	}

	if (version >= SCI_VERSION_2) {
		_deviceId = 7;
	} else if (version >= SCI_VERSION_1_LATE) {
		_deviceId = 12;
	} else {
		error("Unimplemented SCI sound version %d", version);
	}

	Resource *patchData = resMan.findResource(ResourceId(kResourceTypePatch, 4), false);
	SciSpan<const byte> midiData;
	if (patchData) {
		patchData->subspan(0, sizeof(_programMap)).unsafeCopyDataTo(_programMap);
		patchData->subspan(128, sizeof(_noteShift)).unsafeCopyDataTo(_noteShift);
		patchData->subspan(256, sizeof(_volumeShift)).unsafeCopyDataTo(_volumeShift);
		patchData->subspan(384, sizeof(_percussionMap)).unsafeCopyDataTo(_percussionMap);
		_channels[kPercussionChannel].volumeShift = patchData->getInt8At(512);
		patchData->subspan(513, sizeof(_programVelocityMap)).unsafeCopyDataTo(_programVelocityMap);
		patchData->subspan(641, sizeof(_velocityMaps)).unsafeCopyDataTo(_velocityMaps);
		midiData = patchData->subspan(1153 + sizeof(uint16), patchData->getUint16LEAt(1153));
	} else {
		warning("No GM patch data found, using defaults");
		for (int i = 0; i < kNumPrograms; ++i) {
			_programMap[i] = i;
		}
		Common::fill(_noteShift, _noteShift + kNumPrograms, 0);
		Common::fill(_volumeShift, _volumeShift + kNumPrograms, 0);
		for (int i = 0; i < kNumNotes; ++i) {
			_percussionMap[i] = i;
		}
		_percussionVolumeShift = 0;
		Common::fill(_programVelocityMap, _programVelocityMap + kNumPrograms, 0);
		for (int i = 0; i < kNumVelocities; ++i) {
			_velocityMaps[0][i] = i;
		}
		midiData = SciSpan<const byte>(defaultSci32GMPatchMidiData, sizeof(defaultSci32GMPatchMidiData));
	}

	for (int i = 0; i < kNumChannels; ++i) {
		if (i == kPercussionChannel) {
			_channels[i].hw = static_cast<MidiChannel_MPU401 *>(_device->getPercussionChannel());
		} else {
			_channels[i].hw = static_cast<MidiChannel_MPU401 *>(_device->allocateChannel());
		}
	}

	sendBytes(midiData);
	setMasterVolume(12);
}

Sci1GeneralMidiDriver::~Sci1GeneralMidiDriver() {
	// TODO: Send All Notes Off controller message if ScummVM does not clean up
	// for us
}

void Sci1GeneralMidiDriver::noteOn(const uint8 channelNo, uint8 note, uint8 velocity) {
	Channel &channel = _channels[channelNo];
	if (remapNote(channelNo, note)) {
		velocity = _velocityMaps[channel.velocityMap][velocity];
		channel.enabled = true;
		channel.hw->noteOn(note, velocity);
		debug("On  %2d %3d %3d", channelNo, note, velocity);
	} else {
		debug("OX  %2d %3d %3d", channelNo, note, velocity);
	}
}

void Sci1GeneralMidiDriver::noteOff(const uint8 channelNo, uint8 note, uint8 velocity) {
	Channel &channel = _channels[channelNo];
	if (remapNote(channelNo, note)) {
		channel.hw->noteOff(note, velocity);
		debug("Off %2d %3d %3d", channelNo, note, velocity);
	} else {
		debug("OXX %2d %3d %3d", channelNo, note, velocity);
	}
}

bool Sci1GeneralMidiDriver::remapNote(const uint8 channelNo, uint8 &note) const {
	const Channel &channel = _channels[channelNo];
	if (channelNo == kPercussionChannel) {
		if (_percussionMap[note] == kUnmapped) {
			return false;
		}
		note = _percussionMap[note];
		return true;
	} else if (channel.outProgram != kUnmapped) {
		note += channel.noteShift;
		const int8 octave = channel.noteShift > 0 ? -kNotesPerOctave : kNotesPerOctave;
		while (note >= kNumNotes) {
			note += octave;
		}
		return true;
	}
	return false;
}

void Sci1GeneralMidiDriver::controllerChange(const uint8 channelNo, const uint8 controllerNo, uint8 value) {
	Channel &channel = _channels[channelNo];
	switch (controllerNo) {
	case kVolumeController: {
		channel.volume = value;
		if (!_isEnabled) {
			return;
		}
		value += channel.volumeShift;
		if (value > kMaxVolume) {
			value = channel.volumeShift > 0 ? kMaxVolume : 1;
		}
		const uint16 result = value * _masterVolume / kMaxMasterVolume;
		value = result & 0xFF;
		if (!value || (result & 0xFF00)) {
			++value;
		}
		break;
	}
	case kPanController:
		if (channel.pan == value) {
			return;
		}
		channel.pan = value;
		break;
	case kDamperPedalController:
		// The SoundManager code would only ever use a boolean for the damper
		// pedal, but technically this is a ranged value, so let's make sure
		// a musician did not sneak in some damper pedal into the sound data
		// with a non-boolean value
		assert(value == 0 || value == 1);
		if (channel.damperPedalOn == (value != 0)) {
			return;
		}
		channel.damperPedalOn = value;
		break;
	case kAllNotesOffController:
		channel.enabled = false;
		return;
	default:
		return;
	}

	channel.hw->controlChange(controllerNo, value);
	debug("CC %2d %3d %3d", channelNo, controllerNo, value);
}

void Sci1GeneralMidiDriver::programChange(const uint8 channelNo, const uint8 programNo) {
	Channel &channel = _channels[channelNo];
	if (channelNo == kPercussionChannel || channel.program == programNo) {
		return;
	}

	channel.program = programNo;
	channel.velocityMap = _programVelocityMap[programNo];

	bool needsVolumeUpdate = channel.outProgram != kUnmapped;
	channel.outProgram = _programMap[programNo];

	if (channel.outProgram == kUnmapped) {
		channel.hw->controlChange(kAllNotesOffController, 0);
		channel.hw->controlChange(kDamperPedalController, 0);
		return;
	}

	if (channel.noteShift != _noteShift[programNo]) {
		channel.noteShift = _noteShift[programNo];
		channel.hw->controlChange(kAllNotesOffController, 0);
		channel.hw->controlChange(kDamperPedalController, 0);
		needsVolumeUpdate = true;
	}

	if (needsVolumeUpdate || channel.volumeShift != _volumeShift[programNo]) {
		channel.volumeShift = _volumeShift[programNo];
		controllerChange(channelNo, kVolumeController, channel.volume);
	}

	channel.hw->programChange(programNo);
	debug("PC %2d %3d", channelNo, programNo);
}

void Sci1GeneralMidiDriver::pitchBend(const uint8 channelNo, const uint16 bend) {
	Channel &channel = _channels[channelNo];
	if (channel.pitchBend != bend) {
		channel.pitchBend = bend;
		channel.hw->pitchBend(bend);
		debug("PB %2d %04x", channelNo, bend);
	}
}

uint8 Sci1GeneralMidiDriver::setMasterVolume(const uint8 volume) {
	const uint8 oldVolume = _masterVolume;
	_masterVolume = volume;

	if (!_isEnabled) {
		return oldVolume;
	}

	debug("MV %2d", volume);
	for (int i = kMinChannel; i <= kPercussionChannel; ++i) {
		const uint8 channelVolume = _channels[i].volume;
		if (channelVolume != kUnmapped) {
			controllerChange(i, kVolumeController, channelVolume);
		}
	}

	return oldVolume;
}

void Sci1GeneralMidiDriver::enable(const bool enabled) {
	debug("EN %d", enabled);
	Sci1SoundDriver::enable(enabled);
	if (enabled) {
		setMasterVolume(_masterVolume);
	} else {
		for (int i = kMinChannel; i <= kPercussionChannel; ++i) {
			_channels[i].hw->controlChange(kVolumeController, 0);
		}
	}
}

void Sci1GeneralMidiDriver::sendBytes(Common::Span<const byte> data) const {
	byte command = 0;

	uint i = 0;
	while (i < data.size()) {
		byte op1, op2;

		if (data[i] & 0x80)
			command = data[i++];

		switch (command & 0xf0) {
		case 0xf0: {
			const byte *sysExStart = data.getUnsafeDataAt(i, data.size() - i);
			const byte *sysExEnd = (const byte *)memchr(sysExStart, 0xf7, data.size() - i);

			if (!sysExEnd)
				error("Failed to find end of sysEx");

			int len = sysExEnd - sysExStart;

			_device->sysEx(sysExEnd, len);

			// Wait the time it takes to send the SysEx data
			uint32 delay = (len + 2) * 1000 / 3125;
			g_system->updateScreen();
			g_sci->sleep(delay);

			i += len + 1; // One more for the 0xf7
			break;
		}
		case 0x80:
		case 0x90:
		case 0xa0:
		case 0xb0:
		case 0xe0:
			op1 = data[i++];
			op2 = data[i++];
			_device->send(command, op1, op2);
			break;
		case 0xc0:
		case 0xd0:
			op1 = data[i++];
			_device->send(command, op1, 0);
			break;
		default:
			error("Failed to find MIDI command byte");
		}
	}
}

SoundDriver *makeGeneralMidiDriver(ResourceManager &resMan, SciVersion version) {
	if (version <= SCI_VERSION_01) {
		error("General MIDI not supported by SCI0");
	} else {
		return new Sci1GeneralMidiDriver(resMan, version);
	}
}

} // End of namespace Sci