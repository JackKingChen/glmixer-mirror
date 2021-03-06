/*
 * MixerView.cpp
 *
 *  Created on: Nov 9, 2009
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

#include "MixerView.h"

#include "Tag.h"
#include "common.h"
#include "RenderingManager.h"
#include "SelectionManager.h"
#include "ViewRenderWidget.h"
#include "WorkspaceManager.h"
#ifdef GLM_SNAPSHOT
#include "SnapshotManager.h"
#endif

#define MINZOOM 0.04
#define MAXZOOM 1.0
#define DEFAULTZOOM 0.075
#define DEFAULT_PANNING 0.f, 0.f
#define MIXING_EPSILON 0.1


bool MixerSelectionArea::contains(SourceSet::iterator s)
{
    return area.contains(QPointF((*s)->getAlphaX(),(*s)->getAlphaY()));
}

MixerView::MixerView() : View(View::MIXING)
{
    currentAction = View::NONE;
    zoom = DEFAULTZOOM;
    minzoom = MINZOOM;
    maxzoom = MAXZOOM;
    maxpanx = 2.0*SOURCE_UNIT*MAXZOOM*CIRCLE_SIZE;
    maxpany = 2.0*SOURCE_UNIT*MAXZOOM*CIRCLE_SIZE;
    limboSize = DEFAULT_LIMBO_SIZE;
    _specialMode = MODE_NONE;

    icon.load(QString::fromUtf8(":/glmixer/icons/mixer.png"));
    title = " Mixing";
}

void MixerView::setModelview()
{
    View::setModelview();
    glScaled(zoom, zoom, zoom);
    glTranslated(getPanningX(), getPanningY(), 0.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
}

typedef std::map<double, Source*> mixingSourceMap;

mixingSourceMap getMixingSourceMap(SourceList::iterator begin, SourceList::iterator end)
{
    mixingSourceMap map;
    if ( begin == end)
        return map;

    SourceList::iterator sit = begin;
    int size = 1;
    double cx = (*sit)->getAlphaX();
    double cy = (*sit)->getAlphaY();
    for (sit++; sit != end; sit++, ++size) {
        cx += (*sit)->getAlphaX();
        cy += (*sit)->getAlphaY();
    }
    cx /= size;
    cy /= size;

    for (sit = begin; sit != end; sit++) {
        if ( !(*sit) )
            break;
        double sx = (*sit)->getAlphaX();
        double sy = (*sit)->getAlphaY();
        double sd = sqrt( (sx-cx)*(sx-cx) + (sy-cy)*(sy-cy) );
        double angle = atan2(((sy-cy)/sd), ((sx-cx)/sd)) ;
        //  map sorted by angle
        map[angle] = (*sit);
    }

    return map;
}

void MixerView::paint()
{
    static double renderingAspectRatio = 1.0;
    static double ax, ay;

    // First the background stuff
    glCallList(ViewRenderWidget::circle_mixing + 2);


    glCallList(ViewRenderWidget::circle_mixing);
    if (_specialMode == MODE_MOVE_CIRCLE)
        glCallList(ViewRenderWidget::circle_mixing + 1);

    glPushMatrix();
    glScaled( limboSize,  limboSize, 1.0);
    glCallList(ViewRenderWidget::circle_limbo);
    if (_specialMode == MODE_SCALE_LIMBO)
        glCallList(ViewRenderWidget::circle_limbo + 1);
    glPopMatrix();


    // and the selection connection lines
    if (SelectionManager::getInstance()->hasSelection())
    {
        // draw the selection lines
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x00FC);
        glLineWidth(2.0);
        glColor4ub(COLOR_SELECTION, 255);
        glBegin(GL_LINES);

        // loop over a list of sources sorted by angle around the center of selection
        mixingSourceMap selectionMap = getMixingSourceMap(SelectionManager::getInstance()->selectionBegin(), SelectionManager::getInstance()->selectionEnd());
        for(mixingSourceMap::iterator  its2, its1 = selectionMap.begin(); its1 != selectionMap.end(); its1++) {
            its2 = its1;
            its2++;
            if (its2 == selectionMap.end()) {
                if (selectionMap.size() > 2)
                    its2 = selectionMap.begin();
                else
                    break;
            }
            glVertex3d(its1->second->getAlphaX(), its1->second->getAlphaY(), 0.0);
            glVertex3d(its2->second->getAlphaX(), its2->second->getAlphaY(), 0.0);
        }
        glEnd();
        glDisable(GL_LINE_STIPPLE);

    }


    // pre render draw (clear and prepare)
    RenderingManager::getInstance()->preRenderToFrameBuffer();

    // set mode for source
    ViewRenderWidget::setSourceDrawingMode(true);

    // The icons of the sources (reversed depth order)
    for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++) {

        // prevent obvious problem
        Source *s = *its;
        if (!s)
            continue;

        // test if the source is passed the standby line
        ax = s->getAlphaX();
        ay = s->getAlphaY();
        s->setStandby( isInLimbo(s) );

        //
        // 0. prepare texture
        //

        // bind the source textures
        s->bind();
        s->setShaderAttributes();

        //
        // 1. Draw it into render FBO
        //
        if (!s->isStandby())
        {
            RenderingManager::getInstance()->sourceRenderToFrameBuffer(s);
        }

        //
        // 2. Draw it into current view
        //
        glPushMatrix();

        glTranslated(ax, ay, s->getDepth());
        renderingAspectRatio = s->getAspectRatio();
        if ( ABS(renderingAspectRatio) > 1.0)
            glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT, ViewRenderWidget::iconSize * SOURCE_UNIT / renderingAspectRatio,  1.0);
        else
            glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT * renderingAspectRatio, ViewRenderWidget::iconSize * SOURCE_UNIT,  1.0);

        // standard transparency blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);


        // Normal draw in current workspace
        if ( WorkspaceManager::getInstance()->isInCurrent(s) ) {

            if (!s->isStandby())  {
                //   draw stippled version of the source
                ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_stippling, (GLfloat) ViewRenderWidget::getStipplingMode() / 100.f);
            }
            else {
                // draw flat version of the source
                ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_baseAlpha, 1.f);
                ViewRenderWidget::program->setUniformValue(ViewRenderWidget::_fading, 1.f);
            }

            s->draw();

            // switch to drawing mode
            ViewRenderWidget::resetShaderAttributes();

            // draw border (larger if active)
            QColor darker = s->isPlayable() ? Tag::get(s)->getColor().darker() :Tag::get(s)->getColor().darker(150);
            ViewRenderWidget::setBaseColor( s->isStandby() ? darker : Tag::get(s)->getColor());
            if (RenderingManager::getInstance()->isCurrentSource(s))
                glCallList(ViewRenderWidget::border_large_shadow);
            else
                glCallList(ViewRenderWidget::border_thin_shadow);

        }
        // Shadow draw in other workspace
        else if ( !WorkspaceManager::getInstance()->isExclusiveDisplay() ){

            // set shadow color and alpha
            ViewRenderWidget::setBaseColor(s->getColor().darker(WORKSPACE_COLOR_SHIFT), WORKSPACE_MAX_ALPHA);

            // draw texture
            s->draw();

            // switch to drawing mode
            ViewRenderWidget::resetShaderAttributes();

            // draw border (never active)
            ViewRenderWidget::setBaseColor(Tag::get(s)->getColor().darker(WORKSPACE_COLOR_SHIFT), WORKSPACE_MAX_ALPHA);
            glCallList(ViewRenderWidget::border_thin_shadow + 2);

        }


        // done geometry
        glPopMatrix();
    }

    // unset mode for source
    ViewRenderWidget::setSourceDrawingMode(false);

    // post render draw (loop back and recorder)
    RenderingManager::getInstance()->postRenderToFrameBuffer();


    // Then the selection outlines
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
        glPushMatrix();
        glTranslated((*its)->getAlphaX(), (*its)->getAlphaY(), (*its)->getDepth());
        renderingAspectRatio = (*its)->getAspectRatio();
        if ( ABS(renderingAspectRatio) > 1.0)
            glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT, ViewRenderWidget::iconSize * SOURCE_UNIT / renderingAspectRatio,  1.0);
        else
            glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT * renderingAspectRatio, ViewRenderWidget::iconSize * SOURCE_UNIT,  1.0);
        glCallList(ViewRenderWidget::frame_selection);
        glPopMatrix();

    }

    // and the groups connection lines
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xFC00);
    glLineWidth(2.0);
    glPointSize(ViewRenderWidget::iconSize * 10);
    int c = 0;
    for(SourceListArray::iterator itss = groupSources.begin(); itss != groupSources.end(); itss++, c++) {

        // get a reordered map of source
        mixingSourceMap selectionMap = getMixingSourceMap((*itss).begin(), (*itss).end());

        if (selectionMap.empty())
            break;

        // check if the sources are in the current view
        int w = selectionMap.begin()->second->getWorkspace();
        float a = 0.8f;
        if ( WorkspaceManager::getInstance()->isExclusiveDisplay() ) {
            if (WorkspaceManager::getInstance()->current() != w)
                continue;
        }
        a *= (WorkspaceManager::getInstance()->current() == w ? 1.0 : WORKSPACE_MAX_ALPHA);

        // use color of the group
        glColor4f(groupColors[c].redF(), groupColors[c].greenF(),groupColors[c].blueF(), a);

        glBegin(GL_LINES);
        for(mixingSourceMap::iterator  its2, its1 = selectionMap.begin(); its1 != selectionMap.end(); its1++) {
            its2 = its1;
            its2++;
            if (its2 == selectionMap.end()) {
                if (selectionMap.size() > 2)
                    its2 = selectionMap.begin();
                else
                    break;
            }
            glVertex3d(its1->second->getAlphaX(), its1->second->getAlphaY(), 0.0);
            glVertex3d(its2->second->getAlphaX(), its2->second->getAlphaY(), 0.0);
        }
        glEnd();

        // dots to identifiy source in the group
        glBegin(GL_POINTS);
        for(SourceList::iterator  its1 = (*itss).begin(); its1 != (*itss).end(); its1++)
            glVertex3d((*its1)->getAlphaX(), (*its1)->getAlphaY(), 0.0);
        glEnd();
    }

    glDisable(GL_LINE_STIPPLE);

    // show the pivot point
    if (SelectionManager::getInstance()->selectionCount() > 1)
    {
        glPushMatrix();
        glTranslated(SelectionManager::getInstance()->selectionSource()->getAlphaX(), SelectionManager::getInstance()->selectionSource()->getAlphaY(), 0.0);
        glScaled(ViewRenderWidget::iconSize, ViewRenderWidget::iconSize, 1.0);
        glCallList(ViewRenderWidget::center_pivot);
        glPopMatrix();
    }

    // the source dropping icon
    Source *s = RenderingManager::getInstance()->getSourceBasketTop();
    if ( s ){

        glColor4ub(COLOR_SOURCE, 180);
        double ax, ay, az; // mouse cursor in rendering coordinates:
        gluUnProject(double (mousePos.x()), double (viewport[3] - mousePos.y()), 1.0,
                modelview, projection, viewport, &ax, &ay, &az);
        glPushMatrix();
        glTranslated(ax, ay, az);
        glPushMatrix();
        if ( ABS(s->getAspectRatio()) > 1.0)
            glTranslated(SOURCE_UNIT + 1.0, -SOURCE_UNIT / s->getAspectRatio() + 1.0,  0.0);
        else
            glTranslated(SOURCE_UNIT * s->getAspectRatio() + 1.0, -SOURCE_UNIT + 1.0,  0.0);
        for (int i = 1; i < RenderingManager::getInstance()->getSourceBasketSize(); ++i ) {
            glTranslated(2.1, 0.0, 0.0);
            glCallList(ViewRenderWidget::border_thin);
        }
        glPopMatrix();
        renderingAspectRatio = s->getAspectRatio();
        if ( ABS(renderingAspectRatio) > 1.0)
            glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT, ViewRenderWidget::iconSize * SOURCE_UNIT / renderingAspectRatio,  1.0);
        else
            glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT * renderingAspectRatio, ViewRenderWidget::iconSize * SOURCE_UNIT,  1.0);

        glCallList(ViewRenderWidget::border_large_shadow);
        glPopMatrix();
    }

    // The rectangle for selection
    _selectionArea.draw();

}



void MixerView::clear()
{
    View::clear();

    groupSources.clear();
    groupColors.clear();
    limboSize = DEFAULT_LIMBO_SIZE;

}


void MixerView::resize(int w, int h)
{
    View::resize(w, h);
    glViewport(0, 0, viewport[2], viewport[3]);

    // Setup specific projection and view for this window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (viewport[2] > viewport[3])
         glOrtho(-SOURCE_UNIT* (double) viewport[2] / (double) viewport[3], SOURCE_UNIT*(double) viewport[2] / (double) viewport[3], -SOURCE_UNIT, SOURCE_UNIT, -MAX_DEPTH_LAYER, 10.0);
     else
         glOrtho(-SOURCE_UNIT, SOURCE_UNIT, -SOURCE_UNIT*(double) viewport[3] / (double) viewport[2], SOURCE_UNIT*(double) viewport[3] / (double) viewport[2], -MAX_DEPTH_LAYER, 10.0);

    glGetDoublev(GL_PROJECTION_MATRIX, projection);


    // compute largest mixing area for Mixing View (minimum zoom and max panning to both sides)
    double dum;
    double maxmodelview[16];
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScaled(minzoom, minzoom, minzoom);
    glTranslated(maxpanx, maxpany, 0.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, maxmodelview);
    gluUnProject(0.0, 0.0, 0.0, maxmodelview, projection, viewport, _mixingArea, _mixingArea+1, &dum);
    glTranslated(-2.0*maxpanx, -2.0*maxpany, 0.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, maxmodelview);
    gluUnProject((double)viewport[2], (double)viewport[3], 0.0, maxmodelview, projection, viewport, _mixingArea+2, _mixingArea+3, &dum);
    glPopMatrix();

}



void MixerView::setAction(ActionType a){

//    if (a == currentAction)
//        return;

    View::setAction(a);

    switch(a) {
    case View::OVER:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_HAND_OPEN);
        break;
    case View::GRAB:
    case View::TOOL:
        setTool(currentTool);
        break;
    case View::SELECT:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_HAND_INDEX);
        break;
    case View::PANNING:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_SIZEALL);
        break;
    case View::DROP:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_QUESTION);
        break;
    default:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_ARROW);
        break;
    }
}


void MixerView::setTool(toolType t)
{
    currentTool = t;

    switch (t) {
    case SCALE:
        if ( SelectionManager::getInstance()->hasSelection() )
            RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_SIZEALL);
        else
            RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_HAND_CLOSED);
        break;
    case ROTATE:
        if ( SelectionManager::getInstance()->hasSelection() )
            RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_ROT_TOP_LEFT);
        else
            RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_HAND_CLOSED);
        break;
    case MOVE:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_HAND_CLOSED);
        break;
    default:
        RenderingManager::getRenderingWidget()->setMouseCursor(ViewRenderWidget::MOUSE_ARROW);
        break;
    }
}

bool MixerView::mousePressEvent(QMouseEvent *event)
{
    if (!event)
        return false;

    lastClicPos = mousePos = event->pos();

    // remember coordinates of clic in background for selection area
    double cursorx = 0.0, cursory = 0.0, dumm = 0.0;
    gluUnProject((double) event->x(), (double) viewport[3] - event->y(), 0.0, modelview, projection, viewport, &cursorx, &cursory, &dumm);
    _selectionArea.markStart(QPointF(cursorx,cursory));

    //  panning
    if (  isUserInput(event, View::INPUT_NAVIGATE) ||  isUserInput(event, View::INPUT_DRAG) || _specialMode == MODE_MOVE_CIRCLE ) {
        // priority to panning of the view (even in drop mode)
        setAction(View::PANNING);
        return false;
    }

    // DRoP MODE ; explicitly do nothing
    if ( RenderingManager::getInstance()->getSourceBasketTop() ) {
        setAction(View::DROP);
        if (isUserInput(event, View::INPUT_CONTEXT_MENU))
            RenderingManager::getRenderingWidget()->showContextMenu(ViewRenderWidget::CONTEXT_MENU_DROP, event->pos());
        // don't interpret other mouse events in drop mode
        return false;
    }

    // OTHER USER INPUT ; initiate action

    // if a special mode is active
    if ( _specialMode != MODE_NONE) {
        clickedSources.clear();
        return false;
    }

    // if at least one source icon was clicked
    if ( getSourcesAtCoordinates(event->x(), viewport[3] - event->y()) ) {

        // get the top most clicked source
        // (always one as getSourcesAtCoordinates returned true)
        Source *clicsource =  *clickedSources.begin();

        // set the current active source
        RenderingManager::getInstance()->setCurrentSource( clicsource->getId() );

        // context menu
        if ( isUserInput(event, View::INPUT_CONTEXT_MENU) )
            RenderingManager::getRenderingWidget()->showContextMenu(ViewRenderWidget::CONTEXT_MENU_SOURCE, event->pos());
        // zoom
        else if ( isUserInput(event, View::INPUT_ZOOM) )
            zoomBestFit(true);
        // toggle selection of the source
        else if ( isUserInput(event, View::INPUT_SELECT) )
            SelectionManager::getInstance()->select(clicsource);
        // other cases : TOOL
        else {
            // if the source is not in the selection, cancel previous selection
            if ( !SelectionManager::getInstance()->isInSelection(clicsource) ) {
                // get the group of this source (if exists)
                SourceListArray::iterator clicgroup = findGroup(clicsource);
                // NOT in a selection but in a group : select only the group
                if (  clicgroup != groupSources.end() ) {
                    SelectionManager::getInstance()->clearSelection();
                    SelectionManager::getInstance()->select(*clicgroup);
                }
                // NOT in a selection and NOT in a group (single source clicked)
                else
                    SelectionManager::getInstance()->clearSelection();
            }
            // ready for grabbing the current source
            setAction(View::TOOL);
        }
        // current source changed in some way
        return true;
    }
    // else = click in background

    // context menu on the background
    if ( isUserInput(event, View::INPUT_CONTEXT_MENU) ) {
        RenderingManager::getRenderingWidget()->showContextMenu(ViewRenderWidget::CONTEXT_MENU_VIEW, event->pos());
    }
    // zoom button in the background : zoom best fit
    else if ( isUserInput(event, View::INPUT_ZOOM) ) {
        zoomBestFit(false);
    }
    // reset selection and action
    else {
        // unset current source
        RenderingManager::getInstance()->unsetCurrentSource();
        // back to no action
        setAction(View::NONE);
    }


    return false;
}

bool MixerView::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if (!event)
        return false;

    if (currentAction == View::DROP)
        return false;

    // for double tool clic
    if ( isUserInput(event, View::INPUT_TOOL) /*|| isUserInput(event, View::INPUT_TOOL_INDIVIDUAL)*/  ) {

        // left double click on a source : change the group / selection
        if ( getSourcesAtCoordinates(event->x(), viewport[3] - event->y()) ) {

            // get the top most clicked source
            // (always one as getSourcesAtCoordinates returned true)
            Source *clicked = *clickedSources.begin();

            SourceListArray::iterator itss = findGroup(clicked);
            // if double clic on a group which is not part of a selection ; convert group into selection
            if ( itss != groupSources.end() && (*itss).size() >= SelectionManager::getInstance()->copySelection().size()  ) {
                SelectionManager::getInstance()->setSelection(*itss);
                // erase group and its color
                removeGroup(itss);

            }
            // if double clic on a selection ; convert selection into a new group
            else {
                SourceList selection = SelectionManager::getInstance()->copySelection();
                // convert every groups into selection
                for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
                    SourceListArray::iterator itss = findGroup(*its);
                    if (itss != groupSources.end()) {
                        SourceList result;
                        std::set_union(selection.begin(), selection.end(), (*itss).begin(), (*itss).end(), std::inserter(result, result.begin()) );
                        // set new selection
                        selection = SourceList(result);
                        // remove old group
                        removeFromGroup(*its);
                    }
                }
                // if the selection is big enough to form a group
                if ( selection.size()>1 ) {
                    // create a new group from the selection
                    groupSources.push_front(selection);
                    groupColors.prepend( QColor::fromHsv ( rand()%127 + 127, 200, 255) );
                }
            }
            return true;
        }
    }
    // zoom
    else if ( isUserInput(event, View::INPUT_ZOOM) ) {
        zoomReset();
        return true;
    }

    return false;
}


