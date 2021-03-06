/*
 * ViewRenderWidget.cpp
 *
 *  Created on: Feb 13, 2010
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

#include "ViewRenderWidget.moc"

#include "View.h"
#include "MixerView.h"
#include "GeometryView.h"
#include "LayersView.h"
#include "RenderingView.h"
#include "RenderingManager.h"
#include "SelectionManager.h"
#include "OutputRenderWindow.h"
#include "CatalogView.h"
#include "Cursor.h"
#include "SpringCursor.h"
#include "DelayCursor.h"
#include "AxisCursor.h"
#include "LineCursor.h"
#include "FuzzyCursor.h"
#include "MagnetCursor.h"
#include "glmixer.h"
#include "WorkspaceManager.h"

#ifdef GLM_SNAPSHOT
#include "SnapshotManager.h"
#include "SnapshotView.h"
#endif

GLuint ViewRenderWidget::vertex_array_coords = 0;
GLuint ViewRenderWidget::border_thin_shadow = 0,
        ViewRenderWidget::border_large_shadow = 0;
GLuint ViewRenderWidget::border_thin = 0, ViewRenderWidget::border_large = 0;
GLuint ViewRenderWidget::border_scale = 0, ViewRenderWidget::border_tooloverlay = 0;
GLuint ViewRenderWidget::quad_texured = 0, ViewRenderWidget::quad_window[] = {0, 0};
GLuint ViewRenderWidget::frame_selection = 0, ViewRenderWidget::frame_screen = 0;
GLuint ViewRenderWidget::frame_screen_thin = 0, ViewRenderWidget::frame_screen_mask = 0;
GLuint ViewRenderWidget::circle_mixing = 0, ViewRenderWidget::circle_limbo = 0, ViewRenderWidget::layerbg = 0;
QMap<int, GLuint>  ViewRenderWidget::mask_textures;
int ViewRenderWidget::mask_custom = 0;
QMap<int, QPair<QString, QString> >  ViewRenderWidget::mask_description;
GLuint ViewRenderWidget::fading = 0;
GLuint ViewRenderWidget::stipplingMode = 100;
GLuint ViewRenderWidget::black_texture = 0, ViewRenderWidget::white_texture = 0;
GLuint ViewRenderWidget::center_pivot = 0;
GLuint ViewRenderWidget::snapshot = 0;
double ViewRenderWidget::iconSize = DEFAULT_ICON_SIZE;

GLubyte ViewRenderWidget::stippling[] = {
        // stippling fine
        0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
        0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55,
        0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA,
        0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55,
        0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
        0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA,
        0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55,
        0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA,
        0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
        0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55,
        0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA,
        0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55 ,
        // stippling gross
        0xCC, 0xCC, 0xCC, 0xCC, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x33, 0x33, 0x33,
        0x33, 0x33, 0x33, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
        0xCC, 0xCC, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xCC,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x33, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
        0xCC, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xCC, 0xCC,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xCC, 0xCC, 0xCC,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xCC ,
        // stippling lines
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        // stippling GLM
        0x7B, 0xA2, 0x7B, 0xA2, 0x7B, 0xA2, 0x7B, 0xA2,
        0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22,
        0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22,
        0x5A, 0x2A, 0x5A, 0x2A, 0x5A, 0x2A, 0x5A, 0x2A,
        0x42, 0x36, 0x42, 0x36, 0x42, 0x36, 0x42, 0x36,
        0x42, 0x36, 0x42, 0x36, 0x42, 0x36, 0x42, 0x36,
        0x7A, 0x22, 0x7A, 0x22, 0x7A, 0x22, 0x7A, 0x22,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x7B, 0xA2, 0x7B, 0xA2, 0x7B, 0xA2, 0x7B, 0xA2,
        0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22,
        0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22, 0x4A, 0x22,
        0x5A, 0x2A, 0x5A, 0x2A, 0x5A, 0x2A, 0x5A, 0x2A,
        0x42, 0x36, 0x42, 0x36, 0x42, 0x36, 0x42, 0x36,
        0x42, 0x36, 0x42, 0x36, 0x42, 0x36, 0x42, 0x36,
        0x7A, 0x22, 0x7A, 0x22, 0x7A, 0x22, 0x7A, 0x22,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


//GLfloat ViewRenderWidget::coords[12] = {-1.f, -1.f, 0.f ,  1.f, -1.f, 0.f, 1.f, 1.f, 0.f,  -1.f, 1.f, 0.f  };
//GLfloat ViewRenderWidget::texc[8] = {0.f, 1.f,  1.f, 1.f,  1.f, 0.f,  0.f, 0.f};

GLfloat ViewRenderWidget::coords[8] = { -1.f, 1.f,  1.f, 1.f, 1.f, -1.f,  -1.f, -1.f };
GLfloat ViewRenderWidget::texc[8] = {0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 1.f};
GLfloat ViewRenderWidget::maskc[8] = {0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 1.f};
QGLShaderProgram *ViewRenderWidget::program = 0;
QString ViewRenderWidget::glslShaderFile = ":/glsl/shaders/imageProcessing_fragment.glsl";
bool ViewRenderWidget::disableFiltering = false;
int ViewRenderWidget::_baseColor = -1;
int ViewRenderWidget::_baseAlpha = -1;
int ViewRenderWidget::_stippling = -1;
int ViewRenderWidget::_gamma = -1;
int ViewRenderWidget::_levels = -1;
int ViewRenderWidget::_contrast = -1;
int ViewRenderWidget::_brightness = -1;
int ViewRenderWidget::_saturation = -1;
int ViewRenderWidget::_hueshift = -1;
int ViewRenderWidget::_invertMode = -1;
int ViewRenderWidget::_nbColors = -1;
int ViewRenderWidget::_threshold = -1;
int ViewRenderWidget::_chromakey = -1;
int ViewRenderWidget::_chromadelta = -1;
int ViewRenderWidget::_filter_type = -1;
int ViewRenderWidget::_filter_step = -1;
int ViewRenderWidget::_filter_kernel = -1;
int ViewRenderWidget::_fading = -1;
int ViewRenderWidget::_lumakey = -1;


const char * const black_xpm[] = { "2 2 1 1", ". c #000000", "..", ".."};
const char * const white_xpm[] = { "2 2 1 1", ". c #FFFFFF", "..", ".."};

/*
** FILTERS CONVOLUTION KERNELS
*/
#define KERNEL_DEFAULT {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}
#define KERNEL_BLUR_GAUSSIAN {0.0625, 0.125, 0.0625}, {0.125, 0.25, 0.125}, {0.0625, 0.125, 0.0625}
#define KERNEL_BLUR_MEAN {0.111111,0.111111,0.111111},{0.111111,0.111111,0.111111},{0.111111,0.111111,0.111111}
#define KERNEL_SHARPEN {0.0, -1.0, 0.0}, {-1.0, 5.0, -1.0}, {0.0, -1.0, 0.0}
#define KERNEL_SHARPEN_MORE {-1.0, -1.0, -1.0}, {-1.0, 9.0, -1.0}, {-1.0, -1.0, -1.0}
#define KERNEL_EDGE_GAUSSIAN {-0.0943852, -0.155615, -0.0943852}, {-0.155615, 1.0, -0.155615}, {-0.0943852, -0.155615, -0.0943852}
#define KERNEL_EDGE_LAPLACE {0.0, -1.0, 0.0}, {-1.0, 4.0, -1.0}, {0.0, -1.0, 0.0}
#define KERNEL_EDGE_LAPLACE_2 {-2.0, 1.0, -2.0}, {1.0, 4.0, 1.0}, {-2.0, 1.0, -2.0}
#define KERNEL_EMBOSS {-2.0, -1.0, 0.0}, {-1.0, 1.0, 1.0}, {0.0, 1.0, 2.0}
#define KERNEL_EMBOSS_EDGE {5.0, -3.0, -3.0}, {5.0, 0.0, -3.0}, {5.0, -3.0, -3.0}

GLfloat ViewRenderWidget::filter_kernel[10][3][3] = { {KERNEL_DEFAULT},
                                                      {KERNEL_BLUR_GAUSSIAN},
                                                      {KERNEL_BLUR_MEAN},
                                                      {KERNEL_SHARPEN},
                                                      {KERNEL_SHARPEN_MORE},
                                                      {KERNEL_EDGE_GAUSSIAN},
                                                      {KERNEL_EDGE_LAPLACE},
                                                      {KERNEL_EDGE_LAPLACE_2},
                                                      {KERNEL_EMBOSS},
                                                      {KERNEL_EMBOSS_EDGE } };

ViewRenderWidget::ViewRenderWidget() :
    glRenderWidget(), faded(false), suspended(false), busy(false), flashIntensity(0), zoomLabel(0), fpsLabel(0), viewMenu(0), catalogMenu(0), sourceMenu(0), showFps_(0)
{
    setObjectName("ViewRenderWidget");
    setAcceptDrops ( true );
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setMouseCursor(MOUSE_ARROW);

    // create the main views
    _renderView = new View;
    Q_CHECK_PTR(_renderView);
    _mixingView = new MixerView;
    Q_CHECK_PTR(_mixingView);
    _geometryView = new GeometryView;
    Q_CHECK_PTR(_geometryView);
    _layersView = new LayersView;
    Q_CHECK_PTR(_layersView);
    _renderingView = new RenderingView;
    Q_CHECK_PTR(_renderingView);
    // sets the current view
    _currentView = _renderView;

    // create the catalog view
    _catalogView = new CatalogView;
    Q_CHECK_PTR(_catalogView);

#ifdef GLM_SNAPSHOT
    // create the snapshot view
    _snapshotView = new SnapshotView;
    Q_CHECK_PTR(_snapshotView);
#endif

    // create the cursors
    _springCursor = new SpringCursor;
    Q_CHECK_PTR(_springCursor);
    _delayCursor = new DelayCursor;
    Q_CHECK_PTR(_delayCursor);
    _axisCursor = new AxisCursor;
    Q_CHECK_PTR(_axisCursor);
    _lineCursor = new LineCursor;
    Q_CHECK_PTR(_lineCursor);
    _fuzzyCursor = new FuzzyCursor;
    Q_CHECK_PTR(_fuzzyCursor);
    _magnetCursor = new MagnetCursor;
    Q_CHECK_PTR(_magnetCursor);
    // sets the current cursor
    _currentCursor = 0;
    cursorEnabled = false;

    // opengl HID display
    hideMessage();
    connect(&messageTimer, SIGNAL(timeout()), SLOT(hideMessage()));
    zoomLabelTimer.setSingleShot(true);
    connect(&zoomLabelTimer, SIGNAL(timeout()), SLOT(hideZoom()));
    zoomLabelTimer.setSingleShot(true);
    fpsTime_.start();
    fpsCounter_ = 0;
    f_p_s_ = 1000.0 / glRenderTimer::getInstance()->interval();

    // declare masks
    createMask("None", GL_NEAREST);
    createMask("Round", GL_LINEAR, ":/glmixer/textures/mask_roundcorner.png");
    createMask("Circle", GL_LINEAR, ":/glmixer/textures/mask_circle.png");
    createMask("Halo", GL_LINEAR, ":/glmixer/textures/mask_linear_circle.png");
    createMask("Square", GL_LINEAR, ":/glmixer/textures/mask_linear_square.png");
    createMask("Left-right", GL_LINEAR, ":/glmixer/textures/mask_linear_left.png");
    createMask("Right-left", GL_LINEAR, ":/glmixer/textures/mask_linear_right.png");
    createMask("Top-down", GL_LINEAR, ":/glmixer/textures/mask_linear_bottom.png");
    createMask("Bottom-up", GL_LINEAR, ":/glmixer/textures/mask_linear_top.png");
    createMask("Horizontal", GL_LINEAR, ":/glmixer/textures/mask_linear_horizontal.png");
    createMask("Vertical", GL_LINEAR, ":/glmixer/textures/mask_linear_vertical.png");
    createMask("Border", GL_LINEAR, ":/glmixer/textures/mask_antialiasing.png");
    createMask("Scratch", GL_LINEAR, ":/glmixer/textures/mask_scratch.png"); // 12
    createMask("Dirty", GL_LINEAR, ":/glmixer/textures/mask_dirty.png");
    createMask("TV", GL_LINEAR, ":/glmixer/textures/mask_tv.png");
    createMask("Paper", GL_LINEAR, ":/glmixer/textures/mask_paper.png");
    createMask("Towel", GL_LINEAR, ":/glmixer/textures/mask_towel.png");
    createMask("Sand", GL_LINEAR, ":/glmixer/textures/mask_sand.png");
    createMask("Diapo", GL_LINEAR, ":/glmixer/textures/mask_diapo.png");
    createMask("Ink", GL_LINEAR, ":/glmixer/textures/mask_ink.png");
    createMask("Grungy", GL_LINEAR, ":/glmixer/textures/mask_think.png");
    createMask("Pixel H", GL_NEAREST, ":/glmixer/textures/mask_pixel_horizontal.png");
    createMask("Pixel V", GL_NEAREST, ":/glmixer/textures/mask_pixel_vertical.png");
    createMask("PixelGrid", GL_NEAREST, ":/glmixer/textures/mask_says.png");
    createMask("Gabor H", GL_LINEAR, ":/glmixer/textures/mask_gabor_h.png");
    createMask("Gabor V", GL_LINEAR, ":/glmixer/textures/mask_gabor_v.png");
    createMask("GaborGrid", GL_LINEAR, ":/glmixer/textures/mask_grid.png");
    createMask("Vignette", GL_LINEAR, ":/glmixer/textures/mask_checker.png");
    createMask("Sphere", GL_LINEAR, ":/glmixer/textures/mask_bubble.png");
    ViewRenderWidget::mask_custom = createMask("Custom", GL_LINEAR, ":/glmixer/textures/mask_custom.png");

    // events input
    grabGesture(Qt::PinchGesture);
    connect(this, SIGNAL(gestureEvent(QGestureEvent *)), SLOT(onGestureEvent(QGestureEvent *)));
    connect(this, SIGNAL(specialKeyboardEvent(QKeyEvent *)), SLOT(onSpecialKeyboardEvent(QKeyEvent *)));


#ifdef GLM_SNAPSHOT
    connect(SnapshotManager::getInstance(), SIGNAL(clear()), SLOT(activateSnapshot()));
#endif
}

