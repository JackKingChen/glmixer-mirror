/*
 * SessionSwitcherWidget.cpp
 *
 *  Created on: Oct 1, 2010
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

#include <QDomDocument>

#include "common.h"
#include "glmixer.h"
#include "SessionSwitcher.h"
#include "OutputRenderWindow.h"
#include "SourceDisplayWidget.h"

#include "SessionSwitcherWidget.moc"

QModelIndex addFile(QStandardItemModel *model, const QString &name, const QDateTime &date, const QString &filename)
{

    QFile file(filename);
    if ( !file.open(QFile::ReadOnly | QFile::Text) )
        return QModelIndex();

    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;
    if ( !doc.setContent(&file, true, &errorStr, &errorLine, &errorColumn) )
        return QModelIndex();

    QDomElement root = doc.documentElement();
    if ( root.tagName() != "GLMixer" )
        return QModelIndex();

    QDomElement srcconfig = root.firstChildElement("SourceList");
    if ( srcconfig.isNull() )
        return QModelIndex();
    // get number of sources in the session
    int nbElem = srcconfig.childNodes().count();

    // get aspect ratio
    QString aspectRatio;
    standardAspectRatio ar = (standardAspectRatio) srcconfig.attribute("aspectRatio", "0").toInt();
    switch(ar) {
    case ASPECT_RATIO_FREE:
        aspectRatio = "free";
        break;
    case ASPECT_RATIO_16_10:
        aspectRatio = "16:10";
        break;
    case ASPECT_RATIO_16_9:
        aspectRatio = "16:9";
        break;
    case ASPECT_RATIO_3_2:
        aspectRatio = "3:2";
        break;
    default:
    case ASPECT_RATIO_4_3:
        aspectRatio = "4:3";
        break;
    }

    // read notes
    QString tooltip = filename;
    QDomElement notes = root.firstChildElement("Notes");
    if ( !notes.isNull() && !notes.text().isEmpty())
        tooltip = notes.text();

    file.close();

    model->insertRow(0);
    model->setData(model->index(0, 0), name);
    model->setData(model->index(0, 0), filename, Qt::UserRole);
    model->setData(model->index(0, 0), (int) ar, Qt::UserRole+1);
    model->itemFromIndex (model->index(0, 0))->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    model->itemFromIndex (model->index(0, 0))->setToolTip(tooltip);

    model->setData(model->index(0, 1), nbElem);
    model->setData(model->index(0, 1), filename, Qt::UserRole);
    model->itemFromIndex (model->index(0, 1))->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    model->itemFromIndex (model->index(0, 1))->setToolTip(tooltip);

    model->setData(model->index(0, 2), aspectRatio);
    model->setData(model->index(0, 2), filename, Qt::UserRole);
    model->itemFromIndex (model->index(0, 2))->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    model->itemFromIndex (model->index(0, 2))->setToolTip(tooltip);

    model->setData(model->index(0, 3), date.toString("yy/MM/dd hh:mm"));
    model->setData(model->index(0, 3), filename, Qt::UserRole);
    model->itemFromIndex (model->index(0, 3))->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    model->itemFromIndex (model->index(0, 3))->setToolTip(tooltip);

    return model->index(0, 0);
}


void FolderModelFiller::run()
{
    if (model) {

        // empty list
        if (model->rowCount() > 0)
            model->removeRows(0, model->rowCount());

        // here test for folder to exist with QFile Info
        QFileInfo folder(path);
        if (folder.exists() && folder.isDir()) {

            QDir dir(folder.absoluteFilePath());
            QFileInfoList fileList = dir.entryInfoList(QStringList("*.glm"), QDir::Files);

            // fill list
            for (int i = 0; i < fileList.size(); ++i) {
                QFileInfo fileinfo = fileList.at(i);
                addFile(model, fileinfo.completeBaseName(), fileinfo.lastModified(), fileinfo.absoluteFilePath());
            }
        }
    }
}

FolderModelFiller::FolderModelFiller(QObject *parent, QStandardItemModel *m, QString p)
    : QThread(parent), model(m), path(p)
{

}


SessionSwitcherWidget::SessionSwitcherWidget(QWidget *parent, QSettings *settings) : QWidget(parent),
    appSettings(settings), m_iconSize(48,48), nextSessionSelected(false), suspended(false)
{
    QGridLayout *g;

    transitionSelection = new QComboBox;
    transitionSelection->addItem("Instantaneous");
    transitionSelection->addItem("Fade to black");
    transitionSelection->addItem("Fade to custom color   -->");
    transitionSelection->addItem("Fade with last frame");
    transitionSelection->addItem("Fade with media file   -->");
    transitionSelection->setToolTip("Select the transition type");
    transitionSelection->setCurrentIndex(-1);

    /**
     * Tab automatic
     */
    transitionTab = new QTabWidget(this);
    transitionTab->setTabPosition(QTabWidget::East);
    transitionTab->setToolTip("Choose how you control the transition");
    transitionTab->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

    QLabel *transitionDurationLabel;
    transitionDuration = new QSpinBox;
    transitionDuration->setSingleStep(200);
    transitionDuration->setMaximum(5000);
    transitionDuration->setValue(1000);
    transitionDurationLabel = new QLabel(tr("&Duration (ms):"));
    transitionDurationLabel->setBuddy(transitionDuration);

    // create the curves into the transition easing curve selector
    easingCurvePicker = createCurveIcons();
    easingCurvePicker->setViewMode(QListView::IconMode);
    easingCurvePicker->setWrapping (false);
    easingCurvePicker->setIconSize(m_iconSize);
    easingCurvePicker->setFixedHeight(m_iconSize.height()+26);
    easingCurvePicker->setCurrentRow(3);

    transitionTab->addTab( new QWidget(), "Auto");
    g = new QGridLayout;
    g->setContentsMargins(6, 6, 6, 6);
    g->addWidget(transitionDurationLabel, 1, 0);
    g->addWidget(transitionDuration, 1, 1);
    g->addWidget(easingCurvePicker, 2, 0, 1, 2);
    transitionTab->widget(0)->setLayout(g);

    /**
     * Tab manual
     */
    currentSessionLabel = new QLabel;
    currentSessionLabel->setText(tr("100%"));
    currentSessionLabel->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding);
    currentSessionLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignBottom);

    overlayPreview = new SourceDisplayWidget(this);
    overlayPreview->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    overlayPreview->setMinimumSize(QSize(80, 60));

    nextSessionLabel = new QLabel;
    nextSessionLabel->setText(tr("None"));
    nextSessionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    nextSessionLabel->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);
    nextSessionLabel->setStyleSheet("QLabel::disabled {\ncolor: rgb(128, 0, 0);\n}");

    transitionSlider = new QSlider;
    transitionSlider->setMinimum(-100);
    transitionSlider->setMaximum(101);
    transitionSlider->setValue(-100);
    transitionSlider->setOrientation(Qt::Horizontal);
    transitionSlider->setTickPosition(QSlider::TicksAbove);
    transitionSlider->setTickInterval(100);
    transitionSlider->setEnabled(false);

    transitionTab->addTab( new QWidget(), "Manual");
    g = new QGridLayout;
    g->setContentsMargins(6, 6, 6, 6);
    g->addWidget(currentSessionLabel, 0, 0);
    g->addWidget(overlayPreview, 0, 1);
    g->addWidget(nextSessionLabel, 0, 2);
    g->addWidget(transitionSlider, 1, 0, 1, 3);
    transitionTab->widget(1)->setLayout(g);

    /**
     * Folder view
     */

    folderModel = new QStandardItemModel(0, 4, this);
    folderModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Filename"));
    folderModel->setHeaderData(1, Qt::Horizontal, QString("n"));
    folderModel->setHeaderData(2, Qt::Horizontal, QObject::tr("W:H"));
    folderModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Modified"));
    sortingColumn = 3;
    sortingOrder = Qt::AscendingOrder;
    folderModel->sort(sortingColumn, sortingOrder);

    proxyFolderModel = new QSortFilterProxyModel;
    proxyFolderModel->setDynamicSortFilter(true);
    proxyFolderModel->setFilterKeyColumn(0);
    proxyFolderModel->setSourceModel(folderModel);

    QToolButton *dirButton = new QToolButton;
    dirButton->setToolTip("Add a folder to the list");
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/glmixer/icons/folderadd.png"), QSize(), QIcon::Normal, QIcon::Off);
    dirButton->setIcon(icon);

    QToolButton *dirDeleteButton = new QToolButton;
    dirDeleteButton->setToolTip("Remove a folder from the list");
    QIcon icon2;
    icon2.addFile(QString::fromUtf8(":/glmixer/icons/fileclose.png"), QSize(), QIcon::Normal, QIcon::Off);
    dirDeleteButton->setIcon(icon2);

    customButton = new QToolButton(this);
    customButton->setIcon( QIcon() );
    customButton->setVisible(false);

    folderHistory = new QComboBox(this);
    folderHistory->setToolTip("List of folders containing session files");
