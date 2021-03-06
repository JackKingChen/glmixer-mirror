#include "PropertyBrowser.moc"


#include <QtGui>

#include <QtTreePropertyBrowser>
#include <QtButtonPropertyBrowser>
#include <QtGroupBoxPropertyBrowser>
#include <QtDoublePropertyManager>
#include <QtIntPropertyManager>
#include <QtStringPropertyManager>
#include <QtColorPropertyManager>
#include <QtRectFPropertyManager>
#include <QtPointFPropertyManager>
#include <QtSizePropertyManager>
#include <QtEnumPropertyManager>
#include <QtBoolPropertyManager>
#include <QtTimePropertyManager>
#include <QtDoubleSpinBoxFactory>
#include <QtCheckBoxFactory>
#include <QtSpinBoxFactory>
#include <QtSliderFactory>
#include <QtLineEditFactory>
#include <QtEnumEditorFactory>
#include <QtCheckBoxFactory>
#include <QtTimeEditFactory>
#include <QtColorEditorFactory>
#include <ButtonEditorFactory.h>


PropertyBrowser::PropertyBrowser(QWidget *parent) :
    QWidget(parent), displayPropertyTree(true)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setObjectName(QString::fromUtf8("verticalLayout"));

    // property Group Box
    propertyGroupEditor = new QtGroupBoxPropertyBrowser(this);
    propertyGroupEditor->setObjectName(QString::fromUtf8("Property Groups"));
    propertyGroupEditor->setContextMenuPolicy(Qt::NoContextMenu);

    propertyGroupArea = new QScrollArea(this);
    propertyGroupArea->setFrameShape(QFrame::NoFrame);
    propertyGroupArea->setWidgetResizable(true);
    propertyGroupArea->setWidget(propertyGroupEditor);
    propertyGroupArea->setVisible(false);

    // property TREE
    propertyTreeEditor = new QtTreePropertyBrowser(this);
    propertyTreeEditor->setObjectName(QString::fromUtf8("Property Tree"));
    propertyTreeEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    propertyTreeEditor->setResizeMode(QtTreePropertyBrowser::Interactive);
    connect(propertyTreeEditor, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenuTree(const QPoint &)));
    connect(propertyTreeEditor, SIGNAL(currentItemChanged(QtBrowserItem *)), this, SLOT(onCurrentItemChanged(QtBrowserItem *)) );

    // TODO ; read default from application config
    propertyTreeEditor->setVisible(true);
    layout->addWidget(propertyTreeEditor);

    // create the property managers for every possible types
    groupManager = new QtGroupPropertyManager(this);
    doubleManager = new QtDoublePropertyManager(this);
    intManager = new QtIntPropertyManager(this);
    stringManager = new QtStringPropertyManager(this);
    colorManager = new QtColorPropertyManager(this);
    pointManager = new QtPointFPropertyManager(this);
    enumManager = new QtEnumPropertyManager(this);
    boolManager = new QtBoolPropertyManager(this);
    rectManager = new QtRectFPropertyManager(this);
    buttonManager = new ButtonPropertyManager(this);

    connect(colorManager, SIGNAL(valueChanged(QtProperty *, const QColor &)),
                this, SLOT(propertyValueChanged(QtProperty *, const QColor &)));
    connect(enumManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(propertyValueChanged(QtProperty *, int)));
    connect(intManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(propertyValueChanged(QtProperty *, int)));
    connect(boolManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(propertyValueChanged(QtProperty *, bool)));
    connect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(propertyValueChanged(QtProperty *, double)));

    // special managers which are not associated with a factory (i.e non editable)
    infoManager = new QtStringPropertyManager(this);
    sizeManager = new QtSizePropertyManager(this);

    // specify the factory for each of the property managers
    QtDoubleSpinBoxFactory *doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);    // for double
    QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);					// for bool
    QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(this);                      // for int
    QtSliderFactory *sliderFactory = new QtSliderFactory(this);							// for int
    QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);					// for text
    QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(this);				// for enum
    QtColorEditorFactory *colorFactory = new QtColorEditorFactory(this);				// for color
    ButtonEditorFactory *buttonFactory = new ButtonEditorFactory(this);

    propertyTreeEditor->setFactoryForManager(doubleManager, doubleSpinBoxFactory);
    propertyTreeEditor->setFactoryForManager(intManager, spinBoxFactory);
    propertyTreeEditor->setFactoryForManager(stringManager, lineEditFactory);
    propertyTreeEditor->setFactoryForManager(colorManager, colorFactory);
    propertyTreeEditor->setFactoryForManager(pointManager->subDoublePropertyManager(), doubleSpinBoxFactory);
    propertyTreeEditor->setFactoryForManager(enumManager, comboBoxFactory);
    propertyTreeEditor->setFactoryForManager(boolManager, checkBoxFactory);
    propertyTreeEditor->setFactoryForManager(rectManager->subDoublePropertyManager(), doubleSpinBoxFactory);
    propertyTreeEditor->setFactoryForManager(buttonManager, buttonFactory);

    propertyGroupEditor->setFactoryForManager(doubleManager, doubleSpinBoxFactory);
    propertyGroupEditor->setFactoryForManager(intManager, sliderFactory);
    propertyGroupEditor->setFactoryForManager(stringManager, lineEditFactory);
    propertyGroupEditor->setFactoryForManager(colorManager, colorFactory);
    propertyGroupEditor->setFactoryForManager(pointManager->subDoublePropertyManager(), doubleSpinBoxFactory);
    propertyGroupEditor->setFactoryForManager(enumManager, comboBoxFactory);
    propertyGroupEditor->setFactoryForManager(boolManager, checkBoxFactory);
    propertyGroupEditor->setFactoryForManager(rectManager->subDoublePropertyManager(), doubleSpinBoxFactory);
    propertyGroupEditor->setFactoryForManager(buttonManager, buttonFactory);

    // actions of context menus
    defaultValueAction = new QAction( QIcon(":/glmixer/icons/clean.png"), QObject::tr("Default value"), this);
    QObject::connect(defaultValueAction, SIGNAL(triggered()), this, SLOT(defaultValue() ) );

    resetAction = new QAction( QIcon(":/glmixer/icons/view-refresh.png"), QObject::tr("Reset"), this);
    QObject::connect(resetAction, SIGNAL(triggered()), this, SLOT(resetAll() ) );

    openUrlAction = new QAction( QIcon(":/glmixer/icons/folderopen.png"), QObject::tr("Show file in browser"), this);
    QObject::connect(openUrlAction, SIGNAL(triggered()), this, SLOT(showReferenceURL() ) );

    copyClipboardAction = new QAction( QIcon(":/glmixer/icons/textcopy.png"), QObject::tr("Copy text"), this);
    QObject::connect(copyClipboardAction, SIGNAL(triggered()), this, SLOT(copyPropertyText() ) );

    // Actions of the Tree Context Menu
    menuTree.addAction(defaultValueAction);
    menuTree.addAction(resetAction);
    menuTree.addAction(copyClipboardAction);
    menuTree.addAction(openUrlAction);

}