ViewRenderWidget::~ViewRenderWidget()
{
    delete _renderView;
    delete _mixingView;
    delete _geometryView;
    delete _layersView;
    delete _renderingView;
    delete _catalogView;
    delete _springCursor;
    delete _delayCursor;
    delete _axisCursor;
    delete _lineCursor;
    delete _fuzzyCursor;
    delete _magnetCursor;

#ifdef GLM_SNAPSHOT
    delete _snapshotView;
#endif
}


int ViewRenderWidget::createMask(QString description, GLuint filter, QString texture)
{
    int mask = ViewRenderWidget::mask_description.size();
    // store desription string & texture filename
    ViewRenderWidget::mask_description[mask] = QPair<QString, QString>(description, texture);
    ViewRenderWidget::mask_textures[mask] = filter;

    return mask;
}

const QMap<int, QPair<QString, QString> > ViewRenderWidget::getMaskDecription()
{
    return ViewRenderWidget::mask_description;
}

const GLuint ViewRenderWidget::getMaskTexture(int mask)
{
    // custom mask texture
    if (mask == ViewRenderWidget::mask_custom)
        return 0;
    // all other textures
    return ViewRenderWidget::mask_textures[mask];
}

void ViewRenderWidget::initializeGL()
{
    glRenderWidget::initializeGL();
    setBackgroundColor(QColor(COLOR_BGROUND));

    // useful textures
    white_texture = bindTexture(QPixmap(white_xpm), GL_TEXTURE_2D);
    black_texture = bindTexture(QPixmap(black_xpm), GL_TEXTURE_2D);

    // Create mask textures from predefined
    QMapIterator<int, QPair<QString, QString> > i(ViewRenderWidget::mask_description);
    while (i.hasNext()) {
        // loop
        i.next();
        GLint filter = ViewRenderWidget::mask_textures[i.key()];
        // create and store texture index
        if (i.value().second.isNull()) {
            ViewRenderWidget::mask_textures[i.key()] = black_texture;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        } else {
            ViewRenderWidget::mask_textures[i.key()] = bindTexture(QPixmap(i.value().second), GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

    // Create display lists

    // display list to enable the vertex array for drawing
    vertex_array_coords = glGenLists(1);
    glNewList(vertex_array_coords, GL_COMPILE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, ViewRenderWidget::coords);
    glTexCoordPointer(2, GL_FLOAT, 0, ViewRenderWidget::maskc);
    glEndList();

    // display lists for drawing GUI
    quad_texured = buildTexturedQuadList();
    border_thin_shadow = buildLineList();
    border_large_shadow = border_thin_shadow + 1;
    frame_selection = buildSelectList();
    circle_mixing = buildCircleList();
    circle_limbo = buildLimboCircleList();
    layerbg = buildLayerbgList();
    quad_window[0] = buildWindowList(0, 0, 0);
    quad_window[1] = buildWindowList(255, 255, 255);
    frame_screen = buildFrameList();
    frame_screen_thin = frame_screen + 2;
    frame_screen_mask = frame_screen + 3;
    border_thin = buildBordersList();
    border_large = border_thin + 1;
    border_scale = border_thin + 2;
    border_tooloverlay = buildBordersTools();
    fading = buildFadingList();
    center_pivot = buildPivotPointList();
    snapshot = buildSnapshotList();

    // store render View matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-SOURCE_UNIT * OutputRenderWindow::getInstance()->getAspectRatio(), SOURCE_UNIT * OutputRenderWindow::getInstance()->getAspectRatio(), -SOURCE_UNIT, SOURCE_UNIT);
    glGetDoublev(GL_PROJECTION_MATRIX, _renderView->projection);
    glGetDoublev(GL_PROJECTION_MATRIX, _catalogView->projection);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetDoublev(GL_MODELVIEW_MATRIX, _renderView->modelview);
    glGetDoublev(GL_MODELVIEW_MATRIX, _catalogView->modelview);

    // create GLSL program
    ViewRenderWidget::program = new QGLShaderProgram(this);
}

void ViewRenderWidget::setViewMode(View::viewMode mode)
{
    switch (mode)
    {
    case View::MIXING:
        _currentView = (View *) _mixingView;
        break;
    case View::GEOMETRY:
        _currentView = (View *) _geometryView;
        break;
    case View::LAYER:
        _currentView = (View *) _layersView;
        break;
    case View::RENDERING:
        _currentView = (View *) _renderingView;
        break;
    case View::NULLVIEW:
    default:
        _currentView = _renderView;
        break;
    }

    // set action and reset previous action
    _currentView->setAction(View::NONE);
    _currentView->setAction(View::NONE);
    // make sure mouse cursor is arrow
    setMouseCursor(MOUSE_ARROW);

    // update view to match with the changes in modelview and projection matrices (e.g. resized widget)
    makeCurrent();
    refresh();

    emit zoomPercentChanged((int) _currentView->getZoomPercent());

}


void ViewRenderWidget::selectWholeGroup(Source *s)
{
    if (  _mixingView->isInAGroup(s) ) {
        SelectionManager::getInstance()->select( *(_mixingView->findGroup(s)) );
    }
}

void ViewRenderWidget::removeFromSelections(Source *s)
{
    SelectionManager::getInstance()->deselect(s);
    _mixingView->removeFromGroup(s);
}

void ViewRenderWidget::setCatalogVisible(bool on)
{
    _catalogView->setVisible(on);
}

int ViewRenderWidget::catalogWidth()
{
    if (_catalogView->visible())
        return _catalogView->viewport[2];
    else
        return 0;
}

void ViewRenderWidget::setCatalogSizeSmall()
{
    _catalogView->setSize(CatalogView::SMALL);
}

void ViewRenderWidget::setCatalogSizeMedium()
{
    _catalogView->setSize(CatalogView::MEDIUM);
}

void ViewRenderWidget::setCatalogSizeLarge()
{
    _catalogView->setSize(CatalogView::LARGE);
}

void ViewRenderWidget::showContextMenu(ViewContextMenu m, const QPoint &pos)
{
    // emulate mouse released
    emit mousePressed(false);

    // display appropriate menu
    switch (m) {
    case CONTEXT_MENU_VIEW:
        if (viewMenu)
            viewMenu->exec(mapToGlobal(pos));
        break;
    case CONTEXT_MENU_SOURCE:
        if (sourceMenu)
            sourceMenu->exec(mapToGlobal(pos));
        break;
    case CONTEXT_MENU_CATALOG:
        if (catalogMenu)
            catalogMenu->exec(mapToGlobal(pos));
        break;
    case CONTEXT_MENU_DROP:
        {
            static QMenu *dropMenu = 0;
            if (!dropMenu) {
                dropMenu = new QMenu(this);
                QAction *newAct = new QAction(QObject::tr("Cancel drop"), this);
                connect(newAct, SIGNAL(triggered()), RenderingManager::getInstance(), SLOT(clearBasket()));
                dropMenu->addAction(newAct);
            }
            dropMenu->exec(mapToGlobal(pos));
        }
        break;
    }

}

void ViewRenderWidget::setToolMode(toolMode m, View::viewMode v){

    switch (v)
    {
    case View::MIXING:
        _mixingView->setTool( (View::toolType) m );
        break;
    case View::GEOMETRY:
        _geometryView->setTool( (View::toolType) m );
        break;
    case View::NULLVIEW:
    default:
        _currentView->setTool( (View::toolType) m );
        _currentView->setAction( View::NONE );
        break;
    }
}

ViewRenderWidget::toolMode ViewRenderWidget::getToolMode(View::viewMode v){

    View::toolType t = View::MOVE;
    switch (v)
    {
    case View::MIXING:
        t = _mixingView->getTool();
        break;
    case View::GEOMETRY:
        t = _geometryView->getTool();
        break;
    case View::NULLVIEW:
    default:
        t = _currentView->getTool();
        break;
    }

    return (ViewRenderWidget::toolMode) t;
}


void ViewRenderWidget::setCursorEnabled(bool on) {

    if (_currentCursor == 0)
        cursorEnabled = false;
    else
        cursorEnabled = on;
}

void ViewRenderWidget::triggerFlash()
{
    flashIntensity = 240;
}

#ifdef GLM_SNAPSHOT
void ViewRenderWidget::activateSnapshot(QString id)
{
    if (id.isNull())
        // make invisible
        _snapshotView->deactivate();
    else {
        // activate snapshot control view
        if ( !_snapshotView->activate(_currentView, id) )
            showMessage( tr("No change to apply in this view."), 3000 );
    }
}
#endif

void ViewRenderWidget::setCursorMode(cursorMode m){

    switch(m) {
    case ViewRenderWidget::CURSOR_DELAY:
        _currentCursor = _delayCursor;
        break;
    case ViewRenderWidget::CURSOR_SPRING:
        _currentCursor = _springCursor;
        break;
    case ViewRenderWidget::CURSOR_AXIS:
        _currentCursor = _axisCursor;
        break;
    case ViewRenderWidget::CURSOR_LINE:
        _currentCursor = _lineCursor;
        break;
        break;
    case ViewRenderWidget::CURSOR_FUZZY:
        _currentCursor = _fuzzyCursor;
        break;
    case ViewRenderWidget::CURSOR_MAGNET:
        _currentCursor = _magnetCursor;
        break;
    default:
    case ViewRenderWidget::CURSOR_NORMAL:
        _currentCursor = 0;
        break;
    }

    cursorEnabled = false;
}

ViewRenderWidget::cursorMode ViewRenderWidget::getCursorMode(){

    if (_currentCursor == _springCursor)
        return ViewRenderWidget::CURSOR_SPRING;
    if (_currentCursor == _delayCursor)
        return ViewRenderWidget::CURSOR_DELAY;
    if (_currentCursor == _axisCursor)
        return ViewRenderWidget::CURSOR_AXIS;
    if (_currentCursor == _lineCursor)
        return ViewRenderWidget::CURSOR_LINE;
    if (_currentCursor == _fuzzyCursor)
        return ViewRenderWidget::CURSOR_FUZZY;
    if (_currentCursor == _magnetCursor)
        return ViewRenderWidget::CURSOR_MAGNET;

    return ViewRenderWidget::CURSOR_NORMAL;
}

Cursor *ViewRenderWidget::getCursor(cursorMode m)
{
    switch(m) {
    case ViewRenderWidget::CURSOR_DELAY:
        return (Cursor*)_delayCursor;
    case ViewRenderWidget::CURSOR_SPRING:
        return (Cursor*)_springCursor;
    case ViewRenderWidget::CURSOR_AXIS:
        return (Cursor*)_axisCursor;
    case ViewRenderWidget::CURSOR_LINE:
        return (Cursor*)_lineCursor;
    case ViewRenderWidget::CURSOR_FUZZY:
        return (Cursor*)_fuzzyCursor;
    case ViewRenderWidget::CURSOR_MAGNET:
        return (Cursor*)_magnetCursor;
    default:
    case ViewRenderWidget::CURSOR_NORMAL:
        return 0;
    }

}

/**
 *  REDIRECT every calls to the current view implementation
 */

void ViewRenderWidget::resizeGL(int w, int h)
{
#ifdef GLM_SNAPSHOT
    // modify snapshot view
    _snapshotView->resize(w, h);
#endif
    // modify catalog view
    _catalogView->resize(w, h);

    // resize the current view itself
    _currentView->resize(w, h);

    // a font to display text overlay at an appropriate size
    labelfont = QFont(getMonospaceFont(), qBound( 10, qMin(h,w) / 80, 22), QFont::Normal);
}

void ViewRenderWidget::refresh()
{
    makeCurrent();

    // store render View matrices ; output render window may have been resized, and the ViewRenderWidget is told so if necessary
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-SOURCE_UNIT * OutputRenderWindow::getInstance()->getAspectRatio(), SOURCE_UNIT * OutputRenderWindow::getInstance()->getAspectRatio(), -SOURCE_UNIT, SOURCE_UNIT);
    glGetDoublev(GL_PROJECTION_MATRIX, _renderView->projection);
    glGetDoublev(GL_PROJECTION_MATRIX, _catalogView->projection);


#ifdef GLM_SNAPSHOT
    // modify snapshot view
    _snapshotView->resize(width(), height());
#endif

    // default resize ; will refresh everything
    _currentView->resize(width(), height());

    // make sure source icons are updated
    for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++) {
        // update the content of the sources
        (*its)->update();
    }

}

void ViewRenderWidget::paintGL()
{
    static GLfloat angle = 0;

    // for animation
    emit tick();

    //
    // 1. The view
    //
    // background clear
    glRenderWidget::paintGL();

    // if a GLSL fragment shader is requested, load it.
    if (!ViewRenderWidget::glslShaderFile.isEmpty()) {

        // setup GLSL program
        setupFilteringShaderProgram(glslShaderFile);

        // do not need the filename anymore
        glslShaderFile = QString::null;
    }

    // apply modelview transformations from zoom and panning only when requested
    if (_currentView->isModified()) {
        _currentView->setModelview();
    }


    for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++) {

        // update the content of the sources if not in standy
        if (!(*its)->isStandby())
            (*its)->update();
    }

    // draw the view
    _currentView->paint();

    //
    // 3. draw a semi-transparent overlay if view should be faded out
    //
    //

    if (faded){
        // the fading overlay is in a single call list with given color
        glColor4ub(COLOR_FADING, 98);
        glCallList(ViewRenderWidget::fading);
    }

    if (flashIntensity > 0) {
        // the fading overlay is in a single call list with given color
        glColor4ub(COLOR_FLASHING, flashIntensity);
        glCallList(ViewRenderWidget::fading);
        flashIntensity = flashIntensity < 10 ? 0 : flashIntensity / 2;
    }

    if (suspended) {

        if (busy) {
            // the busy overlay is in two call lists for animation
            glCallList(ViewRenderWidget::fading + 1);
            glScalef( 800.0 / (float) width(), 800.0 / (float) height(), 1.0);
            // animation of busy circle by steps of 36 degrees (10 dots)
            angle = fmod(angle + 3602.0, 360.0);
            glRotatef( angle - fmod(angle, 36.0), 0.0, 0.0, 1.0);
            glCallList(ViewRenderWidget::fading + 2);
        }
        else {
            // the fading overlay is in a single call list with given color
            glColor4ub(COLOR_FADING, 148);
            glCallList(ViewRenderWidget::fading);
        }
    }
    // if not suspended, means the area is active
    else
    {
        //
        // 2. the shadow of the cursor
        //
        if (cursorEnabled && _currentCursor->apply(f_p_s_) ) {

            _currentCursor->draw(_currentView->viewport);

            if (_currentView->mouseMoveEvent( _currentCursor->getMouseMoveEvent() ))
            {
                // the view 'mouseMoveEvent' returns true ; there was something changed!
                onCurrentSourceModified();
            }
        }
    }


    //
    // 4. The extra information
    //

#ifdef GLM_SNAPSHOT
    // Snapshot : show if visible
    if (_snapshotView->isActive()) {
        // draw the view
        _snapshotView->paint();
        // draw text overlay
        qglColor(Qt::lightGray);
        int labelwidth = QFontMetrics(labelfont).width(_snapshotView->getInstructions());
        renderText( (width()-labelwidth) / 2, height() - 20, _snapshotView->getInstructions(), labelfont);
        // nothing more
        return;
    }
#endif

    // Catalog : show if visible
    if (_catalogView->visible())
        _catalogView->paint();

    // FPS computation every 5 frames
    if (++fpsCounter_ == 5)
    {
        fpsCounter_ = 0;
        // exponential moving average to smooth a little
        f_p_s_ = 0.6 * f_p_s_ + 0.4 * ( 5000.0 / float(fpsTime_.restart()) );
//        f_p_s_ = 5000.0 / float(fpsTime_.restart()); // debug instantaneous fps

        if (fpsLabel && showFps_) {
            fpsLabel->setText(QString("%1 fps").arg(f_p_s_, 0, 'f', ((f_p_s_ < 10.0) ? 1 : 0)) );
        }
    }
    // HUD display of framerate (on request or if FPS is dangerously slow)
    if (showFps_ || ( f_p_s_ < 800.0 / (float) glRenderTimer::getInstance()->interval() && f_p_s_ > 0) )
        displayFramerate();

    // Pause logo
    if (RenderingManager::getInstance()->isPaused()){
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, width(), 0.0, height());

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        qglColor(Qt::lightGray);
        glRecti(15, height() - 5, 25, height() - 30);
        glRecti(30, height() - 5, 40, height() - 30);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    if (!message.isNull()) {
        qglColor(Qt::lightGray);
        renderText(20, height() - 20, message, labelfont);
    }
}

