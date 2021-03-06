/*
 * glmixer.cpp
 *
 *  Created on: Jul 14, 2009
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
 *   Copyright 2009, 2016 Bruno Herbelin
 *
 */


#include <iostream>

#include <main.h>
#include <QDomDocument>
#include <QtGui>

#include "common.h"
#include "VideoFileDialog.h"
#include "AlgorithmSelectionDialog.h"
#include "UserPreferencesDialog.h"
#include "ViewRenderWidget.h"
#include "RenderingManager.h"
#include "SelectionManager.h"
#include "SourceDisplayWidget.h"
#include "OutputRenderWindow.h"
#include "MixerView.h"
#include "RenderingSource.h"
#include "AlgorithmSource.h"
#include "VideoSource.h"
#include "WebSource.h"
#include "SvgSource.h"
#include "VideoStreamSource.h"
#include "VideoFileDisplayWidget.h"
#include "SourcePropertyBrowser.h"
#include "CloneSource.h"
#include "CatalogView.h"
#include "DelayCursor.h"
#include "SpringCursor.h"
#include "AxisCursor.h"
#include "LineCursor.h"
#include "FuzzyCursor.h"
#include "MagnetCursor.h"
#include "RenderingEncoder.h"
#include "SessionSwitcher.h"
#include "MixingToolboxWidget.h"
#include "LayoutToolboxWidget.h"
#include "GammaLevelsWidget.h"
#include "NewSourceDialog.h"
#include "WebSourceCreationDialog.h"
#include "VideoStreamDialog.h"
#include "CodecManager.h"
#include "WorkspaceManager.h"
#include "OpenSoundControlTranslator.h"
#include "BasketSelectionDialog.h"
#include "CameraDialog.h"


#define GLM_OSC
#ifdef GLM_OSC
#include "OpenSoundControlManager.h"
#endif

#ifdef GLM_UNDO
#include "UndoManager.h"
#endif

#ifdef GLM_SESSION
#include "SessionSwitcherWidget.h"
#endif

#ifdef GLM_TAG
#include "TagsManager.h"
#endif

#ifdef GLM_SNAPSHOT
#include "SnapshotManager.h"
#include "SnapshotManagerWidget.h"
#endif

#ifdef GLM_HISTORY
#include "HistoryRecorderWidget.h"
#endif

#ifdef GLM_SHM
#include "SharedMemorySource.h"
#include "SharedMemoryDialog.h"
#include "SharedMemoryManager.h"
#endif

#ifdef GLM_OPENCV
#include "OpencvSource.h"
#endif

#ifdef GLM_FFGL
#include "FFGLSource.h"
#include "FFGLPluginSource.h"
#include "FFGLPluginSourceShadertoy.h"
#include "FFGLSourceCreationDialog.h"
#include "GLSLCodeEditorWidget.h"
#endif

#include "glmixerdialogs.h"
#include "glmixer.moc"

GLMixer *GLMixer::_instance = 0;
bool GLMixer::_singleInstanceMode = false;

#ifdef GLM_LOGS
QFile *GLMixer::logFile = 0;
QTextStream GLMixer::logStream;
LoggingWidget *GLMixer::logsWidget = 0;
#endif

QByteArray static_windowstate =
QByteArray::fromHex("000000ff00000000fd00000003000000000000014f00000316fc0200000004fb0000002200700072006500760069006500770044006f0063006b0057006900640067006500740100000025000000f3000000d500fffffffc0000011e00000187000000ba00fffffffa000000000100000002fb00000024007300770069007400630068006500720044006f0063006b0057006900640067006500740100000000ffffffff000000ed00fffffffb000000260073006e0061007000730068006f007400730044006f0063006b0057006900640067006500740100000000ffffffff0000006a00fffffffb0000002600730065006c0065006300740069006f006e0044006f0063006b00570069006400670065007401000002ab000000900000009000fffffffb0000001a006c006f00670044006f0063006b005700690064006700650074020000044e0000029d000002ef00000184000000010000014300000316fc0200000006fc00000043000000b60000000000fffffffa000000000100000002fb0000001c00740061006700730044006f0063006b0057006900640067006500740100000000ffffffff0000000000000000fb000000200063007500720073006f00720044006f0063006b0057006900640067006500740000000000ffffffff0000008200fffffffb00000020006d006900780069006e00670044006f0063006b005700690064006700650074010000002500000171000000e700fffffffc0000019c0000019f0000008f00fffffffa000000000100000002fb000000200073006f00750072006300650044006f0063006b0057006900640067006500740100000000ffffffff0000003000fffffffb00000020006c00610079006f007500740044006f0063006b0057006900640067006500740100000000ffffffff0000005200fffffffb000000240062006c006f0063006e006f007400650044006f0063006b005700690064006700650074000000032c0000013b0000008300fffffffc000002f2000000300000000000fffffffa000000000100000001fb0000001e0061006c00690067006e0044006f0063006b0057006900640067006500740100000000ffffffff0000000000000000fb0000001e00670061006d006d00610044006f0063006b0057006900640067006500740100000000ffffffff000000000000000000000003000005fc00000054fc0100000001fb0000002400760063006f006e00740072006f006c0044006f0063006b0057006900640067006500740100000000000005fc000002b10007ffff0000035e0000031600000004000000040000000800000008fc0000000100000002000000060000001a0073006f00750072006300650054006f006f006c0042006100720100000000ffffffff00000000000000000000001600760069006500770054006f006f006c0042006100720100000252ffffffff00000000000000000000001600660069006c00650054006f006f006c004200610072010000031effffffff0000000000000000000000180074006f006f006c00730054006f006f006c00420061007201000003eaffffffff00000000000000000000002000720065006e0064006500720069006e00670054006f006f006c004200610072010000048fffffffff00000000000000000000001a0063007500720073006f00720054006f006f006c004200610072000000056bffffffff0000000000000000");

QByteArray static_performance_windowstate =
QByteArray::fromHex("000000ff00000000fd0000000300000000000001e300000316fc0200000006fb0000002200700072006500760069006500770044006f0063006b00570069006400670065007401000000250000016d000000d500fffffffc000001980000010d000000a800fffffffa000000000100000002fb00000024007300770069007400630068006500720044006f0063006b0057006900640067006500740100000000ffffffff0000013000fffffffb000000260073006e0061007000730068006f007400730044006f0063006b0057006900640067006500740100000000ffffffff0000005a00fffffffb0000002600730065006c0065006300740069006f006e0044006f0063006b00570069006400670065007401000002ab000000900000009000fffffffb00000020006d006900780069006e00670044006f0063006b005700690064006700650074000000021b00000090000000e700fffffffb00000020006c00610079006f007500740044006f0063006b0057006900640067006500740000000143000001c80000006200fffffffb0000001a006c006f00670044006f0063006b005700690064006700650074020000044e0000029d000002ef00000184000000010000011d00000424fc0200000005fb0000001c00740061006700730044006f0063006b005700690064006700650074000000003e000000dd0000000000000000fb000000240062006c006f0063006e006f007400650044006f0063006b0057006900640067006500740000000043000004240000008300fffffffb000000200073006f00750072006300650044006f0063006b005700690064006700650074000000019e0000012c0000001600fffffffc000002f2000000300000000000fffffffa000000000100000001fb0000001e0061006c00690067006e0044006f0063006b0057006900640067006500740100000000ffffffff0000000000000000fb0000001e00670061006d006d00610044006f0063006b0057006900640067006500740100000000ffffffff000000000000000000000003000005fc00000054fc0100000002fb000000200063007500720073006f00720044006f0063006b0057006900640067006500740000000000000001e30000008200fffffffb0000002400760063006f006e00740072006f006c0044006f0063006b0057006900640067006500740100000000000005fc000002b10007ffff000004130000031600000004000000040000000800000008fc0000000100000002000000060000001a0073006f00750072006300650054006f006f006c0042006100720100000000ffffffff00000000000000000000001600760069006500770054006f006f006c0042006100720100000252ffffffff00000000000000000000001600660069006c00650054006f006f006c004200610072010000031effffffff0000000000000000000000180074006f006f006c00730054006f006f006c00420061007201000003eaffffffff00000000000000000000002000720065006e0064006500720069006e00670054006f006f006c004200610072000000042bffffffff00000000000000000000001a0063007500720073006f00720054006f006f006c004200610072010000048fffffffff0000000000000000");

GLMixer *GLMixer::getInstance() {

    if (_instance == 0) {
        _instance = new GLMixer();
        Q_CHECK_PTR(_instance);
    }

    return _instance;
}

void GLMixer::deleteInstance() {

    if (_instance)
        delete _instance;

    _instance = 0;
}

bool GLMixer::isSingleInstanceMode(){
    return _singleInstanceMode;
}

GLMixer::GLMixer ( QWidget *parent): QMainWindow ( parent ),
    usesystemdialogs(false), maybeSave(true), previousSource(NULL), currentVideoFile(NULL),
    _displayTimeAsFrame(false), _restoreLastSession(true),
    _saveExitSession(false), _disableOutputWhenRecord(false),
    _displayTimerEnabled(false)
{
    setupUi ( this );

    // create settings object with default mode
    _settings = new QSettings();

    QFile file(":/style/default");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    // Use embedded fixed size font
    blocNoteEdit->setFontFamily(getMonospaceFont());
    blocNoteEdit->setFontPointSize(10);

#ifndef GLM_SHM
    actionShareToRAM->setVisible(false);
    actionShmSource->setVisible(false);
#endif

#ifndef GLM_SPOUT
    actionShareToSPOUT->setVisible(false);
#endif

#ifdef GLM_FFGL
    pluginGLSLCodeEditor = new GLSLCodeEditorWidget();
#else
    actionFreeframeSource->setVisible(false);
#endif

#ifdef GLM_LOGS
    // The log widget
    if (!logsWidget)
        logsWidget = new LoggingWidget(_settings);
    connect(logsWidget, SIGNAL(saveLogs()), SLOT(saveLogsToFile()));

    QAction *showlog = new QAction(QIcon(":/icons/history.png"), tr("Logs"), this);
    showlog->setShortcut(QKeySequence("Ctrl+L"));
    showlog->setStatusTip(tr("Show the application logs"));
    showlog->setCheckable(true);
    showlog->setChecked(false);
    toolBarsMenu->addAction(showlog);
    connect(showlog, SIGNAL(toggled(bool)), logsWidget, SLOT(setVisible(bool)));
    connect(logsWidget, SIGNAL(isVisible(bool)), showlog, SLOT(setChecked(bool)));
#endif

    // add the show/hide menu items for the dock widgets
    toolBarsMenu->addAction(previewDockWidget->toggleViewAction());
    toolBarsMenu->addAction(sourceDockWidget->toggleViewAction());
    toolBarsMenu->addAction(vcontrolDockWidget->toggleViewAction());
    toolBarsMenu->addAction(mixingDockWidget->toggleViewAction());
#ifdef GLM_SESSION
    toolBarsMenu->addAction(switcherDockWidget->toggleViewAction());
#endif
    toolBarsMenu->addAction(snapshotsDockWidget->toggleViewAction());
    toolBarsMenu->addAction(selectionDockWidget->toggleViewAction());
    toolBarsMenu->addAction(cursorDockWidget->toggleViewAction());
    toolBarsMenu->addAction(layoutDockWidget->toggleViewAction());
    toolBarsMenu->addAction(blocnoteDockWidget->toggleViewAction());

#ifdef GLM_HISTORY
    // The history widget
    QAction *showhistory = actionHistoryDockWidget->toggleViewAction();
    showhistory->setShortcut(QKeySequence("Ctrl+H"));
    toolBarsMenu->addAction(showhistory);
    actionHistoryDockWidget->hide();
#endif

    toolBarsMenu->addSeparator();
    toolBarsMenu->addAction(fileToolBar->toggleViewAction());
    toolBarsMenu->addAction(sourceToolBar->toggleViewAction());
    toolBarsMenu->addAction(viewToolBar->toggleViewAction());
    toolBarsMenu->addAction(toolsToolBar->toggleViewAction());
    toolBarsMenu->addAction(renderingToolBar->toggleViewAction());
    toolBarsMenu->addAction(cursorToolBar->toggleViewAction());

    QActionGroup *viewActions = new QActionGroup(this);
    Q_CHECK_PTR(viewActions);
    viewActions->addAction(actionMixingView);
    actionMixingView->setData(View::MIXING);
    viewActions->addAction(actionGeometryView);
    actionGeometryView->setData(View::GEOMETRY);
    viewActions->addAction(actionLayersView);
    actionLayersView->setData(View::LAYER);
    viewActions->addAction(actionRenderingView);
    actionRenderingView->setData(View::RENDERING);
    QObject::connect(viewActions, SIGNAL(triggered(QAction *)), this, SLOT(setView(QAction *) ) );

    QActionGroup *toolActions = new QActionGroup(this);
    Q_CHECK_PTR(toolActions);
    toolActions->addAction(actionToolGrab);
    actionToolGrab->setData(ViewRenderWidget::TOOL_GRAB);
    toolActions->addAction(actionToolScale);
    actionToolScale->setData(ViewRenderWidget::TOOL_SCALE);
    toolActions->addAction(actionToolRotate);
    actionToolRotate->setData(ViewRenderWidget::TOOL_ROTATE);
    toolActions->addAction(actionToolCut);
    actionToolCut->setData(ViewRenderWidget::TOOL_CUT);
    QObject::connect(toolActions, SIGNAL(triggered(QAction *)), this, SLOT(setTool(QAction *) ) );

    QActionGroup *cursorActions = new QActionGroup(this);
    Q_CHECK_PTR(cursorActions);
    cursorActions->addAction(actionCursorNormal);
    actionCursorNormal->setData(ViewRenderWidget::CURSOR_NORMAL);
    cursorActions->addAction(actionCursorSpring);
    actionCursorSpring->setData(ViewRenderWidget::CURSOR_SPRING);
    cursorActions->addAction(actionCursorDelay);
    actionCursorDelay->setData(ViewRenderWidget::CURSOR_DELAY);
    cursorActions->addAction(actionCursorAxis);
    actionCursorAxis->setData(ViewRenderWidget::CURSOR_AXIS);
    cursorActions->addAction(actionCursorLine);
    actionCursorLine->setData(ViewRenderWidget::CURSOR_LINE);
    cursorActions->addAction(actionCursorFuzzy);
    actionCursorFuzzy->setData(ViewRenderWidget::CURSOR_FUZZY);
    cursorActions->addAction(actionCursorMagnet);
    actionCursorMagnet->setData(ViewRenderWidget::CURSOR_MAGNET);
    QObject::connect(cursorActions, SIGNAL(triggered(QAction *)), this, SLOT(setCursor(QAction *) ) );

    QActionGroup *aspectRatioActions = new QActionGroup(this);
    Q_CHECK_PTR(aspectRatioActions);
    aspectRatioActions->addAction(action4_3_aspect_ratio);
    action4_3_aspect_ratio->setData(ASPECT_RATIO_4_3);
    aspectRatioActions->addAction(action3_2_aspect_ratio);
    action3_2_aspect_ratio->setData(ASPECT_RATIO_3_2);
    aspectRatioActions->addAction(action16_10_aspect_ratio);
    action16_10_aspect_ratio->setData(ASPECT_RATIO_16_10);
    aspectRatioActions->addAction(action16_9_aspect_ratio);
    action16_9_aspect_ratio->setData(ASPECT_RATIO_16_9);
    aspectRatioActions->addAction(actionFree_aspect_ratio);
    actionFree_aspect_ratio->setData(ASPECT_RATIO_ANY);
    QObject::connect(aspectRatioActions, SIGNAL(triggered(QAction *)), this, SLOT(setAspectRatio(QAction *) ) );

    // create menu for the Workspace Manager selection actions
    menuWorkspace->insertActions(actionWorkspaceIncrement, WorkspaceManager::getInstance()->getActions() );
    toolButtonWorkspaceExclusive->setDefaultAction(actionWorkspaceExclusive);

    // create tool buttons in main view
    foreach(QToolButton *b,  WorkspaceManager::getInstance()->getButtons()){
        b->setParent(workspacesFrame);
        workspacesLayout->addWidget(b);
    }

    // create menu for current source workspace actions
    QAction *sep = menuSendToWorkspace->insertSeparator(actionNewWorkspace);
    menuSendToWorkspace->insertActions(sep, WorkspaceManager::getInstance()->getSourceActions());

    // Workspace Management
    QObject::connect(WorkspaceManager::getInstance(), SIGNAL(changed()), this, SLOT(updateWorkspaceActions()) );
    QObject::connect(WorkspaceManager::getInstance(), SIGNAL(countChanged(int)), RenderingManager::getInstance(), SLOT(setWorkspaceCount(int)) );
    QObject::connect(WorkspaceManager::getInstance(), SIGNAL(status(const QString &, int)), RenderingManager::getRenderingWidget(), SLOT(showMessage(const QString &, int)));
    QObject::connect(actionWorkspaceIncrement, SIGNAL(triggered()), WorkspaceManager::getInstance(), SLOT(incrementCount()));
    QObject::connect(actionWorkspaceDecrement, SIGNAL(triggered()), WorkspaceManager::getInstance(), SLOT(decrementCount()));
    QObject::connect(actionNewWorkspace, SIGNAL(triggered()), WorkspaceManager::getInstance(), SLOT(incrementCount()));
    QObject::connect(actionWorkspaceExclusive, SIGNAL(triggered(bool)), WorkspaceManager::getInstance(), SLOT(setExclusiveDisplay(bool)));
    // special behavior when adding workspace
    QObject::connect(actionWorkspaceIncrement, SIGNAL(triggered()), WorkspaceManager::getInstance(), SLOT(setCurrent()) ); // switch to latest when increment
    QObject::connect(actionNewWorkspace, SIGNAL(triggered()), RenderingManager::getInstance(), SLOT(setWorkspaceCurrentSource())); // move current source to latest when creating new
    QObject::connect(actionNewWorkspace, SIGNAL(triggered()), WorkspaceManager::getInstance(), SLOT(setCurrent()) ); // switch to latest when creating new
    QObject::connect(actionWorkspaceMerge, SIGNAL(triggered()), RenderingManager::getInstance(), SLOT(setWorkspaceAllSources()) ); // switch to latest when creating new

    // HIDDEN actions
    // for debugging and development purposes
    QAction *screenshot = new QAction("Screenshot", this);
    screenshot->setShortcut(QKeySequence("Ctrl+<,<"));
    addAction(screenshot);
    QObject::connect(screenshot, SIGNAL(triggered()), this, SLOT(screenshotView() ) );

    QAction *setGLSLFragmentShader = new QAction("setGLSLFragmentShader", this);
    setGLSLFragmentShader->setShortcut(QKeySequence("Shift+Ctrl+G,F"));
    addAction(setGLSLFragmentShader);
    QObject::connect(setGLSLFragmentShader, SIGNAL(triggered()), this, SLOT(selectGLSLFragmentShader()) );

    QAction *startSessionTestingBot = new QAction("startSessionTestingBot", this);
    startSessionTestingBot->setShortcut(QKeySequence("Shift+Ctrl+T,B"));
    addAction(startSessionTestingBot);
    QObject::connect(startSessionTestingBot, SIGNAL(triggered()), this, SLOT(startSessionTestingBot()) );

    // recent files history
    QMenu *recentFiles = new QMenu(this);
    Q_CHECK_PTR(recentFiles);
    actionRecent_session->setMenu(recentFiles);
    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(actionLoad_RecentSession_triggered()));
        recentFiles->addAction(recentFileActs[i]);
    }

    // Setup the property browsers
    specificSourcePropertyBrowser = NULL;
    layoutPropertyBrowser = new QSplitter(Qt::Vertical, this);
    SourcePropertyBrowser *sourcePropertyBrowser = RenderingManager::getPropertyBrowserWidget();
    layoutPropertyBrowser->addWidget(sourcePropertyBrowser);
