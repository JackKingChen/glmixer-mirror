#include "common.h"
#include "FFGLPluginSource.moc"
#include "FFGLPluginInstances.h"

#include <QDebug>
#include <QGLFramebufferObject>


FFGLPluginSource::RTTI FFGLPluginSource::type = FFGLPluginSource::FREEFRAME_PLUGIN;



FFGLPluginSource::FFGLPluginSource(int w, int h, FFGLTextureStruct inputTexture)
    : _filename("Freeframe"), _initialized(false), _isFreeframeTypeSource(false), _elapsedtime(0), _pause(false), _fboSize(w,h)
{
    _plugin =  FFGLPluginInstanceFreeframe::New();

    // check validity of plugin
    if (!_plugin){
        qWarning()<< "Freeframe" << QChar(124).toLatin1() << QObject::tr("FreeframeGL plugin could not be instanciated");
        FFGLPluginException().raise();
    }

    // descriptor for the source texture, used also to store size
    _inputTexture.Handle = inputTexture.Handle;
    _inputTexture.Width = inputTexture.Width;
    _inputTexture.Height = inputTexture.Height;
    _inputTexture.HardwareWidth = inputTexture.HardwareWidth;
    _inputTexture.HardwareHeight = inputTexture.HardwareHeight;

//    qDebug() << _filename << "| " << QObject::tr("FreeframeGL plugin created") << " ("<< info["Name"].toString() <<", "<< _inputTexture.Width << _inputTexture.Height <<")";
}

void FFGLPluginSource::load(QString filename)
{
    _filename = filename;

    // check the file exists
    QFileInfo pluginfile(_filename);
    if (!pluginfile.isFile()){
        qWarning()<< _filename << QChar(124).toLatin1() << QObject::tr("The file does not exist.");
        FFGLPluginException().raise();
    }

    // if the plugin file exists, it might be accompanied by other DLLs
    // and we should add the path for the system to find them
    addPathToSystemPath( pluginfile.absolutePath().toUtf8() );

    // load dll plugin
    char fname[4096];
    strcpy(fname, pluginfile.absoluteFilePath().toLatin1().data());
    if (_plugin->Load(fname) == FF_FAIL){
        qWarning()<< pluginfile.absoluteFilePath() << QChar(124).toLatin1() << QObject::tr("FreeframeGL plugin could not be loaded");
        FFGLPluginException().raise();
    }

    // ensure functionnalities plugin
    FFGLPluginInstanceFreeframe *p = dynamic_cast<FFGLPluginInstanceFreeframe *>(_plugin);
    if ( p ) {
        if ( !p->hasProcessOpenGLCapability()){
            qWarning()<< QFileInfo(_filename).baseName() << QChar(124).toLatin1() << QObject::tr("Invalid FreeframeGL plugin: does not have OpenGL support.");
            FFGLPluginException().raise();
        }

        // fill in the information about this plugin
        _info = p->getInfo();
        _info.unite(p->getExtendedInfo());
        _isFreeframeTypeSource = (_info["Type"].toString() == "Source");
    //    qDebug() << info;

        // remember default values
        _parametersDefaults = p->getParametersDefaults();

    } else {
        qWarning()<< QFileInfo(_filename).baseName() << QChar(124).toLatin1() << QObject::tr("Invalid FreeframeGL plugin");
        FFGLPluginException().raise();
    }
}

QVariantHash FFGLPluginSource::getParameters()
{
    FFGLPluginInstanceFreeframe *p = dynamic_cast<FFGLPluginInstanceFreeframe *>(_plugin);
    if( p )
        return p->getParameters();
    else
        return QVariantHash();
}

void FFGLPluginSource::setParameter(int parameterNum, QVariant value)
{
    FFGLPluginInstanceFreeframe *p = dynamic_cast<FFGLPluginInstanceFreeframe *>(_plugin);
    if( !p || !p->setParameter(parameterNum, value) )
        qWarning()<< QFileInfo(_filename).baseName() << QChar(124).toLatin1() << QObject::tr("Parameter could not be set.");

}