void ViewRenderWidget::displayFramerate()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, width(), 0.0, height());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    qglColor(Qt::lightGray);
    glRecti(width() - 61, height() - 1, width() - 9, height() - 11);
    float p = (float)  glRenderTimer::getInstance()->interval();
    qglColor(f_p_s_ > 800.f / p ? Qt::darkGreen : (f_p_s_ > 500.f / p ? Qt::yellow : Qt::red));
    // Draw a filled rectangle of lengh proportionnal to % of target fps
    glRecti(width() - 60, height() - 2, width() - 60 + qBound(0, (int)( 0.05 * f_p_s_ * p), 50), height() - 10);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void ViewRenderWidget::setFramerateVisible(bool on)
{
    showFps_ = on;

    if (fpsLabel)
        fpsLabel->clear();

}


void ViewRenderWidget::onCurrentSourceModified()
{
    switch (_currentView->getMode()){
    case View::MIXING:
        emit sourceMixingModified();
        break;
    case View::GEOMETRY:
        emit sourceGeometryModified();
        break;
    case View::LAYER:
        emit sourceLayerModified();
        break;
    default:
    case View::NULLVIEW:
    case View::RENDERING:
        break;
    }
}

void ViewRenderWidget::mousePressEvent(QMouseEvent *event)
{
    makeCurrent();
    event->accept();

    if (suspended)
        return;

    // inform mouse was pressed
    emit mousePressed(true);

#ifdef GLM_SNAPSHOT
    if (_snapshotView->mousePressEvent(event))
        return;
#endif

    // ask the catalog view if it wants this mouse press event and then
    // inform the view of the mouse press event
    if (!_catalogView->mousePressEvent(event) )
    {

        if (_currentView->mousePressEvent(event))
            // enable cursor on a clic
            setCursorEnabled(true);
        else
        // if there is something to drop, inform the rendering manager that it can drop the source at the clic coordinates
        if (RenderingManager::getInstance()->getSourceBasketTop() &&  _currentView->isUserInput(event, View::INPUT_TOOL)  )
        {
            double x = 0.0, y = 0.0;
            _currentView->coordinatesFromMouse(event->x(), event->y(), &x, &y);

            // depending on the view, ask the rendering manager to drop the source with the user parameters
            switch (_currentView->getMode()){
            case View::MIXING:
                emit sourceMixingDrop(x, y);
                break;
            case View::GEOMETRY:
                emit sourceGeometryDrop(x, y);
                break;
            case View::LAYER:
                emit sourceLayerDrop(x);
                break;
            default:
            case View::NULLVIEW:
            case View::RENDERING:
                break;
            }
        }
    }

    if ( cursorEnabled && _currentView->isUserInput(event, View::INPUT_TOOL) )
        _currentCursor->update(event);
}

void ViewRenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    makeCurrent();
    event->accept();

    if (suspended)
        return;

#ifdef GLM_SNAPSHOT
    if (_snapshotView->mouseMoveEvent(event))
        return;
#endif

    // ask the catalog view if it wants this mouse move event
    if ( (_currentView->currentAction == View::NONE ||
          _currentView->currentAction == View::OVER )
         // do not enter catalog if already button down
         && (! event->buttons() ^ Qt::NoButton )
         // test catalog move event
         && _catalogView->mouseMoveEvent(event) )
        return;

    if (cursorEnabled && _currentCursor->isActive())
        _currentCursor->update(event);
    else if (_currentView->mouseMoveEvent(event)){
        // the view 'mouseMoveEvent' returns true ; there was something changed!
        onCurrentSourceModified();
    }

    // keep track of cursor when getting out of the widget during an action
    static QCursor previousCursor = QCursor(Qt::BlankCursor);
    // if user currently performing an action
    if ( _currentView->currentAction != View::NONE ) {
        // and the cursor continues out of the window
        if (! geometry().contains(event->pos()) ) {
            // first time getting out ? (previous still blank)
            if (previousCursor.shape() == Qt::BlankCursor){
                // then remember the cursor
                previousCursor = cursor();
                // and hide the cursor
                setCursor(Qt::BlankCursor);
            }
        }
        // the cursor is inside the window
        else
            // and the previous cursor was stored
            if ( previousCursor.shape() != Qt::BlankCursor){
                // therefore we set it back
                setCursor(previousCursor);
                // and set previous to blank
                previousCursor.setShape(Qt::BlankCursor);
            }
    }

}

void ViewRenderWidget::mouseReleaseEvent(QMouseEvent * event)
{
    makeCurrent();
    event->accept();

    if (suspended)
        return;

    if (cursorEnabled) {
        // inform cursor of release event
        _currentCursor->update(event);

        // disable cursor
        setCursorEnabled(false);
    }

#ifdef GLM_SNAPSHOT
    if (_snapshotView->mouseReleaseEvent(event))
        return;
#endif

    // ask the catalog view if it wants this mouse release event
    if ( !_catalogView->mouseReleaseEvent(event)) {

        // not used by catalog, so forward to views
        if (_currentView->mouseReleaseEvent(event)) {
            // the view 'mouseReleaseEvent' returns true ; there was something changed!
            onCurrentSourceModified();
        }

    }

    // inform mouse was released
    emit mousePressed(false);
}

void ViewRenderWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    makeCurrent();
    event->accept();

    if (suspended)
        return;

#ifdef GLM_SNAPSHOT
    if (_snapshotView->mouseDoubleClickEvent(event))
        return;
#endif

    if (_catalogView->mouseDoubleClickEvent(event))
        return;

    if (_currentView->mouseDoubleClickEvent(event)){
        // the view 'mouseDoubleClickEvent' returns true ; there was something changed!
        onCurrentSourceModified();
    }

}

