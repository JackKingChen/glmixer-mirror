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

#ifdef FFGL
#include "FFGLPluginSource.h"
#endif

GLuint Source::lastid = 1;
Source::RTTI Source::type = Source::SIMPLE_SOURCE;
bool Source::playable = false;


// source constructor.
Source::Source(GLuint texture, double depth): ProtoSource(),
    standby(NOT_STANDBY), culled(false), needupdate(true),
    clones(NULL), textureIndex(texture)
{
    // give it a unique identifier
    id = Source::lastid++;

    // default tag
    Tag::apply(this, Tag::getDefault());

    // empty clone list
    clones = new SourceList;
    CHECK_PTR_EXCEPTION(clones)

    // apply depth
    z = CLAMP(depth, MIN_DEPTH_LAYER, MAX_DEPTH_LAYER);
}

Source::~Source() {

    if (clones)
        delete clones;

#ifdef FFGL
    // delete all plugins in the stack
    _ffgl_plugins.clear();
#endif

    if (textureIndex > 0)
        // free the OpenGL texture
        glDeleteTextures(1, &textureIndex);
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
    emit methodCalled("_setName(QString)", S_ARG(name, n));

    ProtoSource::_setName(n);
}

void Source::setX(double v) {
    emit methodCalled("_setX(double)", S_ARG(x, v));

    ProtoSource::_setX(v);
}
void Source::setY(double v) {
    emit methodCalled("_setY(double)", S_ARG(y, v));

    ProtoSource::_setY(v);
}

void Source::setPosition(double posx, double posy) {
    emit methodCalled("_setPosition(double,double)", S_ARG(x, posx), S_ARG(y, posy));

    ProtoSource::_setPosition(posx, posy);

    // test for culling
    testGeometryCulling();
}

void Source::setRotationCenterX(double v) {
    emit methodCalled("_setCenterX(double)", S_ARG(centerx, v));

    ProtoSource::_setRotationCenterX(v);
}
void Source::setRotationCenterY(double v) {
    emit methodCalled("_setCenterY(double)", S_ARG(centery, v));

    ProtoSource::_setRotationCenterY(v);
}
void Source::setRotationAngle(double v) {
    emit methodCalled("_setRotationAngle(double)", S_ARG(rotangle, v));

    ProtoSource::_setRotationAngle(v);
}
void Source::setScaleX(double v) {
    emit methodCalled("_setScaleX(double)", S_ARG(scalex, v));

    ProtoSource::_setScaleX(v);
}
void Source::setScaleY(double v) {
    emit methodCalled("_setScaleY(double)", S_ARG(scaley, v));

    ProtoSource::_setScaleY(v);
}

