/*
 * VideoSource.h
 *
 *  Created on: 3 nov. 2009
 *      Author: herbelin
 *
 *  This file is part of GLMixer.
 *
 *   GLMixer is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   GLMixer is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GLMixer.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Copyright 2009, 2010 Bruno Herbelin
 *
 */

#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

#include <QObject>

#include "common.h"
#include "Source.h"
#include "VideoFile.h"
//class VideoFile;

class VideoSource : public QObject, public Source {

    Q_OBJECT

    friend class RenderingManager;

    // only RenderingManager can create a source
protected:
	VideoSource(VideoFile *f, GLuint texture, double d);
	virtual ~VideoSource();
    void update();

public Q_SLOTS:
	void play(bool on);
    void updateFrame (int i);
    void applyFilter();

public:
	RTTI rtti() const { return type; }
	bool isPlayable() const;
	bool isPlaying() const;

    inline VideoFile *getVideoFile() const { return is; }

	// Adjust brightness factor
	inline void setBrightness(int b) {
		if (isPlayable())
			is->setBrightness(b);
		else
			Source::setBrightness(b);
	}
	inline int getBrightness() const {
		if (isPlayable())
			return is->getBrightness();
		else
			return Source::getBrightness();
	}
	// Adjust contrast factor
	inline void setContrast(int c) {
		if (isPlayable())
			is->setContrast(c);
		else
			Source::setContrast(c);
	}
	inline int getContrast() const {
		if (isPlayable())
			return is->getContrast();
		else
			return Source::getContrast();
	}
	// Adjust saturation factor
	inline void setSaturation(int s) {
		if (isPlayable())
			is->setSaturation(s);
		else
			Source::setSaturation(s);
	}
	inline int getSaturation() const {
		if (isPlayable())
			return is->getSaturation();
		else
			return Source::getSaturation();
	}

	int getFrameWidth() const { return is->getFrameWidth(); }
	int getFrameHeight() const { return is->getFrameHeight(); }

private:

	static RTTI type;

    VideoFile *is;
    VideoPicture copy;

    bool filterChanged;
    int bufferIndex;

};

#endif /* VIDEOSOURCE_H_ */
