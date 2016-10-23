/*
 * VideoSource.cpp
 *
 *  Created on: 3 nov. 2009
 *      Author: herbelin
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

#include "VideoSource.moc"

#include "RenderingManager.h"

#include <QGLFramebufferObject>

Source::RTTI VideoSource::type = Source::VIDEO_SOURCE;

VideoSource::VideoSource(VideoFile *f, GLuint texture, double d) :
    Source(texture, d), format(GL_RGBA), is(f), vp(NULL)
{
    if (!is)
        SourceConstructorException().raise();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIndex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // no PBO by default
    pboIds[0] = 0;
    pboIds[1] = 0;

    // request to update the frame when sending message
    QObject::connect(is, SIGNAL(frameReady(VideoPicture *)), this, SLOT(updateFrame(VideoPicture *)));
    // forward the message on failure
    QObject::connect(is, SIGNAL(failed()), this, SIGNAL(failed()));
    // forward the message on play
    QObject::connect(is, SIGNAL(running(bool)), this, SIGNAL(playing(bool)) );

    // fills in the first frame
    VideoPicture *_vp = is->getResetPicture();
    if (!setVideoFormat(_vp))
        SourceConstructorException().raise();

}

VideoSource::~VideoSource()
{
    // cancel updated frame
    updateFrame(NULL);

    if (is)
        delete is;

    // delete picture buffer
    if (pboIds[0] || pboIds[1])
        glDeleteBuffers(2, pboIds);
}

bool VideoSource::isPlayable() const
{
    return (is && is->getNumFrames() > 1);
}

bool VideoSource::isPlaying() const
{
    return ( isPlayable() && is->isRunning() );
}

void VideoSource::play(bool on)
{
    if ( isPlaying() == on )
        return;

    // cancel updated frame
    updateFrame(NULL);

    // transfer the order to the videoFile
    is->play(on);

    Source::play(on);
}

bool VideoSource::isPaused() const
{
    return ( isPlaying() && is->isPaused() );
}

void VideoSource::pause(bool on)
{
    if (on != isPaused())
        is->pause(on);
}

int VideoSource::getFrameWidth() const { return is->getFrameWidth(); }
int VideoSource::getFrameHeight() const { return is->getFrameHeight(); }
double VideoSource::getFrameRate() const { return is->getFrameRate(); }
double VideoSource::getAspectRatio() const { return is->getStreamAspectRatio(); }


void VideoSource::fillFramePBO(VideoPicture *vp)
{
    if ( vp->getBuffer() ) {
        // In dual PBO mode, increment current index first then get the next index
        index = (index + 1) % 2;
        nextIndex = (index + 1) % 2;

        // bind PBO to read pixels
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[index]);

        // copy pixels from PBO to texture object
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vp->getWidth(), vp->getHeight(), format, GL_UNSIGNED_BYTE, 0);

        // if the video picture contains a buffer, use it to fill the PBO

        // bind PBO to update pixel values
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[nextIndex]);

        glBufferData(GL_PIXEL_UNPACK_BUFFER, imgsize, 0, GL_STREAM_DRAW);

        // map the buffer object into client's memory
        GLubyte* ptr = (GLubyte*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr) {
            // update data directly on the mapped buffer
            memmove(ptr, vp->getBuffer(), imgsize);
            // release pointer to mapping buffer
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// only Rendering Manager can call this
void VideoSource::update()
{

    // update texture if given a new vp
    if ( vp && vp->getBuffer() != NULL )
    {
        glBindTexture(GL_TEXTURE_2D, textureIndex);

        if (internalFormat != vp->getFormat())
            setVideoFormat(vp);

        if ( pboIds[0] && pboIds[1] ) {

            // fill the texture using Pixel Buffer Object mechanism
            fillFramePBO(vp);

            // Do it once more if not refreshing immediately
            // (dual buffer mechanism)
            if ( vp->hasAction(VideoPicture::ACTION_STOP) || vp->hasAction(VideoPicture::ACTION_RESET_PTS) )
                fillFramePBO(vp);

        }
        else {
            // without PBO, use standard opengl (slower)
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vp->getWidth(),
                            vp->getHeight(), format, GL_UNSIGNED_BYTE, vp->getBuffer());
        }

        // done! Cancel (free) updated frame
        updateFrame(NULL);

    }

    Source::update();
}

void VideoSource::updateFrame(VideoPicture *p)
{
    // free the previous video picture if tagged as to be deleted.
    if (vp && vp->hasAction(VideoPicture::ACTION_DELETE))
        delete vp;

    // set new vp
    vp = p;

}

bool VideoSource::setVideoFormat(VideoPicture *p)
{
    if (p)
    {
        internalFormat = p->getFormat();
        format = (internalFormat == AV_PIX_FMT_RGBA) ? GL_RGBA : GL_RGB;

        GLint preferedinternalformat = GL_RGB;

        if (glewIsSupported("GL_ARB_internalformat_query2"))
            glGetInternalformativ(GL_TEXTURE_2D, format, GL_INTERNALFORMAT_PREFERRED, 1, &preferedinternalformat);

        // create texture and fill-in with reset picture
        glTexImage2D(GL_TEXTURE_2D, 0, (GLenum) preferedinternalformat, p->getWidth(),
                     p->getHeight(), 0, format, GL_UNSIGNED_BYTE, p->getBuffer());

        if ( isPlayable() && RenderingManager::usePboExtension())
        {
            imgsize =  p->getWidth() * p->getHeight() * (format == GL_RGB ? 3 : 4);

            // delete picture buffer
            if (pboIds[0] || pboIds[1])
                glDeleteBuffers(2, pboIds);
            // create 2 pixel buffer objects,
            glGenBuffers(2, pboIds);
            // create first PBO
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[0]);
            // glBufferDataARB with NULL pointer reserves only memory space.
            glBufferData(GL_PIXEL_UNPACK_BUFFER, imgsize, 0, GL_STREAM_DRAW);
            // fill in with reset picture
            GLubyte* ptr = (GLubyte*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
            if (ptr)  {
                // update data directly on the mapped buffer
                memmove(ptr, p->getBuffer(), imgsize);
                // release pointer to mapping buffer
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            }
            else
                return false;

            // idem with second PBO
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[1]);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, imgsize, 0, GL_STREAM_DRAW);
            ptr = (GLubyte*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
            if (ptr) {
                // update data directly on the mapped buffer
                memmove(ptr, p->getBuffer(), imgsize);
                // release pointer to mapping buffer
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            }
            else
                return false;

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            index = nextIndex = 0;
        }

    }
    else
        return false;

    return true;
}
