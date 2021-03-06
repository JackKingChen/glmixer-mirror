/*
 * Source.cpp
 *
 *  Created on: Jun 29, 2009
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


#include "Source.moc"

#include "common.h"
#include "SourceSet.h"
#include "ViewRenderWidget.h"
#include "OutputRenderWindow.h"
#include "RenderingManager.h"
#include "PropertyBrowser.h"
#include "Tag.h"

#include <QtProperty>
#include <QtVariantPropertyManager>

#ifdef GLM_FFGL
#include "FFGLPluginSource.h"
#include "FFGLPluginSourceShadertoy.h"
#endif

GLuint Source::lastid = 1;
Source::RTTI Source::type = Source::SIMPLE_SOURCE;
bool Source::playable = false;


// source constructor.
Source::Source(GLuint texture, double depth): ProtoSource(),
    standby(NOT_STANDBY), culled(false),
    clones(NULL), textureIndex(texture),
    customMaskTextureIndex(0), workspace(0), fade(1.0)
{
    // give it a unique identifier
    id = Source::lastid++;

    // empty clone list
    clones = new SourceList;
    CHECK_PTR_EXCEPTION(clones)

    // apply depth
    z = CLAMP(depth, MIN_DEPTH_LAYER, MAX_DEPTH_LAYER);

    glGenTextures(1, &customMaskTextureIndex);
}

Source::~Source() {

    if (clones)
        delete clones;

#ifdef GLM_FFGL
    // delete all plugins in the stack
    _ffgl_plugins.clear();
#endif

    if (textureIndex > 0)
        // free the OpenGL texture
        glDeleteTextures(1, &textureIndex);
}

QString Source::getInfo() const {

    return tr("'%1' (%2x%3)").arg(getName()).arg(getFrameWidth()).arg(getFrameHeight());
}

/****
 *
 *  Implementation of ProtoSource methods
 *
 *  Call the base method of ProtoSource
 * AND
 *  inform the history managers (who might be listening)
 *  that this method has been called.
 *
 * */

void Source::setName(QString n) {

    // do not accept empty strings
    if (n.isEmpty())  return;

    // accept new name only if validated by rendering manager
    QString newname = RenderingManager::getInstance()->getAvailableNameFrom(n);

    // inform undo manager
    emit methodCalled("_setName(QString)", S_ARG(name, newname));

    // set actual name
    ProtoSource::_setName(newname);
}

void Source::setX(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, v), S_ARG(y, y), S_ARG(scalex, scalex), S_ARG(scaley, scaley), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(v, y, scalex, scaley, centerx, centery, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setY(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, v), S_ARG(scalex, scalex), S_ARG(scaley, scaley), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(x, v, scalex, scaley, centerx, centery, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setPosition(double posx, double posy) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, posx), S_ARG(y, posy), S_ARG(scalex, scalex), S_ARG(scaley, scaley), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(posx, posy, scalex, scaley, centerx, centery, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setRotationCenterX(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, y), S_ARG(scalex, scalex), S_ARG(scaley, scaley), S_ARG(centerx, v), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(x, y, scalex, scaley, v, centery, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setRotationCenterY(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, y), S_ARG(scalex, scalex), S_ARG(scaley, scaley), S_ARG(centerx, centerx), S_ARG(centery, v), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(x, y, scalex, scaley, centerx, v, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setRotationAngle(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, y), S_ARG(scalex, scalex), S_ARG(scaley, scaley), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, v));

    ProtoSource::_setGeometry(x, y, scalex, scaley, centerx, centery, v);

    // test for culling
    testGeometryCulling();
}

void Source::setScaleX(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, y), S_ARG(scalex, v), S_ARG(scaley, scaley), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(x, y, v, scaley, centerx, centery, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setScaleY(double v) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, y), S_ARG(scalex, scalex), S_ARG(scaley, v), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(x, y, scalex, v, centerx, centery, rotangle);

    // test for culling
    testGeometryCulling();
}