//    QObject::connect(this, SIGNAL(sourceMarksModified(bool)), sourcePropertyBrowser, SLOT(updateMarksProperties(bool) ) );
    sourceDockWidgetContentsLayout->addWidget(layoutPropertyBrowser);

    // setup the mixing toolbox
    mixingToolBox = new MixingToolboxWidget(this, _settings);
    mixingDockWidgetContentLayout->addWidget(mixingToolBox);
    QObject::connect(RenderingManager::getInstance(), SIGNAL(currentSourceChanged(SourceSet::iterator)), mixingToolBox, SLOT(connectSource(SourceSet::iterator) ) );
    QObject::connect(mixingToolBox, SIGNAL( sourceChanged(SourceSet::iterator)), RenderingManager::getInstance(), SIGNAL(currentSourceChanged(SourceSet::iterator)) );
    // bidirectional link between mixing toolbox and property manager
    QObject::connect(mixingToolBox, SIGNAL(valueChanged(QString, bool)), sourcePropertyBrowser, SLOT(valueChanged(QString, bool)) );
    QObject::connect(mixingToolBox, SIGNAL(valueChanged(QString, int)), sourcePropertyBrowser, SLOT(valueChanged(QString, int)) );
    QObject::connect(mixingToolBox, SIGNAL(valueChanged(QString, const QColor &)), sourcePropertyBrowser, SLOT(valueChanged(QString, const QColor &)) );
    QObject::connect(mixingToolBox, SIGNAL(enumChanged(QString, int)), sourcePropertyBrowser, SLOT(enumChanged(QString, int)) );
    QObject::connect(sourcePropertyBrowser, SIGNAL(propertyChanged(QString, bool)), mixingToolBox, SLOT(propertyChanged(QString, bool)) );
    QObject::connect(sourcePropertyBrowser, SIGNAL(propertyChanged(QString, int)), mixingToolBox, SLOT(propertyChanged(QString, int)) );
    QObject::connect(sourcePropertyBrowser, SIGNAL(propertyChanged(QString, const QColor &)), mixingToolBox, SLOT(propertyChanged(QString, const QColor &)) );
    QObject::connect(sourcePropertyBrowser, SIGNAL(propertyChanged(QString)), mixingToolBox, SLOT(propertyChanged(QString)) );

    // setup the layout toolbox
    layoutToolBox = new LayoutToolboxWidget(this);
    layoutDockWidgetContentLayout->addWidget(layoutToolBox);

    // Set the docking tab vertical
    setDockOptions( dockOptions() | QMainWindow::VerticalTabs);

    // Setup status bar
    infobar = new QLabel(this);
    statusbar->addPermanentWidget(infobar);

    // Setup the central widget
    centralViewLayout->removeWidget(mainRendering);
    delete mainRendering;
    mainRendering = (QGLWidget *) RenderingManager::getRenderingWidget();
    mainRendering->setParent(centralwidget);
    centralViewLayout->addWidget(mainRendering);
    //activate the default view & share labels to display in
    setView(actionMixingView);
    RenderingManager::getRenderingWidget()->setLabels(zoomLabel, fpsLabel);
    // share menus as context menus of the main view
    RenderingManager::getRenderingWidget()->setViewContextMenu(zoomMenu);
    RenderingManager::getRenderingWidget()->setCatalogContextMenu(catalogMenu);
    RenderingManager::getRenderingWidget()->setSourceContextMenu(currentSourceMenu);
    currentSourceMenu->setEnabled(false);

    QObject::connect(this, SIGNAL(status(const QString &, int)), RenderingManager::getRenderingWidget() , SLOT(showMessage(const QString &, int)));

    // link to render window
    QObject::connect(RenderingManager::getInstance(), SIGNAL(frameBufferChanged()), OutputRenderWindow::getInstance(), SLOT(refresh()));


#ifdef GLM_OSC
    // special link between OSC and MixingToolbox to call for presets
    QObject::connect(OpenSoundControlManager::getInstance(), SIGNAL(applyPreset(QString, QString)), mixingToolBox, SLOT(applyPreset(QString, QString)) );
    QObject::connect(OpenSoundControlManager::getInstance(), SIGNAL(applyPreset(QString, int)), mixingToolBox, SLOT(applyPreset(QString, int)) );
#else
    delete actionOSCTranslator;
#endif

#ifdef GLM_TAG
    // setup the tags manager
    tagsManager = new TagsManager(this);
    tagsDockWidgetContentLayout->insertWidget(0, tagsManager);
    QObject::connect(RenderingManager::getInstance(), SIGNAL(currentSourceChanged(SourceSet::iterator)), tagsManager, SLOT(connectSource(SourceSet::iterator) ) );

    // tag menu for source context menu
    QAction *tagsmenu = currentSourceMenu->insertMenu(menuSendToWorkspace->menuAction(), tagsManager->getTagsMenu());
    tagsmenu->setText(tr("Color"));
#endif

#ifdef GLM_HISTORY
    // setup the history toolbox
    actionHistoryView = new HistoryRecorderWidget(this);
    actionHistorydockWidgetContentsLayout->addWidget(actionHistoryView);
#else
    // DISABLE HISTORY MANAGER
    delete actionHistoryDockWidget;
#endif

#ifdef GLM_SNAPSHOT
    // setup snapshot manager
    QObject::connect(SnapshotManager::getInstance(), SIGNAL(snap()), RenderingManager::getRenderingWidget(), SLOT(triggerFlash()) );
    QObject::connect(SnapshotManager::getInstance(), SIGNAL(snapshotRestored()), SelectionManager::getInstance(), SLOT(clearSelection()) );
    QObject::connect(SnapshotManager::getInstance(), SIGNAL(status(const QString &, int)), RenderingManager::getRenderingWidget(), SLOT(showMessage(const QString &, int)));
    // setup the snapshot toolbox
    snapshotManager = new SnapshotManagerWidget(this, _settings);
    snapshotsDockWidgetContentsLayout->addWidget(snapshotManager);
#else
    // DISABLE SNAPSHOT MANAGER
    delete snapshotsDockWidget;
#endif

#ifdef GLM_SESSION
    // Setup the session switcher toolbox
    switcherSession = new SessionSwitcherWidget(this, _settings);
    switcherDockWidgetContentsLayout->addWidget(switcherSession);
    QObject::connect(switcherSession, SIGNAL(sessionTriggered(QString)), this, SLOT(switchToSessionFile(QString)) );
    QObject::connect(this, SIGNAL(sessionLoaded()), switcherSession, SLOT(unsuspend()));
    QObject::connect(this, SIGNAL(filenameChanged(const QString &)), switcherSession, SLOT(updateAndSelectFile(const QString&)));
    QObject::connect(switcherSession, SIGNAL(sessionRenamed(QString, QString)), this, SLOT(renameSessionFile(QString, QString)) );

    QAction *nextSession = new QAction("Next Session", this);
    nextSession->setShortcut(QKeySequence("Ctrl+Right"));
    addAction(nextSession);
    QAction *prevSession = new QAction("Previous Session", this);
    prevSession->setShortcut(QKeySequence("Ctrl+Left"));
    addAction(prevSession);

    QObject::connect(nextSession, SIGNAL(triggered()), SLOT(openNextSession()));
    QObject::connect(prevSession, SIGNAL(triggered()), SLOT(openPreviousSession()));
    QObject::connect(OutputRenderWindow::getInstance(), SIGNAL(keyRightPressed()), SLOT(openNextSession()));
    QObject::connect(OutputRenderWindow::getInstance(), SIGNAL(keyLeftPressed()), SLOT(openPreviousSession()));

#else
    delete switcherDockWidget;
#endif


    // Setup dialogs
    mfd = new VideoFileDialog(this, "GLMixer - Open videos or pictures");
    Q_CHECK_PTR(mfd);
    sfd = new QFileDialog(this);
    Q_CHECK_PTR(sfd);
    sfd->setOption(QFileDialog::DontUseNativeDialog, true);
    sfd->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot);

    upd = new UserPreferencesDialog(this);
    Q_CHECK_PTR(upd);

    // Actions for custom display tools and modes
    QObject::connect(actionPerformanceMode, SIGNAL(triggered(bool)), SLOT(setPerformanceModeEnabled(bool)));
    QObject::connect(actionShowTimers, SIGNAL(triggered(bool)), SLOT(setDisplayTimeEnabled(bool)));

    // Create output preview widget
    outputpreview = new OutputRenderWidget(previewDockWidgetContents, mainRendering);
    Q_CHECK_PTR(outputpreview);
    previewDockWidgetContentsLayout->insertWidget(0, outputpreview);
    QObject::connect(RenderingManager::getInstance(), SIGNAL(frameBufferChanged()), outputpreview, SLOT(refresh()));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), outputpreview, SLOT(displayRecordingLabel(bool)));
    QObject::connect(actionPause, SIGNAL(toggled(bool)), outputpreview, SLOT(displayInformationLabel(bool)));

    // Default state without source selected
    vcontrolDockWidgetContents->setEnabled(false);
    sourceDockWidgetContents->setEnabled(false);

    // signals for source management with RenderingManager
    QObject::connect(RenderingManager::getInstance(), SIGNAL(currentSourceChanged(SourceSet::iterator)), this, SLOT(connectSource(SourceSet::iterator) ) );

    // QUIT event
    QObject::connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(actionAbout_Qt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

    // Rendering control
    QObject::connect(OutputRenderWindow::getInstance(), SIGNAL(toggleFullscreen(bool)), actionFullscreen, SLOT(setChecked(bool)) );
    QObject::connect(actionFullscreen, SIGNAL(toggled(bool)), OutputRenderWindow::getInstance(), SLOT(setFullScreen(bool)));
//    QObject::connect(actionFullscreen, SIGNAL(toggled(bool)), RenderingManager::getInstance(), SLOT(disableProgressBars(bool)));
    QObject::connect(actionPause, SIGNAL(toggled(bool)), RenderingManager::getInstance(), SLOT(pause(bool)));

#ifdef GLM_UNDO

    // connect actions to Undo Manager
    QObject::connect(actionUndo, SIGNAL(triggered()), UndoManager::getInstance(), SLOT(undo()));
    QObject::connect(actionRedo, SIGNAL(triggered()), UndoManager::getInstance(), SLOT(redo()));

    // Connect events to follow up when saving and need for saving
    QObject::connect(this, SIGNAL(sessionLoaded()), UndoManager::getInstance(), SLOT(save()), Qt::UniqueConnection);
    QObject::connect(UndoManager::getInstance(), SIGNAL(changed()), this, SLOT(sessionChanged()), Qt::UniqueConnection);

    // connect undo manager to menu item changer
    QObject::connect(UndoManager::getInstance(), SIGNAL(currentChanged(bool,bool)), this, SLOT(undoChanged(bool,bool)));
    actionUndo->setEnabled(false);
    actionRedo->setEnabled(false);

    // suspend the undo manager during continuous mouse mouvements in View
    QObject::connect(RenderingManager::getRenderingWidget(), SIGNAL(mousePressed(bool)), UndoManager::getInstance(), SLOT(suspend(bool)), Qt::UniqueConnection);

#else
    delete actionUndo;
    delete actionRedo;
#endif

#ifdef GLM_SHM
    QObject::connect(actionShareToRAM, SIGNAL(toggled(bool)), RenderingManager::getInstance(), SLOT(setFrameSharingEnabled(bool)));
#endif

#ifdef GLM_SPOUT
    QObject::connect(actionShareToSPOUT, SIGNAL(triggered(bool)), RenderingManager::getInstance(), SLOT(setSpoutSharingEnabled(bool)));
    QObject::connect(RenderingManager::getInstance(), SIGNAL(spoutSharingEnabled(bool)), actionShareToSPOUT, SLOT(setChecked(bool)));
#endif
    output_onair->setDefaultAction(actionToggleRenderingVisible);
    output_pause->setDefaultAction(actionPause);
    output_recording_pause->setDefaultAction(actionPause_recording);
    output_aspectratio->setMenu(aspectRatioMenu);
    output_fullscreen->setDefaultAction(actionFullscreen);
    QObject::connect(actionToggleRenderingVisible, SIGNAL(triggered(bool)), RenderingManager::getInstance()->getSessionSwitcher(), SLOT(smoothAlphaTransition(bool)));
    QObject::connect(RenderingManager::getInstance()->getSessionSwitcher(), SIGNAL(alphaChanged(int)), output_alpha, SLOT(setValue(int)));

    // session switching
    QObject::connect(this, SIGNAL(sessionLoaded()), this, SLOT(confirmSessionFileName()));

    // Recording triggers
    QObject::connect(actionRecord, SIGNAL(toggled(bool)), RenderingManager::getRecorder(), SLOT(setActive(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionRecord, SLOT(setChecked(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(status(const QString &, int)), RenderingManager::getRenderingWidget(), SLOT(showMessage(const QString &, int)));
    QObject::connect(actionRecord, SIGNAL(toggled(bool)), actionPause_recording, SLOT(setEnabled(bool)));
    QObject::connect(actionPause_recording, SIGNAL(toggled(bool)), actionRecord, SLOT(setDisabled(bool)));
    QObject::connect(actionPause_recording, SIGNAL(toggled(bool)), RenderingManager::getRecorder(), SLOT(setPaused(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(processing(bool)), actionRecord, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(processing(bool)), this, SLOT(setBusy(bool)));

    // connect recorder to disable many actions, like quitting, opening session, preferences, etc.
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionNew_Session, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionClose_Session, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionLoad_Session, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionRecent_session, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionQuit, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), actionPreferences, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), RenderingManager::getInstance(), SLOT(lockAspectRatio(bool)));
    QObject::connect(RenderingManager::getInstance(), SIGNAL(aspectRatioLocked(bool)), aspectRatioMenu, SLOT(setDisabled(bool)));
    QObject::connect(RenderingManager::getInstance(), SIGNAL(aspectRatioLocked(bool)), output_aspectratio, SLOT(setDisabled(bool)));

    // do not allow to record without a fixed aspect ratio
    QObject::connect(actionFree_aspect_ratio, SIGNAL(toggled(bool)), actionRecord, SLOT(setDisabled(bool)));

#ifdef GLM_SESSION
    QObject::connect(RenderingManager::getInstance(), SIGNAL(aspectRatioLocked(bool)), switcherSession, SLOT(enableOnlyRenderingAspectRatio(bool)));
#endif

    // group the menu items of the catalog sizes ;
    QActionGroup *catalogActionGroup = new QActionGroup(this);
    catalogActionGroup->addAction(actionCatalogSmall);
    catalogActionGroup->addAction(actionCatalogMedium);
    catalogActionGroup->addAction(actionCatalogLarge);
    QObject::connect(actionCatalogSmall, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(setCatalogSizeSmall()));
    QObject::connect(actionCatalogMedium, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(setCatalogSizeMedium()));
    QObject::connect(actionCatalogLarge, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(setCatalogSizeLarge()));

    // Signals between GUI and rendering widget
    QObject::connect(actionShow_Catalog, SIGNAL(toggled(bool)), RenderingManager::getRenderingWidget(), SLOT(setCatalogVisible(bool)));
    QObject::connect(actionWhite_background, SIGNAL(toggled(bool)), RenderingManager::getInstance(), SLOT(setClearToWhite(bool)));
    QObject::connect(sliderZoom, SIGNAL(valueChanged(int)), RenderingManager::getRenderingWidget(), SLOT(zoom(int)));
    QObject::connect(RenderingManager::getRenderingWidget(), SIGNAL(zoomPercentChanged(int)), sliderZoom, SLOT(setValue(int)));

    QObject::connect(actionZoomIn, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(zoomIn()));
    QObject::connect(actionZoomOut, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(zoomOut()));
    QObject::connect(actionZoomReset, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(zoomReset()));
    QObject::connect(actionZoomBestFit, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(zoomBestFit()));
    QObject::connect(actionZoomCurrentSource, SIGNAL(triggered()), RenderingManager::getRenderingWidget(), SLOT(zoomCurrentSource()));
    QObject::connect(actionAspectRatioResetOriginal, SIGNAL(triggered()), RenderingManager::getInstance(), SLOT(setOriginalAspectRatioCurrentSource()));
    QObject::connect(actionAspectRatioSetRendering, SIGNAL(triggered()), RenderingManager::getInstance(), SLOT(setRenderingAspectRatioCurrentSource()));
    QObject::connect(actionAspectRatioFixed , SIGNAL(triggered(bool)), RenderingManager::getInstance(), SLOT(toggleFixAspectRatioCurrentSource(bool)));
    QObject::connect(actionResetSource, SIGNAL(triggered()), RenderingManager::getInstance(), SLOT(resetCurrentSource()));
    actionCloneSource->setEnabled(false);

    // Signals between cursors and their configuration gui
    QObject::connect(dynamic_cast<LineCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_LINE)), SIGNAL(speedChanged(int)), cursorLineSpeed, SLOT(setValue(int)) );
    QObject::connect(cursorLineSpeed, SIGNAL(valueChanged(int)), dynamic_cast<LineCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_LINE)), SLOT(setSpeed(int)) );
    QObject::connect(cursorLineWaitDuration, SIGNAL(valueChanged(double)), dynamic_cast<LineCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_LINE)), SLOT(setWaitTime(double)) );
    QObject::connect(dynamic_cast<DelayCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_DELAY)), SIGNAL(latencyChanged(double)), cursorDelayLatency, SLOT(setValue(double)) );
    QObject::connect(cursorDelayLatency, SIGNAL(valueChanged(double)), dynamic_cast<DelayCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_DELAY)), SLOT(setLatency(double)) );
    QObject::connect(cursorDelayFiltering, SIGNAL(valueChanged(int)), dynamic_cast<DelayCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_DELAY)), SLOT(setFiltering(int)) );
    QObject::connect(dynamic_cast<SpringCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_SPRING)), SIGNAL(massChanged(int)), cursorSpringMass, SLOT(setValue(int)) );
    QObject::connect(cursorSpringMass, SIGNAL(valueChanged(int)), dynamic_cast<SpringCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_SPRING)), SLOT(setMass(int)) );
    QObject::connect(dynamic_cast<FuzzyCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_FUZZY)), SIGNAL(radiusChanged(int)), cursorFuzzyRadius, SLOT(setValue(int)) );
    QObject::connect(cursorFuzzyRadius, SIGNAL(valueChanged(int)), dynamic_cast<FuzzyCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_FUZZY)), SLOT(setRadius(int)) );
    QObject::connect(cursorFuzzyFiltering, SIGNAL(valueChanged(int)), dynamic_cast<FuzzyCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_FUZZY)), SLOT(setFiltering(int)) );
    QObject::connect(dynamic_cast<MagnetCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_MAGNET)), SIGNAL(radiusChanged(int)), cursorMagnetRadius, SLOT(setValue(int)) );
    QObject::connect(cursorMagnetRadius, SIGNAL(valueChanged(int)), dynamic_cast<MagnetCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_MAGNET)), SLOT(setRadius(int)) );
    QObject::connect(cursorMagnetStrength, SIGNAL(valueChanged(double)), dynamic_cast<MagnetCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_MAGNET)), SLOT(setStrength(double)) );
    QObject::connect(cursorAxisScroll, SIGNAL(toggled(bool)), dynamic_cast<AxisCursor*>(RenderingManager::getRenderingWidget()->getCursor(ViewRenderWidget::CURSOR_AXIS)), SLOT(setScrollingEnabled(bool)) );

    connect(resetElastic, SIGNAL(clicked()), SLOT(resetCurrentCursor()));
    connect(resetDelay, SIGNAL(clicked()), SLOT(resetCurrentCursor()));
    connect(resetLine, SIGNAL(clicked()), SLOT(resetCurrentCursor()));
    connect(resetFuzzy, SIGNAL(clicked()), SLOT(resetCurrentCursor()));
    connect(resetMagnet, SIGNAL(clicked()), SLOT(resetCurrentCursor()));

    // connect actions with selectionManager
    buttonSelectionPlay->setDefaultAction(actionSourcePlay);
    buttonSelectionPause->setDefaultAction(actionSourcePause);
    buttonSelectionRestart->setDefaultAction(actionSourceRestart);
    buttonSelectionSeekBackward->setDefaultAction(actionSourceSeekBackward);
    buttonSelectionSeekForward->setDefaultAction(actionSourceSeekForward);
    QObject::connect(actionSelectAll, SIGNAL(triggered()), SelectionManager::getInstance(), SLOT(selectAll()));
    QObject::connect(actionSelectInvert, SIGNAL(triggered()), SelectionManager::getInstance(), SLOT(invertSelection()));
    QObject::connect(actionSelectCurrent, SIGNAL(triggered()), SelectionManager::getInstance(), SLOT(selectCurrentSource()));
    QObject::connect(actionSelectNone, SIGNAL(triggered()), SelectionManager::getInstance(), SLOT(clearSelection()));

    // connect selection manager with GUI
    updateStatusControlActions();
    QObject::connect(SelectionManager::getInstance(), SIGNAL(selectionChanged(bool)), this, SLOT(updateStatusControlActions()));
    QObject::connect(SelectionManager::getInstance(), SIGNAL(selectionChanged(bool)), actionSelectInvert, SLOT(setEnabled(bool)));
    QObject::connect(SelectionManager::getInstance(), SIGNAL(selectionChanged(bool)), actionSelectNone, SLOT(setEnabled(bool)));

    // Setup the timeline
    timeLineEdit->setFont( QFont(getMonospaceFont(), 9) );
    timeline->setLabelFont(getMonospaceFont(), 7);
    QObject::connect(markInButton, SIGNAL(clicked()), timeline, SLOT(setBeginToCurrent()));
    QObject::connect(markOutButton, SIGNAL(clicked()), timeline, SLOT(setEndToCurrent()));
    QObject::connect(resetMarkInButton, SIGNAL(clicked()), timeline, SLOT(resetBegin()));
    QObject::connect(resetMarkOutButton, SIGNAL(clicked()), timeline, SLOT(resetEnd()));
    frameForwardButton->setHidden(true);

    // start with new file
    currentSessionFileName = QString::null;
    confirmSessionFileName();

    // activate clipboard
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(CliboardDataChanged()));

    CliboardDataChanged();

}

GLMixer::~GLMixer()
{
    delete mfd;
    delete sfd;
    delete upd;
    delete mixingToolBox;
    delete outputpreview;

#ifdef GLM_SESSION
    delete switcherSession;
#endif
#ifdef GLM_TAG
    delete tagsManager;
#endif
#ifdef GLM_HISTORY
    delete actionHistoryView;
#endif

    // last the settings
    delete _settings;
}


void GLMixer::closeEvent(QCloseEvent * event ){

    if (_saveExitSession && !currentSessionFileName.isEmpty() && maybeSave) {
        saveSession(false, true);
        event->ignore();
    }
//    else if (currentSessionFileName.isEmpty() && maybeSave) {
//        on_actionClose_Session_triggered();
//        event->ignore();
//    }
    else {

        mixingToolBox->close();
        outputpreview->close();
        layoutToolBox->close();
#ifdef GLM_FFGL
        pluginGLSLCodeEditor->close();
#endif
#ifdef GLM_SESSION
        switcherSession->close();
#endif
#ifdef GLM_TAG
        tagsManager->close();
#endif
#ifdef GLM_HISTORY
        actionHistoryView->close();
#endif

        QApplication::closeAllWindows();
        event->accept();
    }
}


