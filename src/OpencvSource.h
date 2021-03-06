/*
 * OpencvSource.h
 *
 *  Created on: Dec 13, 2009
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

#ifndef OPENCVSOURCE_H_
#define OPENCVSOURCE_H_

#include "Source.h"

#include <opencv2/highgui/highgui.hpp>
#include <stdexcept>

#include <QMap>
#include <QMutex>
#include <QWaitCondition>

class CameraThread;

class NoCameraIndexException : public SourceConstructorException {
public:
    virtual QString message() { return "Opencv index is not valid."; }
    void raise() const { throw *this; }
    Exception *clone() const { return new NoCameraIndexException(*this); }
};

class UnavailableCameraIndexException : public SourceConstructorException {
public:
    virtual QString message() { return "Camera is not available."; }
    void raise() const { throw *this; }
    Exception *clone() const { return new UnavailableCameraIndexException(*this); }
};

class brokenCameraException : public SourceConstructorException {
public:
    virtual QString message() { return "Unable to get images from camera."; }
    void raise() const { throw *this; }
    Exception *clone() const { return new brokenCameraException(*this); }
};

class OpencvSource: public Source {

    Q_OBJECT

    friend class CameraDialog;
    friend class RenderingManager;
    friend class CameraThread;

public:

    RTTI rtti() const { return type; }
    QString getInfo() const;
    bool isPlayable() const { return playable; }
    bool isPlaying() const;

    inline int getOpencvCameraIndex() const { return opencvCameraIndex; }
    inline double getFrameRate() const { return framerate; }
    inline int getFrameWidth() const { return width; }
    inline int getFrameHeight() const { return height; }
    typedef enum {
        DEFAULT_MODE = 0,
        LOWRES_MODE,
        HIGHRES_MODE
    } CameraMode;
    inline CameraMode getMode() const { return mode; }

    QDomElement getConfiguration(QDomDocument &doc, QDir current);

    static OpencvSource *getExistingSourceForCameraIndex(int);
    static QString getOpencvVersion();

public slots:
    void play(bool on);

protected:
    // only friends can create a source (need its GL context)
    OpencvSource(int opencvIndex, CameraMode m, GLuint texture, double d);
    ~OpencvSource();

    static RTTI type;
    static bool playable;

    void update();

    int opencvCameraIndex;
    CameraMode mode;
    CvCapture* capture;
    int width, height;
    double framerate;
    IplImage *frame;
    bool needFrameCopy, frameChanged;
    bool failure;

    CameraThread *thread;
    QMutex *mutex;
    QWaitCondition *cond;

    static QMap<int, OpencvSource*> _existingSources;

    GLuint pboIds[2];
    int index, nextIndex;
    int imgsize;

};

#endif /* OPENCVSOURCE_H_ */
