/*
 * Source.h
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

#ifndef SOURCE_H_
#define SOURCE_H_

#include <set>

#include "ProtoSource.h"
#include "defines.h"
#include "common.h"

#ifdef GLM_FFGL
#include "FFGLPluginSourceStack.h"
#endif

class SourceConstructorException : public AllocationException {
public:
    virtual QString message() { return "Could not allocate source"; }
    void raise() const { throw *this; }
    Exception *clone() const { return new SourceConstructorException(*this); }
};

class QtProperty;
class QGLFramebufferObject;

class Source;
typedef std::set<Source *> SourceList;

/**
 * Base class for every source mixed in GLMixer.
 *
 * A source is holding a texture index, and the drawing methods.
 * Every sources shall be instanciated by the rendering manager;
 * this is because the creation and manipulation of sources
 * requires an active opengl context; this is the task of the
 * rendering manager to call source methods after having made
 * an opengl context current.
 *
 * All properties manipulated by the user are held in ProtoSource.
 */
class Source: public ProtoSource {

    Q_OBJECT

    friend class RenderingManager;

public:
    /*
     * Constructor
     */
    Source(GLuint texture = -1, double depth = MAX_DEPTH_LAYER);

    virtual ~Source();


    /*
     * Run-Time Type Information
     */
    typedef enum {
        SIMPLE_SOURCE = 0,
        CLONE_SOURCE,
        VIDEO_SOURCE,
        OPENCV_SOURCE,
        ALGORITHM_SOURCE,
        RENDERING_SOURCE,
        CAPTURE_SOURCE,
        SVG_SOURCE,
        SHM_SOURCE,
        FFGL_SOURCE,
        WEB_SOURCE,
        SPOUT_SOURCE,
        STREAM_SOURCE,
        BASKET_SOURCE
    } RTTI;
    virtual RTTI rtti() const { return type; }
    virtual QString getInfo() const;

    /*
     * Methods to be reimplemented in sub-classes
     */
    virtual bool isPlayable() const { return playable; }
    virtual bool isPlaying() const { return false; }
    virtual void play(bool on);
    virtual int getFrameWidth() const { return 1; }
    virtual int getFrameHeight() const { return 1; }
    virtual double getFrameRate() const { return 0.0; }
    virtual double getAspectRatio() const {
        return (double) getFrameWidth() / (double) getFrameHeight();
    }

    QDomElement getConfiguration(QDomDocument &doc, QDir current);
    bool setConfiguration(QDomElement xmlconfig, QDir current = QDir());

    /**
     *  Rendering
     */
    // In subclasses of Source, the texture content is also updated
    virtual void update();
    // bind the texture
    virtual void bind();
    // apply the blending (including mask)
    // to be called in the OpenGL loop before drawing if the source shall be blended
    void blend() const;
    // begin and end the section which applies the various effects (convolution, color tables, etc).
    void setShaderAttributes() const;
    // to be called in the OpenGL loop to draw this source
    void draw(GLenum mode = GL_RENDER) const;
    //OpenGL access to the texture index
    virtual GLuint getTextureIndex() const;

    /*
     * unique ID of this source
     *
     */
    inline GLuint getId() const {
        return id;
    }
    // testing equality of source is testing if the ids are the same
    inline bool operator==(Source s2) {
        return (id == s2.id);
    }
    /*
     * Source manipulation
     *
     */
    // returns the list of clones of this source (used to delete them)
    inline SourceList *getClones() const {
        return clones;
    }
    inline bool isCloned() const {
        return clones->size() > 0;
    }

    // workspace
    inline void setWorkspace(int w) { workspace = w; }
    inline int getWorkspace() const { return workspace; }

    // custom mask
    void setCustomMaskTexture(QString filename);
    inline QString getCustomMaskTexture() const { return customMaskFilename; }

    /**
     *  Geometry and deformation
     */
    inline bool isCulled() const {
        return culled;
    }

    typedef enum {
        SCALE_CROP= 0,
        SCALE_FIT,
        SCALE_DEFORM,
        SCALE_PIXEL
    } scalingMode;

    void resetScale(scalingMode sm = SCALE_CROP);
    void clampScale(bool updateTextureCoordinates = false);
    // relative scaling
    void scaleBy(double fx, double fy);
    // reset texture coordinates
    void resetTextureCoordinates();