void GLMixer::keyPressEvent(QKeyEvent * event) {

    Qt::KeyboardModifiers m = event->modifiers();

    // React to Key pressed for numerical keys [0..9]
    if ( (event->modifiers() & Qt::ControlModifier) &&  event->key() > Qt::Key_Slash && event->key() < Qt::Key_Colon) {
        emit keyPressed((int) event->key() - (int) Qt::Key_0, true);
        event->accept();
    }
    else
        QMainWindow::keyPressEvent(event);

}


void GLMixer::keyReleaseEvent(QKeyEvent * event) {

    // React to Key release for numerical keys [0..9]
    if ( (event->modifiers() & Qt::ControlModifier) &&  event->key() > Qt::Key_Slash && event->key() < Qt::Key_Colon) {
        emit keyPressed((int) event->key() - (int) Qt::Key_0, false);
        event->accept();
    }
    else
        QMainWindow::keyReleaseEvent(event);

}

void GLMixer::on_actionFormats_and_Codecs_triggered(){

    CodecManager::displayFormatsCodecsInformation(QString::fromUtf8(":/glmixer/icons/video.png"));
}

void GLMixer::on_actionOpenGL_extensions_triggered(){

    glRenderWidget::showGlExtensionsInformationDialog(QString::fromUtf8(":/glmixer/icons/display.png"));
}


void GLMixer::on_copyNotes_clicked() {

    QApplication::clipboard()->setText( blocNoteEdit->toPlainText() );
}

void GLMixer::on_addDateToNotes_clicked() {

    blocNoteEdit->append(QDateTime::currentDateTime().toString("dddd d MMMM yyyy"));
}

void GLMixer::on_addListToNotes_clicked() {

    QStringList list;

    for(SourceSet::iterator  its = RenderingManager::getInstance()->getBegin(); its != RenderingManager::getInstance()->getEnd(); its++) {
        list << QString("* ") + (*its)->getInfo();
    }

    blocNoteEdit->append(list.join("\n\n"));
}


void GLMixer::on_timeLineEdit_clicked() {

    // apply action to current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs) && currentVideoFile ) {

        double time = currentVideoFile->getCurrentFrameTime();
        double min = currentVideoFile->getMarkIn();
        double max = currentVideoFile->getMarkOut();
        double fps = currentVideoFile->getFrameRate();

        TimeInputDialog tid(this, time, min, max, fps, _displayTimeAsFrame);
        if (tid.exec() == QDialog::Accepted) {
            currentVideoFile->seekToPosition(tid.getTime());
        }
    }
}

#ifdef GLM_LOGS

void GLMixer::saveLogsToFile() {

    if (GLMixer::logFile) {

        QString suggestion = QFileInfo(GLMixer::logFile->fileName()).fileName() ;
        QString fileName = getFileName(tr("Save Logs to file"),
                                       tr("GLMixer log file") + " (*.txt)",
                                       QString("txt"),
                                       QFileInfo( QDir::home(), suggestion).absoluteFilePath() );

        // copy the temp log file to the new name (if filename is valid)
        if (!fileName.isEmpty()) {
            // delete previous file if exists
            QFileInfo newfile(fileName);
            if (newfile.exists())
                newfile.dir().remove( newfile.fileName() );

            // close the logfile and copy it to the new name
            GLMixer::logFile->close();
            GLMixer::logFile->copy(newfile.absoluteFilePath());
            // re-open the log file (without removing its content)
            GLMixer::logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);

        }
    }
}

void GLMixer::exitHandler() {

    // cleanup logs on exit (if log file valid)
    if ( GLMixer::logFile ) {

        GLMixer::logStream << QApplication::applicationName() << " closed , " << QTime::currentTime().toString() << "\n";

        // close properly
        GLMixer::logStream.setDevice(0);

        // remove log file (avoid overpopulating temp dir with too many log files)
        GLMixer::logFile->remove();

        // done with log file
        delete GLMixer::logFile;
        GLMixer::logFile = 0;
    }

}


void GLMixer::msgHandler(QtMsgType type, const char *msg)
{
    // static log stream : open file if the stream is not open for writing
    if ( GLMixer::logFile == 0 ) {

        // create unique file name
        GLMixer::logFile = new QFile( GLMixerApp::getLogFileName() );
        // open the log file (create it if didnt exist, removing its content otherwise)
        GLMixer::logFile->open(QIODevice::WriteOnly | QIODevice::Text);
        // attach stream
        GLMixer::logStream.setDevice(GLMixer::logFile);

        // write log info header
#ifdef GLMIXER_REVISION
        GLMixer::logStream << QApplication::applicationName() << " v" << QApplication::applicationVersion() << " r"<< GLMIXER_REVISION << " , " << QDate::currentDate().toString() << "  " << QTime::currentTime().toString() << "\n";
#else
        GLMixer::logStream << QApplication::applicationName() << " v" << QApplication::applicationVersion() << " , " << QDate::currentDate().toString() << "  " << QTime::currentTime().toString() << "\n";
#endif
    }

    QString txt = QString(msg).remove("\"");
    // message handler
    switch (type) {
    case QtCriticalMsg:
    {
        // write message
        GLMixer::logStream << "Critical| " << txt << "\n";
        GLMixer::logStream.flush();

        static bool ignore = false;
        if (!ignore) {

            // create message box
            QMessageBox msgBox(QMessageBox::Warning, tr("Warning"),  tr("<b>The application %1 encountered a problem.</b>").arg(QCoreApplication::applicationName()), QMessageBox::Ok);
            QStringList message = txt.split(QChar(124), QString::SkipEmptyParts);
            QString displaytext = txt;
            if (message.count() > 1 ) {
                displaytext = message[1].simplified();
                if ( !message[0].simplified().isEmpty() )
                    displaytext += tr("\n\nOrigin of the problem:\n") + message[0].simplified();
            } else if (message.count() > 0 )
                displaytext = message[0].simplified();

            // add buttons with more actions
            QPushButton *ignoreButton = NULL;
            ignoreButton = msgBox.addButton(tr("Ignore warnings"), QMessageBox::ActionRole);
            QPushButton *logButton = NULL;
            if (GLMixer::logsWidget)
                logButton = msgBox.addButton(tr("Check logs"), QMessageBox::ActionRole);

            // exec message box
            msgBox.setInformativeText(displaytext);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();

            // show logs if required
            if (GLMixer::logsWidget && msgBox.clickedButton() == logButton ){
                GLMixer::logsWidget->show();
                GLMixer::logsWidget->raise();
                GLMixer::logsWidget->setFocus();
            }
            // set ignore if required
            if ( msgBox.clickedButton() == ignoreButton )
                ignore = true;

        }

    }
        break;
    case QtFatalMsg:
    {
        // write message
        GLMixer::logStream << "Error   | " << txt << "\n";
        GLMixer::logStream.flush();

        QMessageBox msgBox(QMessageBox::Warning, tr("Error"), tr("<b>The application %1 encountered an error.</b>").arg(QCoreApplication::applicationName()), QMessageBox::Ok);
        // add button to show logs
        QPushButton *logButton = msgBox.addButton(tr("Open log file"), QMessageBox::ActionRole);

        msgBox.setInformativeText(txt.simplified() + tr("\n\nThe program will stop now. Logs have been saved in %1").arg(QDir::tempPath()));
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        // show logs if required
        if ( msgBox.clickedButton() == logButton )
            QDesktopServices::openUrl( QUrl::fromLocalFile( GLMixerApp::getLogFileName() ) );

        // exit properly (it is not a crash)
        exit(0);
    }
        break;

    case QtWarningMsg:
        // write message
        GLMixer::logStream << "Warning | " << txt << "\n";
        GLMixer::logStream.flush();
        break;

    default:
        // write message
        GLMixer::logStream << "Info    | " << txt << "\n";
        GLMixer::logStream.flush();
        break;
    }

    // forward message to logger
    if (GLMixer::logsWidget) {
        // invoke a delayed call (in Qt event loop) of the GLMixer real Message handler SLOT
        static int methodIndex = GLMixer::logsWidget->metaObject()->indexOfSlot("Log(int,QString)");
        static QMetaMethod method = GLMixer::logsWidget->metaObject()->method(methodIndex);
        method.invoke(GLMixer::logsWidget, Qt::QueuedConnection, Q_ARG(int, (int)type), Q_ARG(QString, txt));
    }

}

#endif

void GLMixer::setView(QAction *a){

#ifdef GLM_SNAPSHOT
    RenderingManager::getRenderingWidget()->activateSnapshot();
#endif

    // setup the rendering Widget to the requested view
    View::viewMode view = (View::viewMode) a->data().toInt();
    RenderingManager::getRenderingWidget()->setViewMode(view);
    layoutToolBox->setViewMode(view);

    // show appropriate icon
    viewIcon->setPixmap(RenderingManager::getRenderingWidget()->getView()->getIcon());
    viewLabel->setText(RenderingManager::getRenderingWidget()->getView()->getTitle());

    // tools available in corresponding views
    toolsToolBar->setEnabled(view != View::RENDERING);
    actionToolScale->setEnabled( view != View::LAYER);
    actionToolRotate->setEnabled( view != View::LAYER);
    menuAspect_Ratio->menuAction()->setVisible(view == View::GEOMETRY);

    // get back the proper tool from former usage
    switch ( RenderingManager::getRenderingWidget()->getToolMode() ){
    case ViewRenderWidget::TOOL_SCALE:
        actionToolScale->trigger();
        break;
    case ViewRenderWidget::TOOL_ROTATE:
        actionToolRotate->trigger();
        break;
    case ViewRenderWidget::TOOL_CUT:
        actionToolCut->trigger();
        break;
    default:
    case ViewRenderWidget::TOOL_GRAB:
        actionToolGrab->trigger();
        break;
    }

}

void GLMixer::setTool(QAction *a){

    RenderingManager::getRenderingWidget()->setToolMode( (ViewRenderWidget::toolMode) a->data().toInt() );
}

void GLMixer::setCursor(QAction *a){

    RenderingManager::getRenderingWidget()->setCursorMode( (ViewRenderWidget::cursorMode) a->data().toInt() );
    cursorOptionWidget->setCurrentIndex(a->data().toInt());
}

void GLMixer::on_actionNewSource_triggered(){

    static NewSourceDialog *nsd = new NewSourceDialog(this);

    if (nsd->exec() == QDialog::Accepted) {
        newSource(nsd->selectedType());
    }
}

void GLMixer::newSource(Source::RTTI type) {

    switch (type) {
    case Source::VIDEO_SOURCE:
        on_actionMediaSource_triggered();
        break;
    case Source::BASKET_SOURCE:
        on_actionBasketSource_triggered();
        break;
    case Source::OPENCV_SOURCE:
        on_actionCameraSource_triggered();
        break;
    case Source::ALGORITHM_SOURCE:
        on_actionAlgorithmSource_triggered();
        break;
    case Source::RENDERING_SOURCE:
        on_actionRenderingSource_triggered();
        break;
    case Source::CAPTURE_SOURCE:
        on_actionCaptureSource_triggered();
        break;
    case Source::SVG_SOURCE:
        on_actionSvgSource_triggered();
        break;
#ifdef GLM_SHM
    case Source::SHM_SOURCE:
        on_actionShmSource_triggered();
        break;
#endif
#ifdef GLM_FFGL
    case Source::FFGL_SOURCE:
        on_actionFreeframeSource_triggered();
        break;
#endif
    case Source::WEB_SOURCE:
        on_actionWebSource_triggered();
        break;
    case Source::STREAM_SOURCE:
        on_actionStreamSource_triggered();
        break;
    default:
        break;
    }

}


void GLMixer::on_actionMediaSource_triggered(){

    bool smartScale = false;
    bool hwDecoding = false;
    QStringList fileNames = getMediaFileNames(smartScale, hwDecoding);

    // open all files from the list
    QStringListIterator fileNamesIt(fileNames);
    setBusy(true);
    while (fileNamesIt.hasNext()){

        QCoreApplication::processEvents();

        VideoFile *newSourceVideoFile = NULL;
        QString filename = fileNamesIt.next();

        if ( !filename.isEmpty() && QFileInfo(filename).isFile() ){

            // if the dialog did not request recomputation of texture size
            // and if the opengl supports the non power of two textures, then open the source normally
            if ( !smartScale && (glewIsSupported("GL_EXT_texture_non_power_of_two") || glewIsSupported("GL_ARB_texture_non_power_of_two") ) )
                newSourceVideoFile = new VideoFile(this);
            else
                newSourceVideoFile = new VideoFile(this, true, RenderingManager::getInstance()->getFrameBufferWidth(), RenderingManager::getInstance()->getFrameBufferHeight());

            // if the video file was created successfully
            if (newSourceVideoFile){
                // can we open the file ?
                if ( newSourceVideoFile->open( filename, hwDecoding ) ) {
                    Source *s = RenderingManager::getInstance()->newMediaSource(newSourceVideoFile);
                    // create the source as it is a valid video file (this also set it to be the current source)
                    if ( s ) {
                        RenderingManager::getInstance()->addSourceToBasket(s);
                        qDebug() << s->getName() << QChar(124).toLatin1() << tr("New media source created with file ") << filename;
                    } else {
                        qCritical() << filename <<  QChar(124).toLatin1() << tr("Could not create media source with this file.");
                        delete newSourceVideoFile;
                    }
                } else {
                    qCritical() << filename << QChar(124).toLatin1() << tr("The file could not be loaded.");
                    delete newSourceVideoFile;
                }
            }
            else
                qCritical() << filename << QChar(124).toLatin1() << tr("The file could not be opened.");

        }

    }
    // done
    setBusy(false);

    if (RenderingManager::getInstance()->getSourceBasketSize() > 0)
        emit status( tr("%1 media source(s) created; you can now drop them.").arg( RenderingManager::getInstance()->getSourceBasketSize() ), 3000 );
}


