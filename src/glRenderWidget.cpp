/*
 * glRenderWidget.cpp
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
 *   Copyright 2009, 2012 Bruno Herbelin
 *
 */

#include <QTimer>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QDialogButtonBox>
#include <QIcon>
#include <QDialog>
#include <QStringListModel>
#include <QAbstractItemModel>

#include "common.h"
#include "glRenderWidget.moc"

glRenderTimer *glRenderTimer::_instance = 0;


glRenderWidget::glRenderWidget(QWidget *parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
: QGLWidget(glRenderWidgetFormat(), parent, shareWidget, f), aspectRatio(1.0), antialiasing(true)

{
    connect(glRenderTimer::getInstance(), SIGNAL(timeout()), this, SLOT(updateGL()));
}

void glRenderWidget::setAntiAliasing(bool on)
{
    antialiasing = on;
    makeCurrent();

    // OPENGL ANTIALIASING
    if (antialiasing) {
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    } else {
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POINT_SMOOTH);
    }
}

void glRenderWidget::initializeGL()
{
    // Set flat color shading without dithering
    glShadeModel(GL_FLAT);
    glDisable(GL_DITHER);
    glDisable(GL_POLYGON_SMOOTH);

    // disable depth and lighting by default
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);

    // Enables texturing
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    // default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);//We add these two lines
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); //because we don't have mipmaps.

    // ensure alpha channel is modulated ; otherwise the source is not mixed by its alpha channel
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

    // Turn blending on
    glEnable(GL_BLEND);

    // Blending Function For transparency Based On Source Alpha Value
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // This hint can improve the speed of texturing when perspective-correct texture coordinate interpolation isn't needed
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    // This hint can improve the speed of shading when dFdx dFdy aren't needed in GLSL
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_FASTEST);

    // setup default background color to black
    glClearColor(0.0, 0.0, 0.0, 1.0f);

    setAntiAliasing(antialiasing);

}


void glRenderWidget::setBackgroundColor(const QColor &c){

    makeCurrent();
    qglClearColor(c);
}

void glRenderWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    aspectRatio = (float) w / (float) h;

    // Setup specific projection and view for this window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

}

void glRenderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void glRenderWidget::showGlExtensionsInformationDialog(QString iconfile){

    QDialog *openglExtensionsDialog;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QListView *extensionsListView;
    QDialogButtonBox *buttonBox;

    openglExtensionsDialog = new QDialog(0);
    if (!iconfile.isEmpty()){
        QIcon icon;
        icon.addFile(iconfile, QSize(), QIcon::Normal, QIcon::Off);
        openglExtensionsDialog->setWindowIcon(icon);
    }
    openglExtensionsDialog->setWindowTitle(QObject::tr("About OpenGL"));
    openglExtensionsDialog->resize(450, 400);
    verticalLayout = new QVBoxLayout(openglExtensionsDialog);
    verticalLayout->setSpacing(10);

    label = new QLabel(openglExtensionsDialog);
    verticalLayout->addWidget(label);

    label->setWordWrap(true);
    label->setOpenExternalLinks(true);
    label->setTextFormat(Qt::RichText);
    label->setText(QObject::tr("<H3>About OpenGL</H3>"
                               "<br>This program currently runs with <b>OpenGL version %1</b> on %3 (%2)."
                               "<br><br><b>OpenGL</b> is the premier environment for developing portable, interactive 2D and 3D graphics applications."
                               "<br>See <a href=\"https://www.opengl.org\">https://www.opengl.org</a> for more information."
                               "<br><br>Supported extensions:").arg((char *)glGetString(GL_VERSION)).arg((char *)glGetString(GL_VENDOR)).arg((char *)glGetString(GL_RENDERER)) );

    extensionsListView = new QListView(openglExtensionsDialog);
    QAbstractItemModel *model = new QStringListModel(glSupportedExtensions());
    extensionsListView->setModel(model);

    verticalLayout->addWidget(extensionsListView);

    buttonBox = new QDialogButtonBox(openglExtensionsDialog);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, SIGNAL(accepted()), openglExtensionsDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), openglExtensionsDialog, SLOT(reject()));

    openglExtensionsDialog->exec();

    delete verticalLayout;
    delete label;
    delete extensionsListView;
    delete buttonBox;
    delete model;
    delete openglExtensionsDialog;
}

glRenderTimer *glRenderTimer::getInstance()
{
    if (_instance == 0) {
        glRenderTimer::_instance = new glRenderTimer();
    }
    return _instance;
}

glRenderTimer::glRenderTimer() : QObject(), _interval(20), _updater(0), _activeTiming(false)
{
    _elapsedTimer = new QElapsedTimer();
    _timer = new QTimer();
    connect(_timer, SIGNAL(timeout()), this, SIGNAL(timeout()));

    restartTimer(false);
}

void glRenderTimer::setInterval(int ms)
{
    _interval = ms;

    // if not in active update, change qtimer interval
    if (_updater == 0)
        _timer->start(_interval);
}

void glRenderTimer::setActiveTimingMode(bool on)
{
    // set state
    _activeTiming = on;

    // change timer
    restartTimer(_activeTiming);
}

void glRenderTimer::restartTimer(bool active)
{
    // cancel all
    if (_updater){
        killTimer(_updater);
        _updater = 0;
    }
    _timer->stop();

    // active timing mode
    if (active) {
        _updater = startTimer(0);
        _elapsedTimer->start();
    }
    // passive timing mode
    else {
        _timer->start(_interval);
    }
}

void glRenderTimer::beginActiveTiming()
{
    // toggle active bloc only if not in active mode
    if (!_activeTiming && _updater == 0) {
        // activate the active updater
        restartTimer(true);
    }
}

void glRenderTimer::endActiveTiming()
{
    // toggle active bloc only if not in active mode
    if (!_activeTiming) {
        // deactivate the active updater
        restartTimer(false);
    }
}

void glRenderTimer::timerEvent(QTimerEvent * event)
{
    // did not reach interval: discard
    if ( _elapsedTimer->elapsed() < _interval )
        return;

    // restart time counter
    if ( _elapsedTimer->restart() < 2 * _interval )
        // emit extra timeout if too much delay
        emit timeout();

    // NB: sending less signal when system is slow is not ideal
    // but this allows recovering faster
}