PropertyBrowser::~PropertyBrowser()
{
    disconnectManagers();
    // clear the GUI
    propertyTreeEditor->clear();
    propertyGroupEditor->clear();
}


void PropertyBrowser::setHeaderVisible(bool visible){

    propertyTreeEditor->setHeaderVisible(visible);

}

void PropertyBrowser::setReferenceURL(QUrl u){

    referenceURL = u;

}

void PropertyBrowser::showReferenceURL(){

    if (referenceURL.isValid())
        QDesktopServices::openUrl(referenceURL);

}

void PropertyBrowser::copyPropertyText(){

    if ( propertyTreeEditor->currentItem() ) {
        QtProperty *property = propertyTreeEditor->currentItem()->property();
        if ( property && property->hasValue() )
            QApplication::clipboard()->setText( property->valueText() );
    }
}


void PropertyBrowser::setPropertyEnabled(QString propertyName, bool enabled){

    if (idToProperty.contains(propertyName.toLatin1()))
        idToProperty[propertyName.toLatin1()]->setEnabled(enabled);

}


void PropertyBrowser::connectManagers()
{
    connect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(valueChanged(QtProperty *, double)));
    connect(stringManager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(valueChanged(QtProperty *, const QString &)));
    connect(colorManager, SIGNAL(valueChanged(QtProperty *, const QColor &)),
                this, SLOT(valueChanged(QtProperty *, const QColor &)));
    connect(pointManager, SIGNAL(valueChanged(QtProperty *, const QPointF &)),
                this, SLOT(valueChanged(QtProperty *, const QPointF &)));
    connect(enumManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(enumChanged(QtProperty *, int)));
    connect(intManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(valueChanged(QtProperty *, int)));
    connect(boolManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(valueChanged(QtProperty *, bool)));
    connect(rectManager, SIGNAL(valueChanged(QtProperty *, const QRectF &)),
                this, SLOT(valueChanged(QtProperty *, const QRectF &)));
    connect(buttonManager, SIGNAL(valueChanged(QtProperty *, const QString&)),
                this, SLOT(valueChanged(QtProperty *, const QString&)));
}

