#include "RenderingSource.h"
#include "ViewRenderWidget.h"

#ifdef GLM_FFGL
#include "FFGLPluginSource.h"
#include <FFGL.h>
#endif

// list of sources using rendering loopback
SourceSet RenderingSource::_rendering_sources;

void RenderingSource::setRecursive(bool on)
{
    _recursive = on;

    if (_sfbo) {
        delete _sfbo;
        _sfbo = NULL;
    }

    if (_recursive) {
        // initialize loopback rendering for the first source added
        if (_rendering_sources.empty())
            RenderingManager::getInstance()->previous_frame_state = RenderingManager::LOOPBACK_INIT;
        // keep in list of sources using rendering loopback
        _rendering_sources.insert(this);
    }
    else {
        // remove from list of sources using rendering loopback
        _rendering_sources.erase(this);
        // disable loopback rendering after the last source removed
        if (_rendering_sources.empty())
            RenderingManager::getInstance()->previous_frame_state = RenderingManager::LOOPBACK_NONE;
    }

    _textureindexchanged = true;
}

RenderingSource::RenderingSource(bool recursive, double d): Source(0, d),
    _recursive(true), _textureindexchanged(true), _sfbo(NULL)
{
    setRecursive(recursive);
}

RenderingSource::~RenderingSource() {
    setRecursive(false);

    if (_sfbo) {
        delete _sfbo;
        _sfbo = NULL;
    }
}

QString RenderingSource::getInfo() const {

    return Source::getInfo() + " - Loopback ";
}

GLuint RenderingSource::getTextureIndex() const {
    QGLFramebufferObject *fbo = _sfbo;
    if (_recursive || !fbo )
        fbo = RenderingManager::getInstance()->previousframe_fbo;
    if (fbo)
        return fbo->texture();
    else
        return Source::getTextureIndex();
}

int RenderingSource::getFrameWidth() const {
    QGLFramebufferObject *fbo = _sfbo;
    if (_recursive || !fbo )
        fbo = RenderingManager::getInstance()->previousframe_fbo;
    if (fbo)
        return fbo->width();
    else
        return Source::getFrameWidth();
}

int RenderingSource::getFrameHeight() const {
    QGLFramebufferObject *fbo = _sfbo;
    if (_recursive || !fbo )
        fbo = RenderingManager::getInstance()->previousframe_fbo;
    if (fbo)
        return fbo->height();
    else
        return Source::getFrameHeight();
}

double RenderingSource::getFrameRate() const {
    if (_recursive)
        return RenderingManager::getRenderingWidget()->getFramerate() / double(RenderingManager::getInstance()->getPreviousFramePeriodicity());
    else
        return RenderingManager::getRenderingWidget()->getFramerate();
}


void RenderingSource::bind()
{
    QGLFramebufferObject *fbo = RenderingManager::getInstance()->_fbo;
    if (!_recursive && fbo) {
        if (_sfbo && _sfbo->size() != fbo->size()) {
            delete _sfbo;
            _sfbo = NULL;
        }
        if (!_sfbo)
            _sfbo = new QGLFramebufferObject(fbo->size());

        // use the accelerated GL_EXT_framebuffer_blit if available
        if (RenderingManager::blit_fbo_extension)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _sfbo->handle());
            glBlitFramebuffer(0, fbo->height(), fbo->width(), 0, 0, 0,
                              _sfbo->width(), _sfbo->height(),
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }
        // otherwise Draw quad with fbo texture in a more basic OpenGL way
        // (slow)
        else {

            // temporarily disable source mode
            ViewRenderWidget::setSourceDrawingMode(false);

            glPushAttrib(GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);

            glViewport(0, 0, _sfbo->width(), _sfbo->height());

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            // render quad to the frame buffer object
            if (_sfbo->bind())
            {
                glClearColor(0.f, 0.f, 0.f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT);

                glColor4f(1.f, 1.f, 1.f, 1.f);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, fbo->texture());
                glCallList(ViewRenderWidget::quad_texured);

                _sfbo->release();
            }

            // pop the projection matrix and GL state back for rendering the current view
            // to the actual widget
            glDisable(GL_TEXTURE_2D);
            glPopAttrib();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();


            ViewRenderWidget::setSourceDrawingMode(true);
        }
    }

#ifdef GLM_FFGL
    if (_textureindexchanged && !_ffgl_plugins.isEmpty()) {
        // re-connect texture to freeframe gl plugin
        FFGLTextureStruct it;
        it.Handle = (GLuint) getTextureIndex();
        it.Width = getFrameWidth();
        it.Height = getFrameHeight();
        it.HardwareWidth = getFrameWidth();
        it.HardwareHeight = getFrameHeight();
        _ffgl_plugins.first()->setInputTextureStruct(it);
    }
#endif

    Source::bind();
}

QDomElement RenderingSource::getConfiguration(QDomDocument &doc, QDir current)
{
    QDomElement sourceElem = Source::getConfiguration(doc, current);
    sourceElem.setAttribute("playing", isPlaying());
    QDomElement specific = doc.createElement("TypeSpecific");
    specific.setAttribute("type", rtti());

    QDomElement f = doc.createElement("Recursive");
    QDomText recursive = doc.createTextNode(QString::number(_recursive));
    f.appendChild(recursive);
    specific.appendChild(f);

    sourceElem.appendChild(specific);
    return sourceElem;
}


