/*
 * CameraDialog.cpp
 *
 *  Created on: Dec 19, 2009
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
 *   Copyright 2009, 2018 Bruno Herbelin
 *
 */

#include "CameraDialog.moc"
#include "ui_CameraDialog.h"

#include "VideoStreamSource.h"
#include "common.h"
#include "CodecManager.h"

#ifdef GLM_OPENCV
#include "OpencvSource.h"
#endif

#ifdef Q_OS_MAC
#include "avfoundation.h"
#endif

#ifdef Q_OS_WIN
#include "directshow.h"
#endif

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QProcessEnvironment>



CameraDialog::CameraDialog(QWidget *parent, QSettings *settings) :
    QDialog(parent),
    ui(new Ui::CameraDialog),
    s(NULL), appSettings(settings)
{
    testingtimeout = new QTimer(this);
    testingtimeout->setSingleShot(true);
    connect(testingtimeout, SIGNAL(timeout()), this, SLOT(failedInfo()));

    respawn = new QTimer(this);
    respawn->setSingleShot(true);
    connect(respawn, SIGNAL(timeout()), this, SLOT(cancelSourcePreview()));

    ui->setupUi(this);

    // discard opencv if not available
#ifndef GLM_OPENCV
    ui->deviceSelection->removeTab( ui->deviceSelection->indexOf(ui->deviceOpenCV));
#endif

    // fill list of decklink video formats if available
    if ( CodecManager::hasFormat("decklink") ) {
        ui->decklinkVideoFormat->addItem("720x486 at 59.94 fps NTSC interlaced", "ntsc");
        ui->decklinkVideoFormat->addItem("720x486 at 23.98 fps NTSC", "nt23");
        ui->decklinkVideoFormat->addItem("720x576 at 50 fps PAL interlaced", "pal");
        ui->decklinkVideoFormat->addItem("720x486 at 59.94 fps NTSC", "ntsp");
        ui->decklinkVideoFormat->addItem("720x576 at 50 fps PAL", "palp");
        ui->decklinkVideoFormat->addItem("1920x1080 at 23.98 fps", "23ps");
        ui->decklinkVideoFormat->addItem("1920x1080 at 24 fps", "24ps");
        ui->decklinkVideoFormat->addItem("1920x1080 at 25 fps", "Hp25");
        ui->decklinkVideoFormat->addItem("1920x1080 at 29.97 fps", "Hp29");
        ui->decklinkVideoFormat->addItem("1920x1080 at 30 fps", "Hp30");
        ui->decklinkVideoFormat->addItem("1920x1080 at 50 fps interlaced", "Hi50");
        ui->decklinkVideoFormat->addItem("1920x1080 at 59.94 fps interlaced", "Hi59");
        ui->decklinkVideoFormat->addItem("1920x1080 at 60 fps interlaced", "Hi60");
        ui->decklinkVideoFormat->addItem("1280x720 at 50 fps", "hp50");
        ui->decklinkVideoFormat->addItem("1280x720 at 59.94 fps", "hp59");
        ui->decklinkVideoFormat->addItem("1280x720 at 60 fps", "hp60");
    }
    // discard decklink if not available
    else
        ui->deviceSelection->removeTab( ui->deviceSelection->indexOf(ui->deviceDecklink) );

#ifdef Q_OS_MAC
    ui->geometryBox->setVisible(false);
#else
    ui->screenCaptureSelection->addItem( "Capture entire screen" );
    ui->screenCaptureSelection->addItem( "Capture custom area" );
#endif

    // restore settings
    if (appSettings) {
        if (appSettings->contains("dialogCameraGeometry"))
            restoreGeometry(appSettings->value("dialogCameraGeometry").toByteArray());
    }
}

CameraDialog::~CameraDialog()
{
    if (s)
        delete s;
    delete ui;
}