bool MixerView::mouseMoveEvent(QMouseEvent *event)
{
    if (!event)
        return false;

    int dx = event->x() - mousePos.x();
    int dy = mousePos.y() - event->y();
    mousePos = event->pos();

    // DROP MODE : avoid other actions
    if ( RenderingManager::getInstance()->getSourceBasketTop() ) {
        setAction(View::DROP);
        // don't interpret mouse events in drop mode
        return false;
    }

    // PANNING ; move the background
    if ( currentAction == View::PANNING ) {
        // panning background
        panningBy(event->x(), viewport[3] - event->y(), dx, dy);
        // SHIFT ?
        if ( isUserInput(event, View::INPUT_DRAG) || ( isUserInput(event, View::INPUT_TOOL) && _specialMode == MODE_MOVE_CIRCLE ) ) {
            // special move ; move the sources in the opposite
            for(SourceSet::iterator its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++)
                grabSource( *its, event->x(), viewport[3] - event->y(), -dx, -dy);

            SelectionManager::getInstance()->updateSelectionSource();
            // return true as we may have moved the current source
            return true;
        }
        // return false as nothing changed
        return false;
    }

    if ( isUserInput(event, View::INPUT_TOOL) ||
         isUserInput(event, View::INPUT_TOOL_INDIVIDUAL) ||
         isUserInput(event, View::INPUT_SELECT) )
    {
        // No source clicked but mouse button down
        if ( !sourceClicked() ) {

            // get coordinate of cursor
            double cursorx = 0.0, cursory = 0.0, dumm = 0.0;
            gluUnProject((double) event->x(), (double) viewport[3] - event->y(), 0.0, modelview, projection, viewport, &cursorx, &cursory, &dumm);

            // Are we scaling the limbo area ?
            if ( _specialMode == MODE_SCALE_LIMBO ) {
                double sqr_limboSize = CIRCLE_SQUARE_DIST(cursorx, cursory);
                setLimboSize( sqrt(sqr_limboSize) );
            }
            // Are we moving the selection ?
            else if ( _specialMode == MODE_MOVE_SELECTION && SelectionManager::getInstance()->hasSelection()) {
                // grab the selection
                Source *s = *SelectionManager::getInstance()->selectionBegin();
                grabSources(s, event->x(), viewport[3] - event->y(), dx, dy);
            }
            // none of the above, then we are SELECTING AREA
            else {
                // enable drawing of selection area
                _selectionArea.setEnabled(true);

                // set coordinate of end of rectangle selection
                _selectionArea.markEnd(QPointF(cursorx, cursory));

                // consider only large enough surface of selection
                if (_selectionArea.surface() * zoom * zoom > 0.1 )
                {
                    // loop over every sources to check if it is in the rectangle area
                    SourceList rectSources;
                    for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++)
                        if (_selectionArea.contains(its) )
                            rectSources.insert(*its);

                    if ( isUserInput(event, View::INPUT_SELECT) )
                        // extend selection
                        SelectionManager::getInstance()->select(rectSources);
                    else  // new selection
                        SelectionManager::getInstance()->setSelection(rectSources);
                }
            }
        }
        // clicked source not null and grab action
        else if ( currentAction != View::NONE && !isUserInput(event, View::INPUT_SELECT) ) {

            // get the top most clicked source (there is one)
            Source *clicked = *clickedSources.begin();

            // individual tool use
            if ( isUserInput(event, View::INPUT_TOOL_INDIVIDUAL) ) {
                // move single source
                grabSource(clicked, event->x(), viewport[3] - event->y(), dx, dy);
                SelectionManager::getInstance()->updateSelectionSource();
            }
            else {
                // tool use general
                if (currentTool == View::MOVE)
                    grabSources(clicked, event->x(), viewport[3] - event->y(), dx, dy);
                else if (currentTool == View::SCALE)
                    scaleSources(clicked, event->x(), viewport[3] - event->y(), dx, dy);
                else if (currentTool == View::ROTATE)
                    rotateSources(clicked, event->x(), viewport[3] - event->y(), dx, dy);
            }

        }
        // return true as we modified a source
        return true;
    }


    // else Show mouse over cursor only if no user input
    if ( isUserInput(event, View::INPUT_NONE ) )
    {
        _specialMode = MODE_NONE;
        //  change action cursor if over a selection / source
        if ( SelectionManager::getInstance()->selectionCount() > 1
                  && hasObjectAtCoordinates(event->x(), viewport[3] - event->y(), ViewRenderWidget::center_pivot, ViewRenderWidget::iconSize, SelectionManager::getInstance()->selectionSource()->getAlphaX(), SelectionManager::getInstance()->selectionSource()->getAlphaY(), 2.0))
        {
            setAction(View::OVER);
            _specialMode = MODE_MOVE_SELECTION;
        }
        else if ( getSourcesAtCoordinates(event->x(), viewport[3] - event->y(), false) ) {
            setAction(View::OVER);
        }
        else {
            setAction(View::NONE);
            // on the border of the limbo area ?
            if (hasObjectAtCoordinates(event->x(), viewport[3] - event->y(), ViewRenderWidget::circle_limbo + 1, limboSize, 0.0, 0.0, 4.0) )
                _specialMode = MODE_SCALE_LIMBO;

            // on the border of the cirle area ?
            else if (hasObjectAtCoordinates(event->x(), viewport[3] - event->y(), ViewRenderWidget::circle_mixing + 1, 1.0, 0.0, 0.0, 4.0))
                _specialMode = MODE_MOVE_CIRCLE;
        }
    }

    return false;
}