void Source::setScale(double sx, double sy) {
    emit methodCalled("_setGeometry(double,double,double,double,double,double,double)", S_ARG(x, x), S_ARG(y, y), S_ARG(scalex, sx), S_ARG(scaley, sy), S_ARG(centerx, centerx), S_ARG(centery, centery), S_ARG(rotangle, rotangle));

    ProtoSource::_setGeometry(x, y, sx, sy, centerx, centery, rotangle);

    // test for culling
    testGeometryCulling();
}


void Source::setFixedAspectRatio(bool on) {
    emit methodCalled("_setFixedAspectRatio(bool)", S_ARG(fixedAspectRatio, on));

    ProtoSource::_setFixedAspectRatio(on);
}

void Source::setTextureCoordinates(QRectF textureCoords) {
    emit methodCalled("_setTextureCoordinates(QRectF)", S_ARG(textureCoordinates, textureCoords));

    ProtoSource::_setTextureCoordinates(textureCoords);
}

void Source::setAlphaCoordinates(double x, double y) {
    emit methodCalled("_setAlphaCoordinates(double,double)", S_ARG(alphax, x), S_ARG(alphay, y));

    ProtoSource::_setAlphaCoordinates(x, y);
}

void Source::setAlpha(double a) {
    emit methodCalled("_setAlpha(double)", S_ARG(texalpha, a));

    ProtoSource::_setAlpha(a);
}

void Source::setMask(int maskType) {
    emit methodCalled("_setMask(int)", S_ARG(mask_type, maskType));

    ProtoSource::_setMask(maskType);
}

void Source::setColor(QColor c){
    emit methodCalled("_setColor(QColor)", S_ARG(texcolor, c));

    ProtoSource::_setColor(c);
}

void Source::setBrightness(int b) {
    emit methodCalled("_setBrightness(int)", S_ARG(getBrightness(), b));

    ProtoSource::_setBrightness(b);
}

void Source::setContrast(int c) {
    emit methodCalled("_setContrast(int)", S_ARG(getContrast(), c));

    ProtoSource::_setContrast(c);
}

void Source::setSaturation(int s){
    emit methodCalled("_setSaturation(int)", S_ARG(getSaturation(), s));

    ProtoSource::_setSaturation(s);
}

void Source::setHueShift(int h){
    emit methodCalled("_setHueShift(int)", S_ARG(getHueShift(), h));

    ProtoSource::_setHueShift(h);
}

void Source::setThreshold(int l){
    emit methodCalled("_setThreshold(int)", S_ARG(luminanceThreshold, l));

    ProtoSource::_setThreshold(l);
}

void Source::setLumakey(int l){
    emit methodCalled("_setLumakey(int)", S_ARG(lumakeyThreshold, l));

    ProtoSource::_setLumakey(l);
}

void Source::setPosterized(int n){
    emit methodCalled("_setPosterized(int)", S_ARG(numberOfColors, n));

    ProtoSource::_setPosterized(n);
}

void Source::setChromaKey(bool on) {
    emit methodCalled("_setChromaKey(bool)", S_ARG(useChromaKey, on));

    ProtoSource::_setChromaKey(on);
}

void Source::setChromaKeyColor(QColor c) {
    emit methodCalled("_setChromaKeyColor(QColor)", S_ARG(chromaKeyColor, c));

    ProtoSource::_setChromaKeyColor(c);
}

void Source::setChromaKeyTolerance(int t) {
    emit methodCalled("_setChromaKeyTolerance(int)", S_ARG(getChromaKeyTolerance(), t));

    ProtoSource::_setChromaKeyTolerance(t);
}

void Source::setGammaColor(double value, double red, double green, double blue){
    emit methodCalled("_setGammaColor(double,double,double,double)", S_ARG(gamma, value), S_ARG(gammaRed, red), S_ARG(gammaGreen, green), S_ARG(gammaBlue, blue));

    ProtoSource::_setGammaColor(value, red, green, blue);
}