void PropertyBrowser::disconnectManagers()
{
    disconnect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(valueChanged(QtProperty *, double)));
    disconnect(stringManager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(valueChanged(QtProperty *, const QString &)));
    disconnect(colorManager, SIGNAL(valueChanged(QtProperty *, const QColor &)),
                this, SLOT(valueChanged(QtProperty *, const QColor &)));
    disconnect(pointManager, SIGNAL(valueChanged(QtProperty *, const QPointF &)),
                this, SLOT(valueChanged(QtProperty *, const QPointF &)));
    disconnect(enumManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(enumChanged(QtProperty *, int)));
    disconnect(intManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(valueChanged(QtProperty *, int)));
    disconnect(boolManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(valueChanged(QtProperty *, bool)));
    disconnect(rectManager, SIGNAL(valueChanged(QtProperty *, const QRectF &)),
                this, SLOT(valueChanged(QtProperty *, const QRectF &)));
    disconnect(buttonManager, SIGNAL(valueChanged(QtProperty *, const QString&)),
                this, SLOT(valueChanged(QtProperty *, const QString&)));
}

void PropertyBrowser::onCurrentItemChanged(QtBrowserItem *item)
{
    emit currentItemChanged( item != 0 );
}

void PropertyBrowser::addProperty(QtProperty *property)
{
    // add to the group view
    propertyGroupEditor->addProperty(property);

    // add to the tree view
    QtBrowserItem *item = propertyTreeEditor->addProperty(property);
    if (idToExpanded.contains(property->propertyName()))
        propertyTreeEditor->setExpanded(item, idToExpanded[property->propertyName()]);
    else
        propertyTreeEditor->setExpanded(item, false);
}


void PropertyBrowser::updateExpandState(QList<QtBrowserItem *> list)
{
    QListIterator<QtBrowserItem *> it(list);
    while (it.hasNext()) {
        QtBrowserItem *item = it.next();
        idToExpanded[item->property()->propertyName()] = propertyTreeEditor->isExpanded(item);

        updateExpandState(item->children());
    }
}

void PropertyBrowser::restoreExpandState(QList<QtBrowserItem *> list)
{
    QListIterator<QtBrowserItem *> it(list);
    while (it.hasNext()) {
        QtBrowserItem *item = it.next();
        if (idToExpanded.contains(item->property()->propertyName()))
            propertyTreeEditor->setExpanded(item, idToExpanded[item->property()->propertyName()]);
        // default to expanded state
        else
            propertyTreeEditor->setExpanded(item, true);

        restoreExpandState(item->children());
    }
}

void PropertyBrowser::setGlobalExpandState(QList<QtBrowserItem *> list, bool expanded)
{
    QListIterator<QtBrowserItem *> it(list);
    while (it.hasNext()) {
        QtBrowserItem *item = it.next();
        idToExpanded[item->property()->propertyName()] = expanded;
        propertyTreeEditor->setExpanded(item, expanded);

        setGlobalExpandState(item->children(), expanded);
    }
}