void FFGLPluginSource::setParameter(QString parameterName, QVariant value)
{
    FFGLPluginInstanceFreeframe *p = dynamic_cast<FFGLPluginInstanceFreeframe *>(_plugin);
    if( !p || !p->setParameter(parameterName, value) )
        qWarning()<< QFileInfo(_filename).baseName() << QChar(124).toLatin1() << QObject::tr("Parameter could not be set.");

}

FFGLPluginSource::~FFGLPluginSource()
{
    // deletes
    if (_plugin) {
        _plugin->DeInstantiateGL();
        _plugin->Unload();
        delete _plugin;
    }
    if (_fbo)
        delete _fbo;
}

FFGLTextureStruct FFGLPluginSource::getOutputTextureStruct(){

    FFGLTextureStruct it;

    if (initialize()) {
        it.Handle = _fbo->texture();
        it.Width = _fbo->width();
        it.Height = _fbo->height();
        it.HardwareWidth = _fbo->width();
        it.HardwareHeight = _fbo->height();
    }
    return it;
}

FFGLTextureStruct FFGLPluginSource::getInputTextureStruct(){

    FFGLTextureStruct it;

    it.Handle = _inputTexture.Handle;
    it.Width = _inputTexture.Width;
    it.Height = _inputTexture.Height;
    it.HardwareWidth = _inputTexture.HardwareWidth;
    it.HardwareHeight = _inputTexture.HardwareHeight;

    return it;
}


void FFGLPluginSource::setInputTextureStruct(FFGLTextureStruct inputTexture)
{
    // descriptor for the source texture, used also to store size
    _inputTexture.Handle = inputTexture.Handle;
    _inputTexture.Width = inputTexture.Width;
    _inputTexture.Height = inputTexture.Height;
    _inputTexture.HardwareWidth = inputTexture.HardwareWidth;
    _inputTexture.HardwareHeight = inputTexture.HardwareHeight;
}

void FFGLPluginSource::update()
{
    if (initialize() && _fbo && _fbo->bind())
    {
        // Safer to push all attribs ; who knows what is done in the puglin ?!!
        // (but slower)
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        // setup a reasonnable default state
        glColor4f(1.f, 1.f, 1.f, 1.f);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);

        // constrain draw in the viewport area
        glViewport(0, 0, _fbo->width(), _fbo->height());

        //make sure all the matrices are reset
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        //clear all buffers
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClearDepth(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update time
        if (!_pause)
            _elapsedtime += timer.restart();

        //tell plugin about the current time
        _plugin->SetTime(((double) _elapsedtime) / 1000.0 );

        //prepare the structure used to call
        //the plugin's ProcessOpenGL method
        ProcessOpenGLStructTag processStruct;

        //specify our FBO's handle in the processOpenGLStruct
        processStruct.HostFBO = _fbo->handle();

        // if a texture handle was provided
        if (_inputTexture.Handle > 0) {
            //create the array of OpenGLTextureStruct * to be passed
            //to the plugin
            FFGLTextureStruct *inputTextures[1];
            inputTextures[0] = &_inputTexture;
            //provide the 1 input texture structure we allocated above
            processStruct.numInputTextures = 1;
            processStruct.inputTextures = inputTextures;
        } else {
            //provide no input texture
            processStruct.numInputTextures = 0;
            processStruct.inputTextures = NULL;
        }

        //call the plugin's ProcessOpenGL
#ifdef FF_FAIL
        // FFGL 1.5
        DWORD callresult = _plugin->CallProcessOpenGL(processStruct);
#else
        // FFGL 1.6
        FFResult callresult = _plugin->CallProcessOpenGL(processStruct);
  //      FFResult callresult = FF_SUCCESS;
#endif

        // make sure we restore state
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glPopAttrib();

        //deactivate rendering to the fbo
        //(this re-activates rendering to the window)
        _fbo->release();

        // through exception once opengl has returned to normal
        if ( callresult != FF_SUCCESS ){
            qWarning()<< QFileInfo(_filename).baseName()<< QChar(124).toLatin1() << QObject::tr("FreeframeGL plugin could not process OpenGL. Probably missing an input texture.");
            FFGLPluginException().raise();
        }
        else
            emit updated();
    }
}