void ViewRenderWidget::wheelEvent(QWheelEvent * event)
{
    makeCurrent();
    event->accept();

#ifdef GLM_SNAPSHOT
    if (_snapshotView->wheelEvent(event))
        return;
#endif

    if (_catalogView->wheelEvent(event))
        return;

    if (cursorEnabled && _currentCursor->wheelEvent(event))
        return;

    if (_currentView->wheelEvent(event)) {
        showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
        emit zoomPercentChanged((int) _currentView->getZoomPercent());
    }
}


void ViewRenderWidget::tabletEvent ( QTabletEvent * event )
{
    // take input from the pressure sensor for the cursor (if enabled)
    if (cursorEnabled && _currentCursor && _currentCursor->isActive() && event->pressure() > 0.05)
        _currentCursor->setParameter(event->pressure());

    // emulate a mous button (Extra button) when eraser is pressed
    if( event->pointerType() == QTabletEvent::Eraser) {

        static bool previousPressed = false;
        bool pressed = event->pressure() > 0.05;

        if (pressed && !previousPressed) {
            QMouseEvent e(QEvent::MouseButtonPress, QPoint(event->x(),event->y()), Qt::XButton1, Qt::XButton1, event->modifiers());

            mousePressEvent(&e);
        }
        else if (!pressed && previousPressed){
            QMouseEvent e(QEvent::MouseButtonRelease, QPoint(event->x(),event->y()), Qt::NoButton, Qt::NoButton, event->modifiers());

            mouseReleaseEvent(&e);
        }
        else {

            QMouseEvent e(QEvent::MouseMove, QPoint(event->x(),event->y()), pressed ? Qt::XButton1 : Qt::NoButton, pressed ? Qt::XButton1 : Qt::NoButton, event->modifiers());

            mouseMoveEvent(&e);
        }

        previousPressed = pressed;
        event->accept();
    }
    // else, other buttons of the stylus = normal buttons
    else
        event->ignore();
}


void ViewRenderWidget::keyPressEvent(QKeyEvent * event)
{

#ifdef GLM_SNAPSHOT
    if (_snapshotView->keyPressEvent(event))
        return;
#endif

    if (_currentView->keyPressEvent(event))
    {
        event->accept();
        // the view 'keyPressEvent' returns true ; there was something changed!
        onCurrentSourceModified();
    }
    else
        QGLWidget::keyPressEvent(event);
}


void ViewRenderWidget::keyReleaseEvent(QKeyEvent * event)
{
    if (_currentView->keyReleaseEvent(event))
    {
        event->accept();
        // the view 'keyReleaseEvent' returns true ; there was something changed!
        onCurrentSourceModified();
    }
    else
        QGLWidget::keyPressEvent(event);
}


void ViewRenderWidget::leaveEvent ( QEvent * event ){

    // cancel current action
    _currentView->setAction(View::NONE);
    _currentView->setAction(View::NONE);
    // set the catalog  off
    _catalogView->setTransparent(true);
#ifdef GLM_SNAPSHOT
    // deactivate snapshot view
    activateSnapshot();
#endif
}

void ViewRenderWidget::enterEvent ( QEvent * event ){

    // just to be 100% sure no action is current
    _currentView->setAction(View::NONE);
    _currentView->setAction(View::NONE);
    setMouseCursor(ViewRenderWidget::MOUSE_ARROW);

}

void ViewRenderWidget::zoom(int percent)
{
    _currentView->setZoomPercent( double(percent) );

    showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
    // emit zoomPercentChanged((int) _currentView->getZoomPercent());
}

void ViewRenderWidget::zoomIn()
{
    _currentView->zoomIn();

    showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
    emit zoomPercentChanged((int) _currentView->getZoomPercent());
}

void ViewRenderWidget::zoomOut()
{
    _currentView->zoomOut();

    showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
    emit zoomPercentChanged((int) _currentView->getZoomPercent());
}

void ViewRenderWidget::zoomReset()
{
    _currentView->zoomReset();

    showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
    emit zoomPercentChanged((int) _currentView->getZoomPercent());
}

void ViewRenderWidget::zoomBestFit()
{
    _currentView->zoomBestFit();

    showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
    emit zoomPercentChanged((int) _currentView->getZoomPercent());
}

void ViewRenderWidget::zoomCurrentSource()
{
    _currentView->zoomBestFit(true);

    showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
    emit zoomPercentChanged((int) _currentView->getZoomPercent());
}

void ViewRenderWidget::clearViews()
{
    // clear all views
    _mixingView->clear();
    _geometryView->clear();
    _layersView->clear();
    _renderingView->clear();

    refresh();
}

void ViewRenderWidget::showZoom(QString s)
{
    zoomLabelTimer.stop();
    if (zoomLabel)
        zoomLabel->setText(s);
    zoomLabelTimer.start(1000);
}

void ViewRenderWidget::hideZoom()
{
    if (zoomLabel)
        zoomLabel->clear();
}

void ViewRenderWidget::showMessage(const QString &s, int timeout)
{
    message = s;
    messageTimer.start(timeout);
}

void ViewRenderWidget::hideMessage()
{
    message = QString();
}

void ViewRenderWidget::alignSelection(View::Axis a, View::RelativePoint p, View::Reference r)
{
    // if there is NO selection, temporarily select the current source
    bool alignCurrentSource = ! SelectionManager::getInstance()->hasSelection();
    if (alignCurrentSource)
        SelectionManager::getInstance()->selectCurrentSource();

    // apply on current view
    _currentView->alignSelection(a, p, r);

    // restore selection state
    if (alignCurrentSource)
        SelectionManager::getInstance()->clearSelection();
    else
        // update the selection source for geometry view
        SelectionManager::getInstance()->updateSelectionSource();

}

void ViewRenderWidget::distributeSelection(View::Axis a, View::RelativePoint p)
{
    // ignore empty selection
    if (! SelectionManager::getInstance()->hasSelection())
        return;

    // apply on current view
    _currentView->distributeSelection(a, p);

    // update the selection source for geometry view
    SelectionManager::getInstance()->updateSelectionSource();
}

void ViewRenderWidget::transformSelection(View::Transformation t, View::Axis a, View::Reference r)
{
    // if there is NO selection, temporarily select the current source
    bool resizeCurrentSource = ! SelectionManager::getInstance()->hasSelection();
    if (resizeCurrentSource)
        SelectionManager::getInstance()->selectCurrentSource();

    // apply on current view
    _currentView->transformSelection(t, a, r);

    // restore selection state
    if (resizeCurrentSource)
        SelectionManager::getInstance()->clearSelection();
    else
        // update the selection source for geometry view
        SelectionManager::getInstance()->updateSelectionSource();
}


/**
 * save and load configuration
 */
QDomElement ViewRenderWidget::getConfiguration(QDomDocument &doc)
{
    QDomElement config = doc.createElement("Views");

    config.setAttribute("current", (int) _currentView->getMode());
    config.setAttribute("workspace", WorkspaceManager::getInstance()->current());
    config.setAttribute("workspaceCount", WorkspaceManager::getInstance()->count());
    config.setAttribute("workspaceExclusive", WorkspaceManager::getInstance()->isExclusiveDisplay());

    QDomElement mix = _mixingView->getConfiguration(doc);
    mix.setAttribute("name", "Mixing");
    config.appendChild(mix);

    QDomElement geom = _geometryView->getConfiguration(doc);
    geom.setAttribute("name", "Geometry");
    config.appendChild(geom);

    QDomElement depth = _layersView->getConfiguration(doc);
    depth.setAttribute("name", "Depth");
    config.appendChild(depth);

    QDomElement render = _renderingView->getConfiguration(doc);
    render.setAttribute("name", "Rendering");
    config.appendChild(render);

    QDomElement catalog = doc.createElement("Catalog");
    config.appendChild(catalog);
    catalog.setAttribute("visible", _catalogView->visible());
    QDomElement s = doc.createElement("Parameters");
    s.setAttribute("catalogSize", _catalogView->getSize());
    catalog.appendChild(s);

    return config;
}

void ViewRenderWidget::setConfiguration(QDomElement xmlconfig)
{
    int tmp = xmlconfig.attribute("workspaceCount", "3").toInt();
    WorkspaceManager::getInstance()->setCount(tmp);
    tmp = xmlconfig.attribute("workspace", "0").toInt();
    WorkspaceManager::getInstance()->setCurrent(tmp);
    tmp = xmlconfig.attribute("workspaceExclusive", "0").toInt();
    WorkspaceManager::getInstance()->setExclusiveDisplay( (bool) tmp);

    QDomElement child = xmlconfig.firstChildElement("View");
    while (!child.isNull()) {
        // apply configuration node
        if (child.attribute("name") == "Mixing")
            _mixingView->setConfiguration(child);
        else if (child.attribute("name") == "Geometry")
            _geometryView->setConfiguration(child);
        else if (child.attribute("name") == "Depth")
            _layersView->setConfiguration(child);
        else if (child.attribute("name") == "Rendering")
            _renderingView->setConfiguration(child);

        child = child.nextSiblingElement();
    }
    // NB: the catalog is restored in GLMixer::openSessionFile because GLMixer has access to the actions
}

void ViewRenderWidget::setLimboConfiguration(QDomElement xmlconfig)
{
    QDomElement child = xmlconfig.firstChildElement("View");
    while (!child.isNull()) {
        if (child.attribute("name") == "Mixing") {
            _mixingView->setLimboSize(child.firstChildElement("Limbo").attribute("value", "2.5").toFloat());
            break;
        }
        child = child.nextSiblingElement();
    }
}

void ViewRenderWidget::setBaseColor(QColor c, float alpha)
{
    if (_baseColor<0) return;

    program->setUniformValue(_baseColor, c);
    program->setUniformValue(_fading, 1.f);

    if (alpha > -1.0)
        program->setUniformValue(_baseAlpha, alpha);
    else
        program->setUniformValue(_baseAlpha, 1.f);
}