// method called when a source is made current (either after loading, either after clicking on it).
// The goal is to have the GUI display the current state of the video file to be able to control the video playback
// and to read the correct information and configuration options
void GLMixer::connectSource(SourceSet::iterator csi){

    if (specificSourcePropertyBrowser) {
        _settings->setValue("layoutPropertyBrowserState", layoutPropertyBrowser->saveState() );
        delete specificSourcePropertyBrowser;
        specificSourcePropertyBrowser = NULL;
    }

    infobar->setText("");

    // whatever happens, we will drop the control on the current source
    //   (this slot is called by MainRenderWidget through signal currentSourceChanged
    //    which is sent ONLY when the current source is changed)
    if (currentVideoFile) {

//        QObject::disconnect(selectedSourceVideoFile, SIGNAL(running(bool)), startButton, SLOT(setChecked(bool)));
        QObject::disconnect(currentVideoFile, SIGNAL(paused(bool)), pauseButton, SLOT(setChecked(bool)));
        QObject::disconnect(currentVideoFile, SIGNAL(running(bool)), videoFrame, SLOT(setEnabled(bool)));
        QObject::disconnect(currentVideoFile, SIGNAL(running(bool)), timeline, SLOT(setEnabled(bool)));
        QObject::disconnect(currentVideoFile, SIGNAL(running(bool)), timingControlFrame, SLOT(setEnabled(bool)));
        QObject::disconnect(currentVideoFile, SIGNAL(playSpeedChanged(double)), playSpeedDisplay, SLOT(display(double)) );
        QObject::disconnect(currentVideoFile, SIGNAL(playSpeedFactorChanged(int)), playSpeedSlider, SLOT(setValue(int)) );
        QObject::disconnect(currentVideoFile, SIGNAL(seekEnabled(bool)), this, SLOT(enableSeek(bool)));

        // timeline
        QObject::disconnect(currentVideoFile, SIGNAL(timeChanged(double)), timeline, SLOT(setValue(double)));
        QObject::disconnect(currentVideoFile, SIGNAL(playSpeedChanged(double)), timeline, SLOT(setSpeed(double)));
        QObject::disconnect(currentVideoFile, SIGNAL(markInChanged(double)), timeline, SLOT(setBegin(double)));
        QObject::disconnect(currentVideoFile, SIGNAL(markOutChanged(double)), timeline, SLOT(setEnd(double)));
        QObject::disconnect(currentVideoFile, SIGNAL(fadeInChanged(double)), timeline, SLOT(setFadein(double)));
        QObject::disconnect(currentVideoFile, SIGNAL(fadeOutChanged(double)), timeline, SLOT(setFadeout(double)));
        QObject::disconnect(currentVideoFile, SIGNAL(timeChanged(double)), this, SLOT(refreshTiming()));
        QObject::disconnect(timeline, SIGNAL(beginChanged(double)), currentVideoFile, SLOT(setMarkIn(double)) );
        QObject::disconnect(timeline, SIGNAL(endChanged(double)), currentVideoFile, SLOT(setMarkOut(double)) );
        QObject::disconnect(timeline, SIGNAL(fadeinChanged(double)), currentVideoFile, SLOT(setFadeIn(double)) );
        QObject::disconnect(timeline, SIGNAL(fadeoutChanged(double)), currentVideoFile, SLOT(setFadeOut(double)) );
        QObject::disconnect(timeline, SIGNAL(valueRequested(double)), currentVideoFile, SLOT(seekToPosition(double)) );
        timeline->reset();

        // disconnect control buttons
        QObject::disconnect(pauseButton, SIGNAL(toggled(bool)), currentVideoFile, SLOT(pause(bool)));
        QObject::disconnect(playSpeedSlider, SIGNAL(valueChanged(int)), currentVideoFile, SLOT(setPlaySpeedFactor(int)));
        QObject::disconnect(playSpeedReset, SIGNAL(clicked()), currentVideoFile, SLOT(resetPlaySpeed()));
        QObject::disconnect(resetToBlackCheckBox, SIGNAL(toggled(bool)), currentVideoFile, SLOT(setOptionRevertToBlackWhenStop(bool)));
        QObject::disconnect(restartWhereStoppedCheckBox, SIGNAL(toggled(bool)), currentVideoFile, SLOT(setOptionRestartToMarkIn(bool)));
        QObject::disconnect(seekBackwardButton, SIGNAL(clicked()), currentVideoFile, SLOT(seekBackward()));
        QObject::disconnect(seekForwardButton, SIGNAL(clicked()), currentVideoFile, SLOT(seekForward()));
        QObject::disconnect(seekBeginButton, SIGNAL(clicked()), currentVideoFile, SLOT(seekBegin()));
        QObject::disconnect(videoLoopButton, SIGNAL(toggled(bool)), currentVideoFile, SLOT(setLoop(bool)));

        // clear video file control
        currentVideoFile = NULL;
    }

    if (previousSource) {
        previousSource->disconnect(startButton);
        previousSource->disconnect(mixingToolBox);
        previousSource = NULL;
    }
    QObject::disconnect(startButton, SIGNAL(toggled(bool)), this, SLOT(startButton_toogled(bool)));

    // in any case uncheck pause and loop buttons
    pauseButton->setChecked( false );
    videoLoopButton->setChecked( false );

    // if we are given a valid iterator, we have a source to control
    if ( RenderingManager::getInstance()->isValid(csi) ) {

        // remember reference to source
        previousSource = (*csi);

        // display source preview
        QString infotext = tr("Current Source :") + (*csi)->getInfo();
        infotext = infobar->fontMetrics().elidedText(infotext, Qt::ElideRight, statusBar()->width() - 30);
        infobar->setText( infotext );
        sourcePreview->setSource(*csi);
        vcontrolDockWidgetContents->setEnabled( true );

        // display property browser
        specificSourcePropertyBrowser = SourcePropertyBrowser::createSpecificPropertyBrowser((*csi), layoutPropertyBrowser);
        specificSourcePropertyBrowser->setHeaderVisible(false);
        layoutPropertyBrowser->insertWidget(1, specificSourcePropertyBrowser);
        layoutPropertyBrowser->restoreState(_settings->value("layoutPropertyBrowserState").toByteArray());
        specificSourcePropertyBrowser->connectToPropertyTree(RenderingManager::getPropertyBrowserWidget());

        // enable properties and actions on the current valid source
        sourceDockWidgetContents->setEnabled(true);
        currentSourceMenu->setEnabled(true);
        actionCloneSource->setEnabled(true);
        toolButtonZoomCurrent->setEnabled(true);
        actionAspectRatioFixed->setChecked( (*csi)->isFixedAspectRatio() );

        // Enable control pannels if the source is playable
        vcontrolOptionSplitter->setEnabled( (*csi)->isPlayable() );
        QList<int> splitSizes = vcontrolOptionSplitter->sizes();
        splitSizes[0] += splitSizes[1];
        splitSizes[1] = 0;
        vcontrolOptionSplitter->setSizes(splitSizes);

        // except for media source, these panels are disabled
        vcontrolDockWidgetOptions->setEnabled(false);
        videoFrame->setEnabled(false);
        timingControlFrame->setEnabled(false);
        timeline->setEnabled(false);

        // check the menu action of the current source
        WorkspaceManager::getInstance()->getSourceActions()[(*csi)->getWorkspace()]->setChecked(true);

        // default disable source manipulation tools
        startButton->setEnabled(false);
        startButton->setChecked( false );

        // status of mixing toolbox is associated to stanby mode
        mixingToolBox->setEnabled(!(*csi)->isStandby());
        QObject::connect((*csi), SIGNAL(standingby(bool)), mixingToolBox, SLOT(setDisabled(bool)));

        // Among playable sources, there is the particular case of video sources :
        if ((*csi)->isPlayable()) {

            // Set the status of start button
            startButton->setChecked( (*csi)->isPlaying() );
            QObject::connect(startButton, SIGNAL(toggled(bool)), this, SLOT(startButton_toogled(bool)));

            // status of start button is associated to stanby mode
            startButton->setEnabled(!(*csi)->isStandby());
            QObject::connect((*csi), SIGNAL(standingby(bool)), startButton, SLOT(setDisabled(bool)));

            // connect the start button to the state of source
            QObject::connect((*csi), SIGNAL(playing(bool)), startButton, SLOT(setChecked(bool)));

            if ( (*csi)->rtti() == Source::VIDEO_SOURCE ) {
                // get the pointer to the video to control
                currentVideoFile = (dynamic_cast<VideoSource *>(*csi))->getVideoFile();
                // control this video if it is valid
                if (currentVideoFile){

                    // setup GUI button states to match the current state of the videofile; do it before connecting slots to avoid re-emiting signals
                    vcontrolDockWidgetOptions->setEnabled(true);
                    videoFrame->setEnabled(currentVideoFile->isRunning());
                    timingControlFrame->setEnabled(currentVideoFile->isRunning());
                    timeline->setEnabled(currentVideoFile->isRunning());

                    pauseButton->setChecked( currentVideoFile->isPaused());
                    resetToBlackCheckBox->setChecked(currentVideoFile->getOptionRevertToBlackWhenStop());
                    restartWhereStoppedCheckBox->setChecked(currentVideoFile->getOptionRestartToMarkIn());
                    videoLoopButton->setChecked(currentVideoFile->isLoop());
                    playSpeedSlider->setValue(currentVideoFile->getPlaySpeedFactor());
                    playSpeedDisplay->display(currentVideoFile->getPlaySpeed());

                    // timeline
                    timeline->setMinimum(currentVideoFile->getBegin());
                    timeline->setMaximum(currentVideoFile->getEnd());
                    timeline->setStep(currentVideoFile->getFrameDuration());
                    timeline->setRange(qMakePair(currentVideoFile->getMarkIn(), currentVideoFile->getMarkOut()));
                    timeline->setFading(qMakePair(currentVideoFile->getFadeIn(), currentVideoFile->getFadeOut()));
                    timeline->setValue(currentVideoFile->getCurrentFrameTime());
                    timeline->setSpeed(currentVideoFile->getPlaySpeed());

                    // timeline
                    QObject::connect(currentVideoFile, SIGNAL(timeChanged(double)), timeline, SLOT(setValue(double)));
                    QObject::connect(currentVideoFile, SIGNAL(playSpeedChanged(double)), timeline, SLOT(setSpeed(double)));
                    QObject::connect(currentVideoFile, SIGNAL(markInChanged(double)), timeline, SLOT(setBegin(double)));
                    QObject::connect(currentVideoFile, SIGNAL(markOutChanged(double)), timeline, SLOT(setEnd(double)));
                    QObject::connect(currentVideoFile, SIGNAL(fadeInChanged(double)), timeline, SLOT(setFadein(double)));
                    QObject::connect(currentVideoFile, SIGNAL(fadeOutChanged(double)), timeline, SLOT(setFadeout(double)));
                    QObject::connect(currentVideoFile, SIGNAL(timeChanged(double)), this, SLOT(refreshTiming()));
                    QObject::connect(timeline, SIGNAL(beginChanged(double)), currentVideoFile, SLOT(setMarkIn(double)) );
                    QObject::connect(timeline, SIGNAL(endChanged(double)), currentVideoFile, SLOT(setMarkOut(double)) );
                    QObject::connect(timeline, SIGNAL(fadeinChanged(double)), currentVideoFile, SLOT(setFadeIn(double)) );
                    QObject::connect(timeline, SIGNAL(fadeoutChanged(double)), currentVideoFile, SLOT(setFadeOut(double)) );
                    QObject::connect(timeline, SIGNAL(valueRequested(double)), currentVideoFile, SLOT(seekToPosition(double)) );

                    // CONTROL signals from GUI to VideoFile
                    QObject::connect(pauseButton, SIGNAL(toggled(bool)), currentVideoFile, SLOT(pause(bool)));
                    QObject::connect(playSpeedSlider, SIGNAL(valueChanged(int)), currentVideoFile, SLOT(setPlaySpeedFactor(int)));
                    QObject::connect(playSpeedReset, SIGNAL(clicked()), currentVideoFile, SLOT(resetPlaySpeed()));
                    QObject::connect(resetToBlackCheckBox, SIGNAL(toggled(bool)), currentVideoFile, SLOT(setOptionRevertToBlackWhenStop(bool)));
                    QObject::connect(restartWhereStoppedCheckBox, SIGNAL(toggled(bool)), currentVideoFile, SLOT(setOptionRestartToMarkIn(bool)));
                    QObject::connect(seekBackwardButton, SIGNAL(clicked()), currentVideoFile, SLOT(seekBackward()));
                    QObject::connect(seekForwardButton, SIGNAL(clicked()), currentVideoFile, SLOT(seekForward()));
                    QObject::connect(seekBeginButton, SIGNAL(clicked()), currentVideoFile, SLOT(seekBegin()));
                    QObject::connect(videoLoopButton, SIGNAL(toggled(bool)), currentVideoFile, SLOT(setLoop(bool)));

                    // DISPLAY consistency from VideoFile to GUI
//                    QObject::connect(selectedSourceVideoFile, SIGNAL(running(bool)), startButton, SLOT(setChecked(bool)));
                    QObject::connect(currentVideoFile, SIGNAL(paused(bool)), pauseButton, SLOT(setChecked(bool)));
                    QObject::connect(currentVideoFile, SIGNAL(running(bool)), videoFrame, SLOT(setEnabled(bool)));
                    QObject::connect(currentVideoFile, SIGNAL(running(bool)), timeline, SLOT(setEnabled(bool)));
                    QObject::connect(currentVideoFile, SIGNAL(running(bool)), timingControlFrame, SLOT(setEnabled(bool)));
                    QObject::connect(currentVideoFile, SIGNAL(playSpeedChanged(double)), playSpeedDisplay, SLOT(display(double)) );
                    QObject::connect(currentVideoFile, SIGNAL(playSpeedFactorChanged(int)), playSpeedSlider, SLOT(setValue(int)) );
                    QObject::connect(currentVideoFile, SIGNAL(seekEnabled(bool)), this, SLOT(enableSeek(bool)));

                } // end video file
            } // end video source
        }

        // show information
        emit status( tr("Source %1 selected.").arg( (*csi)->getName() ), 3000 );
    }
    else {  // it is not a valid source

        // disable control panel widgets
        vcontrolDockWidgetContents->setEnabled(false);
        vcontrolOptionSplitter->setEnabled( false );

        // disable source related toolboxes
        sourceDockWidgetContents->setEnabled(false);

        // disable options
        currentSourceMenu->setEnabled(false);
        actionCloneSource->setEnabled(false);
        toolButtonZoomCurrent->setEnabled(false);

        // show information
        RenderingManager::getRenderingWidget()->hideMessage();
    }

    // update gui content from timings
    refreshTiming();
    // update the status (enabled / disabled) of source control actions
    updateStatusControlActions();

}


void GLMixer::on_actionBasketSource_triggered(){

    // // DEBUG HACK
    // bool generatePowerOfTwoRequested = false;
    // QStringList fileNames = getMediaFileNames(generatePowerOfTwoRequested);
    // Source *s = RenderingManager::getInstance()->newBasketSource(fileNames, 640, 480, 50, false, false);
    // RenderingManager::getInstance()->addSourceToBasket(s);

   // popup a dialog to select the stream
   static BasketSelectionDialog *bsd = 0;
   if (!bsd)
       bsd = new BasketSelectionDialog(this, _settings);


   if (bsd->exec() == QDialog::Accepted) {

       QStringList fileNames = bsd->getSelectedFiles();
       int w = bsd->getSelectedWidth();
       int h = bsd->getSelectedHeight();
       int p = bsd->getSelectedPeriod();
       bool b = bsd->getSelectedBidirectional();
       bool s = bsd->getSelectedShuffle();
       QString pl = bsd->getSelectedPlayList();

       if (!fileNames.empty()) {

           Source *bs = RenderingManager::getInstance()->newBasketSource(fileNames, w, h, p, b, s, pl);
           if (bs) {
               RenderingManager::getInstance()->addSourceToBasket(bs);
           } else
               qCritical() << tr("Could not create Basket Source (%1 files).").arg(fileNames.size());
       }
   }
}

void GLMixer::sessionChanged() {

    maybeSave = true;

    QString title = windowTitle().section('*', 0, 0);
    title.append('*');
    setWindowTitle( title );

}

void GLMixer::on_actionCameraSource_triggered() {

    static CameraDialog *cd = 0;
    if (!cd)
        cd = new CameraDialog(this, _settings);

    if (cd->exec() == QDialog::Accepted) {

#ifdef GLM_OPENCV
        if ( cd->isOpencvSelected() )
        {
            int selectedCamIndex = cd->getOpencvIndex();
            if (selectedCamIndex > -1 ) {

                Source *s = RenderingManager::getInstance()->newOpencvSource(selectedCamIndex, 0);
                if ( s ) {
                    RenderingManager::getInstance()->addSourceToBasket(s);

                    CloneSource *cs = dynamic_cast<CloneSource*> (s);
                    if (cs) {
                        qDebug() << s->getName() << QChar(124).toLatin1() << tr("OpenCV device source %1 was cloned.").arg(cs->getOriginalName());
                        emit status( tr("The OpenCV source %1 was cloned.").arg(cs->getOriginalName()), 3000 );
                    } else {
                        qDebug() << s->getName() << QChar(124).toLatin1() << tr("New OpenCV source created (device index %2).").arg(selectedCamIndex);
                        emit status( tr("Source created with OpenCV drivers for Camera %1").arg(selectedCamIndex), 3000 );
                    }
                } else
                    qCritical() << tr("Could not open OpenCV device %1. ").arg(selectedCamIndex);
            }

        } else
#endif
        {
            // create stream
            VideoStream *vs = new VideoStream();

            // create source with this stream
            Source *s = RenderingManager::getInstance()->newStreamSource(vs);

            // trigger openning of the stream
            vs->open(cd->getUrl(), cd->getFormat(), cd->getFormatOptions());

            if ( s ){
                // update source aspect ratio when known
                QObject::connect(vs, SIGNAL(openned()), s, SLOT(updateAspectRatioStream()));

                // prepare for drop
                RenderingManager::getInstance()->addSourceToBasket(s);
                qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New stream device source created (")<< cd->getFormat() << cd->getUrl() <<")";
                emit status( tr("Stream source created with device %1.").arg( cd->getFormat() + cd->getUrl() ), 3000 );
            } else {
                qCritical() << cd->getFormat() + cd->getUrl() <<  QChar(124).toLatin1() << tr("Could not create a stream device source.");
            }

        }
    }
}

void GLMixer::on_actionWebSource_triggered(){

    // popup a question dialog to select the type of algorithm
    static WebSourceCreationDialog *webd = 0;
    if (!webd)
        webd = new WebSourceCreationDialog(this, _settings);


    if (webd->exec() == QDialog::Accepted) {

        int w = webd->getSelectedWidth();
        int h = webd->getSelectedHeight();
        int wh = webd->getSelectedWindowHeight();
        int c = webd->getSelectedScroll();
        int u = webd->getSelectedUpdate();
        QUrl web = webd->getSelectedUrl();

        Source *s = RenderingManager::getInstance()->newWebSource(web, w, h, wh, c, u);
        if ( s ){
            RenderingManager::getInstance()->addSourceToBasket(s);
            qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New Web source created with location ")<< web.toString();
            emit status( tr("Source created with the web location %1.").arg( web.toString() ), 3000 );
        } else
            qCritical() << web.toString() <<  QChar(124).toLatin1() << tr("Could not create a web source with this location.");

    }

}

void GLMixer::on_actionSvgSource_triggered(){

    QString fileName = getFileName(tr("GLMixer - New SVG source"), tr("Scalable Vector Graphics") + " (*.svg)");

    if ( !fileName.isEmpty() ) {

        QFileInfo file(fileName);
        if ( !file.isFile() || !file.isReadable()) {
            qCritical() << fileName <<  QChar(124).toLatin1() << tr("File does not exist or is not readable.");
            return;
        }

        QFile svgfile(fileName);
        if(!svgfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << fileName <<  QChar(124).toLatin1() << tr("Could not open file.");
            return;
        }
        QByteArray data = svgfile.readAll();

        Source *s = RenderingManager::getInstance()->newSvgSource(data);
        if ( s ){
            RenderingManager::getInstance()->addSourceToBasket(s);
            qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New vector Graphics source created with file ")<< fileName;
            emit status( tr("Source created with the vector graphics file %1.").arg( fileName ), 3000 );
        } else
            qCritical() << fileName <<  QChar(124).toLatin1() << tr("Could not create a vector graphics source with this file.");
    }
}


void GLMixer::on_actionShmSource_triggered(){

#ifdef GLM_SHM
    // popup a question dialog to select the shared memory block
    static SharedMemoryDialog *shmd = 0;
    if(!shmd)
        shmd = new SharedMemoryDialog(this);

    if (shmd->exec() == QDialog::Accepted) {
        Source *s = RenderingManager::getInstance()->newSharedMemorySource(shmd->getSelectedId());
        if ( s ){
            RenderingManager::getInstance()->addSourceToBasket(s);
            qDebug() << s->getName() <<  QChar(124).toLatin1() <<  tr("New shared memory source created (")<< shmd->getSelectedProcess() << ").";
            emit status( tr("Source created with the process %1.").arg( shmd->getSelectedProcess() ), 3000 );
        } else
            qCritical() << shmd->getSelectedProcess() <<  QChar(124).toLatin1() << tr("Could not create shared memory source.");
    }
#endif
}

void GLMixer::on_actionStreamSource_triggered(){

    // popup a dialog to select the stream
    static VideoStreamDialog *vsd = 0;
    if (!vsd)
        vsd = new VideoStreamDialog(this, _settings);


    if (vsd->exec() == QDialog::Accepted) {

        // create stream
        VideoStream *vs = new VideoStream();

        // create source with this stream
        Source *s = RenderingManager::getInstance()->newStreamSource(vs);

        // trigger openning of the stream
        vs->open(vsd->getUrl(), vsd->getFormat());

        if ( s ){
            // update source aspect ratio when known
            QObject::connect(vs, SIGNAL(openned()), s, SLOT(updateAspectRatioStream()));

            // prepare for drop
            RenderingManager::getInstance()->addSourceToBasket(s);
            qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New Network Stream source created with URL ")<< vsd->getUrl();
            emit status( tr("Source created with network stream %1.").arg( vsd->getUrl() ), 3000 );
        } else {

            qCritical() << vsd->getUrl() <<  QChar(124).toLatin1() << tr("Could not create a streaming source from this network URL.");
        }

    }
}


void GLMixer::on_actionFreeframeSource_triggered(){

#ifdef GLM_FFGL
    // create dialog on first run
    static FFGLSourceCreationDialog *ffgld = 0;
    if (!ffgld)
        ffgld = new FFGLSourceCreationDialog(this, _settings);

    // popup a question dialog to select the type of algorithm
    if (ffgld->exec() == QDialog::Accepted) {

        int w = ffgld->getSelectedWidth();
        int h = ffgld->getSelectedHeight();
        QDomElement config = ffgld->getFreeframePluginConfiguration();
        if ( config.hasChildNodes() ) {

            Source *s = RenderingManager::getInstance()->newFreeframeGLSource(config, w, h);
            FFGLSource *ps = dynamic_cast<FFGLSource *> (s);

            // successful creation of a FFGLSource ?
            if ( s && ps ){

                // show status with different message depending on type : try to cast
                FFGLPluginSourceShadertoy *st = qobject_cast<FFGLPluginSourceShadertoy *> ( ps->freeframeGLPlugin() );

                // if it is a Shadertoy
                if (st) {

                    // shadertoy info
                    qDebug() << s->getName() << QChar(124).toLatin1() << tr("New Shadertoy GPU plugin source created.");
                    emit status( tr("Shadertoy GPU plugin source %1 created.").arg( s->getName() ), 3000 );

                    // connect the signal from the rendering manager to show the code editor after source drop
                    QObject::connect(RenderingManager::getInstance(), SIGNAL(sourceDropped(Source *)), this, SLOT(editShaderToySource(Source *)) );
                }
                // if it is a Freeframe plugin
                else {
                    // freeframe info (filename)
                    qDebug() << s->getName() << QChar(124).toLatin1() << tr("New Freeframe GPU plugin source created with file ") << ps->freeframeGLPlugin()->fileName() ;
                    emit status( tr("Freeframe GPU plugin source %1 created.").arg( s->getName() ), 3000 );
                }

                // add source
                RenderingManager::getInstance()->addSourceToBasket(s);
            }
            else
                qCritical() << config.tagName() <<  QChar(124).toLatin1() << tr("Could not create GPU plugin source.");
        }
    }

#endif
}



#ifdef GLM_FFGL


void GLMixer::editShaderToySource(Source *s)
{
    // this is a rare need (drop of shader toy) so we avoid to do it for all source dropped
    QObject::disconnect(RenderingManager::getInstance(), SIGNAL(sourceDropped(Source *)), this, SLOT(editShaderToySource(Source *)) );

    //  ignore invalid
    if (!s)
        return;

    // try to convert to freeframe source
    FFGLSource *ps = dynamic_cast<FFGLSource *> (s);
    if ( ps )
        // get the plugin and request to edit code
        editShaderToyPlugin( ps->freeframeGLPlugin() );
        // ( this will test if it is a shadertoy )

}

void GLMixer::editShaderToyPlugin(FFGLPluginSource *plugin)
{
    if(!plugin)
        return;

    // test if it is a shadertoy plugin
    if( plugin->rtti() == FFGLPluginSource::SHADERTOY_PLUGIN) {

        // show the Shadertoy code editor
        pluginGLSLCodeEditor->show();
        pluginGLSLCodeEditor->raise();
        pluginGLSLCodeEditor->setFocus();

        // link plugin to GUI editor
        pluginGLSLCodeEditor->linkPlugin(qobject_cast<FFGLPluginSourceShadertoy *>(plugin));

        // update list in case the plugin changed (e.g. name)
        connect(plugin, SIGNAL(changed()), mixingToolBox, SLOT(changed()));
    }

}

#endif

void GLMixer::on_actionAlgorithmSource_triggered(){

    // popup a question dialog to select the type of algorithm
    static AlgorithmSelectionDialog *asd = 0;
    if (!asd)
        asd = new AlgorithmSelectionDialog(this, _settings);

    if (asd->exec() == QDialog::Accepted) {
        Source *s = RenderingManager::getInstance()->newAlgorithmSource(asd->getSelectedAlgorithmIndex(),
                                                                        asd->getSelectedWidth(),
                                                                        asd->getSelectedHeight(),
                                                                        asd->getSelectedVariability(),
                                                                        asd->getUpdatePeriod(),
                                                                        asd->getIngoreAlpha());
        if ( s ){
            // add and set default properties
            RenderingManager::getInstance()->addSourceToBasket(s);

            // change property pixelated
            s->setPixelated( asd->getPixelated() );

            qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New Algorithm source created (")<< AlgorithmSource::getAlgorithmDescription(asd->getSelectedAlgorithmIndex()) << ").";
            emit status( tr("Source created with the algorithm %1.").arg( AlgorithmSource::getAlgorithmDescription(asd->getSelectedAlgorithmIndex())), 3000 );
        } else
            qCritical() << AlgorithmSource::getAlgorithmDescription(asd->getSelectedAlgorithmIndex()) <<  QChar(124).toLatin1() << tr("Could not create algorithm source.");
    }

}


void GLMixer::on_actionRenderingSource_triggered(){

    // popup a question dialog to select the type of rendering source
    RenderingSourceDialog rsd(this, _settings);

    if (rsd.exec() == QDialog::Accepted) {

        Source *s = RenderingManager::getInstance()->newRenderingSource(rsd.getRecursive());
        if ( s ){
            RenderingManager::getInstance()->addSourceToBasket(s);
            qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New rendering loopback source created.");
            emit status( tr("Source created with the rendering output loopback."), 3000 );
        } else
            qCritical() << tr("Could not create rendering loopback source.");
    }

}


void GLMixer::on_actionCloneSource_triggered(){

    if ( RenderingManager::getInstance()->notAtEnd(RenderingManager::getInstance()->getCurrentSource()) ) {
        SourceSet::iterator original = RenderingManager::getInstance()->getCurrentSource();
        Source *s = RenderingManager::getInstance()->newCloneSource(original);
        if ( s ) {
            QString name = (*RenderingManager::getInstance()->getCurrentSource())->getName();

            RenderingManager::getInstance()->addSourceToBasket(s);
            qDebug() << s->getName() <<  QChar(124).toLatin1() << tr("New clone of source %1 created.").arg(name);
            emit status( tr("The current source has been cloned."), 3000);
        } else
            qCritical() << (*RenderingManager::getInstance()->getCurrentSource())->getName()<< QChar(124).toLatin1() << tr("Could not clone source %1.");
    }
}