void Source::setGammaLevels(double minI, double maxI, double minO, double maxO){
    emit methodCalled("_setGammaLevels(double,double,double,double)", S_ARG(gammaMinIn, minI), S_ARG(gammaMaxIn, maxI), S_ARG(gammaMinOut, minO), S_ARG(gammaMaxOut, maxO));

    ProtoSource::_setGammaLevels(minI, maxI, minO, maxO);
}

void Source::setPixelated(bool on) {
    emit methodCalled("_setPixelated(bool)", S_ARG(pixelated, on));

    ProtoSource::_setPixelated(on);
}


void Source::setBlendFunc(uint sfactor, uint dfactor) {
    emit methodCalled("_setBlending(uint,uint,uint)", S_ARG(source_blend, sfactor), S_ARG(destination_blend, dfactor), S_ARG(blend_eq, blend_eq));

    ProtoSource::_setBlending(sfactor, dfactor, blend_eq);
}

void Source::setBlendEquation(uint eq) {
    emit methodCalled("_setBlending(uint,uint,uint)", S_ARG(source_blend, source_blend), S_ARG(destination_blend, destination_blend), S_ARG(blend_eq, eq));

    ProtoSource::_setBlending(source_blend, destination_blend, eq);
}

void Source::setInvertMode(invertModeType i) {
    emit methodCalled("_setInvertMode(invertModeType)", S_ARG(invertMode, i));

    ProtoSource::_setInvertMode(i);
}

void Source::setFilter(filterType c) {
    emit methodCalled("_setFilter(filterType)", S_ARG(filter, c));

    ProtoSource::_setFilter(c);
}


QDomElement Source::getConfiguration(QDomDocument &doc, QDir current)
{
    // get the config from proto source
    QDomElement sourceElem = ProtoSource::getConfiguration(doc);

    // set more config from Source
    sourceElem.setAttribute("stanbyMode", (int) getStandbyMode());
    sourceElem.setAttribute("workspace", getWorkspace());

    // if the mask type is for custom texture
    if ( !ViewRenderWidget::getMaskTexture(mask_type)) {
        // store the filename of the custom mask image
        QDomElement m = doc.createElement("Mask");
        QDomText filename = doc.createTextNode( customMaskFilename );
        m.appendChild(filename);
        sourceElem.appendChild(m);
    }

#ifdef GLM_TAG
    sourceElem.setAttribute("tag", Tag::get(this)->getIndex());
#endif

    // freeframe gl plugin
#ifdef GLM_FFGL
    FFGLPluginSourceStack *plugins = getFreeframeGLPluginStack();
    for (FFGLPluginSourceStack::iterator it = plugins->begin(); it != plugins->end(); ++it ) {

        sourceElem.appendChild( (*it)->getConfiguration(current) );
    }
#endif

    return sourceElem;
}