void ViewRenderWidget::resetShaderAttributes()
{
    if (_baseColor<0) return;

    // set color & alpha
    program->setUniformValue(_baseColor, QColor(Qt::white));
    program->setUniformValue(_baseAlpha, 1.f);
    program->setUniformValue(_stippling, 0.f);
    program->setUniformValue(_fading, 1.f);
    // gamma
    program->setUniformValue(_gamma, 1.f, 1.f, 1.f, 1.f);
    program->setUniformValue(_levels, 0.f, 1.f, 0.f, 1.f);
    // effects
    program->setUniformValue(_contrast, 1.f);
    program->setUniformValue(_saturation, 1.f);
    program->setUniformValue(_brightness, 0.f);
    program->setUniformValue(_hueshift, 0.f);
    program->setUniformValue(_chromakey, 0.f, 0.f, 0.f, 0.f );
    program->setUniformValue(_threshold, -1.f);
    program->setUniformValue(_lumakey, -1.f);
    program->setUniformValue(_nbColors, (GLint) -1);
    program->setUniformValue(_invertMode, (GLint) 0);

    // disable filtering
    if (!disableFiltering) {
        program->setUniformValue(_filter_step, 1.f / 640.f, 1.f / 480.f);
        program->setUniformValue(_filter_type, (GLint) 0);
        program->setUniformValue(_filter_kernel, filter_kernel[0]);
    }

    // activate texture 1 ; double texturing of the mask
    glActiveTexture(GL_TEXTURE1);
    // select and enable the texture corresponding to the mask
    glBindTexture(GL_TEXTURE_2D, mask_textures[0]);
    // back to texture 0 for the following
    glActiveTexture(GL_TEXTURE0);

    // reset texture coordinate
    texc[0] = texc[6] = 0.f;
    texc[1] = texc[3] = 1.f;
    texc[2] = texc[4] = 1.f;
    texc[5] = texc[7] = 0.f;
    program->setAttributeArray ("texCoord", ViewRenderWidget::texc, 2, 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

}

void ViewRenderWidget::setupFilteringShaderProgram(QString fshfile)
{
    if (!program)
        return;

    if (fshfile.isEmpty())
        return;

    // delete previous program if existed
    program->removeAllShaders();

    if (!program->addShaderFromSourceFile(QGLShader::Fragment, fshfile))
        qFatal( "%s", qPrintable( QObject::tr("OpenGL GLSL error in fragment shader; \n\n%1").arg(program->log()) ) );
    else if (program->log().contains("warning"))
        qCritical() << fshfile << QChar(124).toLatin1() << QObject::tr("OpenGL GLSL warning in fragment shader;%1").arg(program->log());

    if (!program->addShaderFromSourceFile(QGLShader::Vertex, ":/glsl/shaders/imageProcessing_vertex.glsl"))
        qFatal( "%s", qPrintable( QObject::tr("OpenGL GLSL error in vertex shader; \n\n%1").arg(program->log()) ) );
    else if (program->log().contains("warning"))
        qCritical() << "imageProcessing_vertex.glsl" << QChar(124).toLatin1()<< QObject::tr("OpenGL GLSL warning in vertex shader;%1").arg(program->log());

    if (!program->link())
        qFatal( "%s", qPrintable( QObject::tr("OpenGL GLSL linking error; \n\n%1").arg(program->log()) ) );

    if (!program->bind())
        qFatal( "%s", qPrintable( QObject::tr("OpenGL GLSL binding error; \n\n%1").arg(program->log()) ) );

    // set the pointer to the array for the texture attributes
    program->setAttributeArray ("texCoord", ViewRenderWidget::texc, 2, 0);
    program->enableAttributeArray("texCoord");
    program->setAttributeArray ("maskCoord", ViewRenderWidget::maskc, 2, 0);
    program->enableAttributeArray("maskCoord");

    // get uniforms
    _baseColor = program->uniformLocation("baseColor");
    _baseAlpha = program->uniformLocation("baseAlpha");
    _stippling = program->uniformLocation("stippling");
    _gamma  = program->uniformLocation("gamma");
    _levels  = program->uniformLocation("levels");
    _contrast  = program->uniformLocation("contrast");
    _brightness  = program->uniformLocation("brightness");
    _saturation  = program->uniformLocation("saturation");
    _hueshift  = program->uniformLocation("hueshift");
    _invertMode  = program->uniformLocation("invertMode");
    _nbColors  = program->uniformLocation("nbColors");
    _threshold  = program->uniformLocation("threshold");
    _chromakey  = program->uniformLocation("chromakey");
    _chromadelta  = program->uniformLocation("chromadelta");
    _fading  = program->uniformLocation("fade");
    _lumakey  = program->uniformLocation("lumakey");

    // set the default values for the uniform variables
    program->setUniformValue("sourceTexture", 0);
    program->setUniformValue("maskTexture", 1);
    program->setUniformValue("sourceDrawing", false);
    program->setUniformValue(_fading, 1.f);

    if (!ViewRenderWidget::disableFiltering) {
        _filter_type  = program->uniformLocation("filter_type");
        _filter_step  = program->uniformLocation("filter_step");
        _filter_kernel  = program->uniformLocation("filter_kernel");
    }

    resetShaderAttributes();

    // ready
    program->release();

    qDebug() << fshfile << QChar(124).toLatin1()<< QObject::tr("OpenGL GLSL Fragment Shader loaded.");
}


void ViewRenderWidget::setFilteringEnabled(bool on)
{
    // ignore if nothing changes
    if ( disableFiltering != on)
        return;

    // apply flag
    disableFiltering = !on;

    // update fragment shader
    if (disableFiltering)
        glslShaderFile = ":/glsl/shaders/imageProcessing_fragment_simplified.glsl";
    else
        glslShaderFile = ":/glsl/shaders/imageProcessing_fragment.glsl";

}

void ViewRenderWidget::setSourceDrawingMode(bool on)
{
    if (on) {
        // start using the GLSL program
        program->bind();

        // texture enabled
        glEnable(GL_TEXTURE_2D);
    }
    else {
        // end using the GLSL program
        program->release();

        // standard transparency blending
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }

}

/**
 * Build a display lists for the line borders and returns its id
 **/
GLuint ViewRenderWidget::buildSelectList()
{
    GLuint base = glGenLists(2);

    // selected
    glNewList(base, GL_COMPILE);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glLineWidth(3.0);
        glColor4ub(COLOR_SELECTION, 255);
        glLineStipple(1, 0x7777);
        // glLineStipple(1, 0x6666);
        glEnable(GL_LINE_STIPPLE);
        glCallList(vertex_array_coords);
        glPushMatrix();
        glScalef(1.12, 1.12, 1.0);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glPopMatrix();
        glDisable(GL_LINE_STIPPLE);

    glEndList();

    // selected
    glNewList(base + 1, GL_COMPILE);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glLineWidth(3.0);
        glLineStipple(1, 0x6186);
        glEnable(GL_LINE_STIPPLE);
        glCallList(vertex_array_coords);
        glPushMatrix();
        glScalef(1.12, 1.12, 1.0);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glPopMatrix();
        glDisable(GL_LINE_STIPPLE);

    glEndList();

    return base;
}

/**
 * Build a display list of a textured QUAD and returns its id
 *
 * This is used only for the source drawing in GL_SELECT mode
 *
 **/
GLuint ViewRenderWidget::buildTexturedQuadList()
{
    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);

        glBegin(GL_QUADS); // begin drawing a square

        glTexCoord2f(0.f, 1.f);
        glVertex2f(-1.0f, -1.0f); // Bottom Left
        glTexCoord2f(1.f, 1.f);
        glVertex2f(1.0f, -1.0f); // Bottom Right
        glTexCoord2f(1.f, 0.f);
        glVertex2f(1.0f, 1.0f); // Top Right
        glTexCoord2f(0.f, 0.f);
        glVertex2f(-1.0f, 1.0f); // Top Left

        glEnd();


    glEndList();
    return id;
}

/**
 * Build 2 display lists for the line borders and shadows
 **/
GLuint ViewRenderWidget::buildLineList()
{
    GLuint base = glGenLists(4);
    glListBase(base);

    // default thin border
    glNewList(base, GL_COMPILE);

        glCallList(vertex_array_coords);

        glLineWidth(2.0);
        glBindTexture(GL_TEXTURE_2D, white_texture);
        glPushMatrix();
        glScalef(1.05, 1.05, 1.0);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glPopMatrix();

    glEndList();

    // over
    glNewList(base + 1, GL_COMPILE);

        glCallList(vertex_array_coords);

        glBindTexture(GL_TEXTURE_2D, white_texture);
        glLineWidth(4.0);
        glPointSize(3.0);
        glPushMatrix();
        glScalef(1.05, 1.05, 1.0);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glDrawArrays(GL_POINTS, 0, 4);
        glPopMatrix();

    glEndList();

    // default border STATIC
    glNewList(base + 2, GL_COMPILE);

        glCallList(vertex_array_coords);

        glBindTexture(GL_TEXTURE_2D, white_texture);
        glLineWidth(1.0);
        glPushMatrix();
        glScalef(1.05, 1.05, 1.0);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glPopMatrix();

    glEndList();

    // over STATIC
    glNewList(base + 3, GL_COMPILE);

        glCallList(vertex_array_coords);

        glBindTexture(GL_TEXTURE_2D, white_texture);
        glLineWidth(3.0);
        glPushMatrix();
        glScalef(1.05, 1.05, 1.0);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glPopMatrix();

    glEndList();

    return base;
}

//#define CIRCLE_PIXELS 512
//#define CIRCLE_PIXEL_RADIUS 65536.0
//#define CIRCLE_PIXELS 256
//#define CIRCLE_PIXEL_RADIUS 16384.0
//#define CIRCLE_PIXELS 128
//#define CIRCLE_PIXEL_RADIUS 4096.0
#define CIRCLE_PIXELS 64
#define CIRCLE_PIXEL_RADIUS 1024.0
//#define CIRCLE_PIXELS 32
//#define CIRCLE_PIXEL_RADIUS 256.0
//#define CIRCLE_PIXELS 16
//#define CIRCLE_PIXEL_RADIUS 64.0


GLuint ViewRenderWidget::buildCircleList()
{
    GLuint id = glGenLists(3);
    GLUquadricObj *quadObj = gluNewQuadric();

//    glActiveTexture(GL_TEXTURE0);

    static GLuint texid = 0;
    if (texid == 0) {
        // generate the texture with alpha exactly as computed for sources
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        GLfloat matrix[CIRCLE_PIXELS*CIRCLE_PIXELS*2];
        GLfloat luminance = 0.f;
        GLfloat alpha = 0.f;
        double distance = 0.0;
        int l = -CIRCLE_PIXELS / 2 + 1, c = 0;
        for (int i = 0; i < CIRCLE_PIXELS / 2; ++i) {
            c = -CIRCLE_PIXELS / 2 + 1;
            for (int j=0; j < CIRCLE_PIXELS / 2; ++j) {
                // distance to the center
                distance = (double) ((c * c) + (l * l)) / CIRCLE_PIXEL_RADIUS;
                // luminance
                luminance = CLAMP( 0.95 - 0.8 * distance, 0.f, 1.f);
                // alpha
                alpha = CLAMP( 1.0 - distance , 0.f, 1.f);

                // 1st quadrant
                matrix[ j * 2 + i * CIRCLE_PIXELS * 2 ] = luminance ;
                matrix[ 1 + j * 2 + i * CIRCLE_PIXELS * 2 ] =  alpha;
                // 2nd quadrant
                matrix[ (CIRCLE_PIXELS -j -1)* 2 + i * CIRCLE_PIXELS * 2 ] = luminance;
                matrix[ 1 + (CIRCLE_PIXELS -j -1) * 2 + i * CIRCLE_PIXELS * 2 ] = alpha;
                // 3rd quadrant
                matrix[ j * 2 + (CIRCLE_PIXELS -i -1) * CIRCLE_PIXELS * 2 ] = luminance;
                matrix[ 1 + j * 2 + (CIRCLE_PIXELS -i -1) * CIRCLE_PIXELS * 2 ] = alpha;
                // 4th quadrant
                matrix[ (CIRCLE_PIXELS -j -1) * 2 + (CIRCLE_PIXELS -i -1) * CIRCLE_PIXELS * 2 ] =  luminance;
                matrix[ 1 + (CIRCLE_PIXELS -j -1) * 2 + (CIRCLE_PIXELS -i -1) * CIRCLE_PIXELS * 2 ] = alpha;

                ++c;
            }
            ++l;
        }
        // two components texture : luminance and alpha
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16_ALPHA16, CIRCLE_PIXELS, CIRCLE_PIXELS, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, (float *) matrix);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }

    // background checkerboard
    GLuint texid2 = bindTexture(QPixmap(QString(":/glmixer/textures/checkerboard.png")), GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glNewList(id, GL_COMPILE);

        glPushMatrix();
        glTranslatef(0.0, 0.0, -1.1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        // grey mask in the surrounding area
        glColor4ub(COLOR_BGROUND, 90);
        gluDisk(quadObj, CIRCLE_SIZE * SOURCE_UNIT, 3 * CIRCLE_SIZE * SOURCE_UNIT,  80, 3);

        // circle with generated texture
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texid);
        glColor4ub(255, 255, 255, 255);
        gluQuadricTexture(quadObj, GL_TRUE);
        gluDisk(quadObj, 0.0, CIRCLE_SIZE * SOURCE_UNIT, 80, 3);
        glDisable(GL_TEXTURE_2D);

        // line contour
        // blended antialiasing
        glColor4ub(COLOR_CIRCLE, 250);
        glLineWidth(1.5);
        glRotatef(90.f, 0, 0, 1);
        glBegin(GL_LINE_LOOP);
        for (float i = 0.f; i < 2.f * M_PI; i += M_PI / 40.f )
            glVertex2f(CIRCLE_SIZE * SOURCE_UNIT * cos(i), CIRCLE_SIZE * SOURCE_UNIT * sin(i));
        glEnd();

        glPopMatrix();

    glEndList();

    glNewList(id + 1, GL_COMPILE);

        glPushMatrix();
        glTranslatef(0.0, 0.0, -1.1);

        // blended antialiasing
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glColor4ub(COLOR_CIRCLE_MOVE, 250);
        glLineWidth(5.0);

        glRotatef(90.f, 0, 0, 1);
        glBegin(GL_LINE_LOOP);
        for (float i = 0.f; i < 2.f * M_PI; i += M_PI / 40.f )
            glVertex2f(CIRCLE_SIZE * SOURCE_UNIT * cos(i), CIRCLE_SIZE * SOURCE_UNIT * sin(i));
        glEnd();

        glPopMatrix();

    glEndList();

    glNewList(id + 2, GL_COMPILE);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.f, 0.f, -1.2f);
        glScalef( 2.f *SOURCE_UNIT, 2.f * SOURCE_UNIT, 1.f);

        glDisable(GL_BLEND);
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texid2);
        glColor4ub(255, 255, 255, 255);

        glBegin(GL_QUADS); // begin drawing a square
        glTexCoord2f(0.f, 50.f);
        glVertex2f(-1.0f, -1.0f); // Bottom Left
        glTexCoord2f(50.f, 50.f);
        glVertex2f(1.0f, -1.0f); // Bottom Right
        glTexCoord2f(50.f, 0.f);
        glVertex2f(1.0f, 1.0f); // Top Right
        glTexCoord2f(0.f, 0.f);
        glVertex2f(-1.0f, 1.0f); // Top Left
        glEnd();

        glDisable(GL_TEXTURE_2D);

        glPopMatrix();

    glEndList();

    // free quadric object
    gluDeleteQuadric(quadObj);

    return id;
}


