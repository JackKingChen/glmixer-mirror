/*
 * MixingToolboxWidget.cpp
 *
 *  Created on: Sep 2, 2012
 *      Author: bh
 */

#include <QColorDialog>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>

#include "common.h"
#include "RenderingManager.h"
#include "GammaLevelsWidget.h"
#include "SourcePropertyBrowser.h"
#include "SourceDisplayWidget.h"
#include "ViewRenderWidget.h"
#include "glmixer.h"
#include "glmixerdialogs.h"
#include "UndoManager.h"

#ifdef GLM_FFGL
#include "FFGLPluginSource.h"
#include "FFGLPluginSourceShadertoy.h"
#include "FFGLPluginBrowser.h"
#include "FFGLEffectSelectionDialog.h"
#endif

#include "MixingToolboxWidget.moc"

// some presets as a string of Hex values: to get from console on program exit
QByteArray static_presets = QByteArray(
"<PresetsList>\n <Source preset=\"Reset\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"0\" Contrast=\"0\" Lumakey=\"0\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.00000000\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"1.00000000\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Quadrichromy\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"1\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"3\" Brightness=\"0\" Saturation=\"100\" Contrast=\"0\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.08873720\" maxOutput=\"1.00000000\" maxInput=\"0.48805460\" value=\"1.65\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Photography\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"17\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"-100\" Contrast=\"20\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.00000000\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"1.17\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Drawing\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"15\" Function=\"771\"/>\n  <Filter InvertMode=\"2\" Filter=\"4\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"-100\" Contrast=\"0\" luminanceThreshold=\"40\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.16064847\" maxOutput=\"1.00000000\" maxInput=\"0.76450515\" value=\"1.53\" minOutput=\"0.25619835\"/>\n  </Source>\n <Source preset=\"Television\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"221\" R=\"188\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"14\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"1\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"0\" Contrast=\"0\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.00000000\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"2.15\" minOutput=\"0.0090\"/>\n  </Source>\n <Source preset=\"Diapo\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"18\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"3\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"0\" Contrast=\"-21\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.33898300\" maxOutput=\"0.86363602\" maxInput=\"0.79096001\" value=\"4.40\" minOutput=\"0.10000000\"/>\n  </Source>\n <Source preset=\"Impressionist\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"12\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"12\" Pixelated=\"1\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"50\" Contrast=\"-10\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.11945392\" maxOutput=\"1.00000000\" maxInput=\"0.84300339\" value=\"1.77\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Hypersaturated\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"100\" Contrast=\"0\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.12637363\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"1.55\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Desaturated\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"-100\" Contrast=\"0\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.00000000\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"1.00000000\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Paper\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"19\" Function=\"771\"/>\n  <Filter InvertMode=\"2\" Filter=\"1\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Saturation=\"-100\" Contrast=\"0\" luminanceThreshold=\"65\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.11945392\" maxOutput=\"0.87603307\" maxInput=\"0.87030715\" value=\"1.80\" minOutput=\"0.06887054\"/>\n  </Source>\n <Source preset=\"Sepia\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"131\" R=\"154\" B=\"104\"/>\n  <Blending Equation=\"32774\" Mask=\"13\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"2\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"25\" Saturation=\"-100\" Contrast=\"20\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.10000000\" maxOutput=\"1.00000000\" maxInput=\"0.89999998\" value=\"0.94\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Greyscale\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"10\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"5\" Brightness=\"20\" Saturation=\"-100\" Contrast=\"50\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.00000000\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"1.00000000\" minOutput=\"0.00000000\"/>\n  </Source>\n <Source preset=\"Psychedelic\">\n  <Depth Z=\"40.00000000\"/>\n  <Color G=\"215\" R=\"255\" B=\"252\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"15\" Pixelated=\"1\"/>\n  <Coloring Hueshift=\"179\" numberOfColors=\"8\" Brightness=\"0\" Saturation=\"100\" Contrast=\"0\" luminanceThreshold=\"0\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.48901099\" maxOutput=\"1.00000000\" maxInput=\"0.72527474\" value=\"1.38\" minOutput=\"0.20083684\"/>\n  </Source>\n <Source preset=\"Mask\">\n  <Depth Z=\"40.000\"/>\n  <Color G=\"255\" R=\"255\" B=\"255\"/>\n  <Blending Equation=\"32774\" Mask=\"0\" Function=\"771\"/>\n  <Filter InvertMode=\"0\" Filter=\"0\" Pixelated=\"0\"/>\n  <Coloring Hueshift=\"0\" numberOfColors=\"0\" Brightness=\"0\" Lumakey=\"55\" Saturation=\"0\" Contrast=\"0\" luminanceThreshold=\"3\"/>\n  <Chromakey Tolerance=\"10\" G=\"255\" R=\"0\" B=\"0\" on=\"0\"/>\n  <Gamma minInput=\"0.00000000\" maxOutput=\"1.00000000\" maxInput=\"1.00000000\" value=\"1.00000000\" red=\"1.00000000\" blue=\"1.00000000\" minOutput=\"0.00000000\" green=\"1.00000000\"/>\n </Source>\n</PresetsList>\n" );


