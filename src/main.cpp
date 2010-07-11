

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QString>

#include "common.h"
#include "glmixer.h"
#include "RenderingManager.h"
#include "OutputRenderWindow.h"

#include <iostream>

QStringList listofextensions;


void GLMixerMessageOutput(QtMsgType type, const char *msg)
{
	 switch (type) {
	 case QtDebugMsg:
		 std::cerr<<"Debug: "<<msg<<std::endl;
		 break;
	 case QtWarningMsg:
		 std::cerr<<"Warning: "<<msg<<std::endl;
		 break;
	 case QtCriticalMsg:
		 std::cerr<<"Critical: "<<msg<<std::endl;
		 QMessageBox::critical(0, QString("%1 Critical Information").arg(QCoreApplication::applicationName()), QString(msg));
		 abort();
		 break;
	 case QtFatalMsg:
		 std::cerr<<"Fatal: "<<msg<<std::endl;
		 QMessageBox::critical(0, QString("%1 Fatal Error").arg(QCoreApplication::applicationName()), QString(msg));
		 abort();
	 }
}

bool glSupportsExtension(QString extname) {
    return listofextensions.contains(extname, Qt::CaseInsensitive);
}

QStringList glSupportedExtensions() {
	return listofextensions;
}

int main(int argc, char **argv)
{
    qInstallMsgHandler(GLMixerMessageOutput);
    QApplication a(argc, argv);

    // -1. sets global application name ; this is used application wide (e.g. QSettings)
    a.setOrganizationName("bhbn");
    a.setOrganizationDomain("bhbn.free.fr");
    a.setApplicationName("GLMixer");

#ifdef GLMIXER_VERSION
    QCoreApplication::setApplicationVersion( QString("%1").arg(GLMIXER_VERSION) );
#else
    QCoreApplication::setApplicationVersion( "-" );
#endif

#ifdef __APPLE__
    // add local bundled lib directory as library path (Qt Plugins)
    QDir dir(QApplication::applicationDirPath());
	dir.cdUp();
	dir.cd("lib");
	QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    // 0. A splash screen to wait
    QPixmap pixmap(":/glmixer/images/glmixer_splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    a.processEvents();

    if (!QGLFormat::hasOpenGL() ) {
    	qCritical("*** ERROR ***\n\nThis system does not support OpenGL and this program cannot work without it.");
    	a.processEvents();
    }

    // fill in the list of extensions by creating a dummy glwidget
    QGLWidget *glw = new QGLWidget();
    glw->makeCurrent();
#ifdef GLEWAPI
    glewInit();
#endif
	QString allextensions = QString( (char *) glGetString(GL_EXTENSIONS));
	listofextensions = allextensions.split(" ", QString::SkipEmptyParts);
	delete glw;
    a.processEvents();

	// 1. The application GUI : it integrates the Rendering Manager QGLWidget
    GLMixer glmixer_widget;

	// 2. The output rendering window ; the rendering manager widget has to be existing
    OutputRenderWindow::getInstance()->setWindowTitle(QString("Output Window"));
    OutputRenderWindow::getInstance()->show();
	
	// 3. show the GUI in front
    glmixer_widget.show();

    // 4. load eventual session file provided in argument
    QStringList params = a.arguments();
    if ( params.count() > 1) {
    	// try to read a file with the first argument
    	glmixer_widget.openSessionFile(params[1]);
    }

    splash.finish(&glmixer_widget);
	return a.exec();
}

