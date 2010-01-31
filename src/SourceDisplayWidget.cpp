/*
 * SourceDisplayWidget.cpp
 *
 *  Created on: Jan 31, 2010
 *      Author: bh
 */

#include <QPalette>

#include <SourceDisplayWidget.h>

SourceDisplayWidget::SourceDisplayWidget(QWidget *parent, const QGLWidget * shareWidget) : glRenderWidget(parent, shareWidget), s(0)
{

}

void SourceDisplayWidget::initializeGL()
{
	glRenderWidget::initializeGL();

	setBackgroundColor(palette().color(QPalette::Window));
    glDisable(GL_BLEND);

}

void SourceDisplayWidget::paintGL()
{
	glRenderWidget::paintGL();

	if (s) {

		glPushMatrix();
		float aspectRatio = s->getAspectRatio();
		float windowaspectratio = (float) width() / (float) height();
		if (windowaspectratio < aspectRatio)
			glScalef(SOURCE_UNIT, SOURCE_UNIT * windowaspectratio / aspectRatio, 1.f);
		else
			glScalef( SOURCE_UNIT * aspectRatio / windowaspectratio, SOURCE_UNIT, 1.f);

		s->draw();
		glPopMatrix();
	}
}

