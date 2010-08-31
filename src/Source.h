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
 *   Copyright 2009, 2010 Bruno Herbelin
 *
 */

#ifndef SOURCE_H_
#define SOURCE_H_

#include <set>
#include <QColor>
#include <QMap>
#include <QDataStream>
#include <QRectF>

#include "common.h"

class QtProperty;

class Source;
typedef std::set<Source *> SourceList;

/**
 * Base class for every source mixed in GLMixer.
 *
 * A source is holding a texture index, all the geometric and mixing attributes, and the corresponding drawing methods.
 * Every sources shall be instanciated by the rendering manager; this is because the creation and manipulation of sources
 * requires an active opengl context; this is the task of the rendering manager to call source methods after having made
 * an opengl context current.
 *
 *
 */
class Source {

	friend class RenderingManager;

public:
	Source();
	virtual ~Source();

	bool operator==(Source s2) {
		return (id == s2.id);
	}

	// Run-Time Type Information
	typedef enum {
		SIMPLE_SOURCE = 0,
		CLONE_SOURCE,
		VIDEO_SOURCE,
		CAMERA_SOURCE,
		ALGORITHM_SOURCE,
		RENDERING_SOURCE,
		CAPTURE_SOURCE,
		MIX_SOURCE
	} RTTI;
	virtual RTTI rtti() const { return type; }
	virtual bool isPlayable() const { return playable; }
	virtual bool isPlaying() const { return false; }
	virtual void play(bool on) {}

	/**
	 *  Rendering
	 */
	// to be called in the OpenGL loop to bind the source texture before drawing
	// In subclasses of Source, the texture content is also updated
	virtual void update();
	// Request update explicitly (e.g. after changing a filter)
	inline void requestUpdate() {
		frameChanged = true;
	}
	// apply the blending (including mask)
	// to be called in the OpenGL loop before drawing if the source shall be blended
	void blend() const;
	// begin and end the section which applies the various effects (convolution, color tables, etc).
	void beginEffectsSection() const;
	void endEffectsSection() const;

	// to be called in the OpenGL loop to draw this source
	void draw(bool withalpha = true, GLenum mode = GL_RENDER) const;

	// OpenGL access to the texture index
	inline GLuint getTextureIndex() {
		return textureIndex;
	}
	// return true if this source is activated (shown as the current with a border)
	inline bool isActive() const {
		return active;
	}
	// sets if this source is active or not
	inline void activate(bool flag) {
		active = flag;
	}

	/**
	 * Manipulation
	 */
	// return the unique ID of this source
	inline GLuint getId() const {
		return id;
	}
	inline QString getName() const {
		return name;
	}
	void setName(QString n);

	// returns the list of clones of this source (used to delete them)
	inline SourceList *getClones() const {
		return clones;
	}

	/**
	 *  Geometry and deformation
	 */
	// gets
	inline GLdouble getAspectRatio() const {
		return aspectratio;
	}
	inline GLdouble getX() const {
		return x;
	}
	inline GLdouble getY() const {
		return y;
	}
	inline GLdouble getDepth() const {
		return z;
	}
	inline GLdouble getScaleX() const {
		return scalex;
	}
	inline GLdouble getScaleY() const {
		return scaley;
	}
	inline GLdouble getCenterX() const {
		return centerx;
	}
	inline GLdouble getCenterY() const {
		return centery;
	}
	inline GLdouble getRotationAngle() const {
		return rotangle;
	}
	inline QRectF getTextureCoordinates() const {
		return textureCoordinates;
	}
	// sets
	inline void setAspectRatio(GLdouble ar) {
		aspectratio = ar;
	}
	inline void setX(GLdouble v) {
		x = v;
	}
	inline void setY(GLdouble v) {
		y = v;
	}
	inline void setScaleX(GLdouble v) {
		scalex = v;
	}
	inline void setScaleY(GLdouble v) {
		scaley = v;
	}
	inline void setCenterX(GLdouble v) {
		centerx = v;
	}
	inline void setCenterY(GLdouble v) {
		centery = v;
	}
	inline void setRotationAngle(GLdouble v) {
		rotangle = v;
	}
	void moveTo(GLdouble posx, GLdouble posy);

