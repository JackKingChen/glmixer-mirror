/****************************************************************************
**

**
****************************************************************************/

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include "glRenderWidget.h"
#include "VideoFile.h"


class VideoFileDisplayWidget : public glRenderWidget
{
    Q_OBJECT

public:
    VideoFileDisplayWidget(QWidget *parent = 0);
    ~VideoFileDisplayWidget();

    // OpenGL implementation
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    virtual void showEvent ( QShowEvent * event ) { QGLWidget::showEvent(event);}


public slots:
    void updateFrame (int i);
    void setVideo(VideoFile* f);
    void setVideoAspectRatio(bool usevideoratio);

protected:

    VideoFile *is;
    GLuint squareDisplayList;
    GLuint textureIndex;
    bool useVideoAspectRatio;

};

#endif