void FFGLPluginSource::bind() const
{
    if (_initialized) {
        // bind the FBO texture
        glBindTexture(GL_TEXTURE_2D, _fbo->texture());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}

bool FFGLPluginSource::initialize()
{
    if ( !_initialized )
    {
        // create an fbo (with internal automatic first texture attachment)
        _fbo = new QGLFramebufferObject(_fboSize);
        if (_fbo) {

            FFGLViewportStruct _fboViewport;
            _fboViewport.x = 0;
            _fboViewport.y = 0;
            _fboViewport.width = _fbo->width();
            _fboViewport.height = _fbo->height();

            //instantiate the plugin passing a viewport that matches
            //the FBO (plugin is rendered into our FBO)
            if ( _plugin->InstantiateGL( &_fboViewport ) == FF_SUCCESS ) {

                timer.start();

                // remember successful initialization
                _initialized = true;

                // inform that its initialized
                emit initialized(this);
                // disconnect because its a one shot signal
                disconnect(SIGNAL(initialized(FFGLPluginSource *)));

            }
            else {
                qWarning()<< QFileInfo(_filename).baseName() << QChar(124).toLatin1() << QObject::tr("FreeframeGL plugin could not be initialized");
                FFGLPluginException().raise();
            }
        }
        else{
            qWarning()<< QFileInfo(_filename).baseName() << QChar(124).toLatin1() << QObject::tr("FreeframeGL plugin could not create FBO");
            FFGLPluginException().raise();
        }
    }

    return _initialized;
}



void FFGLPluginSource::restoreDefaults()
{
    // iterate over the list of parameters
    QHashIterator<QString, QVariant> i(_parametersDefaults);
    unsigned int paramNum = 0;
    while (i.hasNext()) {
        i.next();
        setParameter(paramNum, i.value());
        // increment paramNum
        paramNum++;
    }
}

void FFGLPluginSource::play(bool play) {

    _pause = !play;

    if (!_pause)
        timer.restart();

}


QDomElement FFGLPluginSource::getConfiguration( QDir current )
{
    QDomDocument root;
    QDomElement p = root.createElement("FreeFramePlugin");

    // save filename of the plugin
    QDomElement f = root.createElement("Filename");
    if (current.isReadable())
        f.setAttribute("Relative", current.relativeFilePath( fileName() ));
    QDomText filename = root.createTextNode( QDir::root().absoluteFilePath( fileName() ));
    f.appendChild(filename);
    p.appendChild(f);

    // iterate over the list of parameters
    QHashIterator<QString, QVariant> i( getParameters());
    while (i.hasNext()) {
        i.next();

        // save parameters as XML nodes
        // e.g. <Parameter name='amplitude' type='float'>0.1</param>
        QDomElement param = root.createElement("Parameter");
        param.setAttribute( "name", i.key() );
        param.setAttribute( "type", i.value().typeName() );
        QDomText value = root.createTextNode( i.value().toString() );
        param.appendChild(value);

        // add param
        p.appendChild(param);
    }

    return p;
}

void FFGLPluginSource::setConfiguration(QDomElement xml)
{
    initialize();

    // start loop of parameters to read
    QDomElement p = xml.firstChildElement("Parameter");
    while (!p.isNull()) {
        // read and apply parameter
        setParameter( p.attribute("name"), QVariant(p.text()) );
        // loop
        p = p.nextSiblingElement("Parameter");
    }
}



// for getting debug messages from FFGL code
void FFDebugMessage(const char *msg)
{
    qDebug()<<msg;
}
