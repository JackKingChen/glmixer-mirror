/*
 * AxisCursor.h
 *
 *  Created on: May 9, 2011
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

#ifndef AxisCursor_H_
#define AxisCursor_H_

#include <QObject>

#include "Cursor.h"
#define INIT_THRESHOLD 15.0

class AxisCursor: public QObject, public Cursor
{
    Q_OBJECT

public:
    AxisCursor();

    bool apply(double fpsaverage);
    bool wheelEvent(QWheelEvent * event);
    void draw(GLint viewport[4]);

public slots:
    inline void setScrollingEnabled(bool on) { scrollEnabled = on;}

private:

    bool x_axis, scrollEnabled;
};

#endif /* AxisCursor_H_ */