	inline void setTextureCoordinates(QRectF textureCoords) {
		textureCoordinates = textureCoords;
	}
	inline void resetTextureCoordinates() {
		textureCoordinates.setCoords(0.0, 0.0, 1.0, 1.0);
	}

	void setScale(GLdouble sx, GLdouble sy);
	void scaleBy(GLfloat fx, GLfloat fy);
	void clampScale();

	typedef enum { SCALE_CROP= 0, SCALE_FIT, SCALE_DEFORM, SCALE_PIXEL} scalingMode;
	void resetScale(scalingMode sm = SCALE_CROP);

	inline bool isCulled() const {
		return culled;
	}
	void testCulling();

	/**
	 * Blending
	 */
	void setAlphaCoordinates(GLdouble x, GLdouble y);
	void setAlpha(GLfloat a);
	inline GLdouble getAlphaX() const {
		return alphax;
	}
	inline GLdouble getAlphaY() const {
		return alphay;
	}
	inline GLfloat getAlpha() const {
		return texalpha;
	}
	inline QColor getColor() const {
		return texcolor;
	}
	inline GLenum getBlendEquation() const {
		return blend_eq;
	}
	inline GLenum getBlendFuncSource() const {
		return source_blend;
	}
	inline GLenum getBlendFuncDestination() const {
		return destination_blend;
	}
	inline void setBlendFunc(GLenum sfactor, GLenum dfactor) {
		source_blend = sfactor;
		destination_blend = dfactor;
	}
	inline void setBlendEquation(GLenum eq) {
		blend_eq = eq;
	}

	typedef enum {
		NO_MASK = 0,
		ROUNDCORNER_MASK = 1,
		CIRCLE_MASK = 2,
		GRADIENT_CIRCLE_MASK = 3,
		GRADIENT_SQUARE_MASK = 4,
		GRADIENT_LEFT_MASK = 5,
		GRADIENT_RIGHT_MASK = 6,
		GRADIENT_TOP_MASK = 7,
		GRADIENT_BOTTOM_MASK = 8,
		CUSTOM_MASK = 9
	} maskType;
	void setMask(maskType t, GLuint texture = 0);
	int getMask() const {
		return (int) mask_type;
	}

	/**
	 * Coloring, image processing
	 */
	// set canvas color
	inline void setColor(QColor c){
		texcolor = c;
	}
	// Adjust brightness factor
	inline virtual void setBrightness(int b) {
		 brightness  = GLfloat(b) / 100.f;
	}
	inline virtual int getBrightness() const {
		return (int)(brightness * 100.f);
	}
	// Adjust contrast factor
	inline virtual void setContrast(int c) {
		 contrast  = GLfloat(c + 100) / 100.f;
	}
	inline virtual int getContrast() const {
		return (int)(contrast * 100.f) -100;
	}
	// Adjust saturation factor
	inline virtual void setSaturation(int s){
		saturation  = GLfloat(s + 100) / 100.f;
	}
	inline virtual int getSaturation() const {
		return (int)(saturation * 100.f) -100;
	}

	// Adjust hue shift factor
	inline void setHueShift(int h){
		hueShift = qBound(0.f, GLfloat(h) / 360.f, 1.f);
	}
	inline int getHueShift() const {
		return (int)(hueShift * 360.f);
	}
	// Adjust Luminance Threshold
	inline void setLuminanceThreshold(int l){
		luminanceThreshold = qBound(0, l, 100);
	}
	inline int getLuminanceThreshold() const {
		return luminanceThreshold;
	}
	// Adjust number of colors
	inline void setNumberOfColors(int n){
		numberOfColors = qBound(0, n, 256);
	}
	inline int getNumberOfColors() const {
		return numberOfColors;
	}
	// chroma keying
	inline void setChromaKey(bool on) {
		useChromaKey = on;
	}
	inline bool getChromaKey() const {
		return useChromaKey;
	}
	inline void setChromaKeyColor(QColor c) {
		chromaKeyColor = c;
	}
	inline QColor getChromaKeyColor() const {
		return chromaKeyColor;
	}
	inline void setChromaKeyTolerance(int t) {
		chromaKeyTolerance = qBound(0.f, GLfloat(t) / 100.f, 1.f);;
	}
	inline int getChromaKeyTolerance() const {
		return (int)(chromaKeyTolerance * 100.f);
	}