void CameraDialog::showHelp()
{
    QDesktopServices::openUrl(QUrl("https://sourceforge.net/p/glmixer/wiki/Input%20devices%20support/", QUrl::TolerantMode));
}

void CameraDialog::done(int r)
{
    cancelSourcePreview();

    // save settings
    if (appSettings) {
        appSettings->setValue("ScreenCaptureSize", QRect(ui->screen_x->value(), ui->screen_y->value(), ui->screen_w_selection->currentIndex(), ui->screen_h->value()));
        appSettings->setValue("ScreenCaptureMode", ui->screenCaptureSelection->currentIndex());
        appSettings->setValue("dialogCameraGeometry", saveGeometry());
    }

    QDialog::done(r);
}


void CameraDialog::updateScreenCaptureWidth(int index)
{
    int w = ui->screen_w_selection->itemData(index).toInt();
    ui->screen_x->setMaximum(screendimensions.width() - w);

    setScreenCaptureArea();
}

void CameraDialog::updateScreenCaptureHeight(int value)
{
    ui->screen_y->setMaximum(screendimensions.height() - value);

    setScreenCaptureArea();
}

void CameraDialog::connectedInfo(bool on)
{
    if (on) {
        testingtimeout->stop();
        ui->info->setCurrentIndex(2);
    }
}

void CameraDialog::failedInfo()
{
    ui->info->setCurrentIndex(3);
    respawn->start(1000);
}

void CameraDialog::cancelSourcePreview(){

    testingtimeout->stop();
    ui->info->setCurrentIndex(0);

    // remove source from preview: this deletes the texture in the preview
    ui->preview->setSource(0);

    // delete previous
    if(s) {
        // delete the source:
        delete s;
        s = NULL;
    }
}

void CameraDialog::updateSourcePreview(){

    // texture for source
    GLuint tex = ui->preview->getNewTextureIndex();

    // update GUI
    ui->info->setCurrentIndex(1);
    testingtimeout->start(5000);

#ifdef GLM_OPENCV
    if ( ui->deviceSelection->currentWidget() == ui->deviceOpenCV )
    {
        try {
            // create a new source with a new texture index and the new parameters
            s = new OpencvSource( getOpencvIndex(), OpencvSource::DEFAULT_MODE, tex, 0);
            QObject::connect(s, SIGNAL(failed()), this, SLOT(failedInfo()));
            QObject::connect(s, SIGNAL(playing(bool)), this, SLOT(connectedInfo(bool)));
        }
        catch (...)  {
            qWarning() << tr("Opencv Source Creation error; ");
            // free the OpenGL texture
            glDeleteTextures(1, &tex);
            // return an invalid pointer
            s = NULL;
        }
    }
    else
#endif
    {
        // create video stream
        VideoStream *vs = new VideoStream();
        // open with parameters
        vs->open(getUrl(), getFormat(), getFormatOptions());

        try {
            // create a new source with a new texture index and the new parameters
            s = new VideoStreamSource(vs, tex, 0);
            QObject::connect(s, SIGNAL(failed()), this, SLOT(failedInfo()));
            QObject::connect(vs, SIGNAL(openned()), this, SLOT(connectedInfo()));
            QObject::connect(vs, SIGNAL(openned()), s, SLOT(updateAspectRatioStream()));
        }
        catch (...)  {
            qWarning() << tr("Video Stream Creation error; ");
            // free the OpenGL texture
            glDeleteTextures(1, &tex);
            // return an invalid pointer
            s = NULL;
        }
    }

    // apply the source to the preview (null pointer is ok to reset preview)
    ui->preview->setSource(s);
    ui->preview->playSource(true);

}


#ifdef GLM_OPENCV

bool CameraDialog::isOpencvSelected() const
{
    return ( ui->deviceSelection->currentWidget() == ui->deviceOpenCV ) ;
}

int CameraDialog::getOpencvIndex() const
{
    return ui->opencvId->currentIndex();
}
#endif

