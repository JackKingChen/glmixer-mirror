/*
 *   GLSLCodeEditorWidget
 *
 *   This file is part of GLMixer.
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
#include "defines.h"
#include "common.h"
#include "GLSLCodeEditorWidget.moc"
#include "ui_GLSLCodeEditorWidget.h"
#include "FFGLPluginSourceShadertoy.h"

#include <QDebug>


class wheelEventFilter: public QObject
{
public:
    wheelEventFilter():QObject() {}
    ~wheelEventFilter(){}

    bool eventFilter(QObject* object,QEvent* event)
    {
        if(event->type() == QEvent::Wheel)
            return true;
        else
            return QObject::eventFilter(object,event);
    }
};

GLSLCodeEditorWidget::GLSLCodeEditorWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::GLSLCodeEditorWidget), _currentDirectory(QDir::home()), _currentplugin(NULL), wantToClose(false)
{
    ui->setupUi(this);

    QFile file(":/style/default");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    // header area is read only
    ui->headerText->setReadOnly(true);

    // collapse logs by default
    ui->splitter->setCollapsible(1, true);
    QList<int> sizes;
    sizes << (ui->splitter->sizes()[0] + ui->splitter->sizes()[1]) << 0;
    ui->splitter->setSizes( sizes );

    // Use embedded fixed size font
#ifdef Q_OS_WIN32
    ui->logText->document()->setDefaultFont(QFont(getMonospaceFont(), QApplication::font().pointSize() + 1));
#else
    ui->logText->document()->setDefaultFont(QFont(getMonospaceFont(), QApplication::font().pointSize() - 1));
#endif

    // open example on selection
    connect(ui->examplesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(applyExample(QListWidgetItem*)));

    connect(ui->examplesList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(openExample(QListWidgetItem*)));

}

GLSLCodeEditorWidget::~GLSLCodeEditorWidget()
{
    delete ui;
}

void GLSLCodeEditorWidget::fillExamplesList(bool effect)
{
    // select directory
    QDir dir;
    if (effect) {
        dir = QDir(":/shadertoy/shaders/effect/");
    } else {
        dir = QDir(":/shadertoy/shaders/source/");
    }

    // get list of GLSL
    QStringList filter;
    filter << "*.glsl";
    QFileInfoList list = dir.entryInfoList(filter);

    // fill list
    ui->examplesList->clear();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QListWidgetItem *it = new QListWidgetItem(QIcon(fileInfo.absoluteFilePath() + ".png"),  fileInfo.baseName() );
        it->setData(Qt::UserRole, fileInfo.absoluteFilePath());
        ui->examplesList->addItem(it);
    }

}


void GLSLCodeEditorWidget::applyExample(QListWidgetItem *it)
{
    if (it) {
        openExample(it);
        apply();
    }
}

void GLSLCodeEditorWidget::openExample(QListWidgetItem *it)
{
    if (it) {
        QString filename = it->data(Qt::UserRole).toString();

        QFile fileContent( filename );
        if (!fileContent.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        ui->codeTextEdit->setCode( QTextStream(&fileContent).readAll() );
        ui->examplesList->scrollToItem(it);
        ui->examplesList->setCurrentItem(0);
        ui->nameEdit->setText( QFileInfo(filename).baseName() );
    }
}


void GLSLCodeEditorWidget::linkPlugin(FFGLPluginSourceShadertoy *plugin)
{
    // unlink previous if already linked
    if (_currentplugin != plugin)
        unlinkPlugin();

    // remember which plugin is current
    _currentplugin = plugin;

    // update
    updateFields();

    // unlink when plugin is dying
    // (no need to disconnect as the plugin will be deleted)
    connect(_currentplugin, SIGNAL(dying()), this, SLOT(unlinkPlugin()));

    // associate setting of attributes with plugin
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), _currentplugin, SLOT(setName(QString)));
    connect(ui->aboutEdit, SIGNAL(textChanged(QString)), _currentplugin, SLOT(setAbout(QString)));
    connect(ui->descriptionEdit, SIGNAL(textChanged(QString)), _currentplugin, SLOT(setDescription(QString)));

}

void GLSLCodeEditorWidget::updateFields()
{   
    // clear the logs
    ui->logText->clear();

    if ( _currentplugin )
    {
        // change the header with the plugin code
        ui->headerText->setCode( _currentplugin->getHeaders() );

        // change the text editor to the plugin code
        ui->codeTextEdit->setShiftLineNumber( ui->headerText->lineCount() );
        ui->codeTextEdit->setCode( _currentplugin->getCode() );

        // set fields
        QVariantHash plugininfo = _currentplugin->getInfo();

        // name entry should be validated
        ui->nameEdit->setValidator(new nameValidator(this));
        ui->nameEdit->setText( plugininfo["Name"].toString() );

        ui->aboutEdit->setText( plugininfo["About"].toString() );
        ui->descriptionEdit->setText( plugininfo["Description"].toString() );

        if (_currentplugin->parent())
            ui->sourceEdit->setText( _currentplugin->parent()->objectName() );
        else
            ui->sourceEdit->setText( "*" );

        // restore logs
        ui->logText->appendPlainText(_currentplugin->getLogs());

        // refill the example list depending on source of effect type
        fillExamplesList(ui->headerText->lineCount() > 5);

        // enable actions
        ui->actionsFrame->setEnabled(true);
        ui->codeTextEdit->setEnabled(true);

    } else
    {
        // clear all texts
        ui->codeTextEdit->clear();
        ui->nameEdit->setValidator(0);
        ui->nameEdit->clear();
        ui->aboutEdit->clear();
        ui->descriptionEdit->clear();
        ui->sourceEdit->clear();
        ui->examplesList->clear();

        // disable actions
        ui->actionsFrame->setEnabled(false);
        ui->codeTextEdit->setEnabled(false);
    }

    update();
}


void GLSLCodeEditorWidget::unlinkPlugin()
{
    // if a plugin was edited, disconnect its changed signal
    if (_currentplugin) {
        disconnect(_currentplugin, SIGNAL(changed()), 0, 0);
        disconnect(_currentplugin, SIGNAL(dying()), 0, 0);
    }

    // always disconnect the GUI
    disconnect(ui->nameEdit, SIGNAL(textChanged(QString)), 0, 0);
    disconnect(ui->aboutEdit, SIGNAL(textChanged(QString)), 0, 0);
    disconnect(ui->descriptionEdit, SIGNAL(textChanged(QString)), 0, 0);

    // unset current plugin
    _currentplugin = NULL;

    // update
    updateFields();
}


void GLSLCodeEditorWidget::apply()
{
    if (!_currentplugin)
        return;

    // apply the current code to the plugin
    _currentplugin->setCode(ui->codeTextEdit->code());

    // because the plugin needs 1 frame to compile the GLSL
    // we shall call the display of the logs after plugin's update
    connect(_currentplugin, SIGNAL(updated()), this, SLOT(showLogs()));
}

void GLSLCodeEditorWidget::showEvent(QShowEvent *e){

    wantToClose = false;

	QWidget::showEvent(e);
}

void GLSLCodeEditorWidget::applyAndClose()
{
    if (!_currentplugin)
        return;

    wantToClose = true;

    apply();
}


void GLSLCodeEditorWidget::showLogs()
{
    // clear the logs
    ui->logText->clear();

    // no plugin, no logs
    if (!_currentplugin)
        return;

    // disconnect from updated signal
    disconnect(_currentplugin, SIGNAL(updated()), this, SLOT(showLogs()));

    // read the logs from plugin
    QString logs = _currentplugin->getLogs();
    // green for no error / warning
    QString stylesheet = "background: rgb(60, 210, 60);";;

    // if log contains something
    if (!logs.isEmpty()) {

        // fill the logs
        ui->logText->appendPlainText(logs);

        // show the Shadertoy code editor in front of your face !
        show();
        raise();
        setFocus();

        // if log area is collapsed, show it
        if ( ui->splitter->sizes()[1] == 0) {
            QList<int> sizes;
            sizes << (ui->splitter->sizes()[0] * 3 / 4) << (ui->splitter->sizes()[0] / 4);
            ui->splitter->setSizes( sizes );
        }

        if (logs.count("error") > 0) {
            // red for error
            stylesheet = "background: rgb(210, 60, 60);";
            // disable closing
            wantToClose = false;
        }
        else
            // orange for warning
            stylesheet = "background: rgb(255, 120, 0);";

        // go to the first line number given in error message
        // example syntax GLSL (1) : '0(18) : error C0000: syntax error'
        int linenumber = logs.section('(', 1, 1).section(')', 0, 0).toUInt();
        if (linenumber < 1)
            // example syntax GLSL (2): 'ERROR: 0:11: 'uv' : syntax error'
            linenumber = logs.section(':', 2, 2).section(':', 0, 0).toUInt();
        if (linenumber > 0)
            ui->codeTextEdit->gotoline(linenumber -1);

    }

    if (wantToClose)
        close();
    else {
        ui->titleCompilationLogs->setStyleSheet(stylesheet);
        QTimer::singleShot(250, this, SLOT(restoreStyle()));
    }
}



void GLSLCodeEditorWidget::restoreStyle()
{
    ui->titleCompilationLogs->setStyleSheet("background: palette(dark);");
}

void GLSLCodeEditorWidget::showHelp()
{
    QDesktopServices::openUrl(QUrl("https://www.khronos.org/registry/OpenGL/index_gl.php#apispecs", QUrl::TolerantMode));
}

void GLSLCodeEditorWidget::pasteCode()
{
    ui->codeTextEdit->setCode( QApplication::clipboard()->text() );
}

void GLSLCodeEditorWidget::restoreCode()
{
    ui->codeTextEdit->setCode( _currentplugin->getDefaultCode() );
}

void GLSLCodeEditorWidget::reloadCode()
{
    ui->codeTextEdit->setCode( _currentplugin->getCode() );
}

void GLSLCodeEditorWidget::loadCode()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ShaderToy GLSL fragment shader code"), _currentDirectory.absolutePath(), tr("GLSL code (*.glsl);;Text file (*.txt);;Any file (*.*)") );

    // check validity of file
    QFileInfo fileInfo(fileName);
    if (fileInfo.isFile() && fileInfo.isReadable()) {
        // open file
        QFile fileContent( fileInfo.absoluteFilePath());
        if (!fileContent.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        ui->codeTextEdit->setCode( QTextStream(&fileContent).readAll() );
    }

    _currentDirectory = fileInfo.dir();
}

void GLSLCodeEditorWidget::saveCode()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save ShaderToy GLSL fragment shader code"), _currentDirectory.absolutePath(), tr("GLSL code (*.glsl);;Text file (*.txt);;Any file (*.*)") );

    if ( !fileName.isEmpty() ) {
        _currentDirectory = QFileInfo(fileName).dir();
        // open file and put text into it
        QFile fileContent(fileName);
        fileContent.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&fileContent);
        out << ui->codeTextEdit->code();
    }
}