void GLMixer::on_actionCaptureSource_triggered(){

    // get pixmap
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasImage()) {

        // accept paste of image to create captureSource
        QImage capture = qvariant_cast<QImage>(mimeData->imageData());

        // display and request action with this capture
        CaptureDialog cd(this, capture, tr("Create a Pixmap source with this image ?"), _settings);

        if (cd.exec() == QDialog::Accepted) {

            int width = cd.getWidth();
            if (width)
                capture = capture.scaledToWidth(width);

            Source *s = RenderingManager::getInstance()->newCaptureSource(capture);
            if ( s ){
                RenderingManager::getInstance()->addSourceToBasket(s);
                qDebug() << s->getName()<< QChar(124).toLatin1() << tr("New Pixmap source created.");
            }
            else
                qCritical() << tr("Could not create Pixmap source.");
        }
    }
}


void GLMixer::on_actionCopy_snapshot_triggered(){

    // capture screen
    QImage capture = RenderingManager::getInstance()->captureFrameBuffer();
    QApplication::clipboard()->setImage( capture );

    emit status( tr("Screenshot copied to clipboard."), 3000 );
}

void GLMixer::on_actionSave_snapshot_triggered(){

    // capture screen
    QImage capture = RenderingManager::getInstance()->captureFrameBuffer();

    QString fileName;
    QString suggestion = QString("glmixerimage%1%2").arg(QDate::currentDate().toString("yyMMdd")).arg(QTime::currentTime().toString("hhmmss"));

    if (RenderingManager::getRecorder()->automaticSavingMode()) {
        fileName = RenderingManager::getRecorder()->automaticSavingFolder().absoluteFilePath(suggestion + ".png");
    }
    else {
        // display and request action with this capture
        CaptureDialog cd(this, capture, tr("Save this screenshot ?"), _settings);

        if (cd.exec() == QDialog::Accepted) {

            int width = cd.getWidth();
            if (width)
                capture = capture.scaledToWidth(width);

            fileName = getFileName(tr("Save screenshot"), tr("Image") + " (*.png *.jpg)",
                                   QString("png"), suggestion);

        }
    }

    // threaded saving of image
    ImageSaver::saveImage(capture, fileName);

    // show information
    emit status( tr("File %1 saved.").arg( fileName ), 3000 );
}


void GLMixer::on_actionEditSource_triggered()
{
    // get the current source, if available
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if ( RenderingManager::getInstance()->isValid(cs)) {

        // edit mean show the dialog to change source
        SourceFileEditDialog sed(this, *cs, tr("%1 - Source '%2'").arg(QCoreApplication::applicationName()).arg((*cs)->getName()), _settings);

        // open
        if (sed.exec() == QDialog::Accepted) {
            // requet to re-create source
            replaceCurrentSource();
        }

        // update the property browser of the source
        connectSource(RenderingManager::getInstance()->getCurrentSource());
    }
}

void GLMixer::replaceCurrentSource()
{
    Source::RTTI t = Source::SIMPLE_SOURCE;

    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if ( RenderingManager::getInstance()->isValid(cs) ) {

        // read the type of the current source
        t = (*cs)->rtti();

        // if we replace a video source, help user by showing the same directory
        if ( t == Source::VIDEO_SOURCE ) {
            VideoSource *vs = dynamic_cast<VideoSource *>(*cs);
            if (vs) {
                VideoFile *vf = vs->getVideoFile();
                if (vf) {
                    // indicate location of file to replace
                    _settings->setValue("recentMediaFile", vf->getFileName());
                }
            }
        }
        // if we replace a stream source, help user by showing the appropriate dialog
        else if ( t == Source::STREAM_SOURCE ) {
            VideoStreamSource *vs = dynamic_cast<VideoStreamSource *>(*cs);
            if (vs) {
                VideoStream *vf = vs->getVideoStream();
                // if the URL does not look like a network stream
                if ( !vf->getUrl().contains("://@") ) {
                    // it is probably a device
                    t = Source::OPENCV_SOURCE;
                }
            }
        }
        // if we replace a web source, help user by showing the previous web url
        else if ( t == Source::WEB_SOURCE ) {
            WebSource *ws = dynamic_cast<WebSource *>(*cs);
            if (ws) {
                _settings->setValue("recentWebUrl", ws->getUrl().toString());
            }
        }
#ifdef GLM_FFGL
        // if we replace a shadertoy, copy its code to clipboard
        else if ( t == Source::FFGL_SOURCE ) {

            FFGLSource *ffs = dynamic_cast<FFGLSource *> (*cs);
            if (ffs) {
                FFGLPluginSource *plugin = ffs->freeframeGLPlugin();
                // case of Shadertoy plugins
                if ( plugin->rtti()== FFGLPluginSource::SHADERTOY_PLUGIN ) {
                    FFGLPluginSourceShadertoy *st = qobject_cast<FFGLPluginSourceShadertoy *>(plugin);
                    if (st) {
                        // copy code of shadertoy to clipboard
                        QApplication::clipboard()->setText( st->getCode() );
                        // indicate in settings that FFGLCreationDialog should open on shadertoy clipboard
                        _settings->setValue("recentFFGLPluginSelection", 3);
                    }
                }
                // case of freeframe plugins
                else {
                    // indicate in settings that FFGLCreationDialog should open on ffgl source
                    _settings->setValue("recentFFGLPluginSelection", 0);
                }
            }
        }
#endif

        // show gui to re-create a source of same type
        newSource( t );

        // drop the source and make new source current
        RenderingManager::getInstance()->dropReplaceSource(cs);
    }

}

void GLMixer::on_actionDeleteSource_triggered()
{
    // lisst of sources to delete
    SourceList todelete;

    // if the current source is valid, add it todelete
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if ( RenderingManager::getInstance()->isValid(cs) )
        //  delete only the current
        todelete.insert(*cs);
    // if there is a selection and no source is current, delete the whole selection
    else if ( SelectionManager::getInstance()->hasSelection() )
        // make a copy of the selection (to make sure we do not mess with pointers when removing from selection)
        todelete = SelectionManager::getInstance()->copySelection();

    // remove all the source in the list todelete
    for(SourceList::iterator  its = todelete.begin(); its != todelete.end(); its++) {

        SourceSet::iterator sit = RenderingManager::getInstance()->getById((*its)->getId());
        if ( RenderingManager::getInstance()->isValid(sit) ) {
            // test for clones of this source
            int numclones = (*sit)->getClones()->size();
            // popup a question dialog 'are u sure' if there are clones attached;
            if ( numclones ){
                QString msg = tr("Source '%1' was cloned %2 times.\n\nDo you want to delete all the clones too?").arg((*its)->getName()).arg(numclones);
                if ( QMessageBox::question(this, tr("%1 - Are you sure?").arg(QCoreApplication::applicationName()), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                    numclones = 0;
            }

            if ( !numclones ){
                QString d = (*sit)->getName();
                RenderingManager::getInstance()->removeSource(sit);
                emit status( tr("Source %1 deleted.").arg( d ), 3000 );
            }
        }
    }
}


void GLMixer::on_actionSelect_Next_triggered(){

    RenderingManager::getInstance()->setCurrentNext();
}

void GLMixer::on_actionSelect_Previous_triggered(){

    RenderingManager::getInstance()->setCurrentPrevious();
}

void GLMixer::refreshTiming(){

    // reset to zero if no video file
    if (!currentVideoFile) {
        timeLineEdit->setText( "" );
        return;
    }

    double t = currentVideoFile->getCurrentFrameTime();

    if (_displayTimeAsFrame)
        timeLineEdit->setText( QString("Frame %1").arg((int) (t * currentVideoFile->getFrameRate())) );
    else
        timeLineEdit->setText( getStringFromTime(t) );

}

void GLMixer::enableSeek (bool on){

    seekBackwardButton->setEnabled(on);
    seekForwardButton->setEnabled(on);
}

void GLMixer::on_frameForwardButton_clicked(){

    if (currentVideoFile)
        currentVideoFile->seekForwardOneFrame();
}


void GLMixer::on_fastForwardButton_pressed(){

    if (currentVideoFile)
        currentVideoFile->setFastForward(true);
}

void GLMixer::on_fastForwardButton_released(){

    if (currentVideoFile)
        currentVideoFile->setFastForward(false);
}


void GLMixer::setAspectRatio(QAction *a)
{
    standardAspectRatio ar = (standardAspectRatio) a->data().toInt();

    RenderingManager::getInstance()->setRenderingAspectRatio(ar);

    // apply config; this also refreshes the rendering areas
    // if none of the above, the FREE aspect ratio was requested
    if (ar == ASPECT_RATIO_ANY) {
        OutputRenderWindow::getInstance()->useFreeAspectRatio(true);
        outputpreview->useFreeAspectRatio(true);
        QObject::connect(OutputRenderWindow::getInstance(), SIGNAL(resized()), outputpreview, SLOT(refresh()));
        QObject::connect(OutputRenderWindow::getInstance(), SIGNAL(resized()), RenderingManager::getRenderingWidget(), SLOT(refresh()));
    }
    // otherwise, disable the free aspect ratio
    else {
        OutputRenderWindow::getInstance()->useFreeAspectRatio(false);
        outputpreview->useFreeAspectRatio(false);
        QObject::disconnect(OutputRenderWindow::getInstance(), SIGNAL(resized()), outputpreview, SLOT(refresh()));
        QObject::disconnect(OutputRenderWindow::getInstance(), SIGNAL(resized()), RenderingManager::getRenderingWidget(), SLOT(refresh()));
    }

    RenderingManager::getRenderingWidget()->refresh();
}

void GLMixer::on_actionAbout_triggered(){

    QDialog aboutglm(this);

    setupAboutDialog(&aboutglm);

    aboutglm.exec();
}


void GLMixer::confirmSessionFileName(){

    // recent files history
    QStringList files = _settings->value("recentFileList").toStringList();

    if (currentSessionFileName.isNull() || currentSessionFileName.isEmpty()) {
        setWindowTitle(QString("%1 %2 - unsaved").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));
        actionAppend_Session->setEnabled(false);
        actionReload_Session->setEnabled(false);
    }
    else {
        // title and menu
        setWindowTitle(QString("%1 %2 - %3").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()).arg(QFileInfo(currentSessionFileName).fileName()));
        actionAppend_Session->setEnabled(true);
        actionReload_Session->setEnabled(true);

        files.removeAll(currentSessionFileName);
        files.prepend(currentSessionFileName);
        while (files.size() > MAX_RECENT_FILES)
            files.removeLast();
    }

    for (int i = 0; i < files.size(); ++i) {
         QString text = tr("&%1 - %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
         recentFileActs[i]->setText(text);
         recentFileActs[i]->setData(files[i]);
         recentFileActs[i]->setVisible(true);
    }

    for (int j = files.size(); j < MAX_RECENT_FILES; ++j)
        recentFileActs[j]->setVisible(false);

    _settings->setValue("recentFileList", files);

}


void GLMixer::on_actionNew_Session_triggered()
{
    // the only difference with close session is the post cleanup afterwards
    QObject::connect(this, SIGNAL(sessionLoaded()), this, SLOT(postNewSession()) );

    // close session
    on_actionClose_Session_triggered();

}

void GLMixer::toggleRender()
{
    bool on = RenderingManager::getInstance()->getSessionSwitcher()->alpha() > 0;
    RenderingManager::getInstance()->getSessionSwitcher()->smoothAlphaTransition(on);
}

#ifdef GLM_SESSION
void GLMixer::openNextSession()
{
    switcherSession->startTransitionToNextSession();
}

void GLMixer::openPreviousSession()
{
    switcherSession->startTransitionToPreviousSession();
}
#endif

void GLMixer::on_actionClose_Session_triggered()
{
    // inform the user that data might be lost
    int ret = QMessageBox::Discard;
    if (maybeSave) {
        QMessageBox msgBox;
        msgBox.setText(tr("The session have been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes ?"));
        msgBox.setIconPixmap( QPixmap(QString::fromUtf8(":/glmixer/icons/question.png")) );
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
    }
    // react according to user's answer
    switch (ret) {
    case QMessageBox::Save:
        // Save was clicked (will
        saveSession(true);
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        return;
    case QMessageBox::Discard:
    default:
        // keep on to create new session
        closeSession();
        break;
    }

}

void GLMixer::closeSession()
{
    // make a new session
    currentSessionFileName = QString();

    // trigger newSession after the smooth transition to black is finished (action is disabled meanwhile)
    actionToggleRenderingVisible->setEnabled(false);
    QObject::connect(RenderingManager::getSessionSwitcher(), SIGNAL(animationFinished()), this, SLOT(newSession()) );
    RenderingManager::getSessionSwitcher()->startTransition(false);

    // last session file name
    _settings->setValue("lastSessionFileName", "");
}

void GLMixer::newSession()
{
    // unpause if it was
    actionPause->setChecked ( false );

    // if coming from animation, disconnect it.
    QObject::disconnect(RenderingManager::getSessionSwitcher(), SIGNAL(animationFinished()), this, SLOT(newSession()) );
    actionToggleRenderingVisible->setEnabled(true);
    RenderingManager::getSessionSwitcher()->startTransition(true);

    // reset
    RenderingManager::getInstance()->clearSourceSet();
    RenderingManager::getRenderingWidget()->clearViews();

#ifdef GLM_SNAPSHOT
    SnapshotManager::getInstance()->clearConfiguration();
#endif

    // reset notes
    blocNoteEdit->setPlainText(QString());

    // refreshes the rendering areas
    outputpreview->refresh();

    // broadcast session file ready
    emit sessionLoaded();
    emit filenameChanged(currentSessionFileName);
    maybeSave = false;

    qDebug() << QApplication::applicationName() <<  QChar(124).toLatin1() << "New session.";
}


void GLMixer::postNewSession()
{
    // if coming from session loading, discunnect
    QObject::disconnect(this, SIGNAL(sessionLoaded()), this, SLOT(postNewSession()) );

    //
    // Setup defaults after a new session request
    //

    // select mixing view
    actionMixingView->trigger();

    // default black background
    actionWhite_background->setChecked(false);

    // apply rendering aspect ratio matching rendering output
    QRect g = OutputRenderWindow::getInstance()->getFullScreenMonitorGeometry();
    selectAspectRatio( doubleToAspectRatio( double(g.width()) / double(g.height()) ) );

    // new session given default notes
    QString note = tr("Created on ");
    note.append(QDateTime::currentDateTime().toString("dddd d MMMM yyyy"));
    note.append(" by ");
#ifdef Q_OS_WIN
    note.append(getenv("USERNAME"));
#else
    note.append(getenv("USER"));
#endif
    note.append(".\n");
    blocNoteEdit->setPlainText(note);

    emit status( tr("New session ready!"), 1500);
}

SessionSaver::SessionSaver(QString filename) : QThread(), _filename(filename)
{

}

void SessionSaver::run()
{
    QFile file(_filename);
    if (!file.open(QFile::WriteOnly | QFile::Text) ) {
        qWarning() << _filename << QChar(124).toLatin1() << tr("Problem writing; ") << file.errorString();
        qCritical() << _filename << QChar(124).toLatin1() << tr("Cannot save session file.");
        return;
    }
    QTextStream out(&file);

    QDomDocument doc;
    QDomProcessingInstruction instr = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
    doc.appendChild(instr);

    QDomElement root = doc.createElement("GLMixer");
    root.setAttribute("version", XML_GLM_VERSION);

    QDomElement renderConfig = RenderingManager::getInstance()->getConfiguration(doc, QFileInfo(_filename).dir());
    renderConfig.setAttribute("aspectRatio", (int) RenderingManager::getInstance()->getRenderingAspectRatio());
    root.appendChild(renderConfig);

    QDomElement viewConfig =  RenderingManager::getRenderingWidget()->getConfiguration(doc);
    root.appendChild(viewConfig);

    QDomElement rendering = doc.createElement("Rendering");
    rendering.setAttribute("clearToWhite", (int) RenderingManager::getInstance()->clearToWhite());
    root.appendChild(rendering);

    QDomElement notes = doc.createElement("Notes");
    QDomText text = doc.createTextNode(GLMixer::getInstance()->getNotes() );
    notes.appendChild(text);
    root.appendChild(notes);

#ifdef GLM_SNAPSHOT
    QDomElement snapshotConfig =  SnapshotManager::getInstance()->getConfiguration(doc);
    root.appendChild(snapshotConfig);
#endif

    doc.appendChild(root);
    doc.save(out, 4);

    file.close();
}

void GLMixer::postSaveSession()
{
    // validate that session is saved
    maybeSave = false;

    // log
    emit status( tr("File %1 saved.").arg( currentSessionFileName ), 3000 );
    qDebug() << currentSessionFileName <<  QChar(124).toLatin1() << tr("Session saved.");

    // broadcast session file ready
    emit sessionLoaded();
    emit filenameChanged(currentSessionFileName);
    setBusy(false);

    // last session file name
    _settings->setValue("lastSessionFileName", currentSessionFileName);
}


void GLMixer::saveSession(bool close, bool quit){

    QString fileName = currentSessionFileName;

    // for 'save file as' or new session
    if ( fileName.isEmpty() ) // NULL or empty current sessio filename
    {
        QString suggestion = QString("glmix %1%2").arg(QDate::currentDate().toString("yyMMdd")).arg(QTime::currentTime().toString("hhmmss"));

        fileName = getFileName(tr("Save session"),
                               tr("GLMixer session") + " (*.glm)",
                               QString("glm"), suggestion);
    }

    // do we have a file name ?
    if ( !fileName.isEmpty() )
    {
        setBusy(true);

        currentSessionFileName = fileName;

        // create working thread
        SessionSaver *workerThread = new SessionSaver(currentSessionFileName);

        // launch save
        connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
        connect(workerThread, SIGNAL(finished()), this, SLOT(postSaveSession()));

        // close after save if requested
        if (close)
            connect(workerThread, SIGNAL(finished()), this, SLOT(closeSession()));

        if (quit)
            connect(workerThread, SIGNAL(finished()), actionQuit, SLOT(trigger()));

        // start saving
        emit status( tr("Saving %1...").arg( currentSessionFileName ), 3000 );
        workerThread->start();

    }

}

void GLMixer::on_actionSave_Session_triggered(){

    saveSession();
}

void GLMixer::on_actionSave_Session_as_triggered()
{
    currentSessionFileName = QString::null;
    saveSession();

}

void GLMixer::on_actionLoad_Session_triggered()
{
    QString fileName = getFileName(tr("Open session"),
                                   tr("GLMixer session" )+" (*.glm)" );

    if (!fileName.isEmpty() && QFileInfo(fileName).isFile())
        // get the first file name selected
        switchToSessionFile( fileName );
    else
        qWarning()<< fileName << QChar(124).toLatin1() << tr("Cannot open file.");
}


void GLMixer::actionLoad_RecentSession_triggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        switchToSessionFile(action->data().toString());

}


void GLMixer::renameSessionFile(QString oldfilename, QString newfilename)
{
    // if the session file which was renamed is the current session file
    if ( currentSessionFileName == oldfilename ) {

        if (QFileInfo(newfilename).isFile()) {
            // update current session file name
            currentSessionFileName = newfilename;

            confirmSessionFileName();
        }
    }
}

void GLMixer::switchToSessionFile(QString filename){

    // de-select current source
    RenderingManager::getInstance()->unsetCurrentSource();

    if (filename.isEmpty() || !QFileInfo(filename).isFile())
        newSession();
    else
    {
        // setup filename
        currentSessionFileName = filename;

        // no need for fade off transition if nothing loaded
        if (RenderingManager::getInstance()->empty())
            openSessionFile();
        else {
            // trigger openSessionFile after the smooth transition to black is finished (action is disabled meanwhile)
            actionToggleRenderingVisible->setEnabled(false);
            QObject::connect(RenderingManager::getSessionSwitcher(), SIGNAL(animationFinished()), this, SLOT(openSessionFile()) );
            RenderingManager::getSessionSwitcher()->startTransition(false);
        }
    }
}

QString GLMixer::getCurrentSessionFilename() const
{
    return currentSessionFileName;
}

QString GLMixer::getRestorelastSessionFilename()
{
    // if the option to restore last session is ON, give the name of the last session file
    if (_restoreLastSession && _settings->contains("lastSessionFileName")) {
        QString filename = _settings->value("lastSessionFileName").toString();
        if (!filename.isEmpty()) {
            QFileInfo fi(filename);
            sfd->setDirectory(fi.absolutePath());
            return fi.absoluteFilePath();
        }
    }
    return QString();
}