    /**
     * standby
     */
    typedef enum {
        NOT_STANDBY = 0,
        PLAY_STANDBY,
        STOP_STANDBY
    } StandbyMode;
    void setStandby(bool on);
    inline bool isStandby() const {
        return standby != NOT_STANDBY;
    }
    inline StandbyMode getStandbyMode() const {
        return standby;
    }
    inline void setStandbyMode(StandbyMode m) {
        standby = m;
    }

#ifdef GLM_FFGL
    // freeframe gl plugin
    FFGLPluginSource *addFreeframeGLPlugin(QString filename = QString::null);
    FFGLPluginSourceStack *getFreeframeGLPluginStack();
    bool hasFreeframeGLPlugin();
    void clearFreeframeGLPlugin();
    void reproduceFreeframeGLPluginStack(Source *s);
#endif

    /*
     *  Implementation of geometry methods
     */
    // position X
    void setX(double v);
    // position Y
    void setY(double v);
    // absolute position
    void setPosition(double, double);
    // rotation center X
    void setRotationCenterX(double v);
    // rotation center Y
    void setRotationCenterY(double v);
    // rotation angle
    void setRotationAngle(double v);
    // set absolute scale X
    void setScaleX(double v);
    // set absolute scale Y
    void setScaleY(double v);
    // set absolute scale X & Y
    void setScale(double sx, double sy);

    /*
     *  Implementation of ProtoSource methods
     */
    // name
    void setName(QString n);
    // fix the aspect ratio for scaling
    void setFixedAspectRatio(bool on);
    // change texture coordinates
    void setTextureCoordinates(QRectF textureCoords);
    // set Alpha on mixing view (compute alpha)
    void setAlphaCoordinates(double x, double y);
    // set Alpha (compute alpha coordinates)
    void setAlpha(double a);
    // set canvas mask
    void setMask(int maskType);
    // set canvas color
    void setColor(QColor c);
    // Adjust brightness factor
    void setBrightness(int b);
    // Adjust contrast factor
    void setContrast(int c);
    // Adjust saturation factor
    void setSaturation(int s);
    // Adjust hue shift factor
    void setHueShift(int h);
    // Adjust Luminance Threshold
    void setThreshold(int l);
    // Adjust Luma key Threshold
    void setLumakey(int l);
    // Adjust number of colors
    void setPosterized(int n);
    // chroma keying activation
    void setChromaKey(bool on);
    // chroma keying color
    void setChromaKeyColor(QColor c);
    // chroma keying tolerance
    void setChromaKeyTolerance(int t);
    // Adjust gamma and levels factors
    void setGammaColor(double value, double red, double green, double blue);
    void setGammaLevels(double minI, double maxI, double minO, double maxO);
    // display pixelated ?
    void setPixelated(bool on);
    // Blending function
    void setBlendFunc(uint sfactor, uint dfactor);
    // Blending equation
    void setBlendEquation(uint eq);
    // Color inversion
    void setInvertMode(invertModeType i);
    // filtering
    void setFilter(filterType c);


signals:
    // on playing
    void playing(bool);
    // on failure
    void failed();
    // on standby
    void standingby(bool);

protected:
    /*
     * Depth should only be modified by Rendering Manager
     */
    void setDepth(double v);
    void testGeometryCulling();

    // RTTI
    static RTTI type;
    static bool playable;

    // identity
    GLuint id;

    // standby mode
    StandbyMode standby;

    // flags for updating (or not)
    bool culled;

    // clone list
    SourceList *clones;

    // GL Stuff
    GLuint textureIndex;

    // Custom Mask
    GLuint customMaskTextureIndex;
    QString customMaskFilename;
    QImage customMaskImage;

    // workspace it belongs to
    int workspace;

#ifdef GLM_FFGL
    // freeframe plugin
    FFGLPluginSourceStack _ffgl_plugins;
#endif

    /*
     * Fade should only be modified internally
     */
    double fade;
    inline void setFading(double f) { fade = f; }
    inline double fading() { return fade; }

private:
    // identity counter
    static GLuint lastid;

};


#endif /* SOURCE_H_ */