bool Source::setConfiguration(QDomElement xmlconfig, QDir current)
{
    // apply proto configuration
    bool ret = ProtoSource::setConfiguration(xmlconfig);

    // set workspace
    if (xmlconfig.hasAttribute("workspace"))
        setWorkspace(xmlconfig.attribute("workspace").toInt());
    // compatibility with old version of glm file
    if ( xmlconfig.hasAttribute("modifiable") ) {
        int modifiable = xmlconfig.attribute("modifiable", "1").toInt();
        // non modifiable source are simply moved to another workspace
        setWorkspace( modifiable > 0 ? 0 : 1);
    }

    // read the configuration of the custom mask
    QDomElement m = xmlconfig.firstChildElement("Mask");
    if (!m.isNull())
        setCustomMaskTexture(m.text());

#ifdef GLM_TAG
    // set tag
    if ( xmlconfig.hasAttribute("tag") )
        Tag::get( xmlconfig.attribute("tag").toInt() )->set(this);
#endif

#ifdef GLM_FFGL
    // clear the plugin list if none provided
    if ( xmlconfig.firstChildElement("FreeFramePlugin").isNull()) {
        clearFreeframeGLPlugin();
    }
    // apply FreeFrame plugins configuration if provided
    else {
        // start loop of plugins to load
        int id = 0;
        QDomElement p = xmlconfig.firstChildElement("FreeFramePlugin");
        while (!p.isNull() /*&& ret != false*/) {
            // ignore empty node
            if (p.hasChildNodes()) {
                QDomElement Filename = p.firstChildElement("Filename");

                // for FreeFrame plugins using a DLL
                if (!Filename.isNull()) {

                    // first reads with the absolute file name
                    QString fileNameToOpen = Filename.text();
                    // if there is no such file, try generate a file name from the relative file name
                    if (!QFileInfo(fileNameToOpen).exists())
                        fileNameToOpen = current.absoluteFilePath( Filename.attribute("Relative", "") );
                    // if there is no such file, try generate a file name from the generic basename
                    if (!QFileInfo(fileNameToOpen).exists() && Filename.hasAttribute("Basename"))
                        fileNameToOpen =  FFGLPluginSource::libraryFileName( Filename.attribute("Basename", ""));
                    // if there is such a file
                    if (QFileInfo(fileNameToOpen).exists()) {

                        try {
                            FFGLPluginSource *plugin = NULL;
                            // is this the same plugin than in the current stack ?
                            if ( id < _ffgl_plugins.size()
                                 && _ffgl_plugins.at(id)->fileName() == fileNameToOpen ) {
                                // can keep this plugin as it is the same as in the config
                                plugin = _ffgl_plugins.at(id);
                            }
                            else {
                                // cannot keep the rest of the stack as it is different from config
                                _ffgl_plugins.clearAfterIndex(id);

                                // create and push the plugin to the source
                                plugin = addFreeframeGLPlugin( fileNameToOpen );
                                plugin->setParent( this );
                                qDebug() << xmlconfig.attribute("name") << QChar(124).toLatin1()
                                         << tr("GPU plugin %1 added.").arg(QFileInfo(fileNameToOpen).baseName());
                            }

                            // apply the configuration
                            if (plugin) {
                                plugin->setConfiguration(p);
                            }
                            else {
                                ret = false;
                                qWarning() << xmlconfig.attribute("name")
                                           << QChar(124).toLatin1()
                                           << tr("GPU plugin %1 failed.").arg(QFileInfo(fileNameToOpen).baseName());
                            }
                        }
                        catch (FFGLPluginException &e)  {
                            ret = false;
                            qWarning() << xmlconfig.attribute("name") << QChar(124).toLatin1()<< e.message() << tr("\nGPU Plugin not added.");
                        }

                    }
                    else {
                        ret = false;
                        qWarning() << xmlconfig.attribute("name") << QChar(124).toLatin1()
                                   << tr("No GPU plugin file named %1 or %2.").arg(Filename.text()).arg(fileNameToOpen);
                    }

                }
                // No filename?  this is certainly a FreeFrame plugin
                else {

                    try {

                        // create and push the plugin to the source
                        FFGLPluginSource *plugin = NULL;

                        // is this the same plugin than in the current stack ?
                        if ( id < _ffgl_plugins.size() && _ffgl_plugins.at(id)->rtti() == FFGLPluginSource::SHADERTOY_PLUGIN ) {
                            // can keep this plugin as it is the same as in the config
                            plugin = _ffgl_plugins.at(id);
                        }
                        else {
                            // cannot keep the rest of the stack as it is different from config
                            _ffgl_plugins.clearAfterIndex(id);

                            // create and push the plugin to the source
                            plugin = addFreeframeGLPlugin();
                            plugin->setParent( this );
                            qDebug() << xmlconfig.attribute("name") << QChar(124).toLatin1()
                                     << tr("GPU plugin Shadertoy added.");
                        }

                        // apply the code
                        if (plugin) {

                            FFGLPluginSourceShadertoy *stp = qobject_cast<FFGLPluginSourceShadertoy *>(plugin);

                            if (stp) {
                                // set the plugin config and code
                                stp->setConfiguration(p);
                            }
                            else {
                                ret = false;
                                qWarning() << xmlconfig.attribute("name") << QChar(124).toLatin1()
                                           << QObject::tr("Failed to create Shadertoy plugin.");
                            }

                        }
                    }
                    catch (FFGLPluginException &e)  {
                        ret = false;
                        qWarning() << xmlconfig.attribute("name") << QChar(124).toLatin1()<< e.message() << tr("\nPlugin not added.");
                    }

                }

            }
            // next plugin in configuration
            p = p.nextSiblingElement("FreeFramePlugin");
            // next plugin in stack
            id++;
        }

        // remove the rest of the stack as it is not part of config
        _ffgl_plugins.clearAfterIndex(id);
    }

#endif

    return ret;
}


