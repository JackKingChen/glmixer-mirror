/*
 * RenderingEncoder.h
 *
 *  Created on: Mar 13, 2011
 *      Author: bh
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
 *   Copyright 2009, 2012 Bruno Herbelin
 *
 */

#ifndef RENDERINGENCODER_H_
#define RENDERINGENCODER_H_

#include <QObject>
#include <QTime>
#include <QString>

#define RECORDING_BUFFER_SIZE 10

extern "C" {
#include "video_rec.h"
}

#include "RenderingManager.h"

class EncodingThread;

class RenderingEncoder: public QObject {

	Q_OBJECT

    friend class EncodingThread;

public:
	RenderingEncoder(QObject * parent = 0);
	~RenderingEncoder();

	bool start();
    void addFrame(unsigned char *data = 0);
	bool close();

	// preferences encoding
	void setEncodingFormat(encodingformat f);
	encodingformat encodingFormat() { return format; }
	void setUpdatePeriod(uint ms) { update=ms; }
	uint updatePeriod() { return update; }

	// preferences saving mode
	void setAutomaticSavingMode(bool on);
	bool automaticSavingMode() { return automaticSaving;}
	void setAutomaticSavingFolder(QDir d);
	QDir automaticSavingFolder() { return savingFolder; }

	// status
	bool isActive() { return started; }
	int getRecodingTime();
	bool isRecording() { return started && !paused ; }


public Q_SLOTS:
	void setFrameSize(QSize s) { if (!started) framesSize = s; }
	void setActive(bool on);
	void setPaused(bool on);
	void saveFile();
	void saveFileAs();

Q_SIGNALS:
	void activated(bool);
	void status(const QString &, int);
	void selectAspectRatio(const standardAspectRatio );

protected:
    void timerEvent(QTimerEvent *event);

private:
	// files location
    QString temporaryFileName;
	QDir savingFolder, temporaryFolder;
	bool automaticSaving;

	// state machine
	bool started, paused;
	QTime timer;
    int elapseTimer, skipframecount;

	// encoder
	QSize framesSize;
    EncodingThread *encoder;

	uint update, displayupdate;
	encodingformat format;
	video_rec_t *recorder;
	char errormessage[256];
};

#endif /* RENDERINGENCODER_H_ */
