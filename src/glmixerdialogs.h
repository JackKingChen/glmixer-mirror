#ifndef GLMIXERDIALOGS_H
#define GLMIXERDIALOGS_H

#include <QtGui>

#include "RenderingManager.h"
#include "SourcePropertyBrowser.h"
#include "SourceDisplayWidget.h"

class CaptureDialog: public QDialog {

public:
    QImage img;

    CaptureDialog(QWidget *parent, QImage capture, QString caption): QDialog(parent), img(capture) {

        QVBoxLayout *verticalLayout;
        QLabel *Question, *Display, *Info;
        QDialogButtonBox *DecisionButtonBox;

        setObjectName(QString::fromUtf8("CaptureDialog"));
        setWindowTitle(tr( "GLMixer - Frame capture"));
        verticalLayout = new QVBoxLayout(this);
        verticalLayout->setSpacing(9);

        Question = new QLabel(this);
        Question->setText(caption);
        verticalLayout->addWidget(Question);

        Display = new QLabel(this);
        Display->setPixmap(QPixmap::fromImage(img).scaledToWidth(300));
        verticalLayout->addWidget(Display);

        Info = new QLabel(this);
        Info->setText(tr("Original size: %1 x %2 px").arg(img.width()).arg(img.height()) );
        verticalLayout->addWidget(Info);

        DecisionButtonBox = new QDialogButtonBox(this);
        DecisionButtonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        verticalLayout->addWidget(DecisionButtonBox);

        QObject::connect(DecisionButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
        QObject::connect(DecisionButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    }

};

class SourceEditDialog: public QDialog {

public:
    Source *s;
    PropertyBrowser *specificSourcePropertyBrowser;
    SourcePropertyBrowser *sourcePropertyBrowser;
    SourceDisplayWidget *sourcedisplay;

    SourceEditDialog(QWidget *parent, Source *source, QString caption): QDialog(parent), s(source) {

        QVBoxLayout *verticalLayout;
        QFrame *hline;
        QDialogButtonBox *DecisionButtonBox;

        setObjectName(QString::fromUtf8("SourceEditDialog"));
        setWindowTitle(caption);
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        verticalLayout = new QVBoxLayout(this);
        verticalLayout->setSpacing(9);

        sourcedisplay = new SourceDisplayWidget(this, SourceDisplayWidget::GRID, true);
        sourcedisplay->setMinimumSize(QSize(160, 180));
        sourcedisplay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        sourcedisplay->setSource(s);
        verticalLayout->addWidget(sourcedisplay);

        sourcePropertyBrowser = new SourcePropertyBrowser(this);
        QObject::connect(RenderingManager::getInstance(), SIGNAL(currentSourceChanged(SourceSet::iterator)), sourcePropertyBrowser, SLOT(showProperties(SourceSet::iterator) ) );

        sourcePropertyBrowser->showProperties(s);
        sourcePropertyBrowser->setDisplayPropertyTree(false);
        sourcePropertyBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        verticalLayout->addWidget(sourcePropertyBrowser);

        hline = new QFrame(this);
        hline->setFrameShape(QFrame::HLine);
        verticalLayout->addWidget(hline);

        specificSourcePropertyBrowser = SourcePropertyBrowser::createSpecificPropertyBrowser(s, this);
        specificSourcePropertyBrowser->setDisplayPropertyTree(false);
        specificSourcePropertyBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        verticalLayout->addWidget(specificSourcePropertyBrowser);

        DecisionButtonBox = new QDialogButtonBox(this);
        DecisionButtonBox->setStandardButtons(QDialogButtonBox::Ok);
        verticalLayout->addWidget(DecisionButtonBox);

        QObject::connect(DecisionButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    }

    ~SourceEditDialog() {

        delete sourcePropertyBrowser;
        delete specificSourcePropertyBrowser;

    }

    QSize sizeHint() const {
        return QSize(500, 500);
    }
};



void setupAboutDialog(QDialog *AboutGLMixer)
{
    AboutGLMixer->resize(420, 270);
    AboutGLMixer->setWindowTitle("About GLMixer");
    QGridLayout *gridLayout = new QGridLayout(AboutGLMixer);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    QLabel *Icon = new QLabel(AboutGLMixer);
    Icon->setPixmap(QPixmap(QString::fromUtf8(":/glmixer/icons/glmixer_64x64.png")));
    QLabel *Title = new QLabel(AboutGLMixer);
    Title->setStyleSheet(QString::fromUtf8("font: 14pt \"Sans Serif\";"));
    QLabel *VERSION = new QLabel(AboutGLMixer);
    VERSION->setStyleSheet(QString::fromUtf8("font: 14pt \"Sans Serif\";"));
    QLabel *textsvn = new QLabel(AboutGLMixer);
    QLabel *SVN = new QLabel(AboutGLMixer);
    QTextBrowser *textBrowser = new QTextBrowser(AboutGLMixer);
    textBrowser->setOpenExternalLinks (true);
    QDialogButtonBox *validate = new QDialogButtonBox(AboutGLMixer);
    validate->setOrientation(Qt::Horizontal);
    validate->setStandardButtons(QDialogButtonBox::Close);

    gridLayout->addWidget(Icon, 0, 0, 1, 1);
    gridLayout->addWidget(Title, 0, 1, 1, 1);
    gridLayout->addWidget(VERSION, 0, 2, 1, 1);
    gridLayout->addWidget(textsvn, 1, 1, 1, 1);
    gridLayout->addWidget(SVN, 1, 2, 1, 1);
    gridLayout->addWidget(textBrowser, 2, 0, 1, 3);
    gridLayout->addWidget(validate, 3, 0, 1, 3);

    Icon->setText(QString());
    Title->setText(QObject::tr("Graphic Live Mixer"));
    textBrowser->setHtml(QObject::tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
    "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
    "p, li { white-space: pre-wrap; }\n"
    "</style></head><body style=\" font-family:'Sans Serif'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
    "<p>GLMixer is a video mixing software for live performance.</p>\n"
    "<p>Author:	Bruno Herbelin<br>\n"
    "Contact:	bruno.herbelin@gmail.com<br>\n"
    "License: 	GNU GPL version 3</p>\n"
    "<p>Copyright 2009-2014 Bruno Herbelin</p>\n"
    "<p>Updates and source code at: <br>\n"
    "   	<a href=\"http://sourceforge.net/projects/glmixer//\"><span style=\" text-decoration: underline; color:#7d400a;\">http://sourceforge.net/projects/glmixer/</span>"
    "</a></p>"
    "<p>GLMixer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation.</p>"
    "<p>GLMixer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details (see http://www.gnu.org/licenses).</p>"
    "</body></html>"));

    VERSION->setText( QString("%1").arg(QCoreApplication::applicationVersion()) );

#ifdef GLMIXER_REVISION
    SVN->setText(QString("%1").arg(GLMIXER_REVISION));
    textsvn->setText(QObject::tr("SVN repository revision:"));
#endif

    QObject::connect(validate, SIGNAL(rejected()), AboutGLMixer, SLOT(reject()));

}

#endif // GLMIXERDIALOGS_H
