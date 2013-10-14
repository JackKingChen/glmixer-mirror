/*
 * SourcePropertyBrowser.cpp
 *
 *  Created on: Mar 14, 2010
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

#include "SourcePropertyBrowser.moc"

#include <QVBoxLayout>

#include <QPair>
#include <QtTreePropertyBrowser>
#include <QtButtonPropertyBrowser>
#include <QtGroupBoxPropertyBrowser>
#include <QtDoublePropertyManager>
#include <QtIntPropertyManager>
#include <QtStringPropertyManager>
#include <QtColorPropertyManager>
#include <QtRectFPropertyManager>
#include <QtPointFPropertyManager>
#include <QtSizePropertyManager>
#include <QtEnumPropertyManager>
#include <QtBoolPropertyManager>
#include <QtTimePropertyManager>
#include <QtDoubleSpinBoxFactory>
#include <QtCheckBoxFactory>
#include <QtSpinBoxFactory>
#include <QtSliderFactory>
#include <QtLineEditFactory>
#include <QtEnumEditorFactory>
#include <QtCheckBoxFactory>
#include <QtTimeEditFactory>
#include <QtColorEditorFactory>
#include <QFileInfo>

#include "RenderingManager.h"
#include "ViewRenderWidget.h"
#include "RenderingSource.h"
#include "AlgorithmSource.h"
#include "SharedMemorySource.h"
#include "CaptureSource.h"
#include "CloneSource.h"
#include "VideoSource.h"
#include "SvgSource.h"
#ifdef OPEN_CV
#include "OpencvSource.h"
#endif



SourcePropertyBrowser::SourcePropertyBrowser(QWidget *parent) : PropertyBrowser(parent) {

    currentItem = NULL;

    // the top property holding all the sub-properties
    root = groupManager->addProperty( QLatin1String("root") );

   // use the managers to create the property tree
    createSourcePropertyTree();

}


QString aspectRatioToString(double ar)
{
	if ( ABS(ar - 1.0 ) < EPSILON )
		return QString("1:1");
	else if ( ABS(ar - (5.0 / 4.0) ) < EPSILON )
		return QString("5:4");
	else if ( ABS(ar - (4.0 / 3.0) ) < EPSILON )
		return QString("4:3");
	else if ( ABS(ar - (16.0 / 9.0) ) < EPSILON )
		return QString("16:9");
	else  if ( ABS(ar - (3.0 / 2.0) ) < EPSILON )
		return QString("3:2");
	else if ( ABS(ar - (16.0 / 10.0) ) < EPSILON )
		return QString("16:10");
	else
		return QString::number(ar);

}


void SourcePropertyBrowser::createSourcePropertyTree(){

	QtProperty *property;

	// Name
	property = stringManager->addProperty( QLatin1String("Name") );
	idToProperty[property->propertyName()] = property;
	property->setToolTip("A name to identify the source");
	root->addSubProperty(property);

	// modifyable on/off
	QtProperty *modifyroperty = boolManager->addProperty("Modifiable");
	modifyroperty->setToolTip("Can you modify this source?");
	idToProperty[modifyroperty->propertyName()] = modifyroperty;
	root->addSubProperty(modifyroperty);
	{
		// Alpha
		property = doubleManager->addProperty("Alpha");
		property->setToolTip("Opacity (0 = transparent)");
		idToProperty[property->propertyName()] = property;
		doubleManager->setRange(property, 0.0, 1.0);
		doubleManager->setSingleStep(property, 0.01);
		doubleManager->setDecimals(property, 4);
		modifyroperty->addSubProperty(property);
		// Position
		property = pointManager->addProperty("Position");
		idToProperty[property->propertyName()] = property;
		property->setToolTip("X and Y coordinates of the center");
		pointManager->subDoublePropertyManager()->setSingleStep(property->subProperties().first(), 0.1);
		pointManager->subDoublePropertyManager()->setSingleStep(property->subProperties().last(), 0.1);
		pointManager->subDoublePropertyManager()->setDecimals(property->subProperties()[0], 3);
		pointManager->subDoublePropertyManager()->setDecimals(property->subProperties()[1], 3);
		modifyroperty->addSubProperty(property);
		// Scale
		property = pointManager->addProperty("Scale");
		idToProperty[property->propertyName()] = property;
		property->setToolTip("Scaling factors on X and Y");
		pointManager->subDoublePropertyManager()->setSingleStep(property->subProperties()[0], 0.1);
		pointManager->subDoublePropertyManager()->setSingleStep(property->subProperties()[1], 0.1);
		pointManager->subDoublePropertyManager()->setDecimals(property->subProperties()[0], 3);
		pointManager->subDoublePropertyManager()->setDecimals(property->subProperties()[1], 3);
		modifyroperty->addSubProperty(property);
		// fixed aspect ratio on/off
		property = boolManager->addProperty("Fixed aspect ratio");
		property->setToolTip("Keep width/height proportion when scaling");
		idToProperty[property->propertyName()] = property;
		modifyroperty->addSubProperty(property);
		// Rotation angle
		property = doubleManager->addProperty("Angle");
		property->setToolTip("Angle of rotation in degrees (counter clock wise)");
		idToProperty[property->propertyName()] = property;
		doubleManager->setRange(property, 0, 360);
		doubleManager->setSingleStep(property, 10.0);
		modifyroperty->addSubProperty(property);
		// Texture coordinates
		property = rectManager->addProperty("Crop");
		idToProperty[property->propertyName()] = property;
		property->setToolTip("Texture coordinates");
		rectManager->subDoublePropertyManager()->setSingleStep(property->subProperties()[0], 0.1);
		rectManager->subDoublePropertyManager()->setSingleStep(property->subProperties()[1], 0.1);
		rectManager->subDoublePropertyManager()->setDecimals(property->subProperties()[0], 3);
		rectManager->subDoublePropertyManager()->setDecimals(property->subProperties()[1], 3);
		rectManager->subDoublePropertyManager()->setDecimals(property->subProperties()[2], 3);
		rectManager->subDoublePropertyManager()->setDecimals(property->subProperties()[3], 3);
		modifyroperty->addSubProperty(property);
		// Depth
		property = doubleManager->addProperty("Depth");
		property->setToolTip("Depth of the layer");
		idToProperty[property->propertyName()] = property;
		doubleManager->setRange(property, MIN_DEPTH_LAYER, MAX_DEPTH_LAYER);
		modifyroperty->addSubProperty(property);
	}
	// enum list of Destination blending func
	QtProperty *blendingItem = enumManager->addProperty("Blending");
	idToProperty[blendingItem->propertyName()] = blendingItem;
	blendingItem->setToolTip("How the colors are mixed with the sources in lower layers.");
	QStringList enumNames;
    enumNames << namePresetFromInt(0) << namePresetFromInt(1) << namePresetFromInt(2) << namePresetFromInt(3) << namePresetFromInt(4) << namePresetFromInt(5);
	enumManager->setEnumNames(blendingItem, enumNames);
	// Custom Blending
	// enum list of blending Equations
	property = enumManager->addProperty("Equation");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("OpenGL blending equation");
	enumNames.clear();
	enumNames << "Add" << "Subtract" << "Reverse" << "Min" << "Max";
	enumManager->setEnumNames(property, enumNames);
	blendingItem->addSubProperty(property);
	// enum list of Destination blending func
	property = enumManager->addProperty("Destination");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("OpenGL blending function");
	enumNames.clear();
	enumNames << "Zero" << "One" << "Source Color" << "Invert source color" << "Background color" << "Invert background color" << "Source Alpha" << "Invert source alpha" << "Background Alpha" << "Invert background Alpha";
	enumManager->setEnumNames(property, enumNames);
	blendingItem->addSubProperty(property);
	// Confirm and add the blending item
	root->addSubProperty(blendingItem);
	// enum list of blending masks
	property = enumManager->addProperty("Mask");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("Layer mask (where black is opaque)");
    enumNames.clear();
    QMap<int, QIcon> enumIcons;
    QMapIterator<int, QPair<QString, QString> > i(ViewRenderWidget::getMaskDecription());
    while (i.hasNext()) {
        i.next();
        enumNames << i.value().first;
        enumIcons[ i.key() ] = QIcon( i.value().second );
    }
    enumManager->setEnumNames(property, enumNames);
    enumManager->setEnumIcons(property, enumIcons);
	root->addSubProperty(property);
	// Color
	property = colorManager->addProperty("Color");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("Base tint of the source");
	root->addSubProperty(property);

	// Pixelated on/off
	property = boolManager->addProperty("Pixelated");
	property->setToolTip("Do not smooth pixels");
	idToProperty[property->propertyName()] = property;
	root->addSubProperty(property);

	// enum list of inversion types
	property = enumManager->addProperty("Color inversion");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("Invert colors or luminance");
	enumNames.clear();
	enumNames << "None" << "RGB invert" << "Luminance invert";
	enumManager->setEnumNames(property, enumNames);
	root->addSubProperty(property);
	// Saturation
	property = intManager->addProperty( QLatin1String("Saturation") );
	property->setToolTip("Saturation (from greyscale to enhanced colors)");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, -100, 100);
	intManager->setSingleStep(property, 10);
	root->addSubProperty(property);
	// Brightness
	property = intManager->addProperty( QLatin1String("Brightness") );
	property->setToolTip("Brightness (from black to white)");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, -100, 100);
	intManager->setSingleStep(property, 10);
	root->addSubProperty(property);
	// Contrast
	property = intManager->addProperty( QLatin1String("Contrast") );
	property->setToolTip("Contrast (from uniform color to high deviation)");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, -100, 100);
	intManager->setSingleStep(property, 10);
	root->addSubProperty(property);
	// hue
	property = intManager->addProperty( QLatin1String("Hue shift") );
	property->setToolTip("Hue shift (circular shift of color Hue)");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, 0, 360);
	intManager->setSingleStep(property, 36);
	root->addSubProperty(property);
	// threshold
	property = intManager->addProperty( QLatin1String("Threshold") );
	property->setToolTip("Luminance threshold (convert to black & white, keeping colors above the threshold, 0 to keep original)");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, 0, 100);
	intManager->setSingleStep(property, 10);
	root->addSubProperty(property);
	// nb colors
	property = intManager->addProperty( QLatin1String("Posterize") );
	property->setToolTip("Posterize (reduce number of colors, 0 to keep original)");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, 0, 256);
	intManager->setSingleStep(property, 1);
	root->addSubProperty(property);

	// enum list of filters
	property = enumManager->addProperty("Filter");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("Imaging filters (convolutions & morphological operators)");
	enumNames.clear();
	enumNames << Source::getFilterName( Source::FILTER_NONE )
			  << Source::getFilterName( Source::FILTER_BLUR_GAUSSIAN )
			  << Source::getFilterName( Source::FILTER_BLUR_MEAN )
			  << Source::getFilterName( Source::FILTER_SHARPEN )
			  << Source::getFilterName( Source::FILTER_SHARPEN_MORE )
			  << Source::getFilterName( Source::FILTER_EDGE_GAUSSIAN )
			  << Source::getFilterName( Source::FILTER_EDGE_LAPLACE )
			  << Source::getFilterName( Source::FILTER_EDGE_LAPLACE_2 )
			  << Source::getFilterName( Source::FILTER_EMBOSS )
			  << Source::getFilterName( Source::FILTER_EMBOSS_EDGE )
			  << Source::getFilterName( Source::FILTER_EROSION_3X3 )
			  << Source::getFilterName( Source::FILTER_EROSION_5X5 )
			  << Source::getFilterName( Source::FILTER_EROSION_7X7 )
			  << Source::getFilterName( Source::FILTER_DILATION_3X3 )
			  << Source::getFilterName( Source::FILTER_DILATION_5X5 )
			  << Source::getFilterName( Source::FILTER_DILATION_7X7 ) ;

	enumManager->setEnumNames(property, enumNames);
	root->addSubProperty(property);

	// Chroma key on/off
	QtProperty *chroma = boolManager->addProperty("Chroma key");
	chroma->setToolTip("Enables chroma-keying (removes a key color).");
	idToProperty[chroma->propertyName()] = chroma;
	root->addSubProperty(chroma);
	// chroma key Color
	property = colorManager->addProperty("Key Color");
	idToProperty[property->propertyName()] = property;
	property->setToolTip("Color used for the chroma-keying.");
	chroma->addSubProperty(property);
	// threshold
	property = intManager->addProperty( QLatin1String("Key Tolerance") );
	property->setToolTip("Percentage of tolerance around the key color");
	idToProperty[property->propertyName()] = property;
	intManager->setRange(property, 0, 100);
	intManager->setSingleStep(property, 10);
	chroma->addSubProperty(property);
#ifdef FFGL
    // FreeFrameGL Plugin on/off
    QtProperty *ffgl = groupManager->addProperty("FreeframeGL");
    ffgl->setToolTip("FreeFrameGL Plugin");
    idToProperty[ffgl->propertyName()] = ffgl;
    root->addSubProperty(ffgl);
    // list of plugins
    property = infoManager->addProperty( QLatin1String("Plugin list") );
    idToProperty[property->propertyName()] = property;
    ffgl->addSubProperty(property);
#endif
	// Frames size
	QtProperty *fs = sizeManager->addProperty( QLatin1String("Resolution") );
	fs->setToolTip("Width & height of frames");
	fs->setItalics(true);
	idToProperty[fs->propertyName()] = fs;

	// AspectRatio
	QtProperty *ar = infoManager->addProperty("Aspect ratio");
	ar->setToolTip("Ratio of pixel dimensions of acquired frames");
	ar->setItalics(true);
	idToProperty[ar->propertyName()] = ar;

	// Frame rate
	QtProperty *fr = infoManager->addProperty( QLatin1String("Frame rate") );
	fr->setItalics(true);
	idToProperty[fr->propertyName()] = fr;

	rttiToProperty[Source::VIDEO_SOURCE] = groupManager->addProperty( QLatin1String("Media file"));

		// File Name
		property = infoManager->addProperty( QLatin1String("File name") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// File size
		property = infoManager->addProperty( QLatin1String("File size") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// Codec
		property = infoManager->addProperty( QLatin1String("Codec") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// Pixel Format
		property = infoManager->addProperty( QLatin1String("Pixel format") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// interlacing
		property = infoManager->addProperty( QLatin1String("Interlaced") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);

		// Frames size & aspect ratio
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(fs);
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(ar);

		// Frames size special case when power of two dimensions are generated
		property = sizeManager->addProperty( QLatin1String("Original size") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		// Frame rate
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(fr);
		// Duration
		property = infoManager->addProperty( QLatin1String("Duration") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// mark IN
		property = infoManager->addProperty( QLatin1String("Mark in") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// mark OUT
		property = infoManager->addProperty( QLatin1String("Mark out") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);
		// Ignore alpha channel
		property = boolManager->addProperty("Ignore alpha");
		property->setToolTip("Do not use the alpha channel of the images (black instead).");
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::VIDEO_SOURCE]->addSubProperty(property);

	rttiToProperty[Source::SVG_SOURCE] = groupManager->addProperty( QLatin1String("Vector graphics"));
		// frame size
		rttiToProperty[Source::SVG_SOURCE]->addSubProperty(fs);

	rttiToProperty[Source::CAPTURE_SOURCE] = groupManager->addProperty( QLatin1String("Captured image"));
		// frame size
		rttiToProperty[Source::CAPTURE_SOURCE]->addSubProperty(fs);

#ifdef OPEN_CV
	rttiToProperty[Source::CAMERA_SOURCE] = groupManager->addProperty( QLatin1String("Camera"));

		// Identifier
		property = infoManager->addProperty( QLatin1String("Identifier") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::CAMERA_SOURCE]->addSubProperty(property);
		// Frame rate
		rttiToProperty[Source::CAMERA_SOURCE]->addSubProperty(fr);
		// Frames size & aspect ratio
		rttiToProperty[Source::CAMERA_SOURCE]->addSubProperty(fs);
		rttiToProperty[Source::CAMERA_SOURCE]->addSubProperty(ar);
#endif

	rttiToProperty[Source::RENDERING_SOURCE] = groupManager->addProperty( QLatin1String("Render loop-back"));

		// Identifier
		property = infoManager->addProperty( QLatin1String("Rendering mechanism") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::RENDERING_SOURCE]->addSubProperty(property);
		// Frame rate
		rttiToProperty[Source::RENDERING_SOURCE]->addSubProperty(fr);
		// Frames size & aspect ratio
		rttiToProperty[Source::RENDERING_SOURCE]->addSubProperty(fs);
		rttiToProperty[Source::RENDERING_SOURCE]->addSubProperty(ar);

	rttiToProperty[Source::ALGORITHM_SOURCE] = groupManager->addProperty( QLatin1String("Algorithm"));

		// Identifier
		property = infoManager->addProperty( QLatin1String("Algorithm") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(property);
		// Frame rate
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(fr);
		// Frames size & aspect ratio
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(fs);
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(ar);

		// Ignore alpha channel
		property = boolManager->addProperty("Transparent");
		property->setToolTip("Generate patterns with alpha channel.");
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(property);

		// Variability
		property = intManager->addProperty( QLatin1String("Variability") );
		idToProperty[property->propertyName()] = property;
		intManager->setRange(property, 0, 100);
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(property);
		// Periodicity
		property = intManager->addProperty( QLatin1String("Update frequency") );
		idToProperty[property->propertyName()] = property;
		intManager->setRange(property, 1, 60);
		rttiToProperty[Source::ALGORITHM_SOURCE]->addSubProperty(property);
#ifdef SHM
	rttiToProperty[Source::SHM_SOURCE] = groupManager->addProperty( QLatin1String("Shared Memory"));

		// program Name
		property = infoManager->addProperty( QLatin1String("Program") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::SHM_SOURCE]->addSubProperty(property);
		// Info
		property = infoManager->addProperty( QLatin1String("Info") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::SHM_SOURCE]->addSubProperty(property);
		// Frames size & aspect ratio
		rttiToProperty[Source::SHM_SOURCE]->addSubProperty(fs);
		rttiToProperty[Source::SHM_SOURCE]->addSubProperty(ar);
#endif
	rttiToProperty[Source::CLONE_SOURCE] = groupManager->addProperty( QLatin1String("Clone"));

		// Identifier
		property = infoManager->addProperty( QLatin1String("Clone of") );
		property->setItalics(true);
		idToProperty[property->propertyName()] = property;
		rttiToProperty[Source::CLONE_SOURCE]->addSubProperty(property);

		// Frames size & aspect ratio
		rttiToProperty[Source::CLONE_SOURCE]->addSubProperty(fs);
		rttiToProperty[Source::CLONE_SOURCE]->addSubProperty(ar);


}



void SourcePropertyBrowser::updatePropertyTree(){

	// if source is valid,
	// then set the properties to the corresponding values from the source
    if (currentItem) {

        Source *s = currentItem;

		// disconnect the managers to the corresponding value change
		// because otherwise the source is modified by loopback calls to valueChanged slots.
        disconnectManagers();

		// general properties
		stringManager->setValue(idToProperty["Name"], s->getName() );

		// modification properties
		boolManager->setValue(idToProperty["Modifiable"], s->isModifiable() );
		idToProperty["Position"]->setEnabled(s->isModifiable());
		pointManager->setValue(idToProperty["Position"], QPointF( s->getX() / SOURCE_UNIT, s->getY() / SOURCE_UNIT));
		idToProperty["Angle"]->setEnabled(s->isModifiable());
		doubleManager->setValue(idToProperty["Angle"], s->getRotationAngle() );
		idToProperty["Scale"]->setEnabled(s->isModifiable());
		pointManager->setValue(idToProperty["Scale"], QPointF( s->getScaleX() / SOURCE_UNIT, s->getScaleY() / SOURCE_UNIT));
		idToProperty["Fixed aspect ratio"]->setEnabled(s->isModifiable());
		boolManager->setValue(idToProperty["Fixed aspect ratio"], s->isFixedAspectRatio());
		idToProperty["Crop"]->setEnabled(s->isModifiable());
		rectManager->setValue(idToProperty["Crop"], s->getTextureCoordinates());
		idToProperty["Depth"]->setEnabled(s->isModifiable());
		doubleManager->setValue(idToProperty["Depth"], s->getDepth() );
		idToProperty["Alpha"]->setEnabled(s->isModifiable());
		doubleManager->setValue(idToProperty["Alpha"], s->getAlpha() );

		// properties of blending
        int preset = intFromBlendingPreset( s->getBlendFuncDestination(), s->getBlendEquation() );
		enumManager->setValue(idToProperty["Blending"], preset );
		enumManager->setValue(idToProperty["Destination"], intFromBlendfunction( s->getBlendFuncDestination() ));
		enumManager->setValue(idToProperty["Equation"], intFromBlendequation( s->getBlendEquation() ));
		idToProperty["Destination"]->setEnabled(preset == 0);
		idToProperty["Equation"]->setEnabled(preset == 0);
		enumManager->setValue(idToProperty["Mask"], s->getMask());
		colorManager->setValue(idToProperty["Color"], QColor( s->getColor()));
		boolManager->setValue(idToProperty["Pixelated"], s->isPixelated());
		infoManager->setValue(idToProperty["Aspect ratio"], aspectRatioToString(s->getAspectRatio()) );

		// properties of color effects
		enumManager->setValue(idToProperty["Color inversion"], (int) s->getInvertMode() );
		intManager->setValue(idToProperty["Saturation"], s->getSaturation() );
		intManager->setValue(idToProperty["Brightness"], s->getBrightness() );
		intManager->setValue(idToProperty["Contrast"], s->getContrast() );
		intManager->setValue(idToProperty["Hue shift"], s->getHueShift());
		intManager->setValue(idToProperty["Threshold"], s->getLuminanceThreshold() );
		intManager->setValue(idToProperty["Posterize"], s->getNumberOfColors() );
		boolManager->setValue(idToProperty["Chroma key"], s->getChromaKey());
		colorManager->setValue(idToProperty["Key Color"], QColor( s->getChromaKeyColor() ) );
		intManager->setValue(idToProperty["Key Tolerance"], s->getChromaKeyTolerance() );
#ifdef FFGL
        // enable and fill in the FFGL properties if exist
        if(s->hasFreeframeGLPlugin()) {
            idToProperty["FreeframeGL"]->setEnabled(true);
            // fill in list of FFGL plugins "Plugin list"
            infoManager->setValue(idToProperty["Plugin list"], s->getFreeframeGLPluginStack()->namesList().join(", ") );
        }
        else {
            idToProperty["FreeframeGL"]->setEnabled(false);
            infoManager->setValue(idToProperty["Plugin list"], "");
        }
#endif
		// properties of filters
		if (ViewRenderWidget::filteringEnabled()) {
			enumManager->setValue(idToProperty["Filter"], (int) s->getFilter());
			idToProperty["Filter"]->setEnabled( true );
		} else {
			enumManager->setValue(idToProperty["Filter"], 0);
			idToProperty["Filter"]->setEnabled( false );
		}

// TODO : should we use the ffmpeg color filtering ?
//		else {
//			if (s->rtti() == Source::VIDEO_SOURCE) {
//				idToProperty["Saturation"]->setEnabled( true );
//				idToProperty["Brightness"]->setEnabled( true );
//				idToProperty["Contrast"]->setEnabled( true );
//				intManager->setValue(idToProperty["Saturation"], s->getSaturation() );
//				intManager->setValue(idToProperty["Brightness"], s->getBrightness() );
//				intManager->setValue(idToProperty["Contrast"], s->getContrast() );
//			} else {
//				idToProperty["Saturation"]->setEnabled( false );
//				idToProperty["Brightness"]->setEnabled( false );
//				idToProperty["Contrast"]->setEnabled( false );
//			}
//		}

		//
		// per-type properties
		//
		if (s->rtti() == Source::VIDEO_SOURCE) {
			infoManager->setValue(idToProperty["Type"], QLatin1String("Media file") );

			VideoSource *vs = dynamic_cast<VideoSource *>(s);
			VideoFile *vf = vs->getVideoFile();
			infoManager->setValue(idToProperty["File name"], QFileInfo(vf->getFileName()).fileName() );
			idToProperty["File name"]->setToolTip(vf->getFileName());
			infoManager->setValue(idToProperty["File size"], getSizeString( QFileInfo(vf->getFileName()).size() ) );
			infoManager->setValue(idToProperty["Codec"], vf->getCodecName() );
			infoManager->setValue(idToProperty["Pixel format"], vf->getPixelFormatName() );
			boolManager->setValue(idToProperty["Ignore alpha"], vf->ignoresAlphaChannel());
			idToProperty["Ignore alpha"]->setEnabled(vf->pixelFormatHasAlphaChannel());
			sizeManager->setValue(idToProperty["Resolution"], QSize(vf->getFrameWidth(),vf->getFrameHeight()) );
			// Frames size special case when power of two dimensions are generated
			if (vf->getStreamFrameWidth() != vf->getFrameWidth() || vf->getStreamFrameHeight() != vf->getFrameHeight()) {
				sizeManager->setValue(idToProperty["Original size"], QSize(vf->getStreamFrameWidth(),vf->getStreamFrameHeight()) );
				if ( !rttiToProperty[Source::VIDEO_SOURCE]->subProperties().contains(idToProperty["Original size"]))
					rttiToProperty[Source::VIDEO_SOURCE]->insertSubProperty(idToProperty["Original size"], idToProperty["Ignore alpha"]);
			} else {
				if ( rttiToProperty[Source::VIDEO_SOURCE]->subProperties().contains(idToProperty["Original size"]))
					rttiToProperty[Source::VIDEO_SOURCE]->removeSubProperty(idToProperty["Original size"]);
			}
			infoManager->setValue(idToProperty["Frame rate"], QString::number( vf->getFrameRate() ) + QString(" fps") );
			infoManager->setValue(idToProperty["Duration"], vf->getTimeFromFrame(vf->getEnd()) );
			infoManager->setValue(idToProperty["Mark in"],  vf->getTimeFromFrame(vf->getMarkIn()) );
			infoManager->setValue(idToProperty["Mark out"], vf->getTimeFromFrame(vf->getMarkOut()) );
            infoManager->setValue(idToProperty["Interlaced"], vf->isInterlaced() ? QObject::tr("Yes") : QObject::tr("No") );

		} else
#ifdef OPEN_CV
		if (s->rtti() == Source::CAMERA_SOURCE) {

			infoManager->setValue(idToProperty["Type"], QLatin1String("Camera device") );
			OpencvSource *cvs = dynamic_cast<OpencvSource *>(s);
			infoManager->setValue(idToProperty["Identifier"], QString("OpenCV Camera %1").arg(cvs->getOpencvCameraIndex()) );
			sizeManager->setValue(idToProperty["Resolution"], QSize(cvs->getFrameWidth(), cvs->getFrameHeight()) );
			infoManager->setValue(idToProperty["Frame rate"], QString::number( cvs->getFrameRate() ) + QString(" fps") );
		} else
#endif
		if (s->rtti() == Source::RENDERING_SOURCE) {

			infoManager->setValue(idToProperty["Type"], QLatin1String("Rendering loop-back") );
			if (RenderingManager::getInstance()->getUseFboBlitExtension())
				infoManager->setValue(idToProperty["Rendering mechanism"], "Blit to frame buffer object" );
			else
				infoManager->setValue(idToProperty["Rendering mechanism"], "Draw in frame buffer object" );
			sizeManager->setValue(idToProperty["Resolution"], QSize(RenderingManager::getInstance()->getFrameBufferWidth(), RenderingManager::getInstance()->getFrameBufferHeight()) );
			infoManager->setValue(idToProperty["Frame rate"], QString::number( RenderingManager::getRenderingWidget()->getFramerate() / float(RenderingManager::getInstance()->getPreviousFrameDelay()) ) + QString(" fps") );
		} else
		if (s->rtti() == Source::ALGORITHM_SOURCE) {

			infoManager->setValue(idToProperty["Type"], QLatin1String("Algorithm") );
			AlgorithmSource *as = dynamic_cast<AlgorithmSource *>(s);
			infoManager->setValue(idToProperty["Algorithm"], AlgorithmSource::getAlgorithmDescription(as->getAlgorithmType()) );
			sizeManager->setValue(idToProperty["Resolution"], QSize(as->getFrameWidth(), as->getFrameHeight()) );
			infoManager->setValue(idToProperty["Frame rate"], QString::number(as->getFrameRate() ) + QString(" fps"));

			boolManager->setValue(idToProperty["Transparent"], !as->getIgnoreAlpha());
			intManager->setValue(idToProperty["Variability"], (int) ( as->getVariability() * 100.0 ) );
			intManager->setValue(idToProperty["Update frequency"], (int) ( 1000000.0 / double(as->getPeriodicity()) ) );

		} else
#ifdef SHM
		if (s->rtti() == Source::SHM_SOURCE) {

			SharedMemorySource *shms = dynamic_cast<SharedMemorySource *>(s);
			infoManager->setValue(idToProperty["Program"], shms->getProgram() );
			infoManager->setValue(idToProperty["Info"], shms->getInfo() );
			sizeManager->setValue(idToProperty["Resolution"], QSize(shms->getFrameWidth(), shms->getFrameHeight()) );

		} else
#endif
		if (s->rtti() == Source::CLONE_SOURCE) {

			infoManager->setValue(idToProperty["Type"], QLatin1String("Clone") );
			CloneSource *cs = dynamic_cast<CloneSource *>(s);
			infoManager->setValue(idToProperty["Clone of"], cs->getOriginalName() );
			sizeManager->setValue(idToProperty["Resolution"], QSize(cs->getFrameWidth(), cs->getFrameHeight()) );
		} else
		if (s->rtti() == Source::CAPTURE_SOURCE) {
			infoManager->setValue(idToProperty["Type"], QLatin1String("Captured image") );
			CaptureSource *cs = dynamic_cast<CaptureSource *>(s);
			sizeManager->setValue(idToProperty["Resolution"], QSize(cs->getFrameWidth(), cs->getFrameHeight()) );
		} else
		if (s->rtti() == Source::SVG_SOURCE) {
			infoManager->setValue(idToProperty["Type"], QLatin1String("Vector graphics") );
			SvgSource *cs = dynamic_cast<SvgSource *>(s);
			sizeManager->setValue(idToProperty["Resolution"], QSize(cs->getFrameWidth(), cs->getFrameHeight()) );
		}

		// reconnect the managers to the corresponding value change
        connectManagers();
	}
}


void SourcePropertyBrowser::showProperties(SourceSet::iterator sourceIt)
{
	// this slot is called only when a different source is clicked (or when none is clicked)

	// remember expanding state
    updateExpandState(propertyTreeEditor->topLevelItems());

	// clear the GUI
    propertyTreeEditor->clear();
    propertyGroupEditor->clear();

	if ( RenderingManager::getInstance()->isValid(sourceIt) )
		showProperties(*sourceIt);
	else
		showProperties(0);

}

void SourcePropertyBrowser::showProperties(Source *source)
{
    currentItem = source;

    if (currentItem) {

        updatePropertyTree();

		// show all the Properties into the browser:
		QListIterator<QtProperty *> it(root->subProperties());

		// first property ; the name
		addProperty(it.next());

		// add the sub tree of the properties related to this source type
        if ( rttiToProperty.contains(currentItem->rtti()) )
            addProperty( rttiToProperty[ currentItem->rtti() ] );

		// the rest of the properties
		while (it.hasNext()) {
			addProperty(it.next());
		}

        restoreExpandState(propertyTreeEditor->topLevelItems());
	}
}


bool SourcePropertyBrowser::canChange()
{
	if (currentItem)
		emit changed(currentItem);
	else
		return false;

	return true;
}


void SourcePropertyBrowser::valueChanged(QtProperty *property, const QString &value){

	if (!canChange())
			return;

	if ( property == idToProperty["Name"] ) {
		RenderingManager::getInstance()->renameSource(currentItem, value);
        updatePropertyTree();
	}
}


void SourcePropertyBrowser::valueChanged(QtProperty *property, const QPointF &value){

	if (!canChange())
			return;

	if ( property == idToProperty["Position"] ) {
		currentItem->setX( value.x() * SOURCE_UNIT);
		currentItem->setY( value.y() * SOURCE_UNIT);
	}
	else if ( property == idToProperty["Rotation center"] ) {
		currentItem->setCenterX( value.x() * SOURCE_UNIT );
		currentItem->setCenterY( value.y() * SOURCE_UNIT);
	}
	else if ( property == idToProperty["Scale"] ) {
		currentItem->setScaleX( value.x() * SOURCE_UNIT );
		currentItem->setScaleY( value.y() * SOURCE_UNIT);
	}
}

void SourcePropertyBrowser::valueChanged(QtProperty *property, const QRectF &value){

	if (!canChange())
			return;

	if ( property == idToProperty["Crop"] ) {
		currentItem->setTextureCoordinates(value);
	}
}


void SourcePropertyBrowser::valueChanged(QtProperty *property, const QColor &value){

	if (!canChange())
			return;

	if ( property == idToProperty["Color"] ) {
		currentItem->setColor(value);
	}
	else if ( property == idToProperty["Key Color"] ) {
		currentItem->setChromaKeyColor(value);
	}

}


void SourcePropertyBrowser::valueChanged(QtProperty *property, double value){

	if (!canChange())
			return;

	if ( property == idToProperty["Depth"] ) {
		if ( RenderingManager::getInstance()->notAtEnd(RenderingManager::getInstance()->getCurrentSource()) ) {
			// ask the rendering manager to change the depth of the source
			SourceSet::iterator c = RenderingManager::getInstance()->changeDepth(RenderingManager::getInstance()->getCurrentSource(), value);
			// we need to set current again (the list changed)
			RenderingManager::getInstance()->setCurrentSource(c);

			// forces the update of the value, without calling valueChanded again.
			disconnect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
			doubleManager->setValue(idToProperty["Depth"], (*c)->getDepth() );
			connect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
		}
	}
	else if ( property == idToProperty["Angle"] ) {
		currentItem->setRotationAngle(value);
	}
	else if ( property == idToProperty["Alpha"] ) {
		currentItem->setAlpha(value);
	}
}

void SourcePropertyBrowser::valueChanged(QtProperty *property,  bool value){

	if (!canChange())
		return;

	if ( property == idToProperty["Modifiable"] ) {
		currentItem->setModifiable(value);
        updatePropertyTree();
	}
	else if ( property == idToProperty["Pixelated"] ) {
		currentItem->setPixelated(value);
	}
	else if ( property == idToProperty["Fixed aspect ratio"] ) {
		currentItem->setFixedAspectRatio(value);
	}
	else if ( property == idToProperty["Ignore alpha"] ) {
		if (currentItem->rtti() == Source::VIDEO_SOURCE) {
			VideoSource *vs = dynamic_cast<VideoSource *>(currentItem);
			if (vs != 0) {
				VideoFile *vf = vs->getVideoFile();
				vf->stop();
				vf->close();
				vf->open(vf->getFileName(), vf->getMarkIn(), vf->getMarkOut(), value);
			}
		}
	}
	else if ( property == idToProperty["Transparent"] ) {
		if (currentItem->rtti() == Source::ALGORITHM_SOURCE) {
			AlgorithmSource *as = dynamic_cast<AlgorithmSource *>(currentItem);
			if (as != 0) {
				as->setIgnoreAlpha(!value);
			}
		}
	}
	else if ( property == idToProperty["Chroma key"] ) {
		currentItem->setChromaKey(value);
		idToProperty["Key Color"]->setEnabled(value);
		idToProperty["Key Tolerance"]->setEnabled(value);
	}

}

void SourcePropertyBrowser::valueChanged(QtProperty *property,  int value){

	if (!canChange())
			return;

	if ( property == idToProperty["Brightness"] ) {
		currentItem->setBrightness(value);
	}
	else if ( property == idToProperty["Contrast"] ) {
		currentItem->setContrast(value);
	}
	else if ( property == idToProperty["Saturation"] ) {
		currentItem->setSaturation(value);
	}
	else if ( property == idToProperty["Hue shift"] ) {
		currentItem->setHueShift(value);
	}
	else if ( property == idToProperty["Threshold"] ) {
		currentItem->setLuminanceThreshold(value);
	}
	else if ( property == idToProperty["Posterize"] ) {
		currentItem->setNumberOfColors(value);
	}
	else if ( property == idToProperty["Key Tolerance"] ) {
		currentItem->setChromaKeyTolerance(value);
	}
	else if ( property == idToProperty["Variability"] ) {
		if (currentItem->rtti() == Source::ALGORITHM_SOURCE) {
			AlgorithmSource *as = dynamic_cast<AlgorithmSource *>(currentItem);
			as->setVariability( double(value) / 100.0);
		}
	}
	else if ( property == idToProperty["Update frequency"] ) {
		if (currentItem->rtti() == Source::ALGORITHM_SOURCE) {
			AlgorithmSource *as = dynamic_cast<AlgorithmSource *>(currentItem);
			as->setPeriodicity( (unsigned long) ( 1000000.0 / double(value) ) );
		}
	}

}

void SourcePropertyBrowser::enumChanged(QtProperty *property,  int value){

	if (!canChange())
			return;

	if ( property == idToProperty["Blending"] ) {

		if ( value != 0) {
            QPair<int, int> preset = blendingPresetFromInt(value);
            currentItem->setBlendFunc(GL_SRC_ALPHA, blendfunctionFromInt( preset.first ) );
            currentItem->setBlendEquation( blendequationFromInt( preset.second ) );
			enumManager->setValue(idToProperty["Destination"], intFromBlendfunction( currentItem->getBlendFuncDestination() ) );
			enumManager->setValue(idToProperty["Equation"], intFromBlendequation( currentItem->getBlendEquation() ));
		}
		idToProperty["Destination"]->setEnabled(value == 0);
		idToProperty["Equation"]->setEnabled(value == 0);

	}
	else if ( property == idToProperty["Destination"] ) {

		currentItem->setBlendFunc(GL_SRC_ALPHA, blendfunctionFromInt(value));
	}
	else if ( property == idToProperty["Equation"] ) {

		currentItem->setBlendEquation( blendequationFromInt(value) );
	}
	else if ( property == idToProperty["Mask"] ) {

        currentItem->setMask( value );
	}
	else if ( property == idToProperty["Filter"] ) {
		// set the current filter
		currentItem->setFilter( (Source::filterType) value );
	}
	else if ( property == idToProperty["Color inversion"] ) {

		currentItem->setInvertMode( (Source::invertModeType) value );
	}
}


void SourcePropertyBrowser::updateMixingProperties(){

	if (!canChange())
		return;

	disconnect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
	doubleManager->setValue(idToProperty["Alpha"], currentItem->getAlpha() );
	connect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
}


void SourcePropertyBrowser::updateGeometryProperties(){

	if (!canChange())
			return;

	disconnect(pointManager, SIGNAL(valueChanged(QtProperty *, const QPointF &)), this, SLOT(valueChanged(QtProperty *, const QPointF &)));
	pointManager->setValue(idToProperty["Position"], QPointF( currentItem->getX() / SOURCE_UNIT, currentItem->getY() / SOURCE_UNIT));
	pointManager->setValue(idToProperty["Scale"], QPointF( currentItem->getScaleX() / SOURCE_UNIT, currentItem->getScaleY() / SOURCE_UNIT));
	doubleManager->setValue(idToProperty["Angle"], currentItem->getRotationAngle());
	connect(pointManager, SIGNAL(valueChanged(QtProperty *, const QPointF &)), this, SLOT(valueChanged(QtProperty *, const QPointF &)));

	disconnect(rectManager, SIGNAL(valueChanged(QtProperty *, const QRectF &)), this, SLOT(valueChanged(QtProperty *, const QRectF &)));
	rectManager->setValue(idToProperty["Crop"], currentItem->getTextureCoordinates() );
	connect(rectManager, SIGNAL(valueChanged(QtProperty *, const QRectF &)), this, SLOT(valueChanged(QtProperty *, const QRectF &)));

}

void SourcePropertyBrowser::updateLayerProperties(){

	if (!canChange())
			return;

	disconnect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
	doubleManager->setValue(idToProperty["Depth"], currentItem->getDepth() );
	connect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
}

void SourcePropertyBrowser::updateMarksProperties(bool showFrames){

	if (!canChange())
			return;

	if (currentItem->rtti() == Source::VIDEO_SOURCE) {
		VideoSource *vs = dynamic_cast<VideoSource *>(currentItem);
		if (vs != 0) {
			VideoFile *vf = vs->getVideoFile();
			if (showFrames) {
				infoManager->setValue(idToProperty["Duration"], vf->getExactFrameFromFrame(vf->getEnd()) );
				infoManager->setValue(idToProperty["Mark in"],  vf->getExactFrameFromFrame(vf->getMarkIn()) );
				infoManager->setValue(idToProperty["Mark out"], vf->getExactFrameFromFrame(vf->getMarkOut()) );
			} else {
				infoManager->setValue(idToProperty["Duration"], vf->getTimeFromFrame(vf->getEnd()) );
				infoManager->setValue(idToProperty["Mark in"],  vf->getTimeFromFrame(vf->getMarkIn()) );
				infoManager->setValue(idToProperty["Mark out"], vf->getTimeFromFrame(vf->getMarkOut()) );
			}
		}
	}
}

void SourcePropertyBrowser::resetAll()
{
    RenderingManager::getInstance()->resetCurrentSource();
}

void SourcePropertyBrowser::defaultValue()
{
//    RenderingManager::getInstance()->resetCurrentSource();


}