void GLMixer::openSessionFile()
{
    static long int counter = 0;

    // unpause if it was
    actionPause->setChecked ( false );

    if (currentSessionFileName.isNull() || currentSessionFileName.isEmpty())
        return;

    // if we come from the smooth transition, disconnect the signal and enforce session switcher to show transition
    QObject::disconnect(RenderingManager::getSessionSwitcher(), SIGNAL(animationFinished()), this, SLOT(openSessionFile()) );
    RenderingManager::getSessionSwitcher()->setOverlay(1.0);
    actionToggleRenderingVisible->setEnabled(true);

    // Ok, ready to load XML ?
    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    // open file
    QFile file(currentSessionFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << currentSessionFileName << QChar(124).toLatin1() << tr("Problem reading file; ") << file.errorString();
        qCritical() << currentSessionFileName << QChar(124).toLatin1() << tr("Cannot open file.");
        currentSessionFileName = QString();
        RenderingManager::getSessionSwitcher()->setOverlay(0.0);
        return;
    }
    // load content
    if (!doc.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
        qWarning() << currentSessionFileName << QChar(124).toLatin1() << tr("XML parsing error line ") << errorLine << "(" << errorColumn << "); " << errorStr;
        qCritical() << currentSessionFileName << QChar(124).toLatin1() << tr("Cannot open file.");
        currentSessionFileName = QString();
        RenderingManager::getSessionSwitcher()->setOverlay(0.0);
        return;
    }
    // close file
    file.close();

    // verify it is a GLM file
    QDomElement root = doc.documentElement();
    if (root.tagName() != "GLMixer") {
        qWarning() << currentSessionFileName << QChar(124).toLatin1() << tr("This is not a GLMixer session file; ");
        qCritical() << currentSessionFileName << QChar(124).toLatin1() << tr("Cannot open file.");
        currentSessionFileName = QString();
        RenderingManager::getSessionSwitcher()->setOverlay(0.0);
        return;
    }

    QString version = XML_GLM_VERSION;
    if ( root.hasAttribute("version") )
        version = root.attribute("version");
    if ( version != XML_GLM_VERSION ) {
        qDebug() << currentSessionFileName << QChar(124).toLatin1()<< tr("The version of the file is old (") << root.attribute("version") << tr(") : converting file to current version (") << XML_GLM_VERSION << ")!";
    }

    // if we got up to here, it should be fine ; reset for a new session and apply loaded configurations

    // suspend display
    RenderingManager::getInstance()->pause(true);
    RenderingManager::getRenderingWidget()->setSuspended(true);
    QCoreApplication::processEvents();

    // clear sources and display
    RenderingManager::getInstance()->clearSourceSet();
#ifdef GLM_SNAPSHOT
    SnapshotManager::getInstance()->clearConfiguration();
#endif
    RenderingManager::getRenderingWidget()->clearViews();
    QCoreApplication::processEvents();


    // read and apply the limbo configuration
    // before reading sources to avoid issue with sources in stanby
    // (causing crash on deleting sources in standby)
    QDomElement vconfig = root.firstChildElement("Views");
    RenderingManager::getRenderingWidget()->setLimboConfiguration(vconfig);

    // read the source list and its configuration
    QDomElement renderConfig = root.firstChildElement("SourceList");
    if (renderConfig.isNull())
        qDebug() << currentSessionFileName << QChar(124).toLatin1() << tr("There is no source to load!");
    else {
        // apply rendering aspect ratio
        if ( selectAspectRatio( renderConfig.attribute("aspectRatio", "0").toInt() )  )
        {
            // read the list of sources
            qDebug() << currentSessionFileName << QChar(124).toLatin1() << tr("Loading session: opening sources.");

            // if we got up to here, it should be fine
            int errors = RenderingManager::getInstance()->addConfiguration(renderConfig, QFileInfo(currentSessionFileName).canonicalPath(), version);

            // inform of errors
            if ( errors > 0)
                qCritical() << currentSessionFileName << QChar(124).toLatin1()
                            << errors << tr(" error(s) occurred when opening the session.");

        }
        else {
            qWarning() << currentSessionFileName << QChar(124).toLatin1() << tr("Cannot change rendering aspect ration when recording or live mode.");
            qCritical() << currentSessionFileName << QChar(124).toLatin1() << tr("Cannot open file.");
            currentSessionFileName = QString();
            RenderingManager::getSessionSwitcher()->setOverlay(0.0);
            return;
        }
    }
    QCoreApplication::processEvents();

    // read and apply the views configuration
    if (vconfig.isNull())
        qDebug() << currentSessionFileName << QChar(124).toLatin1() << tr("No configuration specified!");
    else {
        // apply the view config (after sources are loaded to greate groups)
        qDebug() << currentSessionFileName << QChar(124).toLatin1() << tr("Loading session: applying configuration.");
        RenderingManager::getRenderingWidget()->setConfiguration(vconfig);

        // activate the view specified as 'current' in the xml config
        switch (vconfig.attribute("current").toInt()){
        case (View::NULLVIEW):
        case (View::MIXING):
            actionMixingView->trigger();
            break;
        case (View::GEOMETRY):
            actionGeometryView->trigger();
            break;
        case (View::LAYER):
            actionLayersView->trigger();
            break;
        case (View::RENDERING):
            actionRenderingView->trigger();
            break;
        }
        // show the catalog as specified in xlm config
        QDomElement cat = vconfig.firstChildElement("Catalog");
        actionShow_Catalog->setChecked(cat.attribute("visible").toInt());
        switch( (CatalogView::catalogSize) (cat.firstChildElement("Parameters").attribute("catalogSize").toInt()) ){
        case (CatalogView::SMALL):
            actionCatalogSmall->trigger();
            break;
        case (CatalogView::MEDIUM):
            actionCatalogMedium->trigger();
            break;
        case (CatalogView::LARGE):
            actionCatalogLarge->trigger();
            break;
        }
    }
    QCoreApplication::processEvents();

    // read the rendering configuration
    QDomElement rconfig = root.firstChildElement("Rendering");
    if (!rconfig.isNull())
        actionWhite_background->setChecked(rconfig.attribute("clearToWhite").toInt());

    // read the notes text
    QString text;
    QDomElement notes = root.firstChildElement("Notes");
    if (!notes.isNull())
        text = notes.text();
    blocNoteEdit->setPlainText(text);

#ifdef GLM_SNAPSHOT
    // read the snapshots
    QDomElement snaphots = root.firstChildElement("SnapshotList");
    if (!snaphots.isNull()) {
        SnapshotManager::getInstance()->addConfiguration(snaphots);
    }
#endif

    // unsuspend display
    RenderingManager::getRenderingWidget()->setSuspended(false);
    RenderingManager::getInstance()->pause(false);
    QCoreApplication::processEvents();

    // broadcast that the session is loaded
    emit sessionLoaded();
    emit filenameChanged(currentSessionFileName);
    maybeSave = false;

    // start the smooth transition
    RenderingManager::getSessionSwitcher()->startTransition(true);

    // message
    emit status( tr("Session file %1 loaded.").arg( currentSessionFileName ), 5000 );
    qDebug() << currentSessionFileName <<  QChar(124).toLatin1() << "Session loaded." << counter++;

    // last session file name
    _settings->setValue("lastSessionFileName", currentSessionFileName);
}


bool GLMixer::selectAspectRatio(int aspectratio)
{
    standardAspectRatio requestAR = (standardAspectRatio) CLAMP(aspectratio, ASPECT_RATIO_4_3,  ASPECT_RATIO_ANY );
    standardAspectRatio possibleAR = RenderingManager::getInstance()->getLockedAspectRatio();

    // impossible to change aspect ratio if rendering AR is locked
    if ( possibleAR != requestAR && possibleAR != ASPECT_RATIO_ANY )
        return false;

    // usualy it is possible: trigger the action that calls setAspectRatio function
    switch(requestAR) {
    case ASPECT_RATIO_ANY:
        actionFree_aspect_ratio->trigger();
        break;
    case ASPECT_RATIO_16_10:
        action16_10_aspect_ratio->trigger();
        break;
    case ASPECT_RATIO_16_9:
        action16_9_aspect_ratio->trigger();
        break;
    case ASPECT_RATIO_3_2:
        action3_2_aspect_ratio->trigger();
        break;
    default:
    case ASPECT_RATIO_4_3:
        action4_3_aspect_ratio->trigger();
        break;
    }

//    RenderingManager::getInstance()->resetFrameBuffer();    // why reset again?

    return true;
}

void GLMixer::on_actionReload_Session_triggered(){

    switchToSessionFile(currentSessionFileName);

}

void GLMixer::on_actionAppend_Session_triggered(){

    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    QString fileName = getFileName(tr("Open session"),
                                   tr("GLMixer session" )+" (*.glm)" );

    if (!fileName.isEmpty() && QFileInfo(fileName).isFile()) {

        QFile file(fileName);

        if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
            qWarning() << fileName << QChar(124).toLatin1()<< tr("Problem reading file; ") << file.errorString();
            qCritical() << fileName << QChar(124).toLatin1()<< tr("Cannot open file.");
            return;
        }

        if ( !doc.setContent(&file, true, &errorStr, &errorLine, &errorColumn) ) {
            qWarning() << fileName << QChar(124).toLatin1()<< tr("XML parsing error line ") << errorLine << "(" << errorColumn << "); " << errorStr;
            qCritical() << fileName << QChar(124).toLatin1() << tr("Cannot open file.");
            return;
        }

        file.close();

        QDomElement root = doc.documentElement();
        if ( root.tagName() != "GLMixer" ) {
            qWarning() << fileName << QChar(124).toLatin1()<< tr("This is not a GLMixer session file.");
            qCritical() << fileName << QChar(124).toLatin1()<< tr("Cannot open file.");
            return;
        }

        QString version = "0.0";
        if ( root.hasAttribute("version") )
            version = root.attribute("version");

        if ( version != XML_GLM_VERSION ) {
            qWarning() << fileName << QChar(124).toLatin1()<< tr("The version of the file is ") << root.attribute("version") << tr(" instead of ") << XML_GLM_VERSION;
            qCritical() << fileName << QChar(124).toLatin1() << tr("Incorrect file version. Trying to read what is compatible.");
        }

        // read the content of the source list to make sure the file is correct :
        QDomElement srcconfig = root.firstChildElement("SourceList");
        if ( srcconfig.isNull() ) {
            qWarning() << fileName << QChar(124).toLatin1() << tr("There is no source to load.");
            return;
        }

        // if we got up to here, it should be fine
        qDebug() << fileName << QChar(124).toLatin1() << tr("Adding list of sources.");
        int errors = RenderingManager::getInstance()->addConfiguration(srcconfig, QFileInfo(currentSessionFileName).canonicalPath(), version);
        if ( errors > 0)
            qCritical() << currentSessionFileName << QChar(124).toLatin1() << errors << tr(" error(s) occurred when reading session.");

        // confirm the loading of the file
        emit status( tr("Sources from %1 added to %2.").arg( fileName ).arg( currentSessionFileName ), 3000 );
        qDebug() << currentSessionFileName <<  QChar(124).toLatin1() << tr("Sources from %1 added.").arg( fileName );
    }

    // inform session has changed
    sessionChanged();

}


void GLMixer::drop(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QStringList mediaFiles;
    QStringList svgFiles;
    QString glmfile;
    int errors = 0;

    // browse the list of urls dropped
    if (mimeData->hasUrls()) {
        // deal with the urls dropped
        event->acceptProposedAction();
        QList<QUrl> urlList = mimeData->urls();

        // limitation in the amount of drops allowed
        if (urlList.size() > RenderingManager::getInstance()->getAvailableSourceCount())
            qWarning() << "[" << ++errors << "]" << tr("Cannot add more than %1 sources.").arg(RenderingManager::getInstance()->getAvailableSourceCount());

        int max = qMin(urlList.size(), RenderingManager::getInstance()->getAvailableSourceCount() );
        for (int i = 0; i < max; ++i) {

            QFileInfo urlname = getFileInfoFromURL(urlList.at(i));
            if ( urlname.exists() && urlname.isReadable() && urlname.isFile()) {

                if ( urlname.suffix() == "glm") {
                    glmfile = urlname.absoluteFilePath();
                    break;
                }
                else if ( urlname.suffix() == "svg") {
                    svgFiles.append(urlname.absoluteFilePath());
                }
                else //  maybe a video ?
                    mediaFiles.append(urlname.absoluteFilePath());
            }
            else
                qWarning() << urlname.absoluteFilePath() << QChar(124).toLatin1() <<  "[" << ++errors << "]" << tr("Not a valid file; Ignoring.");
        }
    }
    else
        qWarning() << "[" << ++errors << "]" << tr("Not a valid drop; Ignoring!");

    if (!glmfile.isNull()) {
        currentSessionFileName = glmfile;

        if (RenderingManager::getInstance()->empty())
            openSessionFile();
        else {
            // trigger openSessionFile after the smooth transition to black is finished (action is disabled meanwhile)
            actionToggleRenderingVisible->setEnabled(false);
            QObject::connect(RenderingManager::getSessionSwitcher(), SIGNAL(animationFinished()), this, SLOT(openSessionFile()) );
            RenderingManager::getSessionSwitcher()->startTransition(false);
        }

        if (!mediaFiles.isEmpty() || !svgFiles.isEmpty())
            qWarning() <<  "[" << ++errors << "]" << tr("Discarding %1 media files and %2 svg files; only loading the glm session.").arg(mediaFiles.count()).arg(svgFiles.count());

    } else if (!mediaFiles.isEmpty() || !svgFiles.isEmpty()) {
        // loading Media files
        setBusy(true);
        int i = 0;
        for (; i < mediaFiles.size(); ++i)
        {
            QCoreApplication::processEvents();
            VideoFile *newSourceVideoFile  = new VideoFile(this);

            // if the video file was created successfully
            if (newSourceVideoFile){
                // can we open the file ?
                if ( newSourceVideoFile->open( mediaFiles.at(i) ) ) {
                    Source *s = RenderingManager::getInstance()->newMediaSource(newSourceVideoFile);
                    // create the source as it is a valid video file (this also set it to be the current source)
                    if ( s ) {
                        RenderingManager::getInstance()->addSourceToBasket(s);
                        qDebug() << s->getName() << QChar(124).toLatin1()<< tr("New media source created with file ") << mediaFiles.at(i);
                    } else {
                        qWarning() << mediaFiles.at(i) << QChar(124).toLatin1()<< "[" << ++errors << "]" << tr("The source could not be created.");
                        delete newSourceVideoFile;
                    }
                } else {
                    qWarning() << mediaFiles.at(i) << QChar(124).toLatin1() << "[" << ++errors << "]" << tr("The file could not be loaded.");
                    delete newSourceVideoFile;
                }
            }
        }
        // loading SVG files
        for (i = 0; i < svgFiles.size(); ++i)
        {
            QCoreApplication::processEvents();
            QFile svgfile(svgFiles.at(i));
            if(!svgfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning() << svgFiles.at(i) <<  QChar(124).toLatin1() << tr("Could not open file.");
                break;
            }
            QByteArray data = svgfile.readAll();
            Source *s = RenderingManager::getInstance()->newSvgSource(data);
            if ( s ) {
                RenderingManager::getInstance()->addSourceToBasket(s);
                qDebug() << s->getName() << QChar(124).toLatin1()<< tr("New vector Graphics source created with file ")<< svgFiles.at(i);
            } else {
                qWarning() << svgFiles.at(i) << QChar(124).toLatin1() << "[" << ++errors << "]" << tr("The source could not be created.");
            }
        }

        setBusy(false);
        // inform session changed
        sessionChanged();
    }

    if (errors > 0)
        qCritical() << tr("Not all the dropped files could be loaded.");

}

QList<QUrl> getExtendedSidebarUrls(QList<QUrl> sideurls)
{
    // fill side Bar URLS with standard locations
    QList<QUrl> urls = sideurls;
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
         << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation))
         << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::MoviesLocation))
         << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));

#ifdef Q_OS_WIN
    QFileInfoList driv = QDir::drives();
    while (!driv.isEmpty())
         urls << QUrl::fromLocalFile(driv.takeFirst().absoluteFilePath());
#else
#ifdef Q_OS_MAC
    QFileInfoList vol = QDir("/Volumes/").entryInfoList(QDir::Dirs| QDir::Readable | QDir::NoDotAndDotDot);
    if (!vol.isEmpty()) {
        while (!vol.isEmpty())
            urls << QUrl::fromLocalFile(vol.takeFirst().absoluteFilePath());
    }
#else
    QFileInfoList vol = QDir("/media/").entryInfoList(QDir::Dirs| QDir::Readable | QDir::NoDotAndDotDot);
    if (!vol.isEmpty())
            urls << QUrl::fromLocalFile(vol.takeFirst().absoluteFilePath());
#endif
#endif

    return urls;
}

void GLMixer::readSettings( QString pathtobin )
{
    if ( !pathtobin.isNull() )
        _settings->setValue("ExecutionPath", pathtobin);

    // windows config
    if (_settings->contains("geometry"))
        restoreGeometry(_settings->value("geometry").toByteArray());
    else
        _settings->setValue("defaultGeometry", saveGeometry());

    const QRect actualGeometry(_settings->value("actualGeometry").toRect());
    if (actualGeometry.isValid() && windowState() & Qt::WindowMaximized)
        setGeometry(actualGeometry);

    // set normal status of window, widgets and toolbars, or set it to default
    if (_settings->contains("windowState"))
        restoreState(_settings->value("windowState").toByteArray());
    else
        restoreState(static_windowstate);

    // restore status of output window, or save current status as default
    if (_settings->contains("OutputRenderWindowState"))
        OutputRenderWindow::getInstance()->restoreState( _settings->value("OutputRenderWindowState").toByteArray() );
    else
        _settings->setValue("defaultOutputRenderWindowState", OutputRenderWindow::getInstance()->saveState() );

    // restore status of property browser  or save current status as default
    if (_settings->contains("PropertyBrowserState"))
        RenderingManager::getPropertyBrowserWidget()->restoreState( _settings->value("PropertyBrowserState").toByteArray() );
    else
        _settings->setValue("PropertyBrowserState", RenderingManager::getPropertyBrowserWidget()->saveState() );

    // dialogs configs
    if (_settings->contains("VideoFileDialog")) {
        mfd->restoreState(_settings->value("VideoFileDialog").toByteArray());
        mfd->setDirectory( mfd->history().last() );
        mfd->setSidebarUrls( getExtendedSidebarUrls( mfd->sidebarUrls()) );
    }
    if (_settings->contains("SessionFileDialog")) {
        sfd->restoreState(_settings->value("SessionFileDialog").toByteArray());
        sfd->setDirectory( sfd->history().last() );
        sfd->setSidebarUrls( getExtendedSidebarUrls( sfd->sidebarUrls()) );
    }

    // Cursor status
    if (_settings->contains("CursorMode")) {
        switch((ViewRenderWidget::cursorMode) _settings->value("CursorMode").toInt()) {
        case ViewRenderWidget::CURSOR_DELAY:
            actionCursorDelay->trigger();
            break;
        case ViewRenderWidget::CURSOR_SPRING:
            actionCursorSpring->trigger();
            break;
        case ViewRenderWidget::CURSOR_AXIS:
            actionCursorAxis->trigger();
            break;
        case ViewRenderWidget::CURSOR_LINE:
            actionCursorLine->trigger();
            break;
            break;
        case ViewRenderWidget::CURSOR_FUZZY:
            actionCursorFuzzy->trigger();
            break;
        case ViewRenderWidget::CURSOR_MAGNET:
            actionCursorMagnet->trigger();
            break;
        default:
        case ViewRenderWidget::CURSOR_NORMAL:
            actionCursorNormal->trigger();
            break;
        }
    }

    if (_settings->contains("CursorSpringMass"))
        cursorSpringMass->setValue(_settings->value("CursorSpringMass").toInt());
    if (_settings->contains("cursorLineSpeed"))
        cursorLineSpeed->setValue(_settings->value("cursorLineSpeed").toInt());
    if (_settings->contains("cursorLineWaitDuration"))
        cursorLineWaitDuration->setValue(_settings->value("cursorLineWaitDuration").toDouble());
    if (_settings->contains("cursorDelayLatency"))
        cursorDelayLatency->setValue(_settings->value("cursorDelayLatency").toDouble());
    if (_settings->contains("cursorDelayFiltering"))
        cursorDelayFiltering->setValue(_settings->value("cursorDelayFiltering").toInt());
    if (_settings->contains("cursorFuzzyRadius"))
        cursorFuzzyRadius->setValue(_settings->value("cursorFuzzyRadius").toInt());
    if (_settings->contains("cursorFuzzyFiltering"))
        cursorFuzzyFiltering->setValue(_settings->value("cursorFuzzyFiltering").toInt());
    if (_settings->contains("cursorMagnetRadius"))
        cursorMagnetRadius->setValue(_settings->value("cursorMagnetRadius").toInt());
    if (_settings->contains("cursorMagnetStrength"))
        cursorMagnetStrength->setValue(_settings->value("cursorMagnetStrength").toDouble());

    // last tools used
    if (_settings->contains("lastToolMixing"))
        RenderingManager::getRenderingWidget()->setToolMode( (ViewRenderWidget::toolMode) _settings->value("lastToolMixing").toInt() ,View::MIXING);
    if (_settings->contains("lastToolGeometry"))
        RenderingManager::getRenderingWidget()->setToolMode( (ViewRenderWidget::toolMode) _settings->value("lastToolGeometry").toInt() ,View::GEOMETRY);

    // timer config
    bool timeron = (bool) _settings->value("displayTimer", "0").toInt();
    setDisplayTimeEnabled(timeron);
    actionShowTimers->setChecked( timeron );

    // restore user preference
    if (_settings->contains("UserPreferences"))
        restorePreferences(_settings->value("UserPreferences").toByteArray());
    else
        restorePreferences(QByteArray());

    // user mixing presets
    if (_settings->contains("MixingUserPresets"))
        mixingToolBox->restorePresets( _settings->value("MixingUserPresets").toByteArray());

#ifdef GLM_OSC
    // Open Sound Control
    bool useOSC = _settings->value("OSCEnabled", "0").toBool();
    int portOSC = _settings->value("OSCPort", "7000").toInt();
    int portOSCBroadcast = _settings->value("OSCBroadcast", "3000").toInt();
    OpenSoundControlManager::getInstance()->setEnabled(useOSC, (qint16) portOSC, (qint16) portOSCBroadcast);
    if (_settings->contains("OSCVerbose"))
        OpenSoundControlManager::getInstance()->setVerbose(_settings->value("OSCVerbose").toBool());
    // restore table translator
    int size = _settings->beginReadArray("OSCTranslations");
    for (int i = 0; i < size; ++i) {
        _settings->setArrayIndex(i);
        OpenSoundControlManager::getInstance()->addTranslation(_settings->value("before").toString(),
                                                               _settings->value("after").toString());
    }
    _settings->endArray();
#endif

    // ok
    qDebug() << _settings->fileName() << QChar(124).toLatin1() << tr("All settings restored.");
}



