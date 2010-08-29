/*
 * UserPreferencesDialog.cpp
 *
 *  Created on: Jul 16, 2010
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

#include "UserPreferencesDialog.moc"

#include "Source.h"

UserPreferencesDialog::UserPreferencesDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);

    defaultSource = new Source;
    defaultProperties->showProperties(defaultSource);
    defaultProperties->setPropertyEnabled("Type", false);
    defaultProperties->setPropertyEnabled("Scale", false);
    defaultProperties->setPropertyEnabled("Depth", false);
    defaultProperties->setPropertyEnabled("Aspect ratio", false);

    activateBlitFrameBuffer->setEnabled(glSupportsExtension("GL_EXT_framebuffer_blit"));
}

UserPreferencesDialog::~UserPreferencesDialog()
{
	delete defaultSource;
}

void UserPreferencesDialog::restoreDefaultPreferences() {

	if (stackedPreferences->currentWidget() == PageRendering)
		r640x480->setChecked(true);


	if (stackedPreferences->currentWidget() == PageSources) {
		if(defaultSource)
			delete defaultSource;
		defaultSource = new Source;
		defaultProperties->showProperties(defaultSource);

		defaultStartPlaying->setChecked(true);
		scalingModeSelection->setCurrentIndex(0);
		numberOfFramesRendering->setValue(1);
	}


	if (stackedPreferences->currentWidget() == PageInterface){
		FINE->setChecked(true);
	}
}

void UserPreferencesDialog::showPreferences(const QByteArray & state){

	if (state.isEmpty())
	        return;

	QByteArray sd = state;
	QDataStream stream(&sd, QIODevice::ReadOnly);

	const quint32 magicNumber = MAGIC_NUMBER;
    const quint16 currentMajorVersion = QSETTING_PREFERENCE_VERSION;
	quint32 storedMagicNumber;
    quint16 majorVersion = 0;
	stream >> storedMagicNumber >> majorVersion;
	if (storedMagicNumber != magicNumber || majorVersion != currentMajorVersion)
		return;

	// a. Read and show the rendering preferences
	QSize RenderingSize;
	stream  >> RenderingSize;
	sizeToSelection(RenderingSize);

	bool useBlitFboExtension = true;
	stream >> useBlitFboExtension;
	activateBlitFrameBuffer->setChecked(useBlitFboExtension);

	// b. Read and setup the default source properties
	stream >> defaultSource;
    defaultProperties->showProperties(defaultSource);

	// c. Default scaling mode
    unsigned int sm = 0;
    stream >> sm;
    scalingModeSelection->setCurrentIndex(sm);

    // d. DefaultPlayOnDrop
    bool DefaultPlayOnDrop = false;
    stream >> DefaultPlayOnDrop;
    defaultStartPlaying->setChecked(DefaultPlayOnDrop);

	// e.  PreviousFrameDelay
	unsigned int  PreviousFrameDelay = 1;
	stream >> PreviousFrameDelay;
	numberOfFramesRendering->setValue( (unsigned int) PreviousFrameDelay);

	// f. Mixing icons stippling
	unsigned int  stippling = 0;
	stream >> stippling;
	switch (stippling) {
	case 3:
		TRIANGLE->setChecked(true);
		break;
	case 2:
		CHECKERBOARD->setChecked(true);
		break;
	case 1:
		GROSS->setChecked(true);
		break;
	default:
		FINE->setChecked(true);
		break;
	}

}

QByteArray UserPreferencesDialog::getUserPreferences() const {

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    const quint32 magicNumber = MAGIC_NUMBER;
    quint16 majorVersion = QSETTING_PREFERENCE_VERSION;
	stream << magicNumber << majorVersion;

	// a. write the rendering preferences
	stream << selectionToSize() << activateBlitFrameBuffer->isChecked();

	// b. Write the default source properties
	stream 	<< defaultSource;

	// c. Default scaling mode
	stream << (unsigned int) scalingModeSelection->currentIndex();

	// d. defaultStartPlaying
	stream << defaultStartPlaying->isChecked();

	// e. PreviousFrameDelay
	stream << (unsigned int) numberOfFramesRendering->value();

	// f. Mixing icons stippling
	if (FINE->isChecked())
		stream << (unsigned int) 0;
	if (GROSS->isChecked())
		stream << (unsigned int) 1;
	if (CHECKERBOARD->isChecked())
		stream << (unsigned int) 2;
	if (TRIANGLE->isChecked())
		stream << (unsigned int) 3;

	return data;
}

void UserPreferencesDialog::sizeToSelection(QSize s){

	if (s==QSize(640, 480)) r640x480->setChecked(true);
    if (s==QSize(600, 400)) r600x400->setChecked(true);
    if (s==QSize(640, 400)) r640x400->setChecked(true);
    if (s==QSize(854, 480)) r854x480->setChecked(true);
    if (s==QSize(768, 576)) r768x576->setChecked(true);
    if (s==QSize(720, 480)) r720x480->setChecked(true);
    if (s==QSize(800, 600)) r800x600->setChecked(true);
    if (s==QSize(960, 600)) r960x600->setChecked(true);
    if (s==QSize(1042, 768)) r1042x768->setChecked(true);
    if (s==QSize(1152, 768)) r1152x768->setChecked(true);
    if (s==QSize(1280, 800)) r1280x800->setChecked(true);
    if (s==QSize(1280, 720)) r1280x720->setChecked(true);
    if (s==QSize(1600, 1200)) r1600x1200->setChecked(true);
    if (s==QSize(1920, 1200)) r1920x1200->setChecked(true);
    if (s==QSize(1920, 1080)) r1920x1080->setChecked(true);

}

QSize UserPreferencesDialog::selectionToSize() const {

    if(r600x400->isChecked())
    	return QSize(600, 400);
    if(r640x400->isChecked())
    	return QSize(640, 400);
    if(r854x480->isChecked())
    	return QSize(854, 480);
    if(r768x576->isChecked())
    	return QSize(798, 576);
    if(r720x480->isChecked())
    	return QSize(720, 480);
    if(r800x600->isChecked())
    	return QSize(800, 600);
    if(r960x600->isChecked())
    	return QSize(960, 600);
    if(r1042x768->isChecked())
    	return QSize(1024, 768);
    if(r1152x768->isChecked())
    	return QSize(1152, 768);
    if(r1280x800->isChecked())
    	return QSize(1280, 800);
    if(r1280x720->isChecked())
    	return QSize(1280, 720);
    if(r1600x1200->isChecked())
    	return QSize(1600, 1200);
    if(r1920x1200->isChecked())
    	return QSize(1920, 1200);
    if(r1920x1080->isChecked())
    	return QSize(1920, 1080);

    return QSize(640, 480);
}