bool MixerView::mouseReleaseEvent ( QMouseEvent * event )
{

    if ( RenderingManager::getInstance()->getSourceBasketTop() )
        setAction(DROP);
    else if (currentAction == View::TOOL  || currentAction == View::DROP)
        setAction(OVER);
    else if (currentAction == View::PANNING)
        setAction(previousAction);

    // end of selection area in any case
    _selectionArea.setEnabled(false);

    // reset list of clicked sources
    clickedSources.clear();

    return true;
}

bool MixerView::wheelEvent ( QWheelEvent * event )
{
    bool ret = true;

    // remember position of cursor before zoom
    double bx, by, z;
    gluUnProject((double) event->x(), (double) (viewport[3] - event->y()), 0.0,
            modelview, projection, viewport, &bx, &by, &z);

    // apply zoom
    setZoom (zoom + ((double) event->delta() * zoom * minzoom) / (View::zoomSpeed() * maxzoom) );

    // compute position of cursor after zoom
    double ax, ay;
    gluUnProject((double) event->x(), (double) (viewport[3] - event->y()), 0.0,
            modelview, projection, viewport, &ax, &ay, &z);

    if (View::zoomCentered()) {
        // Center view on cursor when zooming ( panning = panning + ( cursor position after zoom - position before zoom ) )
        // BUT with a non linear correction factor when approaching to MINZOOM (close to 0) which allows
        // to re-center the view on the center when zooming out maximally
        double expfactor = 1.0 / ( 1.0 + exp(7.0 - 100.0 * zoom) );
        setPanning((getPanningX() + ax - bx) * expfactor, (getPanningY() + ay - by) * expfactor );
    }

    // in case we were already performing an action
    if ( currentAction == View::TOOL || _specialMode != MODE_NONE || _selectionArea.isEnabled() ){

        // where is the mouse cursor now (after zoom and panning)?
        gluUnProject((double) event->x(), (double) (viewport[3] - event->y()), 0.0,
            modelview, projection, viewport, &ax, &ay, &z);

        // this means we have a delta of mouse position
        deltax = ax - bx;
        deltay = ay - by;

//        fprintf(stderr, "bx %.2f ax %.2f  deltax %.2f \n", bx, ax, deltax);

        // simulate a movement of the mouse
        QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, event->pos(), Qt::NoButton, qtMouseButtons(INPUT_TOOL), qtMouseModifiers(INPUT_TOOL));
        ret = mouseMoveEvent(e);
        delete e;

        // reset delta
        deltax = 0;
        deltay = 0;
    }

    return ret;
}