void Source::setScale(double sx, double sy) {
    emit methodCalled("_setScale(double,double)", S_ARG(scalex, sx), S_ARG(scaley, sy));

    ProtoSource::_setScale(sx, sy);

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

void Source::setLuminanceThreshold(int l){
    emit methodCalled("_setLuminanceThreshold(int)", S_ARG(luminanceThreshold, l));

    ProtoSource::_setLuminanceThreshold(l);
}

void Source::setNumberOfColors(int n){
    emit methodCalled("_setNumberOfColors(int)", S_ARG(numberOfColors, n));

    ProtoSource::_setNumberOfColors(n);
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

void Source::setGamma(double g, double minI, double maxI, double minO, double maxO){
    emit methodCalled("_setGamma(double,double,double,double,double)", S_ARG(gamma, g), S_ARG(gammaMinIn, minI), S_ARG(gammaMaxIn, maxI), S_ARG(gammaMinOut, minO), S_ARG(gammaMaxOut, maxO));

    ProtoSource::_setGamma(g, minI, maxI, minO, maxO);
}

void Source::setPixelated(bool on) {
    emit methodCalled("_setPixelated(bool)", S_ARG(pixelated, on));
    ProtoSource::_setPixelated(on);
}

void Source::setModifiable(bool on) {
    emit methodCalled("_setModifiable(bool)", S_ARG(modifiable, on));

    ProtoSource::_setModifiable(on);
}


void Source::setBlendFunc(uint sfactor, uint dfactor) {
    emit methodCalled("_setBlendFunc(uint,uint)", S_ARG(source_blend, sfactor), S_ARG(destination_blend, dfactor));

    ProtoSource::_setBlendFunc(sfactor, dfactor);
}

void Source::setBlendEquation(uint eq) {
    emit methodCalled("_setBlendEquation(uint)", S_ARG(blend_eq, eq));

    ProtoSource::_setBlendEquation(eq);
}

void Source::setInvertMode(invertModeType i) {
    emit methodCalled("_setInvertMode(invertModeType)", S_ARG(invertMode, i));

    ProtoSource::_setInvertMode(i);
}

void Source::setFilter(filterType c) {
    emit methodCalled("_setFilter(filterType)", S_ARG(filter, c));

    ProtoSource::_setFilter(c);
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

    int w = SOURCE_UNIT;
    int h = SOURCE_UNIT;

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
        int d = sqrt( scalex*scalex + scaley*scaley);
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

    // ignore if there are clones relying on its update
    // TODO  : check dependency of clones : are they in standy ?
    if (isCloned())
        return;

    StandbyMode previous = standby;

#ifdef FFGL
    if (!isPlayable())
        standby = on ? PLAY_STANDBY : NOT_STANDBY;
    else
#endif
        // new status is based on request and play mode
        standby = on ? (isPlaying() ? PLAY_STANDBY : STOP_STANDBY) : NOT_STANDBY;

    // stop the source if in standby
    // else (not standby), replay if previous state was to play
    play( standby == NOT_STANDBY && previous == PLAY_STANDBY );

}

void Source::play(bool on) {

#ifdef FFGL
    // play the plugins only if not standby
    if (! _ffgl_plugins.isEmpty())
        _ffgl_plugins.play( isPlayable() ? on : standby == NOT_STANDBY );
#endif

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

    static int _baseColor = ViewRenderWidget::program->uniformLocation("baseColor");
    static int _baseAlpha = ViewRenderWidget::program->uniformLocation("baseAlpha");
    static int _stippling = ViewRenderWidget::program->uniformLocation("stippling");
    static int _gamma  = ViewRenderWidget::program->uniformLocation("gamma");
    static int _levels  = ViewRenderWidget::program->uniformLocation("levels");
    static int _contrast  = ViewRenderWidget::program->uniformLocation("contrast");
    static int _brightness  = ViewRenderWidget::program->uniformLocation("brightness");
    static int _saturation  = ViewRenderWidget::program->uniformLocation("saturation");
    static int _hueshift  = ViewRenderWidget::program->uniformLocation("hueshift");
    static int _invertMode  = ViewRenderWidget::program->uniformLocation("invertMode");
    static int _nbColors  = ViewRenderWidget::program->uniformLocation("nbColors");
    static int _threshold  = ViewRenderWidget::program->uniformLocation("threshold");
    static int _chromakey  = ViewRenderWidget::program->uniformLocation("chromakey");
    static int _chromadelta  = ViewRenderWidget::program->uniformLocation("chromadelta");
    static int _filter  = ViewRenderWidget::program->uniformLocation("filter");
    static int _filter_step  = ViewRenderWidget::program->uniformLocation("filter_step");
    static int _filter_kernel  = ViewRenderWidget::program->uniformLocation("filter_kernel");

    ViewRenderWidget::program->setUniformValue(_baseColor, texcolor);
    ViewRenderWidget::program->setUniformValue(_baseAlpha, (GLfloat) texalpha);
    ViewRenderWidget::program->setUniformValue(_stippling, 0.f);
    ViewRenderWidget::program->setUniformValue(_gamma, (GLfloat) gamma);
    ViewRenderWidget::program->setUniformValue(_levels, (GLfloat) gammaMinIn, (GLfloat) gammaMaxIn, (GLfloat) gammaMinOut, (GLfloat) gammaMaxOut);
    ViewRenderWidget::program->setUniformValue(_contrast, (GLfloat) contrast);
    ViewRenderWidget::program->setUniformValue(_brightness, (GLfloat) brightness);
    ViewRenderWidget::program->setUniformValue(_saturation, (GLfloat) saturation);
    ViewRenderWidget::program->setUniformValue(_hueshift, (GLfloat) hueShift);
    ViewRenderWidget::program->setUniformValue( _invertMode, (GLint) invertMode);
    ViewRenderWidget::program->setUniformValue( _nbColors, (GLint) numberOfColors);


    if (luminanceThreshold > 0 )
        ViewRenderWidget::program->setUniformValue(_threshold, (GLfloat) luminanceThreshold / 100.f);
    else
        ViewRenderWidget::program->setUniformValue(_threshold, -1.f);

    if (useChromaKey) {
        ViewRenderWidget::program->setUniformValue(_chromakey, (GLfloat) chromaKeyColor.redF(), (GLfloat) chromaKeyColor.greenF(), (GLfloat) chromaKeyColor.blueF(), 1.f );
        ViewRenderWidget::program->setUniformValue(_chromadelta, (GLfloat) chromaKeyTolerance );
    } else
        ViewRenderWidget::program->setUniformValue(_chromakey, 0.f, 0.f, 0.f, 0.f );

    if (ViewRenderWidget::disableFiltering)
        return;

    // else enabled filtering
    ViewRenderWidget::program->setUniformValue(_filter, (GLint) filter);
    ViewRenderWidget::program->setUniformValue(_filter_step, 1.f / (GLfloat)  getFrameWidth(), 1.f / (GLfloat)  getFrameHeight());

    ViewRenderWidget::program->setUniformValue( _filter_kernel, ViewRenderWidget::filter_kernel[filter]);

}

void Source::bind(bool withmask) const {

    // activate texture 1 ; double texturing of the mask
    glActiveTexture(GL_TEXTURE1);
    // select and enable the texture corresponding to the mask
    glBindTexture(GL_TEXTURE_2D, ViewRenderWidget::mask_textures[withmask ? mask_type : 0]);
    // back to texture 0 for the following
    glActiveTexture(GL_TEXTURE0);

#ifdef FFGL
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
#ifdef FFGL
    // to be called at the end of the update of the source itself
    if (! _ffgl_plugins.isEmpty())
        _ffgl_plugins.update();
#endif
    needupdate = false;
}

void Source::blend() const {

//	glBlendEquation(blend_eq);
    glBlendEquationSeparate(blend_eq, GL_MAX);

//	glBlendFunc(source_blend, destination_blend);
    glBlendFuncSeparate(source_blend, destination_blend, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO);

}



// freeframe gl plugin
#ifdef FFGL

FFGLPluginSource *Source::addFreeframeGLPlugin(QString filename) {

    if (filename.isNull())
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
        if (plugin)
            plugin->setConfiguration( (*it)->getConfiguration() );
    }
}

#endif