/****
 *
 *  Implementation of Source specific methods
 *
 *
 * */

void Source::scaleBy(double fx, double fy) {
    setScale(scalex * fx, scaley * fy);
}

void Source::resetTextureCoordinates() {
    setTextureCoordinates ( QRectF(0.0, 0.0, 1.0, 1.0) );
}

GLuint Source::getTextureIndex() const {
    return textureIndex;
}

void Source::testGeometryCulling() {

    double w = SOURCE_UNIT;
    double h = SOURCE_UNIT;

    if (OutputRenderWindow::getInstance()->getAspectRatio() > 1)
        w *= OutputRenderWindow::getInstance()->getAspectRatio();
    else
        h *= OutputRenderWindow::getInstance()->getAspectRatio();

    // if all coordinates of center are between viewport limits, it is obviously visible
    if (x > -w && x < w && y > -h && y < h)
        culled = false;
    else {
        // not obviously visible
        // but it might still be parly visible if the distance from the center to the borders is less than the width
        double d = sqrt( scalex*scalex + scaley*scaley);
        if ((x + d < -w) || (x - d > w))
            culled = true;
        else if ((y + d < -h) || (y - d > h))
            culled = true;
            else
                culled = false;
    }

}

void Source::setDepth(double v) {
    z = CLAMP(v, MIN_DEPTH_LAYER, MAX_DEPTH_LAYER);
}


void Source::setStandby(bool on) {

    // ignore non changing calls
    if ( on == isStandby() )
        return;

    StandbyMode previous = standby;

    if (!isPlayable())
        standby = on ? PLAY_STANDBY : NOT_STANDBY;
    else
        // new status is based on request and play mode
        standby = on ? (isPlaying() ? PLAY_STANDBY : STOP_STANDBY) : NOT_STANDBY;

    // stop the source if in standby
    // else (not standby), replay if previous state was to play
    play( standby == NOT_STANDBY && previous == PLAY_STANDBY );

#ifdef GLM_FFGL
    // play the plugins if not standby
    if (! _ffgl_plugins.isEmpty())
        _ffgl_plugins.play( standby == NOT_STANDBY );
#endif

    emit standingby(standby);
}

void Source::play(bool on) {

    emit playing(on);
}


void Source::clampScale(bool updateTextureCoordinates) {

    // clamp scaling factors
    double newscalex = (scalex > 0 ? 1.0 : -1.0) * CLAMP( ABS(scalex), MIN_SCALE, MAX_SCALE);
    double newscaley = (scaley > 0 ? 1.0 : -1.0) * CLAMP( ABS(scaley), MIN_SCALE, MAX_SCALE);

    // if was cropping the source
    if (updateTextureCoordinates) {
        // compute a correct crop
        QRectF tex = getTextureCoordinates();
        if ( ABS(newscalex) > ABS(scalex))
            tex.setWidth( tex.width() * newscalex / scalex);
        if ( ABS(newscaley) > ABS(scaley))
            tex.setHeight( tex.height() * newscaley / scaley);
        setTextureCoordinates(tex);
    }

    // apply correction of scale
    setScale(newscalex, newscaley);

    // test for culling
    testGeometryCulling();
}