	// Adjust gamma and levels factors
	inline void setGamma(float g, float minI, float maxI, float minO, float maxO){
		gamma = g;
		gammaMinIn = qBound(0.f, minI, 1.f);
		gammaMaxIn = qBound(0.f, maxI, 1.f);
		gammaMinOut = qBound(0.f, minO, 1.f);
		gammaMaxOut = qBound(0.f, maxO, 1.f);
	}
	inline float getGamma() const {
		return gamma;
	}
	inline float getGammaMinInput() const {
		return gammaMinIn;
	}
	inline float getGammaMaxInput() const {
		return gammaMaxIn;
	}
	inline float getGammaMinOuput() const {
		return gammaMinOut;
	}
	inline float getGammaMaxOutput() const {
		return gammaMaxOut;
	}

	// display pixelated ?
	inline void setPixelated(bool on) {
		pixelated = on;
	}
	inline bool isPixelated() const {
		return pixelated;
	}

	// select a color table
	typedef enum {
		INVERT_NONE = 0,
		INVERT_COLOR,
		INVERT_LUMINANCE
	} invertModeType;
	inline void setInvertMode(invertModeType i) {
		invertMode = qBound(INVERT_NONE, i, INVERT_LUMINANCE);
	}
	inline invertModeType getInvertMode() const {
		return invertMode;
	}

	// select a filter
	typedef enum {
		FILTER_NONE = 0,
		FILTER_BLUR_GAUSSIAN,
		FILTER_BLUR_MEAN,
		FILTER_SHARPEN,
		FILTER_SHARPEN_MORE,
		FILTER_EDGE_GAUSSIAN,
		FILTER_EDGE_LAPLACE,
		FILTER_EDGE_LAPLACE_2,
		FILTER_EMBOSS,
		FILTER_EMBOSS_EDGE,
		FILTER_EROSION_3X3,
		FILTER_EROSION_7X7,
		FILTER_EROSION_13X13,
		FILTER_DILATION_3X3,
		FILTER_DILATION_7X7,
		FILTER_DILATION_13X13
	} filterType;
	inline void setFilter(filterType c) {
		filter = qBound(FILTER_NONE, c, FILTER_DILATION_13X13);
	}
	inline filterType getFilter() const {
		return filter;
	}

	void copyPropertiesFrom(const Source *s);

	virtual int getFrameWidth() const { return 0; }
	virtual int getFrameHeight() const { return 0; }

protected:
	/*
	 * Constructor ; only Rendering Manager is allowed
	 */
	Source(GLuint texture, double depth);
	/*
	 * also depth should only be modified by Rendering Manager
	 *
	 */
	void setDepth(GLdouble v);

	// RTTI
	static RTTI type;
	static bool playable;

	// identity and properties
	GLuint id;
	QString name;
	bool active, culled, frameChanged, cropped;
	SourceList *clones;

	// GL Stuff
	GLuint textureIndex, maskTextureIndex, iconIndex;
	GLdouble x, y, z;
	GLdouble scalex, scaley;
	GLdouble alphax, alphay;
	GLdouble centerx, centery, rotangle;
	GLdouble aspectratio;
	GLfloat texalpha;
	QColor texcolor;
	GLenum source_blend, destination_blend;
	GLenum blend_eq;
	QRectF textureCoordinates;

	// if should be set to GL_NEAREST
	bool pixelated;
	// which filter to apply?
	filterType filter;
	invertModeType invertMode;
	// which mask to use ?
	maskType mask_type;
	// Brightness, contrast and saturation
	GLfloat brightness, contrast, saturation;
	// gamma and its levels
	GLfloat gamma, gammaMinIn, gammaMaxIn, gammaMinOut, gammaMaxOut;
	// color manipulation
	GLfloat hueShift, chromaKeyTolerance;
	int luminanceThreshold, numberOfColors;
	QColor chromaKeyColor;
	bool useChromaKey;


	// statics
	static GLuint lastid;

};


QDataStream &operator<<(QDataStream &, const Source *);
QDataStream &operator>>(QDataStream &, Source *);

#endif /* SOURCE_H_ */