//    folderHistory->setEditable(true);
    folderHistory->setValidator(new folderValidator(this));
    folderHistory->setInsertPolicy (QComboBox::InsertAtTop);
    folderHistory->setMaxCount(MAX_RECENT_FOLDERS);
    //	folderHistory->setMaximumWidth(250);
    folderHistory->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    folderHistory->setDuplicatesEnabled(false);

    proxyView = new QTreeView(this);
    proxyView->setRootIsDecorated(false);
    proxyView->setAlternatingRowColors(true);
    proxyView->setSortingEnabled(false);
    proxyView->sortByColumn(sortingColumn, sortingOrder);
    proxyView->setModel(proxyFolderModel);
    proxyView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    proxyView->header()->setResizeMode(QHeaderView::Interactive);
    proxyView->header()->resizeSection(1, 25);
    proxyView->header()->resizeSection(2, 35);
    //    proxyView->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));

    proxyView->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *reloadAction = new QAction(tr("Reload folder"), proxyView);
    proxyView->insertAction(0, reloadAction);
    QObject::connect(reloadAction, SIGNAL(triggered()), this, SLOT(reloadFolder()) );
    QAction *openUrlAction = new QAction(tr("Show folder in browser"), proxyView);
    proxyView->insertAction(0, openUrlAction);
    QObject::connect(openUrlAction, SIGNAL(triggered()), this, SLOT(browseFolder()) );
    QAction *deleteSessionAction = new QAction(tr("Delete file"), proxyView);
    proxyView->insertAction(0, deleteSessionAction);
    QObject::connect(deleteSessionAction, SIGNAL(triggered()), this, SLOT(deleteSession()) );


    connect(dirButton, SIGNAL(clicked()),  this, SLOT(openFolder()));
    connect(dirDeleteButton, SIGNAL(clicked()),  this, SLOT(discardFolder()));
    connect(customButton, SIGNAL(clicked()),  this, SLOT(customizeTransition()));
    connect(folderHistory, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(folderChanged(const QString &)));
    connect(transitionSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setTransitionType(int)));
    connect(transitionDuration, SIGNAL(valueChanged(int)), RenderingManager::getSessionSwitcher(), SLOT(setTransitionDuration(int)));
    connect(easingCurvePicker, SIGNAL(currentRowChanged (int)), RenderingManager::getSessionSwitcher(), SLOT(setTransitionCurve(int)));
    connect(transitionSlider, SIGNAL(valueChanged(int)), this, SLOT(transitionSliderChanged(int)));
    connect(transitionTab, SIGNAL(currentChanged(int)), this, SLOT(setTransitionMode(int)));

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(transitionSelection, 0, 0, 1, 3);
    mainLayout->addWidget(customButton, 0, 3);
    mainLayout->addWidget(transitionTab, 1, 0, 1, 4);
    mainLayout->addWidget(proxyView, 2, 0, 1, 4);
    mainLayout->addWidget(folderHistory, 3, 0, 1, 2);
    mainLayout->addWidget(dirButton, 3, 2);
    mainLayout->addWidget(dirDeleteButton, 3, 3);
    setLayout(mainLayout);

}