#ifdef Q_OS_LINUX

QString CameraDialog::getUrl() const
{
    QString url = "";

    // webcam
    if ( ui->deviceSelection->currentWidget() == ui->deviceWebcam )
        // read data which gives the device id
        url = ui->webcamDevice->itemData( ui->webcamDevice->currentIndex()).toString();
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen )
        // read system DISPLAY
        url = QProcessEnvironment::systemEnvironment().value("DISPLAY");

    return url;
}

QString CameraDialog::getFormat() const
{
    QString format = "";
    // webcam
    if ( ui->deviceSelection->currentWidget() == ui->deviceWebcam )
        format = "video4linux2";
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen )
        format = "x11grab";

    return format;
}

QHash<QString, QString> CameraDialog::getFormatOptions() const
{
    QHash<QString, QString> options;
    // webcam
    if ( ui->deviceSelection->currentWidget() == ui->deviceWebcam ) {
        if (ui->webcamResolution->currentIndex()!=0)
            options["video_size"] = ui->webcamResolution->currentText();
        if (ui->webcamFramerate->currentIndex()!=0)
            options["framerate"] = ui->webcamFramerate->currentText();
    }
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen ) {
        options["framerate"] = "30";
        options["draw_mouse"] = ui->screen_cursor->isChecked() ? "1" : "0";
        options["video_size"] = QString("%1x%2").arg(capturedimensions.width()).arg(capturedimensions.height());
        options["grab_x"] = QString::number(capturedimensions.x());
        options["grab_y"] = QString::number(capturedimensions.y());
    }

    return options;
}

void CameraDialog::showEvent(QShowEvent *e){

    // read the device list
    ui->webcamDevice->clear();
    QHash<QString, QString> devices;

    devices = CodecManager::getDeviceList( "video4linux2" );
    // fill-in list of devices
    QHashIterator<QString, QString> i(devices);
    while (i.hasNext()) {
        i.next();
        ui->webcamDevice->addItem( QString("%1 [%2]").arg(i.value()).arg(i.key()), i.key());
    }

    // read dimensions of the desktop to set screen capture maximum
    screendimensions = QApplication::desktop()->screen()->geometry();
    ui->screen_h->setMaximum(screendimensions.height());

    // fill-in
    ui->screen_w_selection->clear();
    int w =  screendimensions.width() ;
    while ( w > 255 ) {
        ui->screen_w_selection->addItem(QString::number(w),w);
        w = roundPowerOfTwo(w/2);
    }
//    ui->screen_w_selection->setCurrentIndex(0);

    // restore settings
    if (appSettings) {
        if (appSettings->contains("ScreenCaptureSize")) {
            QRect sizescreencapture;
            sizescreencapture = appSettings->value("ScreenCaptureSize").toRect();
            ui->screen_w_selection->setCurrentIndex( sizescreencapture.width() );
            ui->screen_h->setValue( sizescreencapture.height());
            ui->screen_x->setValue( sizescreencapture.x());
            ui->screen_y->setValue( sizescreencapture.y());
        }
        ui->screenCaptureSelection->setCurrentIndex( appSettings->value("ScreenCaptureMode", 0).toInt() );
    }
    else
        ui->screenCaptureSelection->setCurrentIndex( 0 );

    QWidget::showEvent(e);
}


void CameraDialog::setScreenCaptureArea()
{
    int index = ui->screenCaptureSelection->currentIndex();
    ui->geometryBox->setEnabled(index != 0);

    if (index == 0)  {
        capturedimensions = screendimensions;
    }
    else {
        int w = ui->screen_w_selection->currentText().toInt();
        capturedimensions = QRect(ui->screen_x->value(), ui->screen_y->value(), w, ui->screen_h->value());
    }

    cancelSourcePreview();
}

#elif defined Q_OS_MAC