void PropertyBrowser::expandAll()
{
    setGlobalExpandState(propertyTreeEditor->topLevelItems(), true);
}

void PropertyBrowser::collapseAll()
{
    setGlobalExpandState(propertyTreeEditor->topLevelItems(), false);
}


void PropertyBrowser::enumChanged(QString propertyname, int value)
{
    enumManager->setValue(idToProperty[propertyname], value);
}

void PropertyBrowser::valueChanged(QString propertyname, const QColor &value)
{
    colorManager->setValue(idToProperty[propertyname], value);
}

void PropertyBrowser::valueChanged(QString propertyname, const QString &value)
{
    stringManager->setValue(idToProperty[propertyname], value);
}

void PropertyBrowser::valueChanged(QString propertyname, bool value)
{
    boolManager->setValue(idToProperty[propertyname], value);
}

void PropertyBrowser::valueChanged(QString propertyname, int value)
{
    intManager->setValue(idToProperty[propertyname], value);
}

void PropertyBrowser::valueChanged(QString propertyname, double value)
{
    doubleManager->setValue(idToProperty[propertyname], value);
}

void PropertyBrowser::propertyValueChanged(QtProperty *property, bool value)
{
    emit( propertyChanged( property->propertyName(), value) );
}

void PropertyBrowser::propertyValueChanged(QtProperty *property, int value)
{
    emit( propertyChanged( property->propertyName(), value) );
}

void PropertyBrowser::propertyValueChanged(QtProperty *property, const QColor &value)
{
    emit( propertyChanged( property->propertyName(), value) );
}

void PropertyBrowser::propertyValueChanged(QtProperty *property, const QString &value)
{
    emit( propertyChanged( property->propertyName(), value) );
}

void PropertyBrowser::propertyValueChanged(QtProperty *property, double value)
{
    emit( propertyChanged( property->propertyName(), value) );
}


void PropertyBrowser::ctxMenuTree(const QPoint &pos)
{
    if (contextMenuPolicy() == Qt::NoContextMenu)
        return;

    openUrlAction->setVisible( referenceURL.isValid() );

    defaultValueAction->setVisible(false);
    defaultValueAction->setEnabled(true);
    resetAction->setVisible(true);
    copyClipboardAction->setVisible(false);

    if (propertyTreeEditor->currentItem()) {
        QtProperty *property = propertyTreeEditor->currentItem()->property();
        if ( property->hasValue() ) {
            defaultValueAction->setVisible(true);
            copyClipboardAction->setVisible(true);
            resetAction->setVisible(false);

            if ( !property->isEnabled() || property->isItalics()) {
                defaultValueAction->setEnabled(false);
            }
        }
    }

    menuTree.exec( propertyTreeEditor->mapToGlobal(pos) );
}


void PropertyBrowser::setDisplayPropertyTree(bool on)
{
    displayPropertyTree = on;
    if (displayPropertyTree) {
        propertyGroupArea->setVisible(false);
        layout->removeWidget(propertyGroupArea);

        layout->addWidget(propertyTreeEditor);
        propertyTreeEditor->setVisible(true);
    }
    else {
        propertyTreeEditor->setVisible(false);
        layout->removeWidget(propertyTreeEditor);

        layout->addWidget(propertyGroupArea);
        propertyGroupArea->setVisible(true);
    }
}

void PropertyBrowser::connectToPropertyTree(PropertyBrowser *master)
{
    // set identical display property tree mode
    setDisplayPropertyTree(master->displayPropertyTree);

    // connect splitters
    propertyTreeEditor->setSplitterPosition(master->propertyTreeEditor->splitterPosition());
    connect(master->propertyTreeEditor, SIGNAL(splitterPositionChanged(int)), propertyTreeEditor, SLOT(setSplitterPosition(int)));

}

QByteArray PropertyBrowser::saveState()  {

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // splitter pos
    stream << propertyTreeEditor->splitterPosition();

    return data;
}

bool PropertyBrowser::restoreState(const QByteArray &state) {

    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);

    // splitter pos
    int pos = 100;
    stream >> pos;
    propertyTreeEditor->setSplitterPosition(pos);

    return true;
}
