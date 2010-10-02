/*
 * OutputRenderWindow.h
 *
 *  Created on: Feb 10, 2010
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
 *   Copyright 2009, 2010 Bruno Herbelin
 *
 */

#ifndef OUTPUTRENDERWINDOW_H_
#define OUTPUTRENDERWINDOW_H_

class Source;
#include "glRenderWidget.h"
#include <QPropertyAnimation>

class OutputRenderWidget: public glRenderWidget {

	Q_OBJECT
    Q_PROPERTY(float alpha READ alpha WRITE setAlpha)

public:
	OutputRenderWidget(QWidget *parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);

    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w = 0, int h = 0);

	float getAspectRatio() const;
	inline bool freeAspectRatio() const { return !useAspectRatio; }
	void useFreeAspectRatio(bool on);

	void setAlpha(float a) { currentAlpha = a; }
	float alpha() const { return currentAlpha; }

	void setTransitionDuration(int duration);
	int transitionDuration() const;

	void setTransitionCurve(int curveType);
	int transitionCurve() const ;

	typedef enum {
		TRANSITION_NONE = 0,
		TRANSITION_BLACK = 1,
		TRANSITION_LAST_FRAME = 2
	} transitionType;
	void setTransitionType(transitionType t) {transition_type = t;}
	transitionType getTransitionType() const {return transition_type;}

public Q_SLOTS:
	void refresh();
	void smoothAlphaTransition(bool visible, Source *temporaryBackgound = 0);

Q_SIGNALS:
	void animationFinished();

protected:
	bool useAspectRatio, useWindowAspectRatio;
	int rx, ry, rw, rh;

	float currentAlpha;
	Source *backgroundSource;
	QPropertyAnimation *animationAlpha;
	transitionType transition_type;
};

class OutputRenderWindow : public OutputRenderWidget {

	Q_OBJECT

public:
	// get singleton instance
	static OutputRenderWindow *getInstance();

    void initializeGL();
    void resizeGL(int w = 0, int h = 0);

	// events handling
	void keyPressEvent(QKeyEvent * event);
	void mouseDoubleClickEvent(QMouseEvent * event);

public Q_SLOTS:
	void setFullScreen(bool on);

Q_SIGNALS:
	void resized();

	/**
	 * singleton mechanism
	 */
private:
	OutputRenderWindow();
	static OutputRenderWindow *_instance;

};

#endif /* OUTPUTRENDERWINDOW_H_ */
