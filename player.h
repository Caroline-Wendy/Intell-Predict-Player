#ifndef PLAYER_H
#define PLAYER_H

#include "videowidget.h"
#include "thread.h" //by me

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>

QT_BEGIN_NAMESPACE
class QAbstractItemView;
class QLabel;
class QMediaPlayer;
class QModelIndex;
class QPushButton;
class QSlider;
class QVideoProbe;
class QVideoWidget;
class QGroupBox; //by me 组合框
class QCheckBox;
class QThread;
QT_END_NAMESPACE

class PlaylistModel;
class HistogramWidget;

class Player : public QWidget
{
    Q_OBJECT

public:
    Player(QWidget *parent = 0);
    ~Player();

signals:
    void fullScreenChanged(bool fullScreen);

private slots:
    void open();
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void metaDataChanged();

    void seek(int seconds);

    void statusChanged(QMediaPlayer::MediaStatus status);
    void bufferingProgress(int progress);
    void videoAvailableChanged(bool available);

    void displayErrorMessage();

    void showColorDialog();
    void processHorr(); //by me 处理视频
    void processPorn(); //by me 处理色情
    void processVoil(); //by me 处理暴力
    void processEvent(); //Process 事件
    void acceptHorr(const QString& horrResult); //by me 接受恐怖结果

private:
    void setTrackInfo(const QString &info);
    void setStatusInfo(const QString &info);
    void handleCursor(QMediaPlayer::MediaStatus status);
    void updateDurationInfo(qint64 currentInfo);

    QMediaPlayer *player;
    QMediaPlaylist *playlist;
    VideoWidget *videoWidget;
    QLabel *coverLabel;
    QSlider *slider;
    QLabel *labelDuration;
    QPushButton *fullScreenButton;
    QPushButton *colorButton;
    QDialog *colorDialog;

    QLabel *labelHistogram;
    HistogramWidget *histogram;
    QVideoProbe *probe;

    QCheckBox *pornCheckBox; //色情
    QCheckBox *horrCheckBox; //恐怖
    QCheckBox *voilCheckBox; //暴力
    QLabel *outpLabel; //by me 结果标签
    QPushButton* processPushButton; //by me 处理按钮
    QGroupBox *typeGroupBox; //by me 预测组合
    QGroupBox *outpGroupBox; //by me 输出组合
    QGroupBox *infoGroupBox; //by me 信息组合

    QString fileName; //by me 当前文件名
    HorrThread horrThread;

    QAbstractItemView *playlistView;
    QString trackInfo;
    QString statusInfo;
    qint64 duration;
};

#endif // PLAYER_H
