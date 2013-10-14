#ifndef MEDIALAYER_H
#define MEDIAPLAYER_H

#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtCore/QTimerEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QIcon>
#include <QtCore/QBasicTimer>
#include <QtGui/QAction>

#include <phonon/audiooutput.h>
#include <phonon/backendcapabilities.h>
#include <phonon/effect.h>
#include <phonon/effectparameter.h>
#include <phonon/effectwidget.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include <phonon/videowidget.h>
#include <phonon/volumeslider.h>

#include "thread.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QSlider;
class QTextEdit;
class QMenu;
class Ui_settings;
class QGroupBox; //by me 组合框
class QCheckBox; //by me 复选框
QT_END_NAMESPACE

class MediaPlayer;

class MediaVideoWidget : public Phonon::VideoWidget
{
    Q_OBJECT

public:
    MediaVideoWidget(MediaPlayer *player, QWidget *parent = 0);

public slots:
    // Over-riding non-virtual Phonon::VideoWidget slot
    void setFullScreen(bool);

signals:
    void fullScreenChanged(bool);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool event(QEvent *e);
    void timerEvent(QTimerEvent *e);
    void dropEvent(QDropEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);

private:
    MediaPlayer *m_player;
    QBasicTimer m_timer;
    QAction m_action;
};

class MediaPlayer :
            public QWidget
{
    Q_OBJECT
public:
    MediaPlayer();

    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
    void handleDrop(QDropEvent *e);
    void setFile(const QString &text);
    void setLocation(const QString &location);
    void initVideoWindow();
    void initSettingsDialog();
    void setVolume(qreal volume);
    void setSmallScreen(bool smallScreen);

public slots:
    void openFile();
    void rewind();
    void forward();
    void updateInfo();
    void updateTime();
    void finished();
    void playPause();
    void scaleChanged(QAction *);
    void aspectChanged(QAction *);

private slots:
    void setAspect(int);
    void setScale(int);
    void setSaturation(int);
    void setContrast(int);
    void setHue(int);
    void setBrightness(int);
    void stateChanged(Phonon::State newstate, Phonon::State oldstate);
    void effectChanged();
    void showSettingsDialog();
    void showContextMenu(const QPoint& point);
    void bufferStatus(int percent);
    void openUrl();
    void openRamFile();
    void configureEffect();
    void hasVideoChanged(bool);
    void processEvent(); //by me Process 事件
    void acceptPorn(const QString& pornResult); //by me 接收色情
    void acceptHorr(const QString& horrResult); //by me 接收恐怖
    //void acceptViol(const QString& horrResult); //by me 接收恐怖

private:
    bool playPauseForDialog();
    void processHorr(); //by me 处理视频
    void processPorn(); //by me 处理色情
    void processViol(); //by me 处理暴力
    void loadVideoInformation();

    QIcon playIcon;
    QIcon pauseIcon;
    QMenu *fileMenu;
    QPushButton *playButton;
    QPushButton *rewindButton;
    QPushButton *forwardButton;
    Phonon::SeekSlider *slider;
    QLabel *timeLabel;
    QLabel *progressLabel;
    Phonon::VolumeSlider *volume;
    QSlider *m_hueSlider;
    QSlider *m_satSlider;
    QSlider *m_contSlider;
    QLabel *info;
    Phonon::Effect *nextEffect;
    QDialog *settingsDialog;
    Ui_settings *ui;
    QAction *m_fullScreenAction;

    QPushButton* processPushButton; //by me 处理按钮
    QCheckBox *pornCheckBox; //by me 色情
    QCheckBox *horrCheckBox; //be me 恐怖
    QCheckBox *violCheckBox; //be me 暴力
    QLabel *pornResultLabel; //by me 色情结果
    QLabel *horrResultLabel; //by me 恐怖结果
    QLabel *violResultLabel; //by me 暴力结果
    QLabel *frameNumLabel; //by me 输出帧数
    QString prcssfileName; //by me 打开文件名
    HorrThread horrThread; //by me 检查恐怖视频线程
    PornThread pornThread; //by me 检查色情视频线程

    QWidget m_videoWindow;
    Phonon::MediaObject m_MediaObject;
    Phonon::AudioOutput m_AudioOutput;
    MediaVideoWidget *m_videoWidget;
    Phonon::Path m_audioOutputPath;
    bool m_smallScreen;
};

#endif //MEDIAPLAYER_H
