/*
 * videoFileDialog.h
 *
 *  Created on: Aug 3, 2009
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

#ifndef VIDEOFILEDIALOG_H_
#define VIDEOFILEDIALOG_H_

#include <QFileDialog>
#include <QCheckBox>

#include "VideoFileDialogPreview.h"

#define VIDEOFILE_DIALOG_FORMATS "Media (*.mov *.avi *.wmv *.mpeg *.mp4 *.mpg *.mjpeg *.swf *.flv *.mod *.mkv *.xvid *.dv *.webm *.png *.jpg *.jpeg *.tif *.tiff *.gif *.tga *.sgi *.bmp *.ppm);;Any file (*.*)"

class VideoFileDialog: public QFileDialog
{
    Q_OBJECT

public:
    VideoFileDialog( QWidget * parent = 0, const QString & caption = QString());
    ~VideoFileDialog();

    bool requestCustomSize();
    bool requestHarwareDecoder();
    void showEvent ( QShowEvent * );
    void hideEvent ( QHideEvent * );

    QSize sizeHint() const {
        return QSize(800, 600);
    }

public slots:
    void setPreviewVisible(bool visible);
    void updatePreview(const QString &);

private:

    VideoFileDialogPreview *preview;
    QCheckBox *pv;
    QFileInfo file;
};

#endif /* VideoFileDialog_H_ */