void MixerView::zoomReset()
{
    setZoom(DEFAULTZOOM);
    setPanning(DEFAULT_PANNING);
}

void MixerView::zoomBestFit( bool onlyClickedSource )
{
    // nothing to do if there is no source
    if (RenderingManager::getInstance()->empty()){
        zoomReset();
        return;
    }

    SourceSet::iterator current = RenderingManager::getInstance()->getCurrentSource();
    double x_min = std::numeric_limits<double>::max();
    double x_max = -std::numeric_limits<double>::max();
    double y_min = std::numeric_limits<double>::max();
    double y_max = -std::numeric_limits<double>::max();

    // 0. consider either the list of clicked sources, either the full list
    if (onlyClickedSource) {

        if (SelectionManager::getInstance()->hasSelection()) {
            for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
                // get alpha coordinates
                x_min = MINI (x_min, (*its)->getAlphaX() - ViewRenderWidget::iconSize *SOURCE_UNIT * (*its)->getAspectRatio());
                x_max = MAXI (x_max, (*its)->getAlphaX() + ViewRenderWidget::iconSize *SOURCE_UNIT * (*its)->getAspectRatio());
                y_min = MINI (y_min, (*its)->getAlphaY() - ViewRenderWidget::iconSize *SOURCE_UNIT );
                y_max = MAXI (y_max, (*its)->getAlphaY() + ViewRenderWidget::iconSize *SOURCE_UNIT );
            }
        }
        else if ( RenderingManager::getInstance()->isValid(current) ) {
            x_min = (*current)->getAlphaX() - ViewRenderWidget::iconSize *SOURCE_UNIT * (*current)->getAspectRatio();
            x_max = (*current)->getAlphaX() + ViewRenderWidget::iconSize *SOURCE_UNIT * (*current)->getAspectRatio();
            y_min = (*current)->getAlphaY() - ViewRenderWidget::iconSize *SOURCE_UNIT ;
            y_max = (*current)->getAlphaY() + ViewRenderWidget::iconSize *SOURCE_UNIT ;
        }
    }

    // 1. Compute bounding depths of every sources if not already done
    if (x_max < -maxpanx) {
        for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++) {
            // get alpha coordinates
            x_min = MINI (x_min, (*its)->getAlphaX() - ViewRenderWidget::iconSize *SOURCE_UNIT * (*its)->getAspectRatio());
            x_max = MAXI (x_max, (*its)->getAlphaX() + ViewRenderWidget::iconSize *SOURCE_UNIT * (*its)->getAspectRatio());
            y_min = MINI (y_min, (*its)->getAlphaY() - ViewRenderWidget::iconSize *SOURCE_UNIT );
            y_max = MAXI (y_max, (*its)->getAlphaY() + ViewRenderWidget::iconSize *SOURCE_UNIT );
        }
    }

    // 2. Apply the panning to the new center
    setPanning( -( x_min + ABS(x_max - x_min)/ 2.0 ) ,  -( y_min + ABS(y_max - y_min)/ 2.0 )  );

    // 3. get the extend of the area covered in the viewport (the matrices have been updated just above)
    double LLcorner[3];
    double URcorner[3];
    gluUnProject(viewport[0], viewport[1], 0, modelview, projection, viewport, LLcorner, LLcorner+1, LLcorner+2);
    gluUnProject(viewport[2], viewport[3], 0, modelview, projection, viewport, URcorner, URcorner+1, URcorner+2);

    // 4. compute zoom factor to fit to the boundaries
    // initial value = a margin scale of 5%
    double scale = 0.98;
    double scale1 = ABS(URcorner[0]-LLcorner[0]) / ABS(x_max-x_min);
    double scale2 = ABS(URcorner[1]-LLcorner[1]) / ABS(y_max-y_min);
    // depending on the axis having the largest extend
    if ( scale1 < scale2 )
        scale *= scale1;
    else
        scale *= scale2;
    // apply the scaling
    setZoom( zoom * scale );
}