void GLMixer::saveSettings()
{
    // windows config
    _settings->setValue("geometry", saveGeometry());
    _settings->setValue("actualGeometry", geometry());
    if (actionPerformanceMode->isChecked())
        _settings->setValue("performanceWindowState", saveState());
    else
        _settings->setValue("windowState", saveState());
    _settings->setValue("OutputRenderWindowState", OutputRenderWindow::getInstance()->saveState() );
    _settings->setValue("PropertyBrowserState", RenderingManager::getPropertyBrowserWidget()->saveState() );

    // dialogs configs
    _settings->setValue("VideoFileDialog", mfd->saveState());
    _settings->setValue("SessionFileDialog", sfd->saveState());

    // Cursor status
    _settings->setValue("CursorMode", RenderingManager::getRenderingWidget()->getCursorMode() );
    _settings->setValue("CursorSpringMass", cursorSpringMass->value() );
    _settings->setValue("cursorLineSpeed", cursorLineSpeed->value() );
    _settings->setValue("cursorLineWaitDuration", cursorLineWaitDuration->value() );
    _settings->setValue("cursorDelayLatency", cursorDelayLatency->value() );
    _settings->setValue("cursorDelayFiltering", cursorDelayFiltering->value() );
    _settings->setValue("cursorFuzzyRadius", cursorFuzzyRadius->value() );
    _settings->setValue("cursorFuzzyFiltering", cursorFuzzyFiltering->value() );
    _settings->setValue("cursorMagnetRadius", cursorMagnetRadius->value() );
    _settings->setValue("cursorMagnetStrength", cursorMagnetStrength->value() );

    // last session file name
    _settings->setValue("lastSessionFileName", currentSessionFileName);

    // last tools used
    _settings->setValue("lastToolMixing", (int) RenderingManager::getRenderingWidget()->getToolMode(View::MIXING));
    _settings->setValue("lastToolGeometry", (int) RenderingManager::getRenderingWidget()->getToolMode(View::GEOMETRY));

    // timer config
    _settings->setValue("displayTimer", (int) _displayTimerEnabled);

    // user preferences
    _settings->setValue("UserPreferences", getPreferences());

    // user mixing presets
    _settings->setValue("MixingUserPresets", mixingToolBox->getPresets());

#ifdef GLM_OSC
    // Open Sound Control
    _settings->setValue("OSCEnabled", OpenSoundControlManager::getInstance()->isEnabled());
    _settings->setValue("OSCPort", OpenSoundControlManager::getInstance()->getPortReceive());
    _settings->setValue("OSCBroadcast", OpenSoundControlManager::getInstance()->getPortBroadcast());
    _settings->setValue("OSCVerbose", OpenSoundControlManager::getInstance()->isVerbose());
    _settings->remove("OSCTranslations");
    _settings->beginWriteArray("OSCTranslations");
    int i = 0;
    QListIterator< QPair<QString, QString> > t(*(OpenSoundControlManager::getInstance()->getTranslationDictionnary()));
    while (t.hasNext()) {
        QPair<QString, QString> p = t.next();
        _settings->setArrayIndex(i);
        _settings->setValue("before", p.first);
        _settings->setValue("after", p.second);
        ++i;
    }
    _settings->endArray();
#endif

    // make sure system saves settings NOW
    _settings->sync();

    // ok
    qDebug() << _settings->fileName() << QChar(124).toLatin1() << tr("Settings saved.");
}


void GLMixer::on_actionExportSettings_triggered()
{
    // get the filename of an ini file
    QString fileName = getFileName(tr("Export GLMixer settings and preference to file"),
                                   tr("GLMixer ini file") + " (*.ini)",
                                   QString("ini"),
                                   QFileInfo( QDir::home(), "glmixer.ini").absoluteFilePath() );

    // if a valid file name is given
    if (!fileName.isEmpty()) {
        // delete previous file if exists
        QFileInfo inifile(fileName);
        if (inifile.exists())
            inifile.dir().remove( inifile.fileName() );
        // create a temporary settings object
        QSettings exported_preferences(inifile.absoluteFilePath(), QSettings::IniFormat);

        // copy settings to this settings object

        // user preferences
        exported_preferences.setValue("UserPreferences", _settings->value("UserPreferences"));

        // user mixing presets
        exported_preferences.setValue("MixingUserPresets", _settings->value("MixingUserPresets"));

#ifdef GLM_OSC
        // Open Sound Control
        exported_preferences.setValue("OSCPort", _settings->value("OSCPort"));
        exported_preferences.setValue("OSCBroadcast", _settings->value("OSCBroadcast"));
        int size = _settings->beginReadArray("OSCTranslations");
        exported_preferences.beginWriteArray("OSCTranslations");
        for (int i = 0; i < size; ++i) {
            _settings->setArrayIndex(i);
            exported_preferences.setArrayIndex(i);
            exported_preferences.setValue("before", _settings->value("before"));
            exported_preferences.setValue("after", _settings->value("after"));
        }
        exported_preferences.endArray();
        _settings->endArray();
#endif

        // window state
        exported_preferences.setValue("windowState", _settings->value("windowState"));
        exported_preferences.setValue("performanceWindowState", _settings->value("performanceWindowState"));

        // save settings NOW
        exported_preferences.sync();
        qDebug() << exported_preferences.fileName() << QChar(124).toLatin1() << tr("Settings exported.");
    }
}

void GLMixer::on_actionImportSettings_triggered()
{
    // get the filename of an ini file
    QString fileName = getFileName(tr("Import GLMixer settings and preference from file"),
                                   tr("GLMixer ini file") + " (*.ini)");

    // check validity of file
    QFileInfo inifile(fileName);
    // if a valid file name is given
    if (inifile.isFile() && inifile.isReadable()) {
        // try to create a setting object from given file
        QSettings imported_preferences(inifile.absoluteFilePath(), QSettings::IniFormat);
        // if there is no error with the settings
        if (imported_preferences.status() == QSettings::NoError) {
            // popup a question dialog to select the categories of settings to import
            SettingsCategogyDialog scd(this, _settings);
            // if the user continues
            if (scd.exec() == QDialog::Accepted) {
                // build a settings category indicator
                if (scd.preferenceSelected() && imported_preferences.contains("UserPreferences")) {
                    // user preferences
                    _settings->setValue("UserPreferences", imported_preferences.value("UserPreferences"));
                }
                if (scd.presetsSelected() && imported_preferences.contains("MixingUserPresets")) {
                    // user mixing presets
                    _settings->setValue("MixingUserPresets", imported_preferences.value("MixingUserPresets"));
                }
                if (scd.oscSelected()) {
#ifdef GLM_OSC
                    // Open Sound Control
                    if (imported_preferences.contains("OSCPort"))
                        _settings->setValue("OSCPort", imported_preferences.value("OSCPort"));
                    if (imported_preferences.contains("OSCBroadcast"))
                        _settings->setValue("OSCBroadcast", imported_preferences.value("OSCBroadcast"));
                    if (imported_preferences.contains("OSCTranslations")) {
                        _settings->remove("OSCTranslations");
                        int size = imported_preferences.beginReadArray("OSCTranslations");
                        _settings->beginWriteArray("OSCTranslations");
                        for (int i = 0; i < size; ++i) {
                            imported_preferences.setArrayIndex(i);
                            _settings->setArrayIndex(i);
                            _settings->setValue("before", imported_preferences.value("before"));
                            _settings->setValue("after", imported_preferences.value("after"));
                        }
                        _settings->endArray();
                        imported_preferences.endArray();
                    }
#endif
                }
                if (scd.windowstateSelected()) {
                    // window state
                    if (imported_preferences.contains("windowState"))
                        _settings->setValue("windowState", imported_preferences.value("windowState"));
                    if (imported_preferences.contains("performanceWindowState"))
                        _settings->setValue("performanceWindowState", imported_preferences.value("performanceWindowState"));
                }

                // apply all settings we just overwrote
                readSettings();
                // ok
                qDebug() << imported_preferences.fileName() << QChar(124).toLatin1() << tr("Settings imported.");
            }
        }
    }

}

void GLMixer::on_actionResetToolbars_triggered()
{
    restoreGeometry(_settings->value("defaultGeometry").toByteArray());
    restoreState(static_windowstate);
    OutputRenderWindow::getInstance()->restoreState( _settings->value("defaultOutputRenderWindowState").toByteArray() );
    restoreDockWidget(previewDockWidget);
    restoreDockWidget(sourceDockWidget);
    restoreDockWidget(vcontrolDockWidget);
    restoreDockWidget(cursorDockWidget);
    restoreDockWidget(mixingDockWidget);
    restoreDockWidget(selectionDockWidget);

#ifdef GLM_SESSION
    restoreDockWidget(switcherDockWidget);
#endif

#ifdef GLM_HISTORY
    restoreDockWidget(actionHistoryDockWidget);
#endif

    qDebug() << QApplication::applicationName()  << QChar(124).toLatin1()  << tr("Default layout restored.");
}

void GLMixer::on_actionPreferences_triggered()
{
    // fill in the saved preferences
    upd->showPreferences( getPreferences() );

    // show the dialog and apply preferences if it was accepted
    if (upd->exec() == QDialog::Accepted) {

        int mem = VideoFile::getMemoryUsagePolicy();
        bool usepbo = RenderingManager::usePboExtension();
        bool hwacc = CodecManager::useHardwareAcceleration();

        restorePreferences( upd->getUserPreferences() );

        if ( !RenderingManager::getInstance()->empty()
             && ( mem != VideoFile::getMemoryUsagePolicy()
             || usepbo != RenderingManager::usePboExtension()
             || hwacc != CodecManager::useHardwareAcceleration() )
           )
        {
            QMessageBox::information(this, QCoreApplication::applicationName(), "Your preferences will only take effect for new sources.\nTo apply the changes to the sources in the current session, save session and reload.");
        }

    }
}


void GLMixer::restorePreferences(const QByteArray & state){

    const quint32 magicNumber = MAGIC_NUMBER;
    const quint16 currentMajorVersion = QSETTING_PREFERENCE_VERSION;
    quint32 storedMagicNumber = 0;
    quint16 majorVersion = 0;

    if (!state.isEmpty()) {
        QByteArray sd = state;
        QDataStream stream(&sd, QIODevice::ReadOnly);
        stream >> storedMagicNumber >> majorVersion;
    }

    if (storedMagicNumber != magicNumber || majorVersion != currentMajorVersion) {

        // set dialog in minimal mode
        upd->setModeMinimal(true);

        // make sure we reset preferences
        upd->restoreAllDefaultPreferences();

        // show the dialog and apply preferences
        upd->exec();

        restorePreferences( upd->getUserPreferences() );

        upd->setModeMinimal(false);
        return;
    }

    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    stream >> storedMagicNumber >> majorVersion;

    // a. Apply rendering preferences
    uint RenderingQuality;
    bool useBlitFboExtension = true;
    stream >> RenderingQuality >> useBlitFboExtension;
    RenderingManager::setUseFboBlitExtension(useBlitFboExtension);
    RenderingManager::getInstance()->setRenderingQuality((frameBufferQuality) RenderingQuality);

    int targetPeriod = 20;
    stream >> targetPeriod;
    if (targetPeriod > 0)
        glRenderTimer::getInstance()->setInterval( targetPeriod );

    // b. Apply source preferences
    stream >> RenderingManager::getInstance()->defaultSource();

    // c.  DefaultScalingMode
    uint sm = 0;
    stream >> sm;
    RenderingManager::getInstance()->setDefaultScalingMode( (Source::scalingMode) sm );

    // d. defaultStartPlaying
    bool defaultStartPlaying = false;
    stream >> defaultStartPlaying;
    RenderingManager::getInstance()->setDefaultPlayOnDrop(defaultStartPlaying);

    // e. PreviousFrameDelay
    uint  previous_frame_period = 1;
    stream >> previous_frame_period;
    RenderingManager::getInstance()->setPreviousFramePeriodicity(previous_frame_period);

    // f. Stippling mode
    uint stipplingMode = 0;
    stream >> stipplingMode;
    ViewRenderWidget::setStipplingMode(stipplingMode);

    // g. recording format
    uint recformat = 4;
    stream >> recformat;
    RenderingManager::getRecorder()->setEncodingFormat( (encodingformat) recformat);
    uint rtfr = 40;
    stream >> rtfr;
    RenderingManager::getRecorder()->setEncodingFrameInterval(rtfr > 0 ? rtfr : 40);

    // h. recording folder
    bool automaticSave = false;
    stream >> automaticSave;
    RenderingManager::getRecorder()->setAutomaticSavingMode(automaticSave);
    QString automaticSaveFolder;
    stream >> automaticSaveFolder;
    RenderingManager::getRecorder()->setAutomaticSavingFolder(automaticSaveFolder);

    // i. disable filtering
    bool disablefilter = false;
    stream >> disablefilter;
    ViewRenderWidget::setFilteringEnabled(!disablefilter);
    mixingToolBox->blendingButton->click();

    // j. antialiasing
    bool antialiasing = true;
    stream >> antialiasing;
    RenderingManager::getRenderingWidget()->setAntiAliasing(antialiasing);
    mixingToolBox->setAntialiasing(antialiasing);

    // k. mouse buttons and modifiers
    QMap<int, int> mousemap;
    stream >> mousemap;
    View::setMouseButtonsMap(mousemap);
    QMap<int, int> modifiermap;
    stream >> modifiermap;
    View::setMouseModifiersMap(modifiermap);

    // l. zoom config
    int zoomspeed = 120;
    stream >> zoomspeed;
    View::setZoomSpeed((float)zoomspeed);
    bool zoomcentered = true;
    stream >> zoomcentered;
    View::setZoomCentered(zoomcentered);

    // m. useSystemDialogs
    stream >> usesystemdialogs;

    //	 n. shared memory depth
    uint shmdepth = 0;
    stream >> shmdepth;
#ifdef GLM_SHM
    RenderingManager::getInstance()->setSharedMemoryColorDepth(shmdepth);
#endif
    // o. fullscreen monitor index
    uint fsmi = 0;
    stream >> fsmi;
    OutputRenderWindow::getInstance()->setFullScreenMonitor(fsmi);

    // p. options
    bool fs = false;
    stream >> fs >> _displayTimeAsFrame >> _restoreLastSession;
    RenderingManager::getRenderingWidget()->setFramerateVisible(fs);

    // q. view context menu
    int vcm = 0;
    stream >> vcm;
    if (vcm == 1)
        RenderingManager::getRenderingWidget()->setViewContextMenu(cursorMenu);
    else
        RenderingManager::getRenderingWidget()->setViewContextMenu(zoomMenu);

    // r. Memory usage policy
    int mem = VideoFile::getMemoryUsagePolicy();
    stream >> mem;
    VideoFile::setMemoryUsagePolicy(mem);

    // s. save session on exit
    stream >> _saveExitSession;

    // t. disable PBO
    bool usePBO = true;
    stream >> usePBO >> _disableOutputWhenRecord;
    RenderingManager::setUsePboExtension(usePBO);
    if (_disableOutputWhenRecord) {
        QObject::connect(actionRecord, SIGNAL(toggled(bool)), OutputRenderWindow::getInstance(), SLOT(setInactive(bool)));
        QObject::connect(actionRecord, SIGNAL(toggled(bool)), OutputRenderWindow::getInstance(), SLOT(setHidden(bool)));
     } else {
        QObject::disconnect(actionRecord, SIGNAL(toggled(bool)), OutputRenderWindow::getInstance(), SLOT(setHidden(bool)));
    }
    OutputRenderWindow::getInstance()->setActive(true);

    // u. recording buffer
    int percent;
    stream >> percent;
    RenderingManager::getRecorder()->setBufferSize(RenderingEncoder::computeBufferSize(percent));

    // v. icon size
    int isize = 50;
    stream >> isize;
    ViewRenderWidget::setIconSize(MIN_ICON_SIZE + (double)isize * (MAX_ICON_SIZE-MIN_ICON_SIZE) / 100.0);

    // w. Undo level
    int undolevel = 100;
    stream >> undolevel;
#ifdef GLM_UNDO
    UndoManager::getInstance()->setMaximumSize(undolevel);
#endif

    // x.  output frame periodicity
    uint  display_frame_period = 1;
    stream >> display_frame_period;
    RenderingManager::getInstance()->setDisplayFramePeriodicity(display_frame_period);

    // y. recording quality
    uint recquality = 4;
    stream >> recquality;
    RenderingManager::getRecorder()->setEncodingQuality((encodingquality) recquality);

    // z. snap tool preferences
    bool snap = true;
    stream >> snap;
    View::setToolSnap(snap);

    // aa. Single Instance & custom timer
    stream >> GLMixer::_singleInstanceMode;
    bool activetiming = false;
    stream >> activetiming;
    glRenderTimer::getInstance()->setActiveTimingMode(activetiming);

    // ab. Hardware Codec
    bool hwcodec = false;
    stream >> hwcodec;
    CodecManager::setHardwareAcceleration(hwcodec);

    // ac. Output fading duration
    int duration = 500;
    stream >> duration;
    RenderingManager::getInstance()->getSessionSwitcher()->setSmoothAlphaDuration(duration);

    // ensure the Rendering Manager updates
    RenderingManager::getInstance()->resetFrameBuffer();

    // de-select current source
    RenderingManager::getInstance()->unsetCurrentSource();

    qDebug() << QApplication::applicationName() << QChar(124).toLatin1() << tr("Preferences restored.");
}

QByteArray GLMixer::getPreferences() const {

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    const quint32 magicNumber = MAGIC_NUMBER;
    const quint16 majorVersion = QSETTING_PREFERENCE_VERSION;
    stream << magicNumber << majorVersion;

    // a. Store rendering preferences
    stream << (uint) RenderingManager::getInstance()->getRenderingQuality();
    stream << RenderingManager::useFboBlitExtension();
    stream << glRenderTimer::getInstance()->interval();

    // b. Store source preferences
    stream << RenderingManager::getInstance()->defaultSource();

    // c. DefaultScalingMode
    stream << (uint) RenderingManager::getInstance()->getDefaultScalingMode();

    // d. defaultStartPlaying
    stream << RenderingManager::getInstance()->getDefaultPlayOnDrop();

    // e.  PreviousFrameDelay
    stream << RenderingManager::getInstance()->getPreviousFramePeriodicity();

    // f. Stippling mode
    stream << ViewRenderWidget::getStipplingMode();

    // g. recording format
    stream << (uint) RenderingManager::getRecorder()->encodingFormat();
    stream << RenderingManager::getRecorder()->encodingFrameInterval();

    // h. recording folder
    stream << RenderingManager::getRecorder()->automaticSavingMode();
    stream << RenderingManager::getRecorder()->automaticSavingFolder().absolutePath();

    // i. filtering
    stream << !ViewRenderWidget::filteringEnabled();

    // j. antialiasing
    stream << RenderingManager::getRenderingWidget()->antiAliasing();

    // k. mouse buttons and modifiers
    stream << View::getMouseButtonsMap();
    stream << View::getMouseModifiersMap();

    // l. zoom config
    stream << (int) View::zoomSpeed();
    stream << View::zoomCentered();

    // m. useSystemDialogs
    stream << _instance->useSystemDialogs();

    // n. shared memory color depth
#ifdef GLM_SHM
    stream << (uint) RenderingManager::getInstance()->getSharedMemoryColorDepth();
#else
    stream << 0;
#endif

    // o. fullscreen monitor index
    stream << (uint) OutputRenderWindow::getInstance()->getFullScreenMonitor();

    // p. options
    stream << RenderingManager::getRenderingWidget()->getFramerateVisible() << _displayTimeAsFrame << _restoreLastSession;

    // q. view context menu
    stream << ((RenderingManager::getRenderingWidget()->getViewContexMenu() == cursorMenu) ? 1 : 0);

    // r. Memory usage policy
    stream << VideoFile::getMemoryUsagePolicy();

    // s. save session on exit
    stream << _saveExitSession;

    // t. disable PBO
    stream << RenderingManager::usePboExtension();
    stream << _disableOutputWhenRecord;

    // u. recording buffer
    stream << RenderingEncoder::computeBufferPercent(RenderingManager::getRecorder()->getBufferSize());

    // v. icon size
    stream << (int) ( 100.0 * (ViewRenderWidget::getIconSize() - MIN_ICON_SIZE) / (MAX_ICON_SIZE-MIN_ICON_SIZE) );

    // w. Undo level
    int undolevel = 100;
#ifdef GLM_UNDO
    undolevel = UndoManager::getInstance()->maximumSize();
#endif
    stream << undolevel;

    // x.  output frame periodicity
    stream << RenderingManager::getInstance()->getDisplayFramePeriodicity();

    // y. recording quality
    stream << (uint) RenderingManager::getRecorder()->encodingQuality();

    // z. Timers display preferences
    stream << View::toolSnap();

    // aa. Single Instance
    stream << GLMixer::_singleInstanceMode;
    stream << glRenderTimer::getInstance()->isActiveTimingMode();

    // ab. Hardware Codec
    stream << CodecManager::useHardwareAcceleration();

    // ac. Output fading duration
    stream << RenderingManager::getInstance()->getSessionSwitcher()->smoothAlphaDuration();

    return data;
}