SessionSwitcherWidget::~SessionSwitcherWidget()
{
    // remember
    appSettings->setValue("transitionSortingColumn", proxyFolderModel->sortColumn());
    appSettings->setValue("transitionSortingOrder", (int) proxyFolderModel->sortOrder());

    delete proxyFolderModel;
}

void SessionSwitcherWidget::reloadFolder()
{
    QFileInfo sessionFolder( folderHistory->currentText() );

    if ( sessionFolder.exists() )
        folderChanged( sessionFolder.absoluteFilePath() );

}

void SessionSwitcherWidget::browseFolder()
{
    QFileInfo sessionFolder( folderHistory->currentText() );

    if ( sessionFolder.exists() ) {

        QUrl sessionURL = QUrl::fromLocalFile( sessionFolder.absoluteFilePath() ) ;

        if ( sessionURL.isValid() )
            QDesktopServices::openUrl(sessionURL);
    }
}

void SessionSwitcherWidget::deleteSession()
{
    QFileInfo sessionFile( proxyFolderModel->data(proxyView->currentIndex(), Qt::UserRole).toString() );

    if ( sessionFile.isFile() ) {
        QDir sessionDir(sessionFile.canonicalPath());

        sessionDir.remove(sessionFile.fileName());

        reloadFolder();
    }
}