bool MixerView::keyPressEvent ( QKeyEvent * event ){

    // move a source
    SourceSet::iterator its = RenderingManager::getInstance()->getCurrentSource();
    if (its != RenderingManager::getInstance()->getEnd()) {
        int dx =0, dy = 0, factor = 1;
        // ALTERNATE ACTION
        if ( QApplication::keyboardModifiers() & Qt::AltModifier )
            factor *= 10;
        switch (event->key()) {
            case Qt::Key_Left:
                dx = -factor;
                break;
            case Qt::Key_Right:
                dx = factor;
                break;
            case Qt::Key_Down:
                dy = -factor;
                break;
            case Qt::Key_Up:
                dy = factor;
                break;
            default:
                return false;
        }

        // move current source
        if (currentTool == View::MOVE)
            grabSources(*its, 0, 0, dx, dy);
        else if (currentTool == View::SCALE)
            scaleSources(*its, 0, 0, dx, dy);
        else if (currentTool == View::ROTATE)
            rotateSources(*its, 0, 0, dx, dy);

        return true;
    }

    return false;
}

SourceListArray::iterator  MixerView::findGroup(Source *s){

    SourceListArray::iterator itss = groupSources.begin();
    for(; itss != groupSources.end(); itss++) {
        if ( (*itss).count(s) > 0 )
            break;
    }
    return ( itss );
}

bool MixerView::isInAGroup(Source *s){

    return ( findGroup(s) != groupSources.end() );
}

void MixerView::removeFromGroup(Source *s)
{

    // find the group containing the source to delete
    SourceListArray::iterator itss = findGroup(s);

    // if there is a group containing the source to delete
    if(itss != groupSources.end()){
        // remove the source from this group
        (*itss).erase(s);

        // if the group is now a singleton, delete it
        if( (*itss).size() < 2 )
            removeGroup(itss);

    }

}


void MixerView::removeGroup(SourceListArray::iterator i)
{
    int c = 0;
    SourceListArray::iterator itss = groupSources.begin();
    for(; itss != groupSources.end(); itss++, c++ ) {
        if ( itss == i )
        {
            itss = groupSources.erase(itss);
            groupColors.removeAt(c);
            break;
        }
    }
}


bool MixerView::hasObjectAtCoordinates(int mouseX, int mouseY, int objectdisplaylist, double scale, double tx, double ty, double tolerance)
{
    // prepare variables
    GLuint selectBuf[SELECTBUFSIZE] = { 0 };
    GLint hits = 0;

    // init picking
    glSelectBuffer(SELECTBUFSIZE, selectBuf);
    (void) glRenderMode(GL_SELECT);

    // picking in name 0, labels set later
    glInitNames();
    glPushName(0);

    // use the projection as it is, but remember it.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    // setup the projection for picking
    glLoadIdentity();
    gluPickMatrix((double) mouseX, (double) mouseY, tolerance, tolerance, viewport);
    glMultMatrixd(projection);

    // rendering for select mode
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslated(tx, ty, 0.0);
    glScaled(scale, scale, 1.0);
    glCallList(objectdisplaylist);

    // compute picking . return to rendering mode
    hits = glRenderMode(GL_RENDER);

    // set the matrices back
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    return hits > 0;
}