QString CameraDialog::getUrl() const
{
    QString url = "";

    // webcam
    if (ui->deviceSelection->currentWidget() == ui->deviceWebcam )
        // read data which gives the device id
        url = ui->webcamDevice->itemData( ui->webcamDevice->currentIndex() ).toString();
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen )
        // read index of the first screen
        url = ui->screenCaptureSelection->itemData( ui->screenCaptureSelection->currentIndex()).toString();
    // decklink
    else if (ui->deviceSelection->currentWidget() == ui->deviceDecklink )
        // read data which gives the input stream description string
        url = ui->decklinkDevice->itemData( ui->decklinkDevice->currentIndex() ).toString();

    return url;
}

QString CameraDialog::getFormat() const
{
    if (ui->deviceSelection->currentWidget() == ui->deviceDecklink )
        return "decklink";
    else
        return "avfoundation";
}

QHash<QString, QString> CameraDialog::getFormatOptions() const
{
    QHash<QString, QString> options;

    // webcam
    if ( ui->deviceSelection->currentWidget() == ui->deviceWebcam ) {
        if (ui->webcamResolution->currentIndex()!=0)
            options["video_size"] = ui->webcamResolution->currentText();
        if (ui->webcamFramerate->currentIndex()!=0)
            options["framerate"] = ui->webcamFramerate->currentText();
    }
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen ) {
        options["framerate"] = "30";
        options["capture_cursor"] = ui->screen_cursor->isChecked() ? "1" : "0";
    }
    // decklink
    else if (ui->deviceSelection->currentWidget() == ui->deviceDecklink ) {
        options["format_code"] = ui->decklinkVideoFormat->itemData(ui->decklinkVideoFormat->currentIndex() ).toString();
    }

    return options;
}

void CameraDialog::showEvent(QShowEvent *e){

    // read the device list
    ui->webcamDevice->clear();
    QHash<QString, QString> devices;
    devices = avfoundation::getDeviceList();
    QHashIterator<QString, QString> i(devices);
    while (i.hasNext()) {
        i.next();
        ui->webcamDevice->addItem( QString("%1 [%2]").arg(i.value()).arg(i.key()), i.key());
    }

    // fill-in list of screens
    ui->screenCaptureSelection->clear();
    QHash<QString, QString> screens;
    screens = avfoundation::getScreenList();
    QHashIterator<QString, QString> j(screens);
    while (j.hasNext()) {
        j.next();
        ui->screenCaptureSelection->addItem( QString("%1 [%2]").arg(j.value()).arg(j.key()), j.key());
    }

    // decklink if available
    if ( CodecManager::hasFormat("decklink") ) {
        ui->decklinkDevice->clear();
        QHash<QString, QString> devices;
        devices = CodecManager::getDeviceList( "decklink" );
        QHashIterator<QString, QString> i(devices);
        while (i.hasNext()) {
            i.next();
            ui->decklinkDevice->addItem(i.value(), i.key());
        }
    }

    QWidget::showEvent(e);
}

void CameraDialog::setScreenCaptureArea()
{
    cancelSourcePreview();
}

#else

QString CameraDialog::getUrl() const
{
    QString url = "";

    // webcam
    if (ui->deviceSelection->currentWidget() == ui->deviceWebcam )
        // read data which gives the device id
        url = ui->webcamDevice->itemData( ui->webcamDevice->currentIndex() ).toString();
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen )
        // read desktop
        url = "desktop";
    // decklink
    else if (ui->deviceSelection->currentWidget() == ui->deviceDecklink )
        // read data which gives the input stream description string
        url = ui->decklinkDevice->itemData( ui->decklinkDevice->currentIndex() ).toString();

    return url;
}

QString CameraDialog::getFormat() const
{
    QString format = "";
    // webcam
    if ( ui->deviceSelection->currentWidget() == ui->deviceWebcam )
        format = "dshow";
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen )
        format = "gdigrab";
    // decklink
    else if (ui->deviceSelection->currentWidget() == ui->deviceDecklink )
        format = "decklink";

    return format;
}