void SessionSwitcherWidget::fileChanged(const QString & filename )
{
    QFileInfo fileinfo(filename);

    if ( fileinfo.exists() && fileinfo.isFile() ) {

        // if the openning folder function returns true, it did change folder and so the list is updated
        if (openFolder(fileinfo.absolutePath()))
            return;

        // the folder is already the good one, change only the file item
        if (folderModelAccesslock.tryLock(100)) {

            // look for item in the list
            QList<QStandardItem *> items = folderModel->findItems( fileinfo.completeBaseName() );

            // remove all items found
            foreach (const QStandardItem *item, items)
                folderModel->removeRow( item->row() );

            // (re)insert the file into the model
            QModelIndex index = addFile(folderModel, fileinfo.completeBaseName(), fileinfo.lastModified(), fileinfo.absoluteFilePath());

            // idem was added, select it
            if ( index.isValid() )
                proxyView->setCurrentIndex( proxyFolderModel->mapFromSource( index ) );

            folderModelAccesslock.unlock();
        }
    }
}

void SessionSwitcherWidget::folderChanged(const QString & foldername )
{
    // remove (if exist) the path from list and reorder it
    QStringList folders = appSettings->value("recentFolderList").toStringList();
    folders.removeAll(foldername);
    folders.prepend(foldername);
    // limit list size
    while (folders.size() > MAX_RECENT_FOLDERS)
        folders.removeLast();
    appSettings->setValue("recentFolderList", folders);

    folderHistory->updateGeometry();

    folderModelAccesslock.lock();

    // remember sorting before disabling it temporarily
    sortingColumn = proxyFolderModel->sortColumn();
    sortingOrder = proxyFolderModel->sortOrder();
    proxyView->setSortingEnabled(false);

    // Threaded version of fillFolderModel(folderModel, text);
    FolderModelFiller *workerThread = new FolderModelFiller(this, folderModel, foldername);
    if (!workerThread)
        return;

    setEnabled(false);
    connect(workerThread, SIGNAL(finished()), this, SLOT(restoreFolderView()));
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
    workerThread->start();

}


void SessionSwitcherWidget::restoreFolderView()
{
    // restore sorting
    folderModel->sort(sortingColumn, sortingOrder);
    proxyView->setSortingEnabled(true);
    proxyView->sortByColumn(sortingColumn, sortingOrder);

    // restore availability
    folderModelAccesslock.unlock();
    setEnabled(true);
}