bool MixerView::getSourcesAtCoordinates(int mouseX, int mouseY, bool clic) {

    // prepare variables
    GLuint selectBuf[SELECTBUFSIZE] = { 0 };
    GLint hits = 0;

    // init picking
    glSelectBuffer(SELECTBUFSIZE, selectBuf);
    (void) glRenderMode(GL_SELECT);

    // picking in name 0, labels set later
    glInitNames();
    glPushName(0);

    // use the projection as it is, but remember it.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    // setup the projection for picking
    glLoadIdentity();
    gluPickMatrix((double) mouseX, (double) mouseY, 1.0, 1.0, viewport);
    glMultMatrixd(projection);

    // rendering for select mode
    glMatrixMode(GL_MODELVIEW);

    for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++) {
        if (WorkspaceManager::getInstance()->isInCurrent(*its) ) {
            glPushMatrix();
            glTranslated( (*its)->getAlphaX(), (*its)->getAlphaY(), (*its)->getDepth());
            double renderingAspectRatio = (*its)->getAspectRatio();
            if ( ABS(renderingAspectRatio) > 1.0)
                glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT, ViewRenderWidget::iconSize * SOURCE_UNIT / renderingAspectRatio,  1.0);
            else
                glScaled(ViewRenderWidget::iconSize * SOURCE_UNIT * renderingAspectRatio, ViewRenderWidget::iconSize * SOURCE_UNIT,  1.0);

            (*its)->draw(GL_SELECT);
            glPopMatrix();
        }
    }

    // compute picking . return to rendering mode
    hits = glRenderMode(GL_RENDER);

//    qDebug ("%d hits @ (%d,%d) vp (%d, %d, %d, %d)", hits, mouseX, mouseY, viewport[0], viewport[1], viewport[2], viewport[3]);

    // set the matrices back
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if (clic) {
        clickedSources.clear();
        while (hits != 0) {
            clickedSources.insert( *(RenderingManager::getInstance()->getById (selectBuf[ (hits-1) * 4 + 3])) );
            hits--;
        }

        return sourceClicked();
    } else
        return (hits != 0);

}

void MixerView::grabSources(Source *s, int x, int y, int dx, int dy) {

    if (!s) return;

    // if the source is in the selection, move the selection
    if ( SelectionManager::getInstance()->isInSelection(s) ){
        for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
            grabSource( *its, x, y, dx, dy);
        }
        SelectionManager::getInstance()->updateSelectionSource();
    }
    // nothing special, move the source individually
    else
        grabSource(s, x, y, dx, dy);

}

void MixerView::grabSource(Source *s, int x, int y, int dx, int dy) {

    if (!s) return;

    double bx, by, bz; // before movement
    double ax, ay, az; // after  movement

    gluUnProject((double) (x - dx), (double) (y - dy),
            0.0, modelview, projection, viewport, &bx, &by, &bz);
    gluUnProject((double) x, (double) y, 0.0,
            modelview, projection, viewport, &ax, &ay, &az);

    double ix = s->getAlphaX() + ax - bx + deltax;
    double iy = s->getAlphaY() + ay - by + deltay;

//    fprintf(stderr, "dx %.2f deltax %.2f \n", ax-bx, deltax);

    // move icon
    s->setAlphaCoordinates( qBound(_mixingArea[0], ix, _mixingArea[2]), qBound(_mixingArea[1], iy, _mixingArea[3]) );
}


void MixerView::scaleSources(Source *s, int x, int y, int dx, int dy) {

    if (!s) return;

    // read center of selection
    double cx = SelectionManager::getInstance()->selectionSource()->getAlphaX();
    double cy = SelectionManager::getInstance()->selectionSource()->getAlphaY();

    // remember position of s before grab (relative to C)
    double _sx = s->getAlphaX() - cx;
    double _sy = s->getAlphaY() -cy;

    // grab current source in any case
    grabSource(s, x, y, dx, dy);

    // if the source is in the selection, scale the selection
    if ( SelectionManager::getInstance()->isInSelection(s) ){

        bool requireupdate = false;

        // remember position of s after grab (relative to C)
        double sx_ = s->getAlphaX() -cx;
        double sy_ = s->getAlphaY() -cy;

        // avoid division by zero
        if ( ABS(_sx) < MIXING_EPSILON ) {
            sx_ = 1.0;
            requireupdate = true;
        }
        else
            sx_ /= _sx;

        if (ABS(_sy) < MIXING_EPSILON) {
            sy_ = 1.0;
            requireupdate = true;
        }
        else
            sy_ /= _sy;

        // grab all other sources according to center
        for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
            // ingore s
            if ( (*its)->getId() == s->getId() )
                continue;
            // remember position of i before grab
            double _x = (*its)->getAlphaX();
            double _y = (*its)->getAlphaY();
            // center on C
            _x -= cx;
            _y -= cy;
            // compute position after scaling
            double x_ = _x *  sx_;
            double y_ = _y *  sy_;
            // un-center
            x_ += cx;
            y_ += cy;
            // move icon
            (*its)->setAlphaCoordinates( qBound(_mixingArea[0], x_, _mixingArea[2]), qBound(_mixingArea[1], y_, _mixingArea[3]) );
        }

        if (requireupdate)
            SelectionManager::getInstance()->updateSelectionSource();
    }

}


void MixerView::rotateSources(Source *s, int x, int y, int dx, int dy) {

    if (!s) return;

    // read center of selection
    double cx = SelectionManager::getInstance()->selectionSource()->getAlphaX();
    double cy = SelectionManager::getInstance()->selectionSource()->getAlphaY();

    // remember position of s before grab
    double _sx = s->getAlphaX();
    double _sy = s->getAlphaY();
    // compute distance to center before grab
    double _sd = sqrt( (_sx-cx)*(_sx-cx) + (_sy-cy)*(_sy-cy) );

    // grab current source in any case
    grabSource(s, x, y, dx, dy);

    // if the source is in the selection, scale the selection
    if ( SelectionManager::getInstance()->isInSelection(s) ){

        bool requireupdate = false;

        // remember position of s after grab
        double sx_ = s->getAlphaX();
        double sy_ = s->getAlphaY();

        // compute distance to center after grab
        double sd_ = sqrt( (sx_-cx)*(sx_-cx) + (sy_-cy)*(sy_-cy) );

        // avoid division by zero
        if (_sd < MIXING_EPSILON || sd_ < MIXING_EPSILON) {
            _sd = MIXING_EPSILON;
            sd_ = MIXING_EPSILON;
            requireupdate = true;
        }

        // compute the angle between vectors (after - before)
        double angle = atan2(((sy_-cy)/sd_), ((sx_-cx)/sd_)) - atan2(((_sy-cy)/_sd), ((_sx-cx)/_sd) );

        // grab all other sources according to center
        for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
            // ingore s
            if ( (*its)->getId() == s->getId() )
                continue;

            // remember position of i before grab
            double _x = (*its)->getAlphaX();
            double _y = (*its)->getAlphaY();
            // center on C
            _x -= cx;
            _y -= cy;
            // compute position after rotation
            double x_ = _x * cos(angle) - _y * sin(angle);
            double y_ = _y * cos(angle) + _x * sin(angle);
            // scale
            x_ *= (sd_ / _sd) ;
            y_ *= (sd_ / _sd) ;
            // un-center
            x_ += cx;
            y_ += cy;
            // move icon
            (*its)->setAlphaCoordinates( qBound(_mixingArea[0], x_, _mixingArea[2]), qBound(_mixingArea[1], y_, _mixingArea[3]) );
        }

        if (requireupdate)
            SelectionManager::getInstance()->updateSelectionSource();

    }

}

