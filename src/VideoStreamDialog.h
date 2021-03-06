#ifndef VIDEOSTREAMDIALOG_H
#define VIDEOSTREAMDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class VideoStreamDialog;
}

class VideoStreamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoStreamDialog(QWidget *parent = 0, QSettings *settings = 0);
    ~VideoStreamDialog();

    QString getUrl();
    QString getFormat();

public slots:

    void done(int r);
    void updateURL();
    void updateSourcePreview();
    void cancelSourcePreview();
    void connectedInfo();
    void failedInfo();
    void showHelp();

protected:
    void showEvent(QShowEvent *);

private:
    Ui::VideoStreamDialog *ui;

    class VideoStreamSource *s;
    QTimer *testingtimeout, *respawn;
    QSettings *appSettings;
};

#endif // VIDEOSTREAMDIALOG_H
