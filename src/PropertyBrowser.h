#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include <QWidget>
#include <QMenu>
#include <QtCore/QMap>
#include <QtCore/QUrl>


class QtProperty;
class QtBrowserItem;


class PropertyBrowser : public QWidget
{
    Q_OBJECT

public:
    PropertyBrowser(QWidget *parent = 0);
    ~PropertyBrowser();

public slots:
    // Update the object when an action is performed on a property in the browser
    // This concerns every properties editable in the browser
    virtual void valueChanged(QtProperty *property, const QColor &value) {}
    virtual void valueChanged(QtProperty *property, const QPointF &value){}
    virtual void valueChanged(QtProperty *property, bool value){}
    virtual void valueChanged(QtProperty *property, int value){}
    virtual void enumChanged(QtProperty *property, int value){}
    virtual void valueChanged(QtProperty *property, double value){}
    virtual void valueChanged(QtProperty *property, const QString &value){}
    virtual void valueChanged(QtProperty *property, const QRectF &value){}
    virtual void valueChanged(QtProperty *property){}

    // access by property name
    void valueChanged(QString propertyName, bool value);
    void valueChanged(QString propertyName, int value);
    void valueChanged(QString propertyName, double value);
    void valueChanged(QString propertyName, const QColor &value);
    void valueChanged(QString propertyName, const QString &value);
    void enumChanged(QString propertyName, int value);

    // simple slots to always forward the property changed signals
    void propertyValueChanged(QtProperty *, bool);
    void propertyValueChanged(QtProperty *, int);
    void propertyValueChanged(QtProperty *, double);
    void propertyValueChanged(QtProperty *, const QColor &);
    void propertyValueChanged(QtProperty *, const QString &);

    // utility slot for expanding / collapsing all tree items
    void expandAll();
    void collapseAll();
    void setHeaderVisible(bool);
    void setReferenceURL(QUrl u = QUrl());
    void showReferenceURL();
    void onCurrentItemChanged(QtBrowserItem *item);

    // Context menu actions
    virtual void ctxMenuTree(const QPoint &);
    virtual void resetAll() {}
    virtual void defaultValue() {}
    virtual void copyPropertyText();

    // appearance
    void setDisplayPropertyTree(bool on);
    void connectToPropertyTree(PropertyBrowser *master);
    QByteArray saveState();
    bool restoreState(const QByteArray &state);

    // enable / disable a property
    void setPropertyEnabled(QString propertyName, bool enabled);

signals:
    void propertyChanged(QString);
    void propertyChanged(QString, bool);
    void propertyChanged(QString, int);
    void propertyChanged(QString, double);
    void propertyChanged(QString, const QColor &);
    void propertyChanged(QString, const QString &);
    void currentItemChanged(bool);

protected:

    void addProperty(QtProperty *property);
    void connectManagers();
    void disconnectManagers();

    void updateExpandState(QList<QtBrowserItem *> list);
    void restoreExpandState(QList<QtBrowserItem *> list);
    void setGlobalExpandState(QList<QtBrowserItem *> list, bool expanded);

    virtual bool canChange() {return false;}

    // utility lists of properties
    QMap<QString, QtProperty *> idToProperty;
    QMap<QString, bool> idToExpanded;

    // the property browsers
    class QVBoxLayout *layout;
    class QScrollArea *propertyGroupArea;
    class QtTreePropertyBrowser *propertyTreeEditor;
    class QtGroupBoxPropertyBrowser *propertyGroupEditor;

    // managers for different data types
    class QtGroupPropertyManager *groupManager;
    class QtDoublePropertyManager *doubleManager;
    class QtIntPropertyManager *intManager;
    class QtStringPropertyManager *stringManager, *infoManager;
    class QtColorPropertyManager *colorManager;
    class QtPointFPropertyManager *pointManager;
    class QtSizePropertyManager *sizeManager;
    class QtEnumPropertyManager *enumManager;
    class QtBoolPropertyManager *boolManager;
    class QtRectFPropertyManager *rectManager;
    class ButtonPropertyManager *buttonManager;

    // context menus
    QUrl referenceURL;
    QMenu menuTree;
    QAction *resetAction, *defaultValueAction, *openUrlAction, *copyClipboardAction;
    bool displayPropertyTree;
};

#endif // PROPERTYBROWSER_H