void Source::resetScale(scalingMode sm) {

    scalex = SOURCE_UNIT;
    scaley = SOURCE_UNIT;
    double renderingAspectRatio = OutputRenderWindow::getInstance()->getAspectRatio();
    double aspectratio = getAspectRatio();

    switch (sm) {
    case Source::SCALE_PIXEL:
        {
            double U = 1.0;
            if (RenderingManager::getInstance()->getFrameBufferAspectRatio() > 1.0 )
                U = (double) RenderingManager::getInstance()->getFrameBufferHeight();
            else
                U = (double) RenderingManager::getInstance()->getFrameBufferWidth();
            scalex *=  (double) getFrameWidth() / U;
            scaley *=  (double) getFrameHeight() / U;
        }
        break;
    case Source::SCALE_FIT:
        // keep original aspect ratio
        scalex  *= aspectratio;
        // if it is too large, scale it
        if (aspectratio > renderingAspectRatio) {
            scalex *= renderingAspectRatio / aspectratio;
            scaley *= renderingAspectRatio / aspectratio;
        }
        break;
    case Source::SCALE_DEFORM:
        // alter aspect ratio to match the rendering area
        scalex  *= renderingAspectRatio;
        break;
    default:
    case Source::SCALE_CROP:
        // keep original aspect ratio
        scalex  *= aspectratio;
        // if it is not large enough, scale it
        if (aspectratio < renderingAspectRatio) {
            scalex *= renderingAspectRatio / aspectratio;
            scaley *= renderingAspectRatio / aspectratio;
        }
        break;
    }

    // test for culling
    testGeometryCulling();
}



void Source::draw(GLenum mode) const {

    // set id in select mode, avoid texturing if not rendering.
    if (mode == GL_SELECT) {
        glLoadName(id);
        glCallList(ViewRenderWidget::quad_texured);
    }
    else {

        // texture coordinate
        ViewRenderWidget::texc[0] = ViewRenderWidget::texc[6] = textureCoordinates.left();
        ViewRenderWidget::texc[1] = ViewRenderWidget::texc[3] = textureCoordinates.top();
        ViewRenderWidget::texc[2] = ViewRenderWidget::texc[4] = textureCoordinates.right();
        ViewRenderWidget::texc[5] = ViewRenderWidget::texc[7] = textureCoordinates.bottom();

        // draw vertex array
        glCallList(ViewRenderWidget::vertex_array_coords);
        glDrawArrays(GL_QUADS, 0, 4);
    }
}


void Source::setShaderAttributes() const {

    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_baseColor, texcolor);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_fading, (GLfloat) fade);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_baseAlpha, (GLfloat) texalpha);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_stippling, 0.f);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_gamma, (GLfloat) gammaRed, (GLfloat) gammaGreen, (GLfloat) gammaBlue, (GLfloat) gamma);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_levels, (GLfloat) gammaMinIn, (GLfloat) gammaMaxIn, (GLfloat) gammaMinOut, (GLfloat) gammaMaxOut);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_contrast, (GLfloat) contrast);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_brightness, (GLfloat) brightness);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_saturation, (GLfloat) saturation);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_hueshift, (GLfloat) hueShift);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_invertMode, (GLint) invertMode);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_nbColors, (GLint) numberOfColors);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_lumakey, (GLfloat) lumakeyThreshold / 100.f);

    if (luminanceThreshold > 0 )
        ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_threshold, (GLfloat) luminanceThreshold / 100.f);
    else
        ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_threshold, -1.f);

    if (useChromaKey) {
        ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_chromakey, (GLfloat) chromaKeyColor.redF(), (GLfloat) chromaKeyColor.greenF(), (GLfloat) chromaKeyColor.blueF(), 1.f );
        ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_chromadelta, (GLfloat) chromaKeyTolerance );
    } else {
        ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_chromakey, 0.f, 0.f, 0.f, 0.f );
        ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_chromadelta, 0.f );
    }

    // done if no filtering
    if (ViewRenderWidget::disableFiltering)
        return;

    // enable filtering
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_filter_type, (GLint) filter);
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_filter_step, 1.f / (GLfloat)  getFrameWidth(), 1.f / (GLfloat)  getFrameHeight());
    ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_filter_kernel, ViewRenderWidget::filter_kernel[filter]);

}