void GLMixer::on_controlOptionsButton_clicked()
{
    QList<int> splitSizes = vcontrolOptionSplitter->sizes();

    if (splitSizes.last() == 0) {
        splitSizes[0] -= 140;
        splitSizes[1] = 140;
    } else {
        splitSizes[0] += splitSizes[1];
        splitSizes[1] = 0;
    }
    vcontrolOptionSplitter->setSizes(splitSizes);
}



void GLMixer::on_output_alpha_valueChanged(int v){

    static int previous_v = 0;

    if ( (v == 100 || previous_v == 100) && previous_v != v)
        actionToggleRenderingVisible->setChecked( v < 100);

    RenderingManager::getInstance()->getSessionSwitcher()->setAlpha(v);

    previous_v = v;
}


void GLMixer::updateStatusControlActions() {

    bool playEnabled = false, controlsEnabled = false;
    bool clipboardEnabled = false;

    // get current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs)) {
        // test if the current source is playable ; if yes, enable action start/stop
        if ( (*cs)->isPlayable() ) {
            playEnabled = true;
            // test if the current source is Media source ; the selectedSourceVideoFile has been set in currentChanged method
            if ( (*cs)->rtti() == Source::VIDEO_SOURCE )
                // if yes, enable actions for media control (and return)
                controlsEnabled = true;
        }
        // enable copy current source
        clipboardEnabled = true;
    }

     // test the presence of playable source to enable action Start/Stop
    // test the presence of Media source to enable actions for media control
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {
        //  enable action start/stop
        if ( (*its)->isPlayable() ) {
            playEnabled = true;
            if ( (*its)->rtti() == Source::VIDEO_SOURCE )
                // enable actions for media control (and return)
                controlsEnabled = true;
        }
        // enable copy selected sources
        clipboardEnabled = true;
    }

    sourceControlMenu->setEnabled( playEnabled );
    actionSourcePlay->setEnabled( playEnabled );
    actionSourcePause->setEnabled( controlsEnabled );
    selectionPause->setEnabled( controlsEnabled );
    actionSourceRestart->setEnabled( controlsEnabled );
    actionSourceSeekBackward->setEnabled( controlsEnabled );
    actionSourceSeekForward->setEnabled( controlsEnabled );

    actionDeleteSource->setEnabled(clipboardEnabled);
    actionCopy->setEnabled(clipboardEnabled);
    actionCut->setEnabled(clipboardEnabled);
 }

bool GLMixer::useSystemDialogs() const
{
    return usesystemdialogs;
}


void GLMixer::startButton_toogled(bool on) {

    // toggle play/stop of current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs)) {
        (*cs)->play(on);
        // update gui content from timings
        refreshTiming();
//        updateRefreshTimerState(); // timeline ?
    }

}

void GLMixer::on_actionSourcePlay_triggered(){

    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs))
        (*cs)->play(!(*cs)->isPlaying());

    // loop over the selection and toggle play/stop of each source
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++)
        if (*its != *cs)
            (*its)->play(!(*its)->isPlaying());
}

void GLMixer::on_actionSourceRestart_triggered(){

    // apply action to current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs) && currentVideoFile )
        currentVideoFile->seekBegin();

    // loop over the selection and apply action of each source
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++)
        if (*its != *cs && (*its)->rtti() == Source::VIDEO_SOURCE ){
            VideoFile *vf = (dynamic_cast<VideoSource *>(*its))->getVideoFile();
            if ( vf )
                vf->seekBegin();
        }
}
void GLMixer::on_actionSourceSeekBackward_triggered(){

    // apply action to current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs) && currentVideoFile )
        currentVideoFile->seekBackward();

    // loop over the selection and apply action of each source
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++)
        if (*its != *cs && (*its)->rtti() == Source::VIDEO_SOURCE ){
            VideoFile *vf = (dynamic_cast<VideoSource *>(*its))->getVideoFile();
            if ( vf )
                vf->seekBackward();
        }

}


void GLMixer::on_actionSourcePause_triggered(){

    // toggle pause/resume of current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs) && currentVideoFile )
        currentVideoFile->pause(!currentVideoFile->isPaused());

    // loop over the selection and toggle pause/resume of each source
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++)
        if (*its != *cs && (*its)->rtti() == Source::VIDEO_SOURCE ){
            VideoFile *vf = (dynamic_cast<VideoSource *>(*its))->getVideoFile();
            if ( vf )
                vf->pause(!vf->isPaused());
        }
}

void GLMixer::on_actionSourceSeekForward_triggered(){

    // apply action to current source
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs) && currentVideoFile )
        currentVideoFile->seekForward();

    // loop over the selection and apply action of each source
    for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++)
        if (*its != *cs && (*its)->rtti() == Source::VIDEO_SOURCE ){
            VideoFile *vf = (dynamic_cast<VideoSource *>(*its))->getVideoFile();
            if ( vf )
                vf->seekForward();
        }
}


void GLMixer::on_actionCopy_triggered() {

    // copy description of the current source (if valid)
    SourceSet::iterator cs = RenderingManager::getInstance()->getCurrentSource();
    if (RenderingManager::getInstance()->isValid(cs) || SelectionManager::getInstance()->hasSelection() ) {

        // generate XML text
        QDomDocument doc;
        QDomProcessingInstruction instr = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
        doc.appendChild(instr);


        QDomElement sourcelist = doc.createElement("SourceList");

        if ( SelectionManager::getInstance()->hasSelection() ) {
            // copy description of the selected sources
            for(SourceList::iterator  its = SelectionManager::getInstance()->selectionBegin(); its != SelectionManager::getInstance()->selectionEnd(); its++) {

                QDomElement sourceelem = RenderingManager::getInstance()->getSourceConfiguration(its, doc);
                // add source to list
                sourcelist.appendChild(sourceelem);
            }
        }
        else {
            // copy description of the current source
            QDomElement sourceelem = RenderingManager::getInstance()->getSourceConfiguration(cs, doc);
            // add source to list
            sourcelist.appendChild(sourceelem);
        }

        doc.appendChild(sourcelist);

        // copy the XML into the clipboard
        QApplication::clipboard()->setText(doc.toString());

        qDebug() << tr("%1 source(s) copied to clipboard").arg(sourcelist.childNodes().count());
    }
}

void GLMixer::on_actionCut_triggered() {

    // cut means copy and delete
    on_actionCopy_triggered();
    on_actionDeleteSource_triggered();
}

void GLMixer::on_actionPaste_triggered() {

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasText()) {

        // create an XML doc from clipboard text
        QDomDocument doc;
        if (doc.setContent(mimeData->text())) {

            // get the list of sources
            QDomElement sourcelist = doc.firstChildElement("SourceList");
            if ( !sourcelist.isNull()) {
                int c = 0, n = 0;

                // browse the list of sources
                QDomElement child = sourcelist.firstChildElement("Source");
                while (!child.isNull()) {

                    QString name = child.attribute("name");
                    // paste into current workspace
                    child.setAttribute("workspace", WorkspaceManager::getInstance()->current());

                    // try to find the source in list of existing
                    SourceSet::iterator sit = RenderingManager::getInstance()->getByName(name);
                    if (RenderingManager::getInstance()->isValid(sit)) {
                        Source *s = RenderingManager::getInstance()->newCloneSource(sit);
                        if ( s ) {
                            // drop
                            RenderingManager::getInstance()->addSourceToBasket(s);
                            c++;
                            // duplicate properties & plugins (except name)
                            child.removeAttribute("name");
                            s->setConfiguration(child);
                        }
                    }
                    // that name is not in the list
                    else
                    {
#ifdef GLM_UNDO
                        // inform undo manager
                        UndoManager::getInstance()->store();
#endif
                        // create a new source from this description
                        n += 1 - RenderingManager::getInstance()->addSourceConfiguration(child);
                    }

                    // read next source
                    child = child.nextSiblingElement("Source");
                }

                qDebug() << tr("%1 source(s) cloned and %2 newly created from clipboard (%3/%4)").arg(c).arg(n).arg(c+n).arg(sourcelist.childNodes().count());
            }
        }
    }
    else if (mimeData->hasImage()) {

        // accept paste of image to create captureSource
        on_actionCaptureSource_triggered();
    }
}

void GLMixer::CliboardDataChanged() {

    // by default, clipboard cannot be pasted
    actionPaste->setEnabled(false);
    // Pixmap Capture source cannot be created
    actionCaptureSource->setEnabled(false);

    // Check if there is something that can be pasted
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (mimeData->hasText()) {

        QDomDocument doc;
        if (doc.setContent(mimeData->text())) {

            // is there a list of source ?
            QDomElement sourcelist = doc.firstChildElement("SourceList");
            if ( !sourcelist.isNull()) {

                // yes, there is something wich can be pasted
                actionPaste->setEnabled(true);
            }
        }
    }

    if (mimeData->hasImage()) {

        // yes, there is something wich can be pasted
        actionPaste->setEnabled(true);

        // also, the Pixmap Capture source can be created
        actionCaptureSource->setEnabled(true);
    }

}


void GLMixer::screenshotView(){

    // get screenshot from view GLWidget
    /*RenderingManager::getRenderingWidget()->makeCurrent();
    QImage s = RenderingManager::getRenderingWidget()->grabFrameBuffer();*/
    QPixmap s = QPixmap::grabWindow(RenderingManager::getRenderingWidget()->winId());
    if (s.isNull())
        return;
    // paint a cursor at the curent mouse coordinates
    QPainter p(&s);
    QPoint c = RenderingManager::getRenderingWidget()->mapFromGlobal( RenderingManager::getRenderingWidget()->cursor().pos() );
    p.drawPixmap(c, RenderingManager::getRenderingWidget()->cursor().pixmap());
    // create a unique filename and save to file
    QFileInfo f(QDir::home(), QString("glmixer_view_%1_%2.png").arg(QDate::currentDate().toString()).arg(QTime::currentTime().toString()) );

    s.save( f.absoluteFilePath() );
    // log
    qDebug() << f.absoluteFilePath() << QChar(124).toLatin1() << "Screenshot saved.";
}


void GLMixer::selectGLSLFragmentShader()
{
    QString newfile = getFileName(tr("Open GLSL File"),
                                  tr("GLSL Fragment Shader") + " (*.glsl *.fsh *.txt)" );
    if ( !newfile.isEmpty() && QFileInfo(newfile).isFile())
        // this will cause paintGL to reload shader
        ViewRenderWidget::glslShaderFile = newfile;
    else {
        // FORCE re-enable filtering
        ViewRenderWidget::setFilteringEnabled(false);
        ViewRenderWidget::setFilteringEnabled(true);
    }
}

void GLMixer::startSessionTestingBot()
{
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), switcherSession, SLOT(openSession()));
    timer->start(4000);

    qDebug() << "GLMixer" << QChar(124).toLatin1() << "Testing Bot started.";
}

QString GLMixer::getFileName(QString title, QString filter, QString saveExtention, QString suggestion)
{
    QString fileName = "";

    if (usesystemdialogs || filter.contains("bundle"))
    {
        static QDir dir(QDir::home());

        // open file or save file?
        if (!saveExtention.isNull())
            fileName = QFileDialog::getSaveFileName(this, title, dir.absolutePath() + "/" + suggestion, filter );
        else
            fileName = QFileDialog::getOpenFileName(this, title, dir.absolutePath(), filter );

        if (!fileName.isEmpty() && QFileInfo(fileName).exists())
            dir = QFileInfo(fileName).absoluteDir();

    } else {

        QFileDialog::AcceptMode mode = QFileDialog::AcceptOpen;

        sfd->setWindowTitle(title);
        sfd->setNameFilter(filter);
        sfd->selectFile(" ");

        // open file or save file?
        // if a saving extension is provided, the dialog is in save file selection mode
        if (!saveExtention.isNull()) {
            mode = QFileDialog::AcceptSave;
            sfd->setDefaultSuffix(saveExtention);
            if (!suggestion.isNull())
                sfd->selectFile(suggestion);
        }

        sfd->setAcceptMode(mode);
        sfd->setFileMode(mode == QFileDialog::AcceptOpen ? QFileDialog::ExistingFile : QFileDialog::AnyFile);

        if (sfd->exec())
            fileName = sfd->selectedFiles().front();

    }

    return fileName;
}

QStringList GLMixer::getMediaFileNames(bool &smartScalingRequest, bool &hwDecodingRequest) {

    QStringList fileNames;
    QFileInfo fi( _settings->value("recentMediaFile", "").toString() );
    QDir di(QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));

    // open dialog for openning media files> system QFileDialog, or custom (mfd)
    if (usesystemdialogs) {
        // restore location and file selection at recent file
        QString selectedfile = fi.isReadable() ? fi.absoluteFilePath() : di.absolutePath();
        // open standard dialog at location of recent file
        fileNames = QFileDialog::getOpenFileNames(this,
                                                  tr("GLMixer - Open videos or Pictures"),
                                                  selectedfile, VIDEOFILE_DIALOG_FORMATS );
    }
    else {
        // restore location and file selection at recent file
        if (fi.isReadable()) {
            mfd->setDirectory( fi.dir() );
            mfd->selectFile( fi.absoluteFilePath() );
        }
        else
            mfd->setDirectory( di );
        // open dialog
        if (mfd->exec()) {
            fileNames = mfd->selectedFiles();
            smartScalingRequest = mfd->requestCustomSize();
            hwDecodingRequest   = mfd->requestHarwareDecoder();
        }
    }

    if (!fileNames.empty())
        _settings->setValue("recentMediaFile", fileNames.front());

    return fileNames;
}

QString GLMixer::getMaskFileName(QString suggestion) {

    // try to help user by proposing suggested file
    QString previousfilename = suggestion;
    if (previousfilename.isEmpty())
        // the cource to operate on didn't have a custom mask : try with the one used here before
        previousfilename = _settings->value("previousCustomMaskImage", "").toString();

    // try to re-open where previous
    QFileInfo fi( previousfilename );
    QDir di(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    previousfilename = fi.isReadable() ? fi.absoluteFilePath() : di.absolutePath();
    // open file
    QString fileName = QFileDialog::getOpenFileName(NULL, "Open Custom Mask image", previousfilename, "Portable Network Graphics (*.png)" );

    // check validity of file
    QFileInfo fileInfo(fileName);
    if (fileInfo.isFile() && fileInfo.isReadable()) {
        _settings->setValue("previousCustomMaskImage", fileName);
        return fileInfo.absoluteFilePath() ;
    }
    else
        return QString("");
}

void GLMixer::on_actionWebsite_triggered() {

    QDesktopServices::openUrl(QUrl("https://sourceforge.net/projects/glmixer/", QUrl::TolerantMode));
}

void GLMixer::on_actionTutorials_triggered() {

    QDesktopServices::openUrl(QUrl("https://vimeo.com/album/2401475", QUrl::TolerantMode));
}

void GLMixer::undoChanged(bool undo, bool redo)
{
    actionUndo->setEnabled(undo);
    actionRedo->setEnabled(redo);

    // update display of source on calls of undo (i.e. redo enabled)
    if (redo)
        RenderingManager::getInstance()->refreshCurrentSource();
}

void GLMixer::updateWorkspaceActions()
{
    actionWorkspaceIncrement->setEnabled(WorkspaceManager::getInstance()->count()<WORKSPACE_MAX);
    actionWorkspaceDecrement->setEnabled(WorkspaceManager::getInstance()->count()>WORKSPACE_MIN);
    actionNewWorkspace->setEnabled(WorkspaceManager::getInstance()->count()<WORKSPACE_MAX);
    actionWorkspaceExclusive->setChecked(WorkspaceManager::getInstance()->isExclusiveDisplay());
}

void GLMixer::on_actionOSCTranslator_triggered()
{
#ifdef GLM_OSC
    static OpenSoundControlTranslator *oscwidget = new OpenSoundControlTranslator(_settings);
    oscwidget->show();
#endif
}


void GLMixer::setBusy(bool busy)
{
    static QTimer *timer = NULL;
    if (timer == NULL) {
        timer = new QTimer();
        timer->setSingleShot(true);

        connect(timer, SIGNAL(timeout()), RenderingManager::getRenderingWidget(), SLOT(setBusy()));
        connect(timer, SIGNAL(timeout()), this, SLOT(disable()));
    }

    timer->stop();

    if (busy)
        timer->start(150);
    else {
        RenderingManager::getRenderingWidget()->setBusy(false);
        setDisabled(false);
    }
}

void GLMixer::resetCurrentCursor()
{

    switch( cursorOptionWidget->currentIndex() ) {
    case 1:
        cursorSpringMass->setValue(5);
        break;
    case 2:
        cursorDelayLatency->setValue(1.0);
        cursorDelayFiltering->setValue(10);
        break;
    case 4:
        cursorLineSpeed->setValue(100);
        cursorLineWaitDuration->setValue(1.0);
        break;
    case 5:
        cursorFuzzyRadius->setValue(50);
        cursorFuzzyFiltering->setValue(5);
        break;
    case 6:
        cursorMagnetRadius->setValue(150);
        cursorMagnetStrength->setValue(0.7);
        break;
    default:
        break;
    }
}

QString GLMixer::getNotes() const
{
    return blocNoteEdit->toPlainText().trimmed();
}


void GLMixer::timerEvent ( QTimerEvent * event )
{
    session_elapsed_label->setText( getStringFromTime( (int) RenderingManager::getInstance()->getElapsedTime()) );
    output_elapsed_label->setText( getStringFromTime( (int) (_displayTimer.elapsed() / 1000.0) ) );
}

void GLMixer::setDisplayTimeEnabled(bool on)
{
    static int timerId = 0;

    // kill timer anyway
    if (timerId > 0) {
        killTimer(timerId);
        timerId = 0;
    }

    // remember status
    _displayTimerEnabled = on;

    // activate display update
    if (on) {
        // reset
        _displayTimer.restart();
        output_elapsed_label->setText( getStringFromTime( 0 ) );

        // start update
        timerId = startTimer(1000);

        QObject::connect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), output_recording_widget, SLOT(setVisible(bool)));
        QObject::connect(RenderingManager::getRecorder(), SIGNAL(timing(QString)), output_recording_label, SLOT(setText(QString)));
    }
    else {
        QObject::disconnect(RenderingManager::getRecorder(), SIGNAL(activated(bool)), output_recording_widget, SLOT(setVisible(bool)));
        QObject::disconnect(RenderingManager::getRecorder(), SIGNAL(timing(QString)), output_recording_label, SLOT(setText(QString)));
    }

    // labels visibility
    output_elapsed_widget->setVisible(on);

    // the recording timer
    output_recording_label->setText("");
    output_recording_widget->setVisible(false);
}


void GLMixer::setPerformanceModeEnabled(bool on)
{
    // Uncomment to copy window state to default static_windowstate :    qDebug() << "windowState" << saveState().toHex();

    /**
     * 1. Remember layout and status
     * */
    if (on) {
        // remember normal layout
        _settings->setValue("windowState", saveState());
//       qDebug() << "windowState" << saveState().toHex();
    } else {
        // remember performance layout
        _settings->setValue("performanceWindowState", saveState());
//       qDebug() << "performanceWindowState" << saveState().toHex();
    }

    /**
     * 2. Setup Widgets and toolbars
     * */
#ifdef GLM_SESSION
    // simple mode of session switcher
    switcherSession->setViewSimplified(on);
#endif
#ifdef GLM_SNAPSHOT
    // simple mode of session switcher
    snapshotManager->setViewSimplified(on);
#endif
    // disable actions menu
    actionPreferences->setDisabled(on);
    actionQuit->setDisabled(on);

    /**
     * 3. Apply layout
     * */
    if (on) {
        // apply performance layout, or default one
        if (_settings->contains("performanceWindowState"))
            restoreState(_settings->value("performanceWindowState").toByteArray());
        else {
            restoreState(static_performance_windowstate);
            // first time in live mode: explain
            QMessageBox::information(this, QCoreApplication::applicationName(), tr("Welcome to GLMixer Live Mode!\n\nPart of the interface is simplified and some critical actions are disabled.\n\nYou can adjust widgets and toolbars in a layout dedicated to your needs during life performance.\n\nDisable the Live Mode to return to standard editing mode."));
        }
    } else {
        // apply normal layout
        restoreState(_settings->value("windowState").toByteArray());
    }

}

ImageSaver::ImageSaver(QImage image, QString filename) : QThread(), _image(image), _filename(filename)
{

}

void ImageSaver::run()
{
    if ( !_filename.isEmpty() && !_image.isNull()) {
        if (!_image.save(_filename)) {
            qCritical() << _filename << QChar(124).toLatin1()<< tr("Could not save file.");
        } else {
            qDebug() << _filename << QChar(124).toLatin1() << tr("Image saved.");
        }
    }
}

void ImageSaver::saveImage(QImage image, QString filename)
{
    // create working thread
    ImageSaver *workerThread = new ImageSaver(image, filename);

    // kill thread when done
    QObject::connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));

    // start saving
    workerThread->start();
}
