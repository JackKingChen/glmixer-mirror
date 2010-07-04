/*
 * ViewRenderWidget.h
 *
 *  Created on: Feb 13, 2010
 *      Author: bh
 */

#ifndef VIEWRENDERWIDGET_H_
#define VIEWRENDERWIDGET_H_

#include <QDomElement>

#include "common.h"
#include "glRenderWidget.h"
#include "SourceSet.h"

class View;
class MixerView;
class GeometryView;
class LayersView;
class CatalogView;

class ViewRenderWidget: public glRenderWidget {

	Q_OBJECT

	friend class RenderingManager;
	friend class Source;
	friend class MixerView;
	friend class GeometryView;
	friend class LayersView;
	friend class CatalogView;
	friend class OutputRenderWidget;

public:
	ViewRenderWidget();
	virtual ~ViewRenderWidget();

	/**
	 * QGLWidget implementation
	 */
	void paintGL();
	void initializeGL();
	void resizeGL(int w = 0, int h = 0);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent ( QMouseEvent * event );
    void mouseDoubleClickEvent ( QMouseEvent * event );
    void wheelEvent ( QWheelEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    bool eventFilter(QObject *object, QEvent *event);
    void hideEvent ( QHideEvent * event ) { QGLWidget::hideEvent(event); }  // keep updating even if hidden
    void leaveEvent ( QEvent * event );

    /**
     * Specific methods
     */
    void displayFPS();
	float getFPS() { return f_p_s_; }
	void showFramerate(bool on) { showFps_ = on; }
	void setViewContextMenu(QMenu *m) { viewMenu = m; }
	void setCatalogContextMenu(QMenu *m) { catalogMenu = m; }

	/**
	 * management of the manipulation views
	 */
	typedef enum {NONE = 0, MIXING=1, GEOMETRY=2, LAYER=3 } viewMode;
	void setViewMode(viewMode mode);
	QPixmap getViewIcon();

	typedef enum {MOUSE_ARROW = 0, MOUSE_HAND_OPEN, MOUSE_HAND_CLOSED, MOUSE_SCALE_F, MOUSE_SCALE_B, MOUSE_ROT_TOP_RIGHT, MOUSE_ROT_TOP_LEFT, MOUSE_ROT_BOTTOM_RIGHT, MOUSE_ROT_BOTTOM_LEFT, MOUSE_QUESTION, MOUSE_SIZEALL, MOUSE_HAND_INDEX} mouseCursor;
	void setMouseCursor(mouseCursor c);

	typedef enum {TOOL_GRAB=0, TOOL_SCALE, TOOL_ROTATE, TOOL_CUT } toolMode;
	void setToolMode(toolMode m);
	toolMode getToolMode();

	/**
	 * save and load configuration
	 */
	QDomElement getConfiguration(QDomDocument &doc);
	void setConfiguration(QDomElement xmlconfig);

Q_SIGNALS:
	void sourceMixingModified();
	void sourceGeometryModified();
	void sourceLayerModified();

	void sourceMixingDrop(double, double);
	void sourceGeometryDrop(double, double);
	void sourceLayerDrop(double);

public Q_SLOTS:

	void clearViews();
	void zoomIn();
	void zoomOut();
	void zoomReset();
	void zoomBestFit();
	void refresh();
	void showMessage(QString s);
	inline void hideMessage() { displayMessage = false; }
	inline void setStipplingMode(int m) { quad_half_textured = quad_stipped_textured[CLAMP(m, 0, 3)]; }
	void contextMenu(const QPoint &);
	void setCatalogVisible(bool on = false);
	void setCatalogSizeSmall();
	void setCatalogSizeMedium();
	void setCatalogSizeLarge();
	inline void setFaded(bool on) { faded = on; }

protected:

	// all the display lists
	static GLuint border_thin_shadow, border_large_shadow;
	static GLuint border_thin, border_large, border_scale, border_rotate;
	static GLuint frame_selection, frame_screen;
	static GLuint quad_texured, quad_half_textured, quad_black;
	static GLuint circle_mixing, layerbg, catalogbg;
	static GLuint quad_stipped_textured[4];
	static GLuint mask_textures[8];
	static GLuint fading;

	// utility to build the display lists
    GLuint buildHalfList_fine();
    GLuint buildHalfList_gross();
    GLuint buildHalfList_checkerboard();
    GLuint buildHalfList_triangle();
    GLuint buildSelectList();
    GLuint buildLineList();
    GLuint buildTexturedQuadList();
    GLuint buildCircleList();
    GLuint buildLayerbgList();
    GLuint buildCatalogbgList();
    GLuint buildFrameList();
    GLuint buildBlackList();
    GLuint buildBordersList();
    GLuint buildFadingList();

private:
    // V i e w s
	View *currentManipulationView, *renderView;
	MixerView *mixingManipulationView;
	GeometryView *geometryManipulationView;
	LayersView *layersManipulationView;
	CatalogView *catalogView;

	// M e s s a g e s
	QString message;
	bool displayMessage;
	QTimer messageTimer;
	bool faded;
	QMenu *viewMenu, *catalogMenu;

	// F P S    d i s p l a y
	QTime fpsTime_;
	unsigned int fpsCounter_;
	float f_p_s_;
	bool showFps_;
};

#endif /* VIEWRENDERWIDGET_H_ */