void MixerView::panningBy(int x, int y, int dx, int dy) {

    double bx, by, bz; // before movement
    double ax, ay, az; // after  movement

    gluUnProject((double) (x - dx), (double) (y - dy),
            0.0, modelview, projection, viewport, &bx, &by, &bz);
    gluUnProject((double) x, (double) y, 0.0,
            modelview, projection, viewport, &ax, &ay, &az);

    // apply panning
    setPanning(getPanningX() + ax - bx, getPanningY() + ay - by);
}


QDomElement MixerView::getConfiguration(QDomDocument &doc){

    QDomElement mixviewelem = View::getConfiguration(doc);

    // Mixer View limbo size parameter
    QDomElement l = doc.createElement("Limbo");
    l.setAttribute("value", QString::number(getLimboSize(),'f',4) );
    mixviewelem.appendChild(l);

    // Mixer View groups
    int c = 0;
    QDomElement groups = doc.createElement("Groups");
    for(SourceListArray::iterator itss = groupSources.begin(); itss != groupSources.end(); itss++, c++) {
        // if this group has more than 1 element
        if (itss->size() > 1) {
            // create a group dom element, with color
            QDomElement group = doc.createElement("Group");
            group.setAttribute("R", groupColors[c].red());
            group.setAttribute("G", groupColors[c].green());
            group.setAttribute("B", groupColors[c].blue());
            // fill in the group with the list of sources.
            for(SourceList::iterator  its = (*itss).begin(); its != (*itss).end(); its++) {
                QDomElement s = doc.createElement("Source");
                QDomText sname = doc.createTextNode((*its)->getName());
                s.appendChild(sname);
                group.appendChild(s);
            }
            groups.appendChild(group);
        }
    }
    mixviewelem.appendChild(groups);

    return mixviewelem;
}


void MixerView::setConfiguration(QDomElement xmlconfig){

    // apply generic View config
    View::setConfiguration(xmlconfig);

    // Mixer View limbo size parameter
    setLimboSize(xmlconfig.firstChildElement("Limbo").attribute("value", "2.5").toFloat());

    QDomElement groups = xmlconfig.firstChildElement("Groups");
    // if there is a list of groups
    if (!groups.isNull()){
        QDomElement group = groups.firstChildElement("Group");
        // if there is a group in the list
        while (!group.isNull()) {
            SourceList newgroup;
            // if this group has more than 1 element (singleton group would be a bug)
            if (group.childNodes().count() > 1) {
                QDomElement sourceelem = group.firstChildElement("Source");
                // Add every source which name is in the list
                while (!sourceelem.isNull()) {
                    SourceSet::iterator sit = RenderingManager::getInstance()->getByName(sourceelem.text());
                    if (RenderingManager::getInstance()->isValid(sit))
                        newgroup.insert( *sit );
                    sourceelem = sourceelem.nextSiblingElement();
                }

                groupSources.push_front(newgroup);
                groupColors.prepend( QColor( group.attribute("R").toInt(),group.attribute("G").toInt(), group.attribute("B").toInt() ) );
            }
            group = group.nextSiblingElement();
        }
    }

}

void MixerView::setLimboSize(double s)
{
    limboSize = CLAMP(s, MIN_LIMBO_SIZE, MAX_LIMBO_SIZE);
    modified = true;
}

bool MixerView::isInLimbo(Source *s)
{
    return CIRCLE_SQUARE_DIST( s->getAlphaX(), s->getAlphaY() ) > (limboSize * limboSize);
}

QPointF MixerView::getGravityCenter(const SourceList &l)
{
    QPointF c(0.0, 0.0);

    if (!l.empty()) {
        SourceList::iterator sit = l.begin();
        double x = (*sit)->getAlphaX();
        double y = (*sit)->getAlphaY();
        for (sit++; sit != l.end(); sit++) {
            x += (*sit)->getAlphaX();
            y += (*sit)->getAlphaY();
        }
        c.setX( x / l.size() );
        c.setY( y / l.size() );
    }

    return c;
}

QRectF MixerView::getBoundingBox(const SourceList &l)
{
    double bbox[2][2];

    // init bbox to max size
    bbox[0][0] = 2.0*SOURCE_UNIT*MAXZOOM*CIRCLE_SIZE;
    bbox[0][1] = 2.0*SOURCE_UNIT*MAXZOOM*CIRCLE_SIZE;
    bbox[1][0] = -2.0*SOURCE_UNIT*MAXZOOM*CIRCLE_SIZE;
    bbox[1][1] = -2.0*SOURCE_UNIT*MAXZOOM*CIRCLE_SIZE;
    // compute Axis aligned bounding box of all sources in the list
    for(SourceList::const_iterator  its = l.begin(); its != l.end(); its++) {
        bbox[0][0] = qMin( (*its)->getAlphaX(), bbox[0][0]);
        bbox[0][1] = qMin( (*its)->getAlphaY(), bbox[0][1]);
        bbox[1][0] = qMax( (*its)->getAlphaX(), bbox[1][0]);
        bbox[1][1] = qMax( (*its)->getAlphaY(), bbox[1][1]);

    }
    // return bottom-left ; top-right
    return QRectF(QPointF(bbox[0][0], bbox[0][1]), QPointF(bbox[1][0], bbox[1][1]));
}

