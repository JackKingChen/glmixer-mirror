/*
 * LayersView.h
 *
 *  Created on: Feb 26, 2010
 *      Author: bh
 */

#ifndef LAYERSVIEW_H_
#define LAYERSVIEW_H_

#include "View.h"

class LayersView: public View {

public:

	LayersView();

    void paint();
    void setModelview();
    void resize(int w = -1, int h = -1);
    bool mousePressEvent(QMouseEvent *event);
    bool mouseMoveEvent(QMouseEvent *event);
    bool mouseReleaseEvent ( QMouseEvent * event );
    bool wheelEvent ( QWheelEvent * event );
    bool keyPressEvent ( QKeyEvent * event );
    // TODO void tabletEvent ( QTabletEvent * event ); // handling of tablet features like pressure and rotation

	void zoomReset();
	void zoomBestFit();

    void unProjectDepth(int x, int y, int dx, int dy, double *depth, double *depthBeforeDelta);

private:
    float lookatdistance;
    float currentSourceDisplacement;

    bool getSourcesAtCoordinates(int mouseX, int mouseY);
    void grabSource(SourceSet::iterator s, int x, int y, int dx, int dy);
    void panningBy(int x, int y, int dx, int dy);

    void setAction(actionType a);
};

#endif /* LAYERSVIEW_H_ */