bool SessionSwitcherWidget::openFolder(QString directory)
{
    // open file dialog if no directory is given
    QString dirName;
    if ( directory.isNull() )
        dirName = QFileDialog::getExistingDirectory(0, QObject::tr("Select a directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly );
    else
        dirName = directory;

    // make sure we have a valid directory
    QFileInfo sessionFolder (dirName);
    if ( !sessionFolder.isDir() )
        return false;

    // nothing to do is its already the current directory
    QFileInfo currentFolder( folderHistory->currentText() );
    if ( sessionFolder == currentFolder )
        return false;

    // find the index of the given directory
    int index = folderHistory->findText(dirName);

    // if not found, then insert it
    if ( index < 0 ) {
        // (disable signal to not trigger reloading of the list)
        disconnect(folderHistory, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(folderChanged(const QString &)));

        folderHistory->insertItem(0, sessionFolder.absoluteFilePath());
        index = 0;

        // (reconnect signal)
        connect(folderHistory, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(folderChanged(const QString &)));
    }

    // change the current index (this triggers the reloading of the list)
    folderHistory->setCurrentIndex( index );

    return true;
}

void SessionSwitcherWidget::discardFolder()
{
    if (folderHistory->count() > 0) {
        // remove the folder from the settings
        QStringList folders = appSettings->value("recentFolderList").toStringList();
        folders.removeAll( folderHistory->currentText() );
        appSettings->setValue("recentFolderList", folders);

        // remove the item
        folderHistory->removeItem(folderHistory->currentIndex());
    }
}


void SessionSwitcherWidget::startTransitionToSession(const QModelIndex & index)
{
    // transfer info to glmixer
    emit sessionTriggered(proxyFolderModel->data(index, Qt::UserRole).toString());

    // make sure no other events are accepted until the end of the transition
    disconnect(proxyView, SIGNAL(activated(QModelIndex)), this, SLOT(startTransitionToSession(QModelIndex) ));
    proxyView->setEnabled(false);
    QTimer::singleShot( transitionSelection->currentIndex() > 0 ? transitionDuration->value() : 100, this, SLOT(restoreTransition()));
}

void SessionSwitcherWidget::startTransitionToNextSession()
{
    startTransitionToSession( proxyView->indexBelow (proxyView->currentIndex())	);
    proxyView->setCurrentIndex( proxyView->indexBelow (proxyView->currentIndex())	);
}

void SessionSwitcherWidget::startTransitionToPreviousSession()
{
    startTransitionToSession( proxyView->indexAbove (proxyView->currentIndex())	);
    proxyView->setCurrentIndex( proxyView->indexAbove (proxyView->currentIndex())	);
}

void SessionSwitcherWidget::selectSession(const QModelIndex & index)
{
    // read file name
    nextSession = proxyFolderModel->data(index, Qt::UserRole).toString();
    transitionSlider->setEnabled(true);
    currentSessionLabel->setEnabled(true);
    nextSessionLabel->setEnabled(true);

    // display that we can do transition to new selected session
    nextSessionLabel->setText(QString("0% %1").arg(QFileInfo(nextSession).baseName()));
}

void SessionSwitcherWidget::setTransitionType(int t)
{
    SessionSwitcher::transitionType tt = (SessionSwitcher::transitionType) CLAMP(SessionSwitcher::TRANSITION_NONE, t, SessionSwitcher::TRANSITION_CUSTOM_MEDIA);
    RenderingManager::getSessionSwitcher()->setTransitionType( tt );

    customButton->setStyleSheet("");
    //	transitionTab->setEnabled(tt != SessionSwitcher::TRANSITION_NONE);
    transitionTab->setVisible(tt != SessionSwitcher::TRANSITION_NONE);
    // hack ; NONE transition type should emulate automatic transition mode
    setTransitionMode(tt == SessionSwitcher::TRANSITION_NONE ? 0 : transitionTab->currentIndex());

    if ( tt == SessionSwitcher::TRANSITION_CUSTOM_COLOR ) {
        QPixmap c = QPixmap(16, 16);
        c.fill(RenderingManager::getSessionSwitcher()->transitionColor());
        customButton->setIcon( QIcon(c) );
        customButton->setVisible(true);
        customButton->setToolTip("Choose color");

    } else if ( tt == SessionSwitcher::TRANSITION_CUSTOM_MEDIA ) {
        customButton->setIcon(QIcon(QString::fromUtf8(":/glmixer/icons/folderopen.png")));

        if ( !QFileInfo(RenderingManager::getSessionSwitcher()->transitionMedia()).exists() )
            customButton->setStyleSheet("QToolButton { border: 1px solid red }");
        customButton->setVisible(true);
        customButton->setToolTip("Choose media");
    } else
        customButton->setVisible(false);

}


void SessionSwitcherWidget::customizeTransition()
{
    if (RenderingManager::getSessionSwitcher()->getTransitionType() == SessionSwitcher::TRANSITION_CUSTOM_COLOR ) {

        QColor color = QColorDialog::getColor(RenderingManager::getSessionSwitcher()->transitionColor(), parentWidget());
        if (color.isValid()) {
            RenderingManager::getSessionSwitcher()->setTransitionColor(color);
            setTransitionType( (int) SessionSwitcher::TRANSITION_CUSTOM_COLOR);
        }
    }
    else if (RenderingManager::getSessionSwitcher()->getTransitionType() == SessionSwitcher::TRANSITION_CUSTOM_MEDIA ) {

        bool generatePowerOfTwoRequested = false;
        QStringList fileNames = GLMixer::getInstance()->getMediaFileNames(generatePowerOfTwoRequested);

        // media file dialog returns a list of filenames :
        if (!fileNames.empty() && QFileInfo(fileNames.front()).exists()) {

            RenderingManager::getSessionSwitcher()->setTransitionMedia(fileNames.front(), generatePowerOfTwoRequested);
            customButton->setStyleSheet("");
        }
        // no valid file name was given
        else {
            // not a valid file ; show a warning only if the QFileDialog did not return null (cancel)
            if (!fileNames.empty() && !fileNames.front().isNull())
                qCritical() << fileNames.front() << QChar(124).toLatin1() << QObject::tr("File does not exist.");
            // if no valid oldfile neither; show icon in red
            if (RenderingManager::getSessionSwitcher()->transitionMedia().isEmpty())
                customButton->setStyleSheet("QToolButton { border: 1px solid red }");
        }
    }
    // remember
    saveSettings();
}


void SessionSwitcherWidget::saveSettings()
{
    appSettings->setValue("transitionSelection", transitionSelection->currentIndex());
    appSettings->setValue("transitionDuration", transitionDuration->value());
    appSettings->setValue("transitionCurve", easingCurvePicker->currentRow());
    appSettings->setValue("transitionTab", transitionTab->currentIndex());

    if (RenderingManager::getSessionSwitcher()) {
        QVariant variant = RenderingManager::getSessionSwitcher()->transitionColor();
        appSettings->setValue("transitionColor", variant);
        appSettings->setValue("transitionMedia", RenderingManager::getSessionSwitcher()->transitionMedia());
    }

    if (proxyFolderModel) {
        appSettings->setValue("transitionSortingColumn", proxyFolderModel->sortColumn());
        appSettings->setValue("transitionSortingOrder", (int) proxyFolderModel->sortOrder());
    }
}

void SessionSwitcherWidget::restoreSettings()
{
    // list of folders
    QStringList folders(QDir::currentPath());
    if ( appSettings->contains("recentFolderList") )
        folders = appSettings->value("recentFolderList").toStringList();

    // apply folder list
    disconnect(folderHistory, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(folderChanged(const QString &)));
    folderHistory->addItems( folders );
    folderHistory->setCurrentIndex(0);
    connect(folderHistory, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(folderChanged(const QString &)));

    // order of transition
    sortingColumn = appSettings->value("transitionSortingColumn", "3").toInt();
    sortingOrder = (Qt::SortOrder) appSettings->value("transitionSortingOrder", "0").toInt();
    folderModel->sort(sortingColumn, sortingOrder);
    proxyView->sortByColumn(sortingColumn, sortingOrder);

    // read folder
    folderChanged(folderHistory->itemText(0));

    RenderingManager::getSessionSwitcher()->setTransitionColor( appSettings->value("transitionColor").value<QColor>());

    QString mediaFileName = appSettings->value("transitionMedia", "").toString();
    if (QFileInfo(mediaFileName).exists())
        RenderingManager::getSessionSwitcher()->setTransitionMedia(mediaFileName);

    transitionSelection->setCurrentIndex(appSettings->value("transitionSelection", "0").toInt());
    transitionDuration->setValue(appSettings->value("transitionDuration", "1000").toInt());
    easingCurvePicker->setCurrentRow(appSettings->value("transitionCurve", "3").toInt());
    transitionTab->setCurrentIndex(appSettings->value("transitionTab", 0).toInt());

    connect(transitionSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(saveSettings()));
    connect(transitionDuration, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
    connect(easingCurvePicker, SIGNAL(currentRowChanged (int)), this, SLOT(saveSettings()));
    connect(transitionTab, SIGNAL(currentChanged(int)), this, SLOT(saveSettings()));
}

QListWidget *SessionSwitcherWidget::createCurveIcons()
{
    QListWidget *easingCurvePicker = new QListWidget;
    QPixmap pix(m_iconSize);
    QPainter painter(&pix);

    // Skip QEasingCurve::Custom
    for (int i = 0; i < QEasingCurve::NCurveTypes - 3; ++i) {
        painter.fillRect(QRect(QPoint(0, 0), m_iconSize), QApplication::palette().base());
        QEasingCurve curve((QEasingCurve::Type)i);
        painter.setPen(QApplication::palette().alternateBase().color());
        qreal xAxis = m_iconSize.height()/1.15;
        qreal yAxis = m_iconSize.width()/5.5;
        painter.drawLine(0, xAxis, m_iconSize.width(),  xAxis);
        painter.drawLine(yAxis, 0, yAxis, m_iconSize.height());

        qreal curveScale = m_iconSize.height()/1.4;

        painter.setPen(Qt::NoPen);

        // start point
        painter.setBrush(Qt::red);
        QPoint start(yAxis, xAxis - curveScale * curve.valueForProgress(0));
        painter.drawRect(start.x() - 1, start.y() - 1, 3, 3);

        // end point
        painter.setBrush(Qt::blue);
        QPoint end(yAxis + curveScale, xAxis - curveScale * curve.valueForProgress(1));
        painter.drawRect(end.x() - 1, end.y() - 1, 3, 3);

        QPainterPath curvePath;
        curvePath.moveTo(start);
        for (qreal t = 0; t <= 1.0; t+=1.0/curveScale) {
            QPoint to;
            to.setX(yAxis + curveScale * t);
            to.setY(xAxis - curveScale * curve.valueForProgress(t));
            curvePath.lineTo(to);
        }
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.strokePath(curvePath, QApplication::palette().text().color());
        painter.setRenderHint(QPainter::Antialiasing, false);
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(pix));
        //        item->setText(metaEnum.key(i));
        easingCurvePicker->addItem(item);
    }
    return easingCurvePicker;
}



void SessionSwitcherWidget::setAllowedAspectRatio(const standardAspectRatio ar)
{
    if (folderModelAccesslock.tryLock(100)) {

        // quick redisplay of folder list
        for (int r = 0; r < folderModel->rowCount(); ++r )
        {
            standardAspectRatio sar = (standardAspectRatio) folderModel->data(folderModel->index(r, 0), Qt::UserRole+1).toInt();
            Qt::ItemFlags flags = Qt::ItemIsSelectable;
            if (ar == ASPECT_RATIO_FREE || sar == ar)
                flags |= Qt::ItemIsEnabled;

            folderModel->itemFromIndex(folderModel->index(r, 0))->setFlags (flags);
            folderModel->itemFromIndex(folderModel->index(r, 1))->setFlags (flags);
            folderModel->itemFromIndex(folderModel->index(r, 2))->setFlags (flags);
            folderModel->itemFromIndex(folderModel->index(r, 3))->setFlags (flags);
        }
    }

    folderModelAccesslock.unlock();
}


void SessionSwitcherWidget::resetTransitionSlider()
{
    // enable / disable transition slider
    transitionSlider->setEnabled(nextSessionSelected);
    currentSessionLabel->setEnabled(nextSessionSelected);
    nextSessionLabel->setEnabled(nextSessionSelected);
    // enable / disable changing session
    proxyView->setEnabled(!nextSessionSelected);
    // clear the selection
    proxyView->clearSelection();

    // ensure correct re-display
    if (!nextSessionSelected) {
        RenderingManager::getSessionSwitcher()->setTransitionType(RenderingManager::getSessionSwitcher()->getTransitionType());
        nextSessionLabel->setText(QObject::tr("None"));
    }
}

void  SessionSwitcherWidget::setTransitionMode(int m)
{
    resetTransitionSlider();

    // mode is manual (and not with instantaneous transition selected)
    if ( m == 1  && transitionSelection->currentIndex() > 0) {
        RenderingManager::getSessionSwitcher()->manual_mode = true;
        // adjust slider to represent current transparency
        transitionSlider->setValue(RenderingManager::getSessionSwitcher()->overlay() * 100.f - (nextSessionSelected?0.f:100.f));
        // single clic to select next session
        proxyView->setToolTip("Click on a session to choose target session");
        disconnect(proxyView, SIGNAL(activated(QModelIndex)), this, SLOT(startTransitionToSession(QModelIndex) ));
        connect(proxyView, SIGNAL(activated(QModelIndex)), this, SLOT(selectSession(QModelIndex) ));
    }
    // mode is automatic
    else {
        RenderingManager::getSessionSwitcher()->manual_mode = false;
        // enable changing session
        proxyView->setEnabled(true);
        //  activate transition to next session (double clic or Return)
        proxyView->setToolTip("Double click on a session to initiate the transition");
        connect(proxyView, SIGNAL(activated(QModelIndex)), this, SLOT(startTransitionToSession(QModelIndex) ));
        disconnect(proxyView, SIGNAL(activated(QModelIndex)), this, SLOT(selectSession(QModelIndex) ));
    }

}

void SessionSwitcherWidget::restoreTransition()
{
    setTransitionMode(transitionTab->currentIndex());

    // restore focus and selection in tree view
    proxyView->setFocus();
    proxyView->setCurrentIndex(proxyView->currentIndex());
}

void SessionSwitcherWidget::transitionSliderChanged(int t)
{
    // apply transition
    RenderingManager::getSessionSwitcher()->setTransparency( ABS(t) );

    if (suspended) {
        transitionSlider->setValue(0);
        return;
    }

    if ( nextSessionSelected ) {

        // display that we can do transition to new selected session
        nextSessionLabel->setText(QString("%1% %2").arg(ABS(t)).arg(QFileInfo(nextSession).baseName()));

        // prevent coming back to previous
        if (t < 0) {
            transitionSlider->setValue(0);
        } else

            // detect end of transition
            if ( t > 100 ) {
                // reset
                nextSessionSelected = false;
                RenderingManager::getSessionSwitcher()->endTransition();
                // no target
                transitionSlider->setValue(-100);
                resetTransitionSlider();
            }


    } else {

        // detect change of session
        if ( t >= 0 ){
            nextSessionSelected = true;
            suspended = true;
            resetTransitionSlider();
            // request to load session file
            emit sessionTriggered(nextSession);

        } else if (t > -100 && RenderingManager::getSessionSwitcher()->getTransitionType() == SessionSwitcher::TRANSITION_CUSTOM_MEDIA) {
            overlayPreview->playSource(true);
        }
        // show percent of mixing
        currentSessionLabel->setText(QString("%1%").arg(ABS(t)));
    }
}


void SessionSwitcherWidget::setTransitionSourcePreview(Source *s)
{
    // set the overlay source
    overlayPreview->setSource(s);
}

void SessionSwitcherWidget::unsuspend()
{
    suspended = false;
}