GLuint ViewRenderWidget::buildPivotPointList()
{
    GLuint id = glGenLists(1);
    GLUquadricObj *quadObj = gluNewQuadric();

    glNewList(id, GL_COMPILE);

    // blended antialiasing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    //limbo area
    glColor4ub(COLOR_SELECTION_AREA, 80);
    gluDisk(quadObj, 0, 3, 30, 3);

    // border
    glLineWidth(1.0);
    glColor4ub(COLOR_SELECTION, 255);
    glBegin(GL_LINE_LOOP);
    for (float i = 0; i < 2.0 * M_PI; i += 0.2)
        glVertex2f(3 * cos(i), 3 * sin(i));
    glEnd();
    glPopMatrix();

    glEndList();

    return id;
}

GLuint ViewRenderWidget::buildLimboCircleList()
{
    GLuint id = glGenLists(2);
    glListBase(id);
    GLUquadricObj *quadObj = gluNewQuadric();


    // limbo area
    glNewList(id, GL_COMPILE);
        glPushMatrix();
        glTranslatef(0.0, 0.0, -1.0);

        // blended antialiasing
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        //limbo area
        glColor4ub(COLOR_BGROUND, 255);
        gluDisk(quadObj, CIRCLE_SIZE * SOURCE_UNIT, 10 * CIRCLE_SIZE * SOURCE_UNIT,  80, 3);

        // border
        glLineWidth(1.0);
        glColor4ub(COLOR_LIMBO_CIRCLE, 255);
        glBegin(GL_LINE_LOOP);
        for (float i = 0; i < 2.0 * M_PI; i += 0.07)
            glVertex2f(CIRCLE_SIZE * SOURCE_UNIT * cos(i), CIRCLE_SIZE * SOURCE_UNIT * sin(i));
        glEnd();
        glPopMatrix();
    glEndList();

    glNewList(id+1, GL_COMPILE);
        glPushMatrix();
        glTranslatef(0.0, 0.0, -1.0);
        // blended antialiasing
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        // border
        glLineWidth(3.0);
        glColor4ub(COLOR_LIMBO_CIRCLE, 255);
        glBegin(GL_LINE_LOOP);
        for (float i = 0; i < 2.0 * M_PI; i += 0.07)
            glVertex2f(CIRCLE_SIZE * SOURCE_UNIT * cos(i), CIRCLE_SIZE * SOURCE_UNIT * sin(i));
        glEnd();
        glPopMatrix();
    glEndList();

    // free quadric object
    gluDeleteQuadric(quadObj);

    return id;
}

GLuint ViewRenderWidget::buildLayerbgList()
{
    GLuint id = glGenLists(1);

    glNewList(id, GL_COMPILE);

        glColor4ub(COLOR_DRAWINGS, 255);
        glLineWidth(0.7);
        glBegin(GL_LINES);
        for (float i = -4.0; i < 6.0; i += CLAMP( ABS(i)/2.f , 0.01, 5.0))
        {
            glVertex3f(i - 1.3, -1.1 + exp(-10 * (i + 0.2)), MIN_DEPTH_LAYER);
            glVertex3f(i - 1.3, -1.1 + exp(-10 * (i + 0.2)), MAX_DEPTH_LAYER);
        }
        glEnd();

    glEndList();

    return id;
}

/**
 * Build a display list of a black QUAD and returns its id
 **/
GLuint ViewRenderWidget::buildWindowList(GLubyte r, GLubyte g, GLubyte b)
{
    static GLuint texid = 0;

    if (texid == 0) {
        // generate the texture with optimal performance ;
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        QImage p(":/glmixer/textures/shadow.png");
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COMPRESSED_RGBA, p.width(), p. height(), GL_RGBA, GL_UNSIGNED_BYTE, p.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLclampf highpriority = 1.0;
        glPrioritizeTextures(1, &texid, &highpriority);
    }

    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texid); // 2d texture (x and y size)

        glPushMatrix();
        glTranslatef(0.02 * SOURCE_UNIT, -0.1 * SOURCE_UNIT, 0.1);
        glScalef(1.4 * SOURCE_UNIT, 1.4 * SOURCE_UNIT, 1.0);
        glCallList(vertex_array_coords);
        glDrawArrays(GL_QUADS, 0, 4);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

        glColor4ub(r, g, b, 255);
        glPushMatrix();
        glScalef(1.0 * SOURCE_UNIT, 1.0 * SOURCE_UNIT, 1.0);
        glDrawArrays(GL_QUADS, 0, 4);
        glPopMatrix();

    glEndList();
    return id;
}

/**
 * Build a display list of the front line border of the render area and returns its id
 **/