void setItemTooltip(QListWidgetItem *item, QDomElement xmlconfig)
{
    QDomElement tmp;
    QStringList tooltip;
    tooltip << QString("Preset '%1'\n").arg(xmlconfig.attribute("preset"));

    tmp = xmlconfig.firstChildElement("Blending");
    tooltip << QString("Blending  \t%1").arg( namePresetFromInt(intFromBlendingPreset( (uint) tmp.attribute("Function", "1").toInt(), (uint) tmp.attribute("Equation", "32774").toInt() ) )  );

    if ( tmp.attribute("Mask", "0").toInt() != 0 )
        tooltip << QString("Mask     \t%1").arg(ViewRenderWidget::getMaskDecription()[tmp.attribute("Mask", "0").toInt()].first  );

    tmp = xmlconfig.firstChildElement("Color");
    QColor color( tmp.attribute("R", "255").toInt(),
                  tmp.attribute("G", "255").toInt(),
                  tmp.attribute("B", "255").toInt() );
    if ( color != QColor(255, 255, 255, 255)  )
        tooltip << QString("Color   \t(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());

    tmp = xmlconfig.firstChildElement("Filter");
    if ( tmp.attribute("Pixelated", "0").toInt() )
        tooltip << QString("Pixelated\tON");
    if ( tmp.attribute("InvertMode", "0").toInt() == (int) Source::INVERT_COLOR )
        tooltip << QString("Inversion \tRGB");
    else if ( tmp.attribute("InvertMode", "0").toInt() == (int) Source::INVERT_LUMINANCE )
        tooltip << QString("Inversion \tLuminance");
    if ( tmp.attribute("Filter", "0").toInt() != 0 )
        tooltip << QString("Filter    \t%1").arg(Source::getFilterNames()[tmp.attribute("Filter").toInt()] );

    tmp = xmlconfig.firstChildElement("Coloring");
    if ( tmp.attribute("Brightness", "0").toInt() != 0 )
        tooltip << QString("Brightness\t%1").arg(tmp.attribute("Brightness").toInt());
    if ( tmp.attribute("Contrast", "0").toInt() != 0 )
        tooltip << QString("Contrast  \t%1").arg(tmp.attribute("Contrast").toInt());
    if ( tmp.attribute("Saturation", "0").toInt() != 0 )
        tooltip << QString("Saturation\t%1").arg(tmp.attribute("Saturation").toInt() );
    if ( tmp.attribute("Hueshift", "0").toInt()  != 0 )
        tooltip << QString("HueShift  \t%1").arg(tmp.attribute("Hueshift").toInt() );
    if ( tmp.attribute("luminanceThreshold", "0").toInt() != 0 )
        tooltip << QString("Threshold \t%1").arg(tmp.attribute("luminanceThreshold").toInt());
    if ( tmp.attribute("Lumakey", "0").toInt() != 0 )
        tooltip << QString("Lumakey \t%1").arg(tmp.attribute("Lumakey").toInt());
    if ( tmp.attribute("numberOfColors", "0").toInt() != 0 )
        tooltip << QString("Posterized\t%1").arg(tmp.attribute("numberOfColors").toInt());

    tmp = xmlconfig.firstChildElement("Gamma");
    if ( qAbs(tmp.attribute("value", "1").toDouble() - 1.0) > 0.001  )
        tooltip << QString("Gamma     \t%1").arg(tmp.attribute("value").toDouble());

    tmp = xmlconfig.firstChildElement("Chromakey");
    if ( tmp.attribute("on", "0").toInt() ) {
        QColor color( tmp.attribute("R", "255").toInt(),
                      tmp.attribute("G", "255").toInt(),
                      tmp.attribute("B", "255").toInt() );
        tooltip << QString("Chromakey \t(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    }

#ifdef GLM_FFGL
    QStringList plugins;
    tmp = xmlconfig.firstChildElement("FreeFramePlugin");
    while (!tmp.isNull()) {

        QDomElement p = tmp.firstChildElement("Filename");
        if (!p.isNull())
            plugins << QString(" - %1").arg( p.attribute("Basename") ); // freeframe
        else {
            p = tmp.firstChildElement("Name");
            if (!p.isNull())
                plugins << QString(" - %1 (ShaderToy)").arg( p.text() );  // shadertoy
        }

        // next plugin in configuration
        tmp = tmp.nextSiblingElement("FreeFramePlugin");
    }
    if (plugins.count())
        tooltip << "Plugins:" << plugins.join("\n");
#endif

    item->setToolTip(tooltip.join("\n"));
}



class CustomBlendingWidget : public QDialog {

public:

    QComboBox *functionBox;
    QComboBox *equationBox;

    CustomBlendingWidget(QWidget *parent, Source *s): QDialog(parent) {

        setObjectName(QString::fromUtf8("CustomBlendingWidget"));
        setWindowTitle(QObject::tr( "Custom blending"));

        QGridLayout *gridLayout = new QGridLayout(this);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QLabel *label = new QLabel(QObject::tr("On white"), this);
        gridLayout->addWidget(label, 0, 2, 1, 1);
        label = new QLabel(QObject::tr("Transparent"), this);
        gridLayout->addWidget(label, 0, 1, 1, 1);
        label = new QLabel(QObject::tr("On black"), this);
        gridLayout->addWidget(label, 0, 0, 1, 1);

        SourceDisplayWidget *previewWhitebg = new SourceDisplayWidget(this, SourceDisplayWidget::WHITE);
        previewWhitebg->setSource(s);
        previewWhitebg->setMinimumSize(QSize(150, 100));
        previewWhitebg->setSizePolicy(sizePolicy);
        gridLayout->addWidget(previewWhitebg, 1, 2, 1, 1);
        SourceDisplayWidget *previewTransparentbg = new SourceDisplayWidget(this, SourceDisplayWidget::GRID);
        previewTransparentbg->setSource(s);
        previewTransparentbg->setMinimumSize(QSize(150, 100));
        previewTransparentbg->setSizePolicy(sizePolicy);
        gridLayout->addWidget(previewTransparentbg, 1, 1, 1, 1);
        SourceDisplayWidget *previewBlackbg = new SourceDisplayWidget(this, SourceDisplayWidget::BLACK);
        previewBlackbg->setSource(s);
        previewBlackbg->setMinimumSize(QSize(150, 100));
        previewBlackbg->setSizePolicy(sizePolicy);
        gridLayout->addWidget(previewBlackbg, 1, 0, 1, 1);

        QLabel *labelEquation = new QLabel(QObject::tr("Equation :"), this);
        gridLayout->addWidget(labelEquation, 2, 0, 1, 1);
        equationBox = new QComboBox(this);
        equationBox->insertItems(0, QStringList()
                                 << QObject::tr("Add")
                                 << QObject::tr("Subtract")
                                 << QObject::tr("Reverse")
                                 << QObject::tr("Minimum")
                                 << QObject::tr("Maximum")
                                 );
        gridLayout->addWidget(equationBox, 2, 1, 1, 2);

        QLabel *labelFunction = new QLabel(QObject::tr("Destination :"), this);
        gridLayout->addWidget(labelFunction, 3, 0, 1, 1);
        functionBox = new QComboBox(this);
        functionBox->insertItems(0, QStringList()
                                 << QObject::tr("Zero")
                                 << QObject::tr("One")
                                 << QObject::tr("Source Color")
                                 << QObject::tr("Invert Source Color ")
                                 << QObject::tr("Background color")
                                 << QObject::tr("Invert Background Color")
                                 << QObject::tr("Source Alpha")
                                 << QObject::tr("Invert Source Alpha")
                                 << QObject::tr("Background Alpha")
                                 << QObject::tr("Invert Background Alpha")
                                 );
        gridLayout->addWidget(functionBox, 3, 1, 1, 2);

        QLabel *labelWarning = new QLabel(QObject::tr("Warning: some configurations do not allow to change\nthe transparency of the source anymore.\n"), this);
        gridLayout->addWidget(labelWarning, 4, 0, 1, 3);

        QDialogButtonBox *dialogBox = new QDialogButtonBox(this);
        dialogBox->setOrientation(Qt::Horizontal);
        dialogBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        gridLayout->addWidget(dialogBox, 5, 0, 1, 3);

        QObject::connect(functionBox, SIGNAL(currentIndexChanged(int)), previewWhitebg, SLOT(setBlendingFunction(int)));
        QObject::connect(equationBox, SIGNAL(currentIndexChanged(int)), previewWhitebg, SLOT(setBlendingEquation(int)));
        QObject::connect(functionBox, SIGNAL(currentIndexChanged(int)), previewTransparentbg, SLOT(setBlendingFunction(int)));
        QObject::connect(equationBox, SIGNAL(currentIndexChanged(int)), previewTransparentbg, SLOT(setBlendingEquation(int)));
        QObject::connect(functionBox, SIGNAL(currentIndexChanged(int)), previewBlackbg, SLOT(setBlendingFunction(int)));
        QObject::connect(equationBox, SIGNAL(currentIndexChanged(int)), previewBlackbg, SLOT(setBlendingEquation(int)));

        QObject::connect(dialogBox, SIGNAL(accepted()), this, SLOT(accept()));
        QObject::connect(dialogBox, SIGNAL(rejected()), this, SLOT(reject()));

        functionBox->setCurrentIndex( intFromBlendfunction(s->getBlendFuncDestination()) );
        equationBox->setCurrentIndex( intFromBlendequation(s->getBlendEquation()) );
    }
};

MixingToolboxWidget::MixingToolboxWidget(QWidget *parent, QSettings *settings) : QWidget(parent), source(0), appSettings(settings)
{
    setupUi(this);
    setEnabled(false);

    // Tooltip font
    presetsList->setStyleSheet(QString::fromUtf8("QToolTip{ font: 8pt \"%1\";}\n")
                               .arg(getMonospaceFont()));

    // fill the list of masks
    QMapIterator<int, QPair<QString, QString> > i(ViewRenderWidget::getMaskDecription());
    while (i.hasNext()) {
        i.next();

        QPixmap pix;
        QString mask_image = i.value().second;

        if (mask_image.isEmpty()) {
            pix = QPixmap(QSize(64,64));
            pix.fill(QColor(0,0,0,1));
        } else {
            // paint the texture on white background and with vertical flip
            pix = QPixmap(mask_image);
            pix.fill(QColor(0,0,0,0));
            QPainter p;
            p.begin(&pix);
            p.drawImage(0,0,QImage(mask_image).mirrored(0,1));
            p.end();
        }

        // set icon (also when selected to avoid automatic color overlay)
        QIcon icon;
        icon.addPixmap(pix, QIcon::Normal, QIcon::Off);
        icon.addPixmap(pix, QIcon::Selected, QIcon::Off);

        // add element into the list
        QListWidgetItem *item = new QListWidgetItem( icon, i.value().first);
        item->setToolTip(i.value().first);
        blendingMaskList->addItem( item );
    }
    blendingMaskList->setCurrentRow(0);

    // Setup the gamma levels toolbox
    gammaAdjust = new GammaLevelsWidget(this);
    Q_CHECK_PTR(gammaAdjust);
    gammaContentsLayout->addWidget(gammaAdjust);

    // hide custom blending button
    blendingCustomButton->setVisible(false);

    // make sure it is not sorting alphabetically the list
    presetsList->setSortingEnabled(false);

#ifdef GLM_FFGL
    // Setup the FFGL plugin property browser
    pluginBrowser = new FFGLPluginBrowser(Plugin);

    pluginBrowserLayout->insertWidget(2, pluginBrowser);
    QObject::connect(pluginBrowser, SIGNAL(currentItemChanged(bool)), removePlugin, SLOT(setEnabled(bool)) );
    QObject::connect(removePlugin, SIGNAL(clicked(bool)), pluginBrowser, SLOT(removePlugin()) );
    QObject::connect(pluginBrowser, SIGNAL(pluginChanged()), this, SLOT(changed()) );
    QObject::connect(pluginBrowser, SIGNAL(edit(FFGLPluginSource *)), parent, SLOT(editShaderToyPlugin(FFGLPluginSource *)) );
#else
    pluginButton->hide();
#endif

    // enable context menu
    presetsList->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Action to apply current preset
    applyAction = new QAction(QIcon(":/glmixer/icons/paint.png"), tr("Apply"), presetsList);
    applyAction->setToolTip(tr("Apply preset to the current source"));
    presetsList->insertAction(0, applyAction);
    QObject::connect(applyAction, SIGNAL(triggered()), this, SLOT(applyPreset()) );
    presetApply->setDefaultAction(applyAction);

    // Action to re-apply and update preset
    reapplyAction = new QAction(QIcon(":/glmixer/icons/re-apply.png"), tr("Update"), presetsList);
    reapplyAction->setToolTip(tr("Update preset with the current source"));
    presetsList->insertAction(0, reapplyAction);
    QObject::connect(reapplyAction, SIGNAL(triggered()), this, SLOT(reApplyPreset()) );
    presetReApply->setDefaultAction(reapplyAction);

    // Action to rename
    renameAction = new QAction(QIcon(":/glmixer/icons/rename.png"), tr("Rename"), presetsList);
    renameAction->setToolTip(tr("Rename the preset"));
    presetsList->insertAction(0, renameAction);
    QObject::connect(renameAction, SIGNAL(triggered()), this, SLOT(renamePreset()) );

    // Action to remove preset
    removeAction = new QAction(QIcon(":/glmixer/icons/fileclose.png"), tr("Delete"), presetsList);
    removeAction->setToolTip(tr("Delete the preset"));
    presetsList->insertAction(0, removeAction);
    QObject::connect(removeAction, SIGNAL(triggered()), this, SLOT(removePreset()) );
    presetRemove->setDefaultAction(removeAction);

    // Action to clear all user presets
    clearAction = new QAction(QIcon(":/glmixer/icons/clean.png"), tr("Delete all user presets"), presetsList);
    clearAction->setToolTip(tr("Delete all user presets"));
    presetsList->insertAction(0, clearAction);
    QObject::connect(clearAction, SIGNAL(triggered()), this, SLOT(removeAllPresets()) );
    presetClear->setDefaultAction(clearAction);

    // no action enabled at start
    applyAction->setEnabled(false);
    reapplyAction->setEnabled(false);
    removeAction->setEnabled(false);
    renameAction->setEnabled(false);

    // create default presets
    QDomDocument doc;
    doc.setContent(static_presets, false);
    QDomElement presetsconfig = doc.firstChildElement("PresetsList");
    if ( !presetsconfig.isNull() ) {

        // start loop of presets to create
        QDomElement child = presetsconfig.lastChildElement("Source");
        while (!child.isNull()) {

            // create item
            presetsList->insertItem(0, child.attribute("preset"));
            presetsList->item(0)->setFlags( Qt::ItemIsEnabled );

            // set string of config as user data
            QString xmlchild;
            QTextStream xmlstream(&xmlchild);
            child.save(xmlstream, 0);
            presetsList->item(0)->setData(Qt::UserRole, xmlchild);

            // set tooltip
            setItemTooltip(presetsList->item(0), child);

            // differenciate default presets with italic
            QFont f = presetsList->item(0)->font();
            f.setItalic(true);
            presetsList->item(0)->setFont( f );

            // loop
            child = child.previousSiblingElement("Source");
        }
    }

    // restore settings
    if (appSettings) {
#ifdef GLM_FFGL
        // colum width table plugins
        if (appSettings->contains("PluginPropertyBrowserState"))
            pluginBrowser->restoreState( appSettings->value("PluginPropertyBrowserState").toByteArray() );
#endif
    }
}


MixingToolboxWidget::~MixingToolboxWidget()
{
    // save settings
    if (appSettings) {
#ifdef GLM_FFGL
        // colum width table plugins
        appSettings->setValue("PluginPropertyBrowserState", pluginBrowser->saveState() );
#endif
    }

    blendingMaskList->clear();

    delete gammaAdjust;

#ifdef GLM_FFGL
    delete pluginBrowser;
#endif
}

void MixingToolboxWidget::connectSource(SourceSet::iterator csi)
{
    // show or hide Filter section
    filterButton->setVisible(ViewRenderWidget::filteringEnabled());
    if ( !ViewRenderWidget::filteringEnabled() && mixingToolBox->currentWidget() == Filter )
        mixingToolBox->setCurrentWidget(Blending);

    // connect gamma adjustment to the current source
    gammaAdjust->connectSource(csi);

    // enable / disable toolbox depending on availability of current source
    if (RenderingManager::getInstance()->isValid(csi)) {
        setEnabled(true);
        source = *csi;
        propertyChanged("Color", source->getColor());
        propertyChanged("ChromaKeyColor", source->getChromaKeyColor());

#ifdef GLM_FFGL
        pluginBrowser->showProperties( source->getFreeframeGLPluginStack() );
#endif
    }
    else {
        setEnabled(false);
        presetsList->setCurrentItem(0);
        source = 0;
        propertyChanged("Color", palette().color(QPalette::Window));
        propertyChanged("ChromaKeyColor",  palette().color(QPalette::Window));
#ifdef GLM_FFGL
        pluginBrowser->clear();
#endif
    }

}

void MixingToolboxWidget::setAntialiasing(bool antialiased)
{
    gammaAdjust->setAntialiasing(antialiased);
}


void MixingToolboxWidget::propertyChanged(QString propertyname)
{
    if (propertyname.contains("Gamma"))
        gammaAdjust->updateSource();
}

void MixingToolboxWidget::propertyChanged(QString propertyname, bool value)
{
    if (propertyname == "Pixelated")
        blendingPixelatedButton->setChecked(value);
    else if (propertyname == "ChromaKey")
        chromakeyEnable->setChecked(value);
}

void MixingToolboxWidget::propertyChanged(QString propertyname, int value)
{
    if (propertyname == "Mask")
    {
        blendingMaskList->setCurrentRow(value);
        blendingMaskList->scrollTo( blendingMaskList->currentIndex() );
    }
    else if (propertyname == "Blending")
        blendingBox->setCurrentIndex(value);
    else if (propertyname == "Invert")
        EffectsInvertBox->setCurrentIndex(value);
    else if (propertyname == "Saturation")
        saturationSlider->setValue(value);
    else if (propertyname == "Brightness")
        brightnessSlider->setValue(value);
    else if (propertyname == "Contrast")
        contrastSlider->setValue(value);
    else if (propertyname == "Threshold")
        thresholdSlider->setValue(value);
    else if (propertyname == "Lumakey")
        lumakeySlider->setValue(value);
    else if (propertyname == "Posterized")
        posterizeSlider->setValue(value < 1 ? 255 : value);
    else if (propertyname == "HueShift")
        hueSlider->setValue(value);
    else if (propertyname == "Filter")
    {
        filterList->setCurrentRow(value);
        filterList->scrollTo( filterList->currentIndex() );
    }
    else if (propertyname == "ChromaKeyTolerance")
        chromakeySlider->setValue(value);

}


void MixingToolboxWidget::propertyChanged(QString propertyname, const QColor &c)
{
    QPixmap p(32, 32);
    p.fill(c);

    if (propertyname == "Color")
        blendingColorButton->setIcon( QIcon(p) );
    else if (propertyname == "ChromaKeyColor")
        chromakeyColor->setIcon( QIcon(p) );
}

void MixingToolboxWidget::on_blendingMaskList_currentRowChanged(int value)
{
    emit(enumChanged("Mask", value));
}

void MixingToolboxWidget::on_blendingMaskList_itemDoubleClicked(QListWidgetItem * item)
{
    // special case of Custom Mask (no mask texture)
    if ( !ViewRenderWidget::getMaskTexture( blendingMaskList->currentRow() ) ) {
        QString filename = GLMixer::getInstance()->getMaskFileName( source->getCustomMaskTexture() );
        if (!filename.isEmpty())
            source->setCustomMaskTexture( filename );
    }
}

void MixingToolboxWidget::on_blendingBox_currentIndexChanged(int value)
{
    emit(enumChanged("Blending", value));
    blendingCustomButton->setVisible(value==0);
}

void MixingToolboxWidget::on_blendingCustomButton_pressed(){

    // new widget everytime, because its a new source each time
    CustomBlendingWidget cbw(this, source);

    // restore status
    if (appSettings && appSettings->contains("dialogBlendingCustomGeometry"))
        cbw.restoreGeometry(appSettings->value("dialogBlendingCustomGeometry").toByteArray());

    if (cbw.exec() == QDialog::Accepted) {
        emit(enumChanged("Equation", cbw.equationBox->currentIndex()));
        emit(enumChanged("Destination", cbw.functionBox->currentIndex()));
    }

    // remember status
    if (appSettings)
        appSettings->setValue("dialogBlendingCustomGeometry", cbw.saveGeometry());
}

void MixingToolboxWidget::on_blendingPixelatedButton_toggled(bool value){

    emit(valueChanged("Pixelated", value));
}

void MixingToolboxWidget::on_blendingColorButton_pressed() {

    QColor color(Qt::white);
    if (source)
        color = source->getColor();

    color = QColorDialog::getColor(color, this);
    if (color.isValid())
        emit( valueChanged("Color", color));
}

void MixingToolboxWidget::on_EffectsInvertBox_currentIndexChanged(int value)
{
    emit(enumChanged("Invert", value));
}

void MixingToolboxWidget::on_invertReset_pressed()
{
    EffectsInvertBox->setCurrentIndex(0);
}

void MixingToolboxWidget::on_chromakeyEnable_toggled(bool value)
{
    emit(valueChanged("ChromaKey", value));
}

void MixingToolboxWidget::on_chromakeyColor_pressed()
{
    if (source) {
        QColor color = QColorDialog::getColor(source->getChromaKeyColor(), this);

        if (color.isValid())
            emit( valueChanged("ChromaKeyColor", color));
    }
}

void MixingToolboxWidget::on_chromakeySlider_valueChanged(int value)
{
    emit(valueChanged("ChromaKeyTolerance", value));
}

void MixingToolboxWidget::on_saturationSlider_valueChanged(int value)
{
    emit(valueChanged("Saturation", value));
}

void MixingToolboxWidget::on_brightnessSlider_valueChanged(int value)
{
    emit(valueChanged("Brightness", value));
}

void MixingToolboxWidget::on_contrastSlider_valueChanged(int value)
{
    emit(valueChanged("Contrast", value));
}

void MixingToolboxWidget::on_hueSlider_valueChanged(int value)
{
    emit(valueChanged("HueShift", value));
}

void MixingToolboxWidget::on_thresholdSlider_valueChanged(int value)
{
    emit(valueChanged("Threshold", value));
}

void MixingToolboxWidget::on_lumakeySlider_valueChanged(int value)
{
    emit(valueChanged("Lumakey", value));
}

void MixingToolboxWidget::on_posterizeSlider_valueChanged(int value)
{
    emit(valueChanged("Posterized", value > 254 ? 0 : value));
}

void MixingToolboxWidget::on_saturationReset_pressed()
{
    saturationSlider->setValue(0);
}

void MixingToolboxWidget::on_brightnessReset_pressed()
{
    brightnessSlider->setValue(0);
}

void MixingToolboxWidget::on_contrastReset_pressed()
{
    contrastSlider->setValue(0);
}

void MixingToolboxWidget::on_hueReset_pressed()
{
    hueSlider->setValue(0);
}

void MixingToolboxWidget::on_thresholdReset_pressed()
{
    thresholdSlider->setValue(0);
}

void MixingToolboxWidget::on_lumakeyReset_pressed()
{
    lumakeySlider->setValue(0);
}

void MixingToolboxWidget::on_posterizeReset_pressed()
{
    posterizeSlider->setValue(255);
}

void MixingToolboxWidget::on_colorPreview_pressed()
{
    if (source) {
        switch (colorToolBox->currentIndex()) {
        case 0: // saturation
            source->_setSaturation();
            break;
        case 1: // brightness
            source->_setBrightness();
            break;
        case 2: // contrast
            source->_setContrast();
            break;
        case 3: // hue
            source->_setHueShift();
            break;
        case 4: // Threshold
            source->_setThreshold();
            break;
        case 5: // posterize
            source->_setPosterized();
            break;
        case 6: // luma key
            source->_setLumakey();
            break;
        case 7: // chroma key
            source->_setChromaKey();
            break;
        case 8: // Invert
            source->_setInvertMode();
            break;
        }
    }
}
void MixingToolboxWidget::on_colorPreview_released()
{
    if (source) {

        switch (colorToolBox->currentIndex()) {
        case 0: // saturation
            source->_setSaturation(saturationSlider->value());
            break;
        case 1: // brightness
            source->_setBrightness(brightnessSlider->value());
            break;
        case 2: // contrast
            source->_setContrast(contrastSlider->value());
            break;
        case 3: // hue
            source->_setHueShift(hueSlider->value());
            break;
        case 4: // Threshold
            source->_setThreshold(thresholdSlider->value());
            break;
        case 5: // posterize
            source->_setPosterized(posterizeSlider->value());
            break;
        case 6: // luma key
            source->_setLumakey(lumakeySlider->value());
            break;
        case 7: // chroma key
            source->_setChromaKey(chromakeyEnable->isEnabled());
            break;
        case 8: // Invert
            source->_setInvertMode(EffectsInvertBox->currentIndex());
            break;
        }
    }
}

void MixingToolboxWidget::on_filterList_currentRowChanged(int value)
{
    emit(enumChanged("Filter", value));
}


void MixingToolboxWidget::applyPreset(Source *s, QListWidgetItem *itemListPreset)
{
    if (s) {
        QDomDocument doc;
        if (doc.setContent( itemListPreset->data(Qt::UserRole).toString() )) {
            QDomElement child = doc.firstChildElement("Source");
            if (!child.isNull()) {
#ifdef GLM_UNDO
                // inform undo manager
                UndoManager::getInstance()->store();
#endif
                // apply configuration
                s->Source::setConfiguration(child, QDir());
            }
        }
        changed();
    }
}

void MixingToolboxWidget::applyPreset(QString sourceName, QString nameItemListPreset)
{
    SourceSet::const_iterator sit = RenderingManager::getInstance()->getByName(sourceName);

    if ( RenderingManager::getInstance()->notAtEnd(sit)) {

        QList<QListWidgetItem *> items = presetsList->findItems( nameItemListPreset, Qt::MatchExactly);

        if (items.size() > 0 )
            applyPreset( *sit, items.first() );
    }
}

void MixingToolboxWidget::applyPreset(QString sourceName, int idItemListPreset)
{
    SourceSet::const_iterator sit = RenderingManager::getInstance()->getByName(sourceName);

    if ( RenderingManager::getInstance()->notAtEnd(sit)) {

        if ( idItemListPreset > 0 && idItemListPreset <= presetsList->count() )
            applyPreset( *sit, presetsList->item(idItemListPreset -1) );
    }
}

void MixingToolboxWidget::on_presetsList_itemDoubleClicked(QListWidgetItem *item)
{
    applyPreset( source, item );
}

void MixingToolboxWidget::applyPreset()
{
    applyPreset( source, presetsList->currentItem() );
}

void MixingToolboxWidget::reApplyPreset()
{
    if (source && presetsList->currentItem()->flags() & Qt::ItemIsEditable) {

        QDomDocument doc;
        QDomElement xmlconfig = source->getConfiguration(doc, QDir());

        // rename
        xmlconfig.setAttribute("preset", presetsList->currentItem()->text());

        // remove attributes that we do not want to keep in presets
        xmlconfig.removeAttribute("name");
        xmlconfig.removeAttribute("fixedAR");
        xmlconfig.removeAttribute("workspace");
        xmlconfig.removeAttribute("stanbyMode");
        xmlconfig.removeChild(xmlconfig.firstChildElement("Position"));
        xmlconfig.removeChild(xmlconfig.firstChildElement("Scale"));
        xmlconfig.removeChild(xmlconfig.firstChildElement("Center"));
        xmlconfig.removeChild(xmlconfig.firstChildElement("Angle"));
        xmlconfig.removeChild(xmlconfig.firstChildElement("Alpha"));
        xmlconfig.removeChild(xmlconfig.firstChildElement("Crop"));

        // set the XML config to the document
        doc.appendChild(xmlconfig);
        presetsList->currentItem()->setData(Qt::UserRole, doc.toString());

    }
}

void MixingToolboxWidget::on_presetAdd_pressed()
{
    if (source) {
        // create list item with default name
        presetsList->insertItem(0, source->getName());
        presetsList->item(0)->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

        // Set as current and offer user to edit the name
        presetsList->setCurrentItem(presetsList->item(0));
        presetsList->editItem(presetsList->currentItem());

        // associate the properties of a source imported from the current source
        reApplyPreset();

        // ready GUI
        removeAction->setEnabled(true);
        reapplyAction->setEnabled(true);
    }
}

void MixingToolboxWidget::renamePreset()
{
    if (source) {

        presetsList->editItem(presetsList->currentItem());
    }
}

void MixingToolboxWidget::removePreset()
{
    // take the element out of the list
    if ( presetsList->item( presetsList->currentRow() )->flags() & Qt::ItemIsEditable ) {
        QListWidgetItem *it = presetsList->takeItem( presetsList->currentRow() );
        delete it;
    }
}

void MixingToolboxWidget::on_presetsList_itemChanged(QListWidgetItem *item){

    if ( presetsList->findItems( item->text(), Qt::MatchFixedString ).length() > 1 )
        item->setText( item->text() + "_bis" );

    // Rename in xml
    QDomDocument doc;
    if (doc.setContent( item->data(Qt::UserRole).toString() )) {
        QDomElement child = doc.firstChildElement("Source");
        if (!child.isNull()) {
            child.setAttribute("preset", item->text());
            item->setData(Qt::UserRole, doc.toString());

            // validate item and set its tooltip
            setItemTooltip(item, child);
        }
    }
}


void MixingToolboxWidget::on_presetsList_currentItemChanged(QListWidgetItem *item){

    if (item) {
        applyAction->setEnabled(true);

        if ( item->flags() & Qt::ItemIsEditable  ) {
            reapplyAction->setEnabled(true);
            removeAction->setEnabled(true);
            renameAction->setEnabled(true);
        }
        else {
            reapplyAction->setEnabled(false);
            removeAction->setEnabled(false);
            renameAction->setEnabled(false);
        }
    }
    else {
        applyAction->setEnabled(false);
        reapplyAction->setEnabled(false);
        removeAction->setEnabled(false);
        renameAction->setEnabled(false);
    }
}

QByteArray MixingToolboxWidget::getPresets() const {

    int count = 0;
    QDomDocument config;
    QDomElement xmllist = config.createElement("PresetsList");

    for (int i = 0; i < presetsList->count(); ++i) {
        // do not save items from preset
        if ( presetsList->item(i)->flags() & Qt::ItemIsEditable ) {
            // get user data and append an xml element
            QDomDocument doc;
            if (doc.setContent( presetsList->item(i)->data(Qt::UserRole).toString() )) {
                QDomElement child = doc.firstChildElement("Source");
                if (!child.isNull()) {
                    ++count;
                    xmllist.appendChild( child.cloneNode() );
                }
            }
        }
    }
    config.appendChild(xmllist);

    return config.toByteArray();
}


bool MixingToolboxWidget::restorePresets(const QByteArray &state) {

    int count = 0;
    QDomDocument config;
    config.setContent(state, false);

//    qDebug() << "LOAD User presets" << config.toString();

    QDomElement presetsconfig = config.firstChildElement("PresetsList");
    if ( !presetsconfig.isNull() ) {

        // start loop of presets to create
        QDomElement child = presetsconfig.lastChildElement("Source");
        while (!child.isNull()) {

            // create item
            presetsList->insertItem(0, child.attribute("preset"));
            presetsList->item(0)->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

            // set string of config as user data
            QString xmlchild;
            QTextStream xmlstream(&xmlchild);
            child.save(xmlstream, 0);
            presetsList->item(0)->setData(Qt::UserRole, xmlchild);

            // set tooltip
            setItemTooltip(presetsList->item(0), child);

            ++count;
            child = child.previousSiblingElement("Source");
        }
    }

    qDebug() << count << QObject::tr("Mixing presets restored");
    return true;
}


void MixingToolboxWidget::on_resetBlending_pressed()
{
    blendingBox->setCurrentIndex(5);
    blendingMaskList->setCurrentRow(0);;
    blendingPixelatedButton->setChecked(false);
    emit( valueChanged("Color", QColor(Qt::white)) );
}

void MixingToolboxWidget::on_resetGamma_pressed()
{
    gammaAdjust->resetAll();
}

void MixingToolboxWidget::on_resetColor_pressed()
{
    saturationReset->click();
    brightnessReset->click();
    contrastReset->click();
    hueReset->click();
    thresholdReset->click();
    lumakeyReset->click();
    posterizeReset->click();
    invertReset->click();

    // reset chromakey
    emit( valueChanged("ChromaKey", false));
    emit( valueChanged("ChromaKeyTolerance", 10));
    emit( valueChanged("ChromaKeyColor", QColor(Qt::green)));
}

void MixingToolboxWidget::on_resetFilter_pressed()
{
    filterList->setCurrentRow(0);
}

void MixingToolboxWidget::removeAllPresets()
{
    while (presetsList->item(0)->flags() & Qt::ItemIsEditable ) {
        QListWidgetItem *it = presetsList->takeItem( 0 );
        delete it;
    }

}

void MixingToolboxWidget::on_resetPlugins_pressed()
{
#ifdef GLM_FFGL
    pluginBrowser->resetAll();
#endif
}

void MixingToolboxWidget::changed()
{
    if (source) {
        // inform undo manager
        UndoManager::getInstance()->store();
        // inform all listeners
        emit sourceChanged( RenderingManager::getInstance()->getById( source->getId()) );
    }
}


void MixingToolboxWidget::on_blendingButton_clicked(bool on)
{
    if (on)
        mixingToolBox->setCurrentWidget(Blending);
}
void MixingToolboxWidget::on_gammaButton_clicked(bool on)
{
    if (on)
        mixingToolBox->setCurrentWidget(Gamma);
}
void MixingToolboxWidget::on_colorButton_clicked(bool on)
{
    if (on)
        mixingToolBox->setCurrentWidget(Color);
}
void MixingToolboxWidget::on_filterButton_clicked(bool on)
{
    if (on)
        mixingToolBox->setCurrentWidget(Filter);
}
void MixingToolboxWidget::on_pluginButton_clicked(bool on)
{
    if (on)
        mixingToolBox->setCurrentWidget(Plugin);
}
void MixingToolboxWidget::on_presetButton_clicked(bool on)
{
    if (on)
        mixingToolBox->setCurrentWidget(Preset);
}

#ifdef GLM_FFGL

void MixingToolboxWidget::on_addPlugin_pressed()
{

    static FFGLEffectSelectionDialog *effectDialog = new FFGLEffectSelectionDialog(this, appSettings);

    effectDialog->exec();

    QString fileName = effectDialog->getSelectedFreeframePlugin();

    if (!fileName.isNull())
    {

        QFileInfo pluginfile(fileName);
#ifdef Q_OS_MAC
        if (pluginfile.isBundle())
            pluginfile.setFile( pluginfile.absoluteFilePath() + "/Contents/MacOS/" + pluginfile.baseName() );
#endif
        if (source && pluginfile.isFile()) {

            // inform undo manager
            UndoManager::getInstance()->store();

            // add a the given freeframe plugin
            FFGLPluginSource *plugin = source->addFreeframeGLPlugin( pluginfile.absoluteFilePath() );
            plugin->setParent( source );

            // test if plugin was added
            if ( plugin ) {
                pluginBrowser->showProperties( source->getFreeframeGLPluginStack() );
                changed();
            }
        }
    }
}

void MixingToolboxWidget::on_addShadertoyPlugin_pressed()
{
    if (source) {

        // inform undo manager
        UndoManager::getInstance()->store();

        // add a generic freeframe plugin
        FFGLPluginSource *plugin = source->addFreeframeGLPlugin();
        plugin->setParent( source );

        // test if plugin was added
        if ( plugin ) {
            // show the updated list of plugins in the browser
            pluginBrowser->showProperties( source->getFreeframeGLPluginStack() );

            // open editor when plugin will be initialized
            connect(plugin, SIGNAL(initialized(FFGLPluginSource *)), GLMixer::getInstance(), SLOT(editShaderToyPlugin(FFGLPluginSource *)));
//            connect(plugin, SIGNAL(initialized(FFGLPluginSource *)), pluginBrowser, SIGNAL(edit(FFGLPluginSource *)));

            // update view
            changed();
        }
    }
}


#endif