QHash<QString, QString> CameraDialog::getFormatOptions() const
{
    QHash<QString, QString> options;

    // webcam
    if ( ui->deviceSelection->currentWidget() == ui->deviceWebcam ) {
        if (ui->webcamResolution->currentIndex()!=0)
            options["video_size"] = ui->webcamResolution->currentText();
        if (ui->webcamFramerate->currentIndex()!=0)
            options["framerate"] = ui->webcamFramerate->currentText();
    }
    // screen capture
    else if (ui->deviceSelection->currentWidget() == ui->deviceScreen ) {
        options["framerate"] = "30";
        options["draw_mouse"] = ui->screen_cursor->isChecked() ? "1" : "0";
        if (ui->screenCaptureSelection->currentIndex() != 0) {
            options["video_size"] = QString("%1x%2").arg(capturedimensions.width()).arg(capturedimensions.height());
            options["offset_x"] = QString::number(capturedimensions.x());
            options["offset_y"] = QString::number(capturedimensions.y());
            //options["show_region"] = "0";
        }
    }
    // decklink
    else if (ui->deviceSelection->currentWidget() == ui->deviceDecklink ) {
        options["format_code"] = ui->decklinkVideoFormat->itemData(ui->decklinkVideoFormat->currentIndex() ).toString();
    }

    return options;
}

void CameraDialog::showEvent(QShowEvent *e){

    // read the device list
    ui->webcamDevice->clear();
    QHash<QString, QString> devices;
    devices = directshow::getDeviceList();
    QHashIterator<QString, QString> i(devices);
    while (i.hasNext()) {
        i.next();
        ui->webcamDevice->addItem( QString("%1").arg(i.value()), i.key());
    }

    // read dimensions of the desktop to set screen capture maximum
    screendimensions = QApplication::desktop()->screen()->geometry();
    ui->screen_h->setMaximum(screendimensions.height());

    // fill-in
    ui->screen_w_selection->clear();
    int w =  screendimensions.width() ;
    while ( w > 255 ) {
        ui->screen_w_selection->addItem(QString::number(w),w);
        w = roundPowerOfTwo(w/2);
    }

    // restore settings
    if (appSettings) {
        if (appSettings->contains("ScreenCaptureSize")) {
            QRect sizescreencapture;
            sizescreencapture = appSettings->value("ScreenCaptureSize").toRect();
            ui->screen_w_selection->setCurrentIndex( sizescreencapture.width() );
            ui->screen_h->setValue( sizescreencapture.height());
            ui->screen_x->setValue( sizescreencapture.x());
            ui->screen_y->setValue( sizescreencapture.y());
        }
        ui->screenCaptureSelection->setCurrentIndex( appSettings->value("ScreenCaptureMode", 0).toInt() );
    }
    else
        ui->screenCaptureSelection->setCurrentIndex( 0 );


    // decklink if available
    if ( CodecManager::hasFormat("decklink") ) {
        ui->decklinkDevice->clear();
        QHash<QString, QString> devices;
        devices = CodecManager::getDeviceList( "decklink" );
        QHashIterator<QString, QString> i(devices);
        while (i.hasNext()) {
            i.next();
            ui->decklinkDevice->addItem(i.value(), i.key());
        }
    }

    QWidget::showEvent(e);
}

void CameraDialog::setScreenCaptureArea()
{
    int index = ui->screenCaptureSelection->currentIndex();
    ui->geometryBox->setEnabled(index != 0);

    if (index == 0)  {
        capturedimensions = screendimensions;
    }
    else {
        int w = ui->screen_w_selection->currentText().toInt();
        capturedimensions = QRect(ui->screen_x->value(), ui->screen_y->value(), w, ui->screen_h->value());
    }

    cancelSourcePreview();
}



#endif