GLuint ViewRenderWidget::buildFrameList()
{
    static GLuint texid = 0;

    if (texid == 0) {
        // generate the texture with optimal performance ;
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        QImage p(":/glmixer/textures/shadow_corner_selected.png");
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COMPRESSED_RGBA, p.width(), p. height(), GL_RGBA, GL_UNSIGNED_BYTE, p.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLclampf highpriority = 1.0;
        glPrioritizeTextures(1, &texid, &highpriority);
    }

    GLuint base = glGenLists(4);
    glListBase(base);

    // default
    glNewList(base, GL_COMPILE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(4.0);
    glColor4ub(COLOR_FRAME, 250);

    glBegin(GL_LINE_LOOP); // begin drawing the frame (with marks on axis)
        glVertex2f(-1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Left
        glVertex2f(0.0f, -1.00001f * SOURCE_UNIT);
        glVertex2f(0.0f, -1.07f * SOURCE_UNIT);
        glVertex2f(0.0f, -1.00001f * SOURCE_UNIT);
        glVertex2f(1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Right
        glVertex2f(1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.05f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Right
        glVertex2f(0.0f, 1.00001f * SOURCE_UNIT);
        glVertex2f(0.0f, 1.07f * SOURCE_UNIT);
        glVertex2f(0.0f, 1.00001f * SOURCE_UNIT);
        glVertex2f(-1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Left
        glVertex2f(-1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(-1.05f * SOURCE_UNIT, 0.0f);
        glVertex2f(-1.00001f * SOURCE_UNIT, 0.0f);
    glEnd();

    glPointSize(3);
    glBegin(GL_POINTS);  // draw the corners to make them nice
        glVertex2f(-1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Left
        glVertex2f(1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Right
        glVertex2f(1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Right
        glVertex2f(-1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Left
    glEnd();

    glEndList();


    // move frame
    glNewList(base + 1, GL_COMPILE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(4.0);
    glColor4ub(COLOR_FRAME_MOVE, 250);

    glBegin(GL_LINE_LOOP); // begin drawing the frame (with marks on axis)
        glVertex2f(-1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Left
        glVertex2f(0.0f, -1.00001f * SOURCE_UNIT);
        glVertex2f(0.0f, -1.07f * SOURCE_UNIT);
        glVertex2f(0.0f, -1.00001f * SOURCE_UNIT);
        glVertex2f(1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Right
        glVertex2f(1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.05f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Right
        glVertex2f(0.0f, 1.00001f * SOURCE_UNIT);
        glVertex2f(0.0f, 1.07f * SOURCE_UNIT);
        glVertex2f(0.0f, 1.00001f * SOURCE_UNIT);
        glVertex2f(-1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Left
        glVertex2f(-1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(-1.05f * SOURCE_UNIT, 0.0f);
        glVertex2f(-1.00001f * SOURCE_UNIT, 0.0f);
    glEnd();

    glPointSize(3);
    glBegin(GL_POINTS); // draw the corners to make them nice
        glVertex2f(-1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Left
        glVertex2f(1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Right
        glVertex2f(1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Right
        glVertex2f(-1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Left
    glEnd();

    glEndList();

    // thin
    glNewList(base + 2, GL_COMPILE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(1.0);
    glColor4ub(COLOR_FRAME, 250);

    glBegin(GL_LINE_LOOP); // begin drawing the frame (with marks on axis)
        glVertex2f(-1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Left
        glVertex2f(0.0f, -1.00001f * SOURCE_UNIT);
        glVertex2f(0.0f, -1.07f * SOURCE_UNIT);
        glVertex2f(0.0f, -1.00001f * SOURCE_UNIT);
        glVertex2f(1.00001f * SOURCE_UNIT, -1.00001f * SOURCE_UNIT); // Bottom Right
        glVertex2f(1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.05f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Right
        glVertex2f(0.0f, 1.00001f * SOURCE_UNIT);
        glVertex2f(0.0f, 1.07f * SOURCE_UNIT);
        glVertex2f(0.0f, 1.00001f * SOURCE_UNIT);
        glVertex2f(-1.00001f * SOURCE_UNIT, 1.00001f * SOURCE_UNIT); // Top Left
        glVertex2f(-1.00001f * SOURCE_UNIT, 0.0f);
        glVertex2f(-1.05f * SOURCE_UNIT, 0.0f);
        glVertex2f(-1.00001f * SOURCE_UNIT, 0.0f);
    glEnd();

    glEndList();

    // default thickness
    glNewList(base + 3, GL_COMPILE);

    // blended antialiasing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // draw a mask around the window frame
    glColor4ub(COLOR_BGROUND, 255);

    // add shadow
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texid); // 2d texture (x and y size)
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBegin(GL_TRIANGLE_STRIP); // begin drawing the frame as triangle strip

        glTexCoord2f(0.09, 0.09);
        glVertex2f(-1.0f * SOURCE_UNIT, 1.0f * SOURCE_UNIT); // Top Left
        glTexCoord2f(-5.0f, 5.0f);
        glVertex2f(10.0f * SOURCE_UNIT, 10.0f * SOURCE_UNIT);
        glTexCoord2f(0.09, 0.91f);
        glVertex2f(1.0f * SOURCE_UNIT, 1.0f * SOURCE_UNIT); // Top Right
        glTexCoord2f(5.0f, 5.0f);
        glVertex2f(10.0f * SOURCE_UNIT, -10.0f * SOURCE_UNIT);
        glTexCoord2f(0.91f, 0.91f);
        glVertex2f(1.0f * SOURCE_UNIT, -1.0f * SOURCE_UNIT); // Bottom Right
        glTexCoord2f(5.0f, -5.0f);
        glVertex2f(-10.0f * SOURCE_UNIT, -10.0f * SOURCE_UNIT);
        glTexCoord2f(0.91f, 0.09);
        glVertex2f(-1.0f * SOURCE_UNIT, -1.0f * SOURCE_UNIT); // Bottom Left
        glTexCoord2f(-5.0f, -5.0f);
        glVertex2f(-10.0f * SOURCE_UNIT, 10.0f * SOURCE_UNIT);
        glTexCoord2f(0.09, 0.09);
        glVertex2f(-1.0f * SOURCE_UNIT, 1.0f * SOURCE_UNIT); // Top Left
        glTexCoord2f(-5.0f, 5.0f);
        glVertex2f(10.0f * SOURCE_UNIT, 10.0f * SOURCE_UNIT);

    glEnd();

    // back to normal texture mode
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_2D);

    // draw a thin border
    glLineWidth(1.0);
    glColor4ub(COLOR_FRAME, 255);
    glBegin(GL_LINE_LOOP); // begin drawing the frame (with marks on axis)
        glVertex2f(-1.0f * SOURCE_UNIT, -1.0f * SOURCE_UNIT); // Bottom Left
        glVertex2f(1.0f * SOURCE_UNIT, -1.0f * SOURCE_UNIT); // Bottom Right
        glVertex2f(1.0f * SOURCE_UNIT, 1.0f * SOURCE_UNIT); // Top Right
        glVertex2f(-1.0f * SOURCE_UNIT, 1.0f * SOURCE_UNIT); // Top Left
    glEnd();

    glEndList();


    return base;
}

/**
 * Build 3 display lists for the line borders of sources and returns the base id
 **/
GLuint ViewRenderWidget::buildBordersList()
{
    GLuint base = glGenLists(12);
    glListBase(base);

    // default thin border
    glNewList(base, GL_COMPILE);
    glLineWidth(1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glCallList(vertex_array_coords);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glEndList();

    // selected large border (no action)
    glNewList(base + 1, GL_COMPILE);
    glLineWidth(3.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glCallList(vertex_array_coords);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glEndList();

    // selected for TOOL
    glNewList(base + 2, GL_COMPILE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(3.0);
    glCallList(vertex_array_coords);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glLineWidth(1.0);
    glBegin(GL_LINES); // begin drawing handles
    // Bottom Left
    glVertex2f(-BORDER_SIZE, -1.0f);
    glVertex2f(-BORDER_SIZE, -BORDER_SIZE);
    glVertex2f(-BORDER_SIZE, -BORDER_SIZE);
    glVertex2f(-1.0f, -BORDER_SIZE);
    // Bottom Right
    glVertex2f(1.0f, -BORDER_SIZE);
    glVertex2f(BORDER_SIZE, -BORDER_SIZE);
    glVertex2f(BORDER_SIZE, -BORDER_SIZE);
    glVertex2f(BORDER_SIZE, -1.0f);
    // Top Right
    glVertex2f(BORDER_SIZE, 1.0f);
    glVertex2f(BORDER_SIZE, BORDER_SIZE);
    glVertex2f(BORDER_SIZE, BORDER_SIZE);
    glVertex2f(1.0f, BORDER_SIZE);
    // Top Left
    glVertex2f(-BORDER_SIZE, 1.0f);
    glVertex2f(-BORDER_SIZE, BORDER_SIZE);
    glVertex2f(-BORDER_SIZE, BORDER_SIZE);
    glVertex2f(-1.0f, BORDER_SIZE);
    glEnd();
    glEndList();

    // Normal source color
    glNewList(base + 3, GL_COMPILE);
//    glColor4ub(COLOR_SOURCE, 180);
    glCallList(base);
    glEndList();

    glNewList(base + 4, GL_COMPILE);
//    glColor4ub(COLOR_SOURCE, 200);
    glCallList(base+1);
    glEndList();

    glNewList(base + 5, GL_COMPILE);
//    glColor4ub(COLOR_SOURCE, 220);
    glCallList(base+2);
    glEndList();


    // Static source
    glNewList(base + 6, GL_COMPILE);
    glLineWidth(1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glCallList(vertex_array_coords);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glEndList();

    glNewList(base + 7, GL_COMPILE);
//    glColor4ub(COLOR_SOURCE_STATIC, 200);
//    glLineStipple(1, 0x7777);
//    glEnable(GL_LINE_STIPPLE);
    glCallList(base+1);
//    glDisable(GL_LINE_STIPPLE);
//    glPointSize(8.0);
////    glColor4ub(COLOR_SOURCE_STATIC, 180);
//    glCallList(vertex_array_coords);
//    glPushMatrix();
//    glScalef(0.8, 0.8, 1.0);
//    glDrawArrays(GL_POINTS, 0, 4);
//    glPopMatrix();
    glEndList();

    glNewList(base + 8, GL_COMPILE);
//    glColor4ub(COLOR_SOURCE_STATIC, 220);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glCallList(base+1);
    glDisable(GL_LINE_STIPPLE);
//    glPointSize(8.0);
////    glColor4ub(COLOR_SOURCE_STATIC, 180);
//    glCallList(vertex_array_coords);
//    glPushMatrix();
//    glScalef(0.8, 0.8, 1.0);
//    glDrawArrays(GL_POINTS, 0, 4);
//    glPopMatrix();
    glEndList();


    // Selection source color
    glNewList(base + 9, GL_COMPILE);
    glColor4ub(COLOR_SELECTION , 180);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glScalef(1.01, 1.01, 1.01);
    glCallList(base);
    glDisable(GL_LINE_STIPPLE);
    glEndList();

    glNewList(base + 10, GL_COMPILE);
    glColor4ub(COLOR_SELECTION, 200);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glScalef(1.01, 1.01, 1.01);
    glCallList(base+1);
    glDisable(GL_LINE_STIPPLE);
    glEndList();

    glNewList(base + 11, GL_COMPILE);
    glColor4ub(COLOR_SELECTION, 220);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glScalef(1.01, 1.01, 1.01);
    glCallList(base+2);
    glDisable(GL_LINE_STIPPLE);
    glEndList();

    return base + 3;
}


GLuint ViewRenderWidget::buildBordersTools()
{
    GLuint base = glGenLists(3);
    glListBase(base);

    // rotation center
    glNewList(base, GL_COMPILE);
    glLineWidth(1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);
    for (float i = 0; i < 2.0 * M_PI; i += 0.2)
        glVertex2f(CENTER_SIZE * cos(i), CENTER_SIZE * sin(i));
    glEnd();
    glBegin(GL_LINES); // begin drawing a cross
    glVertex2f(0.f, CENTER_SIZE);
    glVertex2f(0.f, -CENTER_SIZE);
    glVertex2f(CENTER_SIZE, 0.f);
    glVertex2f(-CENTER_SIZE, 0.f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    glEndList();

    // Proportionnal scaling
    glNewList(base + 1, GL_COMPILE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth(1.0);
    glBegin(GL_LINES); // begin drawing handles
    // Bottom Left
    glVertex2f(-1.f, -1.f);
    glVertex2f(-BORDER_SIZE, -BORDER_SIZE);
    // Bottom Right
    glVertex2f(1.f, -1.f);
    glVertex2f(BORDER_SIZE, -BORDER_SIZE);
    // Top Right
    glVertex2f(1.f, 1.f);
    glVertex2f(BORDER_SIZE, BORDER_SIZE);
    // Top Left
    glVertex2f(-1.f, 1.f);
    glVertex2f(-BORDER_SIZE, BORDER_SIZE);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    glEndList();

    // Crop
    glNewList(base + 2, GL_COMPILE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineStipple(1, 0x7777);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth(1.0);
    glBegin(GL_LINES); // begin drawing handles
    // Bottom Left
    glVertex2f(-BORDER_SIZE*2.f, -1.f);
    glVertex2f(-1.f, -1.f);
    glVertex2f(-1.f, -1.f);
    glVertex2f(-1.0f, -BORDER_SIZE*2.f);
    // Bottom Right
    glVertex2f(1.0f, -BORDER_SIZE*2.f);
    glVertex2f(1.f, -1.f);
    glVertex2f(1.f, -1.f);
    glVertex2f(BORDER_SIZE*2.f, -1.0f);
    // Top Right
    glVertex2f(BORDER_SIZE*2.f, 1.0f);
    glVertex2f(1.f, 1.f);
    glVertex2f(1.f, 1.f);
    glVertex2f(1.0f, BORDER_SIZE*2.f);
    // Top Left
    glVertex2f(-BORDER_SIZE*2.f, 1.0f);
    glVertex2f(-1.f, 1.f);
    glVertex2f(-1.f, 1.f);
    glVertex2f(-1.0f, BORDER_SIZE*2.f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    glEndList();

    return base;
}

GLuint ViewRenderWidget::buildFadingList()
{
    static GLuint texid = 0;

    if (texid == 0) {
        // generate the texture
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        QImage p(":/glmixer/images/loading.png");
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COMPRESSED_RGBA, p.width(), p. height(), GL_RGBA, GL_UNSIGNED_BYTE, p.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    GLuint id = glGenLists(4);

    // simple overlay for fading the view
    glNewList(id, GL_COMPILE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glRectf(-1, -1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();

    glEndList();

    // overlay for fading the view and
    // displaying the loading animation

    // PART 1
    // as simple fading, just more opaque
    glNewList(id + 1, GL_COMPILE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glColor4ub(COLOR_FADING, 160);
    glRectf(-1, -1, 1, 1);

    glEndList();

    // PART 2
    // draw the busy loading overlay
    glNewList(id + 2, GL_COMPILE);

    glScalef(0.3, 0.3, 0.3);
    glColor4ub(255, 255, 255, 220);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texid);
    glCallList(quad_texured);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEndList();

    return id;
}


/**
 * Build a display list of a black QUAD and returns its id
 **/
GLuint ViewRenderWidget::buildSnapshotList()
{
    static GLuint texid = 0;

    if (texid == 0) {
        // generate the texture with optimal performance ;
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        QImage p(":/glmixer/textures/mask_linear_bottom.png");
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COMPRESSED_RGBA, p.width(), p. height(), GL_RGBA, GL_UNSIGNED_BYTE, p.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLclampf highpriority = 1.0;
        glPrioritizeTextures(1, &texid, &highpriority);
    }

    GLuint id = glGenLists(4);

    glNewList(id, GL_COMPILE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glCallList(vertex_array_coords);

        // uniform background
        glDisable(GL_TEXTURE_2D);
        glDrawArrays(GL_QUADS, 0, 4);

        // vertical gradient
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texid); // 2d texture (x and y size)
        glColor4ub(255, 255, 255, 220);
        glDrawArrays(GL_QUADS, 0, 4);

        glDisable(GL_TEXTURE_2D);

    glEndList();

    glNewList(id + 1, GL_COMPILE);

        glDisable(GL_TEXTURE_2D);
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex3d(-1.0, -1.0, 0.0);
        glVertex3d(1.0, -1.0, 0.0);
        glEnd();

    glEndList();

    glNewList(id + 2, GL_COMPILE);

        glBindTexture(GL_TEXTURE_2D, white_texture);
        glLineWidth(2.0);
        glPushMatrix();
        glScalef(1.1, 1.1, 1.0);
        glBegin(GL_LINE_LOOP);
        for (float i = 0; i < 2.0 * M_PI; i += 0.251)
            glVertex2d( cos(i), sin(i));
        glEnd();
        glPopMatrix();

    glEndList();

    glNewList(id + 3, GL_COMPILE);

        glBindTexture(GL_TEXTURE_2D, white_texture);
        glLineWidth(3.0);
        glPushMatrix();
        glScalef(1.13, 1.13, 1.0);
        glBegin(GL_LINE_LOOP);
        for (float i = 0; i < 2.0 * M_PI; i += 0.251)
            glVertex2d( cos(i), sin(i));
        glEnd();
        glPopMatrix();

    glEndList();

    return id;
}

const char * rotate_top_right[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ",
        "                         ",
        "           +             ",
        "          +.+            ",
        "          +..+           ",
        "        +++...+  ++      ",
        "      ++.......++..+     ",
        "     +.........++..+     ",
        "    +.....+...+  ++      ",
        "    +...+++..+   ++      ",
        "   +...+  +.+   +..+     ",
        "   +...+   +   +....+    ",
        "  +...+       +......+   ",
        "  +...+      +........+  ",
        "  +...+       +++..+++   ",
        "   +...+       +...+     ",
        "   +...+       +...+     ",
        "    +...++   ++...+      ",
        "    +.....+++.....+      ",
        "     +...........+       ",
        "      ++.......++        ",
        "        ++...++          ",
        "          +++            ",
        "                         ",
        "                         "};


const char * rotate_top_left[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ",
        "                         ",
        "             +           ",
        "            +.+          ",
        "           +..+          ",
        "      ++  +...+++        ",
        "     +..++.......++      ",
        "     +..++.........+     ",
        "      ++  +...+.....+    ",
        "      ++   +..+++...+    ",
        "     +..+   +.+  +...+   ",
        "    +....+   +   +...+   ",
        "   +......+       +...+  ",
        "  +........+      +...+  ",
        "   +++..+++       +...+  ",
        "     +...+       +...+   ",
        "     +...+       +...+   ",
        "      +...++   ++...+    ",
        "      +.....+++.....+    ",
        "       +...........+     ",
        "        ++.......++      ",
        "          ++...++        ",
        "            +++          ",
        "                         ",
        "                         "};

const char * rotate_bot_left[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ",
        "                         ",
        "            +++          ",
        "          ++...++        ",
        "        ++.......++      ",
        "       +...........+     ",
        "      +.....+++.....+    ",
        "      +...++   ++...+    ",
        "     +...+       +...+   ",
        "     +...+       +...+   ",
        "   +++..+++       +...+  ",
        "  +........+      +...+  ",
        "   +......+       +...+  ",
        "    +....+   +   +...+   ",
        "     +..+   +.+  +...+   ",
        "      ++   +..+++...+    ",
        "      ++  +...+.....+    ",
        "     +..++.........+     ",
        "     +..++.......++      ",
        "      ++  +...+++        ",
        "           +..+          ",
        "            +.+          ",
        "             +           ",
        "                         ",
        "                         "};

const char * rotate_bot_right[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ",
        "                         ",
        "          +++            ",
        "        ++...++          ",
        "      ++.......++        ",
        "     +...........+       ",
        "    +.....+++.....+      ",
        "    +...++   ++...+      ",
        "   +...+       +...+     ",
        "   +...+       +...+     ",
        "  +...+       +++..+++   ",
        "  +...+      +........+  ",
        "  +...+       +......+   ",
        "   +...+   +   +....+    ",
        "   +...+  +.+   +..+     ",
        "    +...+++..+   ++      ",
        "    +.....+...+  ++      ",
        "     +.........++..+     ",
        "      ++.......++..+     ",
        "        +++...+  ++      ",
        "          +..+           ",
        "          +.+            ",
        "           +             ",
        "                         ",
        "                         "};


const char * cursor_arrow_xpm[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ", "                         ",
        "       +                 ", "       ++                ",
        "       +.+               ", "       +..+              ",
        "       +...+             ", "       +....+            ",
        "       +.....+           ", "       +......+          ",
        "       +.......+         ", "       +........+        ",
        "       +.........+       ", "       +......+++++      ",
        "       +...+..+          ", "       +..++..+          ",
        "       +.+  +..+         ", "       ++   +..+         ",
        "       +     +..+        ", "             +..+        ",
        "              +..+       ", "              +..+       ",
        "               ++        ", "                         ",
        "                         " };

const char * cursor_openhand_xpm[] =
{ "16 16 3 1", " 	g None", ".	g #000000", "+	g #EEEEEE", "       ..       ",
        "   .. .++...    ", "  .++..++.++.   ", "  .++..++.++. . ",
        "   .++.++.++..+.", "   .++.++.++.++.", " .. .+++++++.++.",
        ".++..++++++++++.", ".+++.+++++++++. ", " .++++++++++++. ",
        "  .+++++++++++. ", "  .++++++++++.  ", "   .+++++++++.  ",
        "    .+++++++.   ", "     .++++++.   ", "                " };

const char * cursor_closedhand_xpm[] =
{ "16 16 3 1", " 	g None", ".	g #000000", "+	g #EEEEEE", "                ",
        "                ", "                ", "    .. .. ..    ",
        "   .++.++.++..  ", "   .++++++++.+. ", "    .+++++++++. ",
        "   ..+++++++++. ", "  .+++++++++++. ", "  .++++++++++.  ",
        "   .+++++++++.  ", "    .+++++++.   ", "     .++++++.   ",
        "     .++++++.   ", "                ", "                " };

const char * cursor_sizef_xpm[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ", "                         ",
        "                         ", "                         ",
        "    +++++++++            ", "    +.......+            ",
        "    +......+             ", "    +.....+              ",
        "    +.....+              ", "    +......+             ",
        "    +..++...+            ", "    +.+  +...+           ",
        "    ++    +...+    ++    ", "           +...+  +.+    ",
        "            +...++..+    ", "             +......+    ",
        "              +.....+    ", "              +.....+    ",
        "             +......+    ", "            +.......+    ",
        "            +++++++++    ", "                         ",
        "                         ", "                         ",
        "                         " };

const char * cursor_sizeb_xpm[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ", "                         ",
        "                         ", "                         ",
        "            +++++++++    ", "            +.......+    ",
        "             +......+    ", "              +.....+    ",
        "              +.....+    ", "             +......+    ",
        "            +...++..+    ", "           +...+  +.+    ",
        "    ++    +...+    ++    ", "    +.+  +...+           ",
        "    +..++...+            ", "    +......+             ",
        "    +.....+              ", "    +.....+              ",
        "    +......+             ", "    +.......+            ",
        "    +++++++++            ", "                         ",
        "                         ", "                         ",
        "                         " };

const char * cursor_question_xpm[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "+                        ", "++          .......      ",
        "+.+        .+++++++.     ", "+..+      .++....+++.    ",
        "+...+    .+++.  .+++.    ", "+....+   .+++.  .+++.    ",
        "+.....+  .+++.  .+++.    ", "+......+ .+++.  .+++.    ",
        "+.......+ ...  .+++.     ", "+........+    .+++.      ",
        "+.....+++++  .+++.       ", "+..+..+      .+++.       ",
        "+.+ +..+     .+++.       ", "++  +..+     .+++.       ",
        "+    +..+    .....       ", "     +..+    .+++.       ",
        "      +..+   .+++.       ", "      +..+   .....       ",
        "       ++                ", "                         ",
        "                         ", "                         ",
        "                         ", "                         ",
        "                         " };

const char * cursor_sizeall_xpm[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "                         ", "           ++            ",
        "          +..+           ", "         +....+          ",
        "        +......+         ", "       +........+        ",
        "        +++..+++         ", "     +    +..+    +      ",
        "    +.+   +..+   +.+     ", "   +..+   +..+   +..+    ",
        "  +...+++++..+++++...+   ", " +....................+  ",
        " +....................+  ", "  +...+++++..+++++...+   ",
        "   +..+   +..+   +..+    ", "    +.+   +..+   +.+     ",
        "     +    +..+    +      ", "        +++..+++         ",
        "       +........+        ", "        +......+         ",
        "         +....+          ", "          +..+           ",
        "           ++            ", "                         ",
        "                         " };

const char * cursor_hand_xpm[] =
{ "25 25 3 1", " 	c None", ".	c #EEEEEE", "+	c #000000",
        "         ..              ", "        .++.             ",
        "        +..+             ", "        +..+             ",
        "        +..+             ", "        +..+             ",
        "        +..+             ", "        +..+++           ",
        "        +..+..+++        ", "        +..+..+..++      ",
        "     ++ +..+..+..+.+     ", "    +..++..+..+..+.+     ",
        "    +...+..........+     ", "     +.............+     ",
        "      +............+     ", "      +............+     ",
        "       +..........+      ", "       +..........+      ",
        "        +........+       ", "        +........+       ",
        "        ++++++++++       ", "        ++++++++++       ",
        "        ++++++++++       ", "                         ",
        "                         " };

void ViewRenderWidget::setMouseCursor(mouseCursor c)
{
    // create QCursors for each pixmap
    static QCursor arrowCursor = QCursor(QPixmap(cursor_arrow_xpm), 8, 3);
    static QCursor handOpenCursor = QCursor(QPixmap(cursor_openhand_xpm), 8, 8);
    static QCursor handCloseCursor = QCursor(QPixmap(cursor_closedhand_xpm), 8, 8);
    static QCursor scaleBCursor = QCursor(QPixmap(cursor_sizeb_xpm), 12, 12);
    static QCursor scaleFCursor = QCursor(QPixmap(cursor_sizef_xpm), 12, 12);
    static QCursor rotTopRightCursor = QCursor(QPixmap(rotate_top_right), 12, 12);
    static QCursor rotTopLeftCursor = QCursor(QPixmap(rotate_top_left), 12, 12);
    static QCursor rotBottomRightCursor = QCursor(QPixmap(rotate_bot_right), 12, 12);
    static QCursor rotBottomLeftCursor = QCursor(QPixmap(rotate_bot_left), 12, 12);
    static QCursor questionCursor = QCursor(QPixmap(cursor_question_xpm), 1, 1);
    static QCursor sizeallCursor = QCursor(QPixmap(cursor_sizeall_xpm), 12, 12);
    static QCursor handIndexCursor = QCursor(QPixmap(cursor_hand_xpm), 10, 1);

    switch (c)
    {
    case MOUSE_HAND_OPEN:
        setCursor(handOpenCursor);
        break;
    case MOUSE_HAND_CLOSED:
        setCursor(handCloseCursor);
        break;
    case MOUSE_SCALE_B:
        setCursor(scaleBCursor);
        break;
    case MOUSE_SCALE_F:
        setCursor(scaleFCursor);
        break;
    case MOUSE_ROT_BOTTOM_LEFT:
        setCursor(rotBottomLeftCursor);
        break;
    case MOUSE_ROT_BOTTOM_RIGHT:
        setCursor(rotBottomRightCursor);
        break;
    case MOUSE_ROT_TOP_LEFT:
        setCursor(rotTopLeftCursor);
        break;
    case MOUSE_ROT_TOP_RIGHT:
        setCursor(rotTopRightCursor);
        break;
    case MOUSE_QUESTION:
        setCursor(questionCursor);
        break;
    case MOUSE_SIZEALL:
        setCursor(sizeallCursor);
        break;
    case MOUSE_HAND_INDEX:
        setCursor(handIndexCursor);
        break;
    default:
    case MOUSE_ARROW:
        setCursor(arrowCursor);
    }
}

void ViewRenderWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void ViewRenderWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void ViewRenderWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void ViewRenderWidget::dropEvent(QDropEvent *event)
{
#ifdef GLM_SNAPSHOT
    // detect internal drop events
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {

        if (!suspended && !RenderingManager::getInstance()->isPaused()) {
            event->accept();

            QStandardItemModel model;
            if (model.dropMimeData(event->mimeData(), Qt::CopyAction, 0,0, QModelIndex()) ) {
                // read the id of the attached data
                QString snapshotid = model.item(0,0)->data(Qt::UserRole).toString();
                // activate the snapshot view
                activateSnapshot(snapshotid);
                // unset current source
                RenderingManager::getInstance()->unsetCurrentSource();
                // set focus
                setFocus(Qt::OtherFocusReason);
            }
        }
        else {
            showMessage(tr("Cannot interpolate between snapshots when paused."), 2000);
            event->ignore();
        }
    }
    else
#endif
        // external drop events
        GLMixer::getInstance()->drop(event);
}

bool ViewRenderWidget::event(QEvent *event)
{
    // event() is called at every event, even DRAW event
    // DO NOT perform computations here to ensure smooth rendering
    // Instead, emit signals to stack events for later

    // handling of gesture events
    if (event->type() == QEvent::Gesture) {
        emit gestureEvent(static_cast<QGestureEvent*>(event));
        event->accept();
        return true;
    }
    // handling of special keyboard events
    else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> (event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Escape)
        {
            emit specialKeyboardEvent(keyEvent);
            keyEvent->accept();
            return true;
        }
    }

    return QGLWidget::event(event);
}

void ViewRenderWidget::onGestureEvent(QGestureEvent *event)
{
    // only deal with Pinch Gesture (2 fingers in or out)
    if (QGesture *g = event->gesture(Qt::PinchGesture)) {
        QPinchGesture *pinch = static_cast<QPinchGesture *>(g);
        // only consider scaling for pinch
        if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
            // zoom in or out (scale factor = 1.0 for no change)
            _currentView->setZoom( _currentView->getZoom() * pinch->scaleFactor() );
            showZoom(QString("%1 \%").arg(_currentView->getZoomPercent(), 0, 'f', 1));
            emit zoomPercentChanged((int) _currentView->getZoomPercent());
            // reset pinch scale; we only need delta of gesture
            pinch->setScaleFactor( 1.0 );
        }
    }
}

void ViewRenderWidget::onSpecialKeyboardEvent(QKeyEvent *event)
{
    // Tab key switches to the next source, CTRl-Tab the previous (ALT-Tab under OSX).
    if (event->key() == Qt::Key_Tab)
    {
#ifdef Q_OS_MAC
        if (event->modifiers() & Qt::AltModifier)
#else
        if (event->modifiers() & Qt::ControlModifier)
#endif
            RenderingManager::getInstance()->setCurrentPrevious();
        else
            RenderingManager::getInstance()->setCurrentNext();
    }
    // Esc to cancel selection
    else if (event->key() == Qt::Key_Escape)
    {
        RenderingManager::getInstance()->unsetCurrentSource();
#ifdef GLM_SNAPSHOT
        _snapshotView->deactivate();
#endif
    }
}