void MixerView::alignSelection(View::Axis a, View::RelativePoint p, View::Reference r)
{
    // center of selection
    QPointF selectionCenter = MixerView::getGravityCenter(SelectionManager::getInstance()->copySelection());

    if (r == View::REFERENCE_SOURCES) {
        // bopunding box for left and right extends
        QRectF selectionBox = MixerView::getBoundingBox(SelectionManager::getInstance()->copySelection());
        // perform the computations
        for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++){

            if (!(*its))
                continue;

            QPointF point = QPointF((*its)->getAlphaX(), (*its)->getAlphaY());

            // CENTERED
            if (p==View::ALIGN_CENTER) {
                if (a==View::AXIS_HORIZONTAL)
                    point.setX( selectionCenter.x());
                else
                    point.setY( selectionCenter.y());
            }
            // View::ALIGN_BOTTOM_LEFT (inverted y)
            else if (p==View::ALIGN_BOTTOM_LEFT) {
                if (a==View::AXIS_HORIZONTAL)
                    point.setX( selectionBox.topLeft().x());
                else
                    point.setY( selectionBox.topLeft().y());
            }
            // View::ALIGN_TOP_RIGHT (inverted y)
            else if (p==View::ALIGN_TOP_RIGHT) {
                if (a==View::AXIS_HORIZONTAL)
                    point.setX( selectionBox.bottomRight().x());
                else
                    point.setY( selectionBox.bottomRight().y());
            }
            // move icon
            (*its)->setAlphaCoordinates( point.x() , point.y() );
        }
    }
    // REFERENCE_FRAME
    else {
        QRectF circleBox = QRectF(-CIRCLE_SIZE * SOURCE_UNIT,-CIRCLE_SIZE * SOURCE_UNIT,2.0*CIRCLE_SIZE * SOURCE_UNIT, 2.0*CIRCLE_SIZE * SOURCE_UNIT);

        double targetx = 0.0, targety = 0.0;

        // CENTERED
        // Nothing to compute: target at (0,0)

        // View::ALIGN_BOTTOM_LEFT (inverted y)
        if (p==View::ALIGN_BOTTOM_LEFT) {

            if (a==View::AXIS_HORIZONTAL){
                // try to push the source to the border of the mixing circle
                double sina = selectionCenter.y() / (CIRCLE_SIZE * SOURCE_UNIT);
                if ( ABS(sina) < 1.0)
                    // geometric solution inside the circle
                    // trigonometric formula cos2 = 1 - sin2
                    targetx = - sqrt( 1.0 - (sina * sina) ) * (CIRCLE_SIZE * SOURCE_UNIT);
                else
                    // outside the circle
                    targetx = circleBox.topLeft().x();
            }
            else {
                double cosa = selectionCenter.x() / (CIRCLE_SIZE * SOURCE_UNIT);
                if ( ABS(cosa) < 1.0)
                    targety = - sqrt( 1.0 - (cosa * cosa) ) * (CIRCLE_SIZE * SOURCE_UNIT);
                else
                    targety = circleBox.topLeft().y();
            }
        }
        // View::ALIGN_TOP_RIGHT (inverted y)
        else if (p==View::ALIGN_TOP_RIGHT) {

            if (a==View::AXIS_HORIZONTAL){
                double sina = selectionCenter.y() / (CIRCLE_SIZE * SOURCE_UNIT);
                if ( ABS(sina) < 1.0)
                    targetx = sqrt( 1.0 - (sina * sina) ) * (CIRCLE_SIZE * SOURCE_UNIT);
                else
                    targetx = circleBox.bottomRight().x();
            }
            else {
                double cosa = selectionCenter.x() / (CIRCLE_SIZE * SOURCE_UNIT);
                if ( ABS(cosa) < 1.0)
                    targety = sqrt( 1.0 - (cosa * cosa) ) * (CIRCLE_SIZE * SOURCE_UNIT);
                else
                    targety = circleBox.bottomRight().y();
            }

        }

        // compute delta
        QPointF delta = QPointF(targetx, targety) - selectionCenter;

        // perform the computations
        for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++){

            if (!(*its))
                continue;
            if (a==View::AXIS_HORIZONTAL)
                (*its)->setAlphaCoordinates( (*its)->getAlphaX() + delta.x(), (*its)->getAlphaY() );
            else
                (*its)->setAlphaCoordinates( (*its)->getAlphaX(), (*its)->getAlphaY() + delta.y() );
        }

    }
}

void MixerView::distributeSelection(View::Axis a, View::RelativePoint p)
{
    // get selection and discard useless operation
    SourceList selection = SelectionManager::getInstance()->copySelection();
    if (selection.size() < 2)
        return;

    QMap< int, QPair<Source*, QPointF> > sortedlist;
    // do this for horizontal alignment
    if (a==View::AXIS_HORIZONTAL) {
        for(SourceList::iterator i = SelectionManager::getInstance()->selectionBegin(); i != SelectionManager::getInstance()->selectionEnd(); i++){
            QPointF point = QPointF((*i)->getAlphaX(), (*i)->getAlphaY());
            int index = int(point.x()*100000.0);
            while (sortedlist.contains(index))
                ++index;
            sortedlist[index] = qMakePair(*i, point);
        }
    }
    // do this for the vertical alignment
    else {
        // sort the list of sources by  y (inverted)
        for(SourceList::iterator i = SelectionManager::getInstance()->selectionBegin(); i != SelectionManager::getInstance()->selectionEnd(); i++){
            QPointF point = QPointF((*i)->getAlphaX(), (*i)->getAlphaY());
            int index = int(point.y()*100000.0);
            while (sortedlist.contains(index))
                ++index;
            sortedlist[index] = qMakePair(*i, point);
        }
    }

    // compute the step of translation
    QSizeF s = MixerView::getBoundingBox(selection).size() / double(sortedlist.count()-1);
    QPointF translation(s.width(),s.height());
    QPointF position = sortedlist[sortedlist.keys().first()].second;

    // loop over source list, except bottom-left & top-right most
    sortedlist.remove( sortedlist.keys().first() );
    sortedlist.remove( sortedlist.keys().last() );
    QMapIterator< int, QPair<Source*, QPointF> > its(sortedlist);
    while (its.hasNext()) {
        its.next();
        position += translation;

        QPointF point =  its.value().second;

        if (a==View::AXIS_HORIZONTAL)
            point.setX(position.x());
        else // View::VERTICAL (inverted y)
            point.setY(position.y());

        // move icon
        its.value().first->setAlphaCoordinates( point.x() , point.y()  );
    }

}

#ifdef GLM_SNAPSHOT

void MixerView::applyTargetSnapshot(double percent, QMap<Source *, QVector< QPair<double,double> > > config)
{
    // linear interpolation to dest by percent of delta
    double a = 1.0 - qBound(0.0, percent, 1.0);
    a = a < EPSILON ? 0.0 : a;

    // loop over all sources
    QMapIterator<Source *,  QVector< QPair<double,double> > > it(config);
    while (it.hasNext()) {
        it.next();
        // if in exclusive workspace mode, do not apply changes to sources in other workspaces
        if ( WorkspaceManager::getInstance()->isExclusiveDisplay() ) {
            if ( !WorkspaceManager::getInstance()->isInCurrent(it.key()))
                continue;
        }
        // interpolate change for this source
        double x = it.value()[0].first - a * it.value()[0].second;
        double y = it.value()[1].first - a * it.value()[1].second;
        it.key()->_setAlphaCoordinates(x, y);
    }

    // selection might have changed
    SelectionManager::getInstance()->updateSelectionSource();
}

bool MixerView::usableTargetSnapshot(QMap<Source *, QVector< QPair<double,double> > > config)
{
    QMapIterator<Source *,  QVector< QPair<double,double> > > it(config);
    while (it.hasNext()) {
        it.next();
        // if in exclusive workspace mode, do not apply changes to sources in other workspaces
        if ( WorkspaceManager::getInstance()->isExclusiveDisplay() ) {
            if ( !WorkspaceManager::getInstance()->isInCurrent(it.key()))
                continue;
        }
        // return true whenever a source can be modified
        if ( qAbs(it.value()[0].second) > EPSILON || qAbs(it.value()[1].second) > EPSILON )
            return true;
    }
    return false;
}

#endif