void Source::setCustomMaskTexture(QString filename)
{
    // reset custom image
    customMaskImage = QImage();

    // if we are provided with a filename
    if ( !filename.isEmpty() ) {
        // try to create an image with it
        QImage tmpimage(filename);
        // if the image is valid, it will be used in bind()
        if (!tmpimage.isNull()) {
            // make sure it is at the right format
            customMaskImage = tmpimage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            // store requested filename
            customMaskFilename = filename;
        }
    }

    // if failed, just create an inoperant mask texture
    if (customMaskImage.isNull()){
        customMaskImage = QImage(8,8,QImage::Format_ARGB32_Premultiplied);
        customMaskImage.fill(Qt::black);
        customMaskFilename = "";
        qWarning() << name << QChar(124).toLatin1()
                   << tr("Could not set custom texture ") << filename;
    }

}

void Source::bind() {

    GLuint texid = ViewRenderWidget::getMaskTexture(mask_type);

    // activate texture 1 ; double texturing of the mask
    glActiveTexture(GL_TEXTURE1);

    // bind the texture corresponding to the mask
    if (texid)
        glBindTexture(GL_TEXTURE_2D, texid);
    else
    {
        // bind custom mask texture
        glBindTexture(GL_TEXTURE_2D, customMaskTextureIndex);

        // if we have a new image to build the custom mask with
        if ( !customMaskImage.isNull()) {
            // fill texture with valid image
#if QT_VERSION >= 0x040700
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  customMaskImage.width(), customMaskImage.height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, customMaskImage.constBits() );
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, customMaskImage.width(), customMaskImage.height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, customMaskImage.bits() );
#endif
            // setup texturing parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            // no more use for the image; reset to null image
            customMaskImage = QImage();
        }
    }
    // back to texture 0 for the following
    glActiveTexture(GL_TEXTURE0);

#ifdef GLM_FFGL
    // if there are plugins, then the texture to bind is the
    // texture of the top of the plugins stack
    if (! _ffgl_plugins.isEmpty())
        _ffgl_plugins.bind();
    else
#endif
        glBindTexture(GL_TEXTURE_2D, getTextureIndex());

    // magnification filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixelated ? GL_NEAREST : GL_LINEAR);

}

void Source::update()  {
#ifdef GLM_FFGL
    // to be called at the end of the update of the source itself
    if (! _ffgl_plugins.isEmpty())
        _ffgl_plugins.update();
#endif

}

void Source::blend() const {

//	glBlendEquation(blend_eq);
    glBlendEquationSeparate(blend_eq, GL_MAX);

//	glBlendFunc(source_blend, destination_blend);
    glBlendFuncSeparate(source_blend, destination_blend, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO);

}



// freeframe gl plugin
#ifdef GLM_FFGL

FFGLPluginSource *Source::addFreeframeGLPlugin(QString filename) {

    if (filename.isNull() || filename == FFGLPluginSource::libraryFileName("ShadertoyEffect", false) )
        return _ffgl_plugins.pushNewPlugin(getFrameWidth(), getFrameHeight(), getTextureIndex());
    else
        return _ffgl_plugins.pushNewPlugin(filename, getFrameWidth(), getFrameHeight(), getTextureIndex());

}

FFGLPluginSourceStack *Source::getFreeframeGLPluginStack() {

    return &_ffgl_plugins;
}

bool Source::hasFreeframeGLPlugin() {

    return (_ffgl_plugins.count() > 0);
}

void Source::clearFreeframeGLPlugin() {

    _ffgl_plugins.clear();
}


void Source::reproduceFreeframeGLPluginStack(Source *s)
{
    clearFreeframeGLPlugin();

    // copy the Freeframe plugin stack
    for (FFGLPluginSourceStack::const_iterator it = s->getFreeframeGLPluginStack()->begin(); it != s->getFreeframeGLPluginStack()->end(); ++it) {

        FFGLPluginSource *plugin = addFreeframeGLPlugin( (*it)->fileName() );
        // set configuration
        if (plugin) {
            plugin->setParent( this );
            plugin->setConfiguration( (*it)->getConfiguration() );
        }
    }
}

#endif
