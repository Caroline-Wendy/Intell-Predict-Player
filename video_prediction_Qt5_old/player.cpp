#include "player.h"

#include "playercontrols.h"
#include "histogramwidget.h"
#include "thread"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QMediaMetaData>
#include <QtWidgets>

#include "horrorvideoprediction.h" //by me

Player::Player(QWidget *parent)
    : QWidget(parent)
    , videoWidget(0)
    , coverLabel(0)
    , slider(0)
    , colorDialog(0)
{
//! [create-objs]
    player = new QMediaPlayer(this);
//! [create-objs]

    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));

    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(player, SIGNAL(videoAvailableChanged(bool)), this, SLOT(videoAvailableChanged(bool)));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));

//! [2]
    videoWidget = new VideoWidget(this); //视频播放部件
    player->setVideoOutput(videoWidget); //放入播放器
//! [2]

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, player->duration() / 1000);

    labelDuration = new QLabel(this);
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));
    
    labelHistogram = new QLabel(this);
    labelHistogram->setText("Histogram:");
    histogram = new HistogramWidget(this);
    QHBoxLayout *histogramLayout = new QHBoxLayout;
    histogramLayout->addWidget(labelHistogram);
    histogramLayout->addWidget(histogram, 1);

    probe = new QVideoProbe(this);
    connect(probe, SIGNAL(videoFrameProbed(const QVideoFrame&)), histogram, SLOT(processFrame(QVideoFrame)));
    probe->setSource(player);

    QPushButton *openButton = new QPushButton(tr("Open"), this);

    connect(openButton, SIGNAL(clicked()), this, SLOT(open()));

    PlayerControls *controls = new PlayerControls(this);
    controls->setState(player->state());
    controls->setVolume(player->volume());
    controls->setMuted(controls->isMuted());

    connect(controls, SIGNAL(play()), player, SLOT(play()));
    connect(controls, SIGNAL(pause()), player, SLOT(pause()));
    connect(controls, SIGNAL(stop()), player, SLOT(stop()));
    connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
    connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));
    connect(controls, SIGNAL(changeRate(qreal)), player, SLOT(setPlaybackRate(qreal)));

    connect(controls, SIGNAL(stop()), videoWidget, SLOT(update()));

    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),
            controls, SLOT(setState(QMediaPlayer::State)));
    connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));
    connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));

    fullScreenButton = new QPushButton(tr("FullScreen"), this);
    fullScreenButton->setCheckable(true);

    colorButton = new QPushButton(tr("Color Options..."), this);
    colorButton->setEnabled(false);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

    QBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(videoWidget);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addStretch(1);
    controlLayout->addWidget(controls);
    controlLayout->addStretch(1);
    controlLayout->addWidget(fullScreenButton);
    controlLayout->addWidget(colorButton);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(slider);
    hLayout->addWidget(labelDuration);

    QBoxLayout *layout = new QVBoxLayout; //主框架
    layout->addLayout(displayLayout, 5); //视频
    layout->addLayout(hLayout); //滚动条+时间
    layout->addLayout(controlLayout); //控制栏
    layout->addLayout(histogramLayout); //声音梯度

    //by me
    //类型组合框
    processPushButton = new QPushButton(tr("Process"));

    typeGroupBox = new QGroupBox(tr("Type"));
    pornCheckBox = new QCheckBox(tr("&Pornography")); //色情
    horrCheckBox = new QCheckBox(tr("&Horror")); //恐怖
    voilCheckBox = new QCheckBox(tr("&Violence")); //暴力
    horrCheckBox->setChecked(true);

    QVBoxLayout *typeBoxLayout = new QVBoxLayout;
    typeBoxLayout->addWidget(pornCheckBox);
    typeBoxLayout->addWidget(horrCheckBox);
    typeBoxLayout->addWidget(voilCheckBox);
    typeGroupBox->setLayout(typeBoxLayout);

    //输出组合框
    outpGroupBox = new QGroupBox(tr("Output"));
    outpLabel = new QLabel;
    outpLabel->setAlignment(Qt::AlignCenter);
    outpLabel->setText(tr("<font color=red><h3>Unknown</h3></font>"));
    QVBoxLayout *outpBoxLayout = new QVBoxLayout;
    outpBoxLayout->addWidget(outpLabel);
    outpGroupBox->setLayout(outpBoxLayout);

    //信息组合框
    infoGroupBox = new QGroupBox(tr("Information"));

    processPushButton->setEnabled(false); //初始设定false
    connect(processPushButton, SIGNAL(clicked()), this, SLOT(processEvent()));

    QLabel *framesLabel = new QLabel(tr("Frames: ")); //帧数
    QLabel *frameNumLabel = new QLabel;
    /*QLabel *shotsLabel = new QLabel(tr("Shots:")); //镜头数
    QLabel *shotNumLabel = new QLabel;
    QLabel *audioLabel = new QLabel(tr("Audio:")); //音频
    QLabel *isAudioLabel = new QLabel;
    QLabel *shortLabel = new QLabel(tr("Short:")); //短视频
    QLabel *isShortLabel = new QLabel;*/
    frameNumLabel->setText(tr("<font color=blue>*****</font>"));
    /*shotNumLabel->setText(tr("<font color=blue>***</font>"));
    isAudioLabel->setText(tr("<font color=blue>Yes</font>"));
    isShortLabel->setText(tr("<font color=blue>Yes</font>"));*/
    QHBoxLayout *processBoxLayout = new QHBoxLayout;
    processBoxLayout->addWidget(processPushButton);
    QHBoxLayout *frameBoxLayout = new QHBoxLayout;
    frameBoxLayout->addWidget(framesLabel, 1);
    frameBoxLayout->addWidget(frameNumLabel, 1);
    /*QHBoxLayout *shotBoxLayout = new QHBoxLayout;
    shotBoxLayout->addWidget(shotsLabel, 1);
    shotBoxLayout->addWidget(shotNumLabel, 1);
    QHBoxLayout *audioBoxLayout = new QHBoxLayout;
    audioBoxLayout->addWidget(audioLabel, 1);
    audioBoxLayout->addWidget(isAudioLabel, 1);
    QHBoxLayout *shortBoxLayout = new QHBoxLayout;
    shortBoxLayout->addWidget(shortLabel, 1);
    shortBoxLayout->addWidget(isShortLabel, 1);*/

    QVBoxLayout *infoBoxLayout = new QVBoxLayout;
    infoBoxLayout->addLayout(processBoxLayout);
    infoBoxLayout->addLayout(frameBoxLayout);
    /*infoBoxLayout->addLayout(shotBoxLayout);
    infoBoxLayout->addLayout(audioBoxLayout);
    infoBoxLayout->addLayout(shortBoxLayout);*/

    infoGroupBox->setLayout(infoBoxLayout);


    QBoxLayout *prcssLayout = new QVBoxLayout;
    prcssLayout->addWidget(typeGroupBox, 1); //第二个参数为等分数
    prcssLayout->addWidget(outpGroupBox, 1);
    prcssLayout->addWidget(infoGroupBox, 1);

    QBoxLayout *myLayout = new QHBoxLayout;
    myLayout->addLayout(layout, 3);
    myLayout->addLayout(prcssLayout, 1);

    this->setFixedSize(720, 320); //设置固定大小

    //end by me

    //setLayout(layout); //by src
    setLayout(myLayout); //by me

    if (!player->isAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        playlistView->setEnabled(false);
        openButton->setEnabled(false);
        colorButton->setEnabled(false);
        processPushButton->setEnabled(false); //by me
        fullScreenButton->setEnabled(false);
    }

    metaDataChanged();
}

Player::~Player()
{
}

void Player::open()
{
   fileName = QFileDialog::getOpenFileName(this, tr("Open Movie"),QDir::homePath());

    if (!fileName.isEmpty()) {
        player->setMedia(QUrl::fromLocalFile(fileName));
    }

    outpLabel->setText(tr("<font color=red><h3>Unknown</h3></font>"));
}

void Player::durationChanged(qint64 duration)
{
    this->duration = duration/1000;
    slider->setMaximum(duration / 1000);
}

void Player::positionChanged(qint64 progress)
{
    if (!slider->isSliderDown()) {
        slider->setValue(progress / 1000);
    }
    updateDurationInfo(progress / 1000);
}

void Player::metaDataChanged()
{
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(player->metaData(QMediaMetaData::AlbumArtist).toString())
                .arg(player->metaData(QMediaMetaData::Title).toString()));

        if (coverLabel) {
            QUrl url = player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

            coverLabel->setPixmap(!url.isEmpty()
                    ? QPixmap(url.toString())
                    : QPixmap());
        }
    }
}

void Player::seek(int seconds)
{
    player->setPosition(seconds * 1000);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Media Stalled"));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}

void Player::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void Player::bufferingProgress(int progress)
{
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void Player::videoAvailableChanged(bool available)
{
    if (!available) {
        disconnect(fullScreenButton, SIGNAL(clicked(bool)),
                    videoWidget, SLOT(setFullScreen(bool)));
        disconnect(videoWidget, SIGNAL(fullScreenChanged(bool)),
                fullScreenButton, SLOT(setChecked(bool)));
        videoWidget->setFullScreen(false);
    } else {
        connect(fullScreenButton, SIGNAL(clicked(bool)),
                videoWidget, SLOT(setFullScreen(bool)));
        connect(videoWidget, SIGNAL(fullScreenChanged(bool)),
                fullScreenButton, SLOT(setChecked(bool)));

        if (fullScreenButton->isChecked())
            videoWidget->setFullScreen(true);
    }

    colorButton->setEnabled(available);
    processPushButton->setEnabled(available);
}

void Player::setTrackInfo(const QString &info)
{
    trackInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void Player::setStatusInfo(const QString &info)
{
    statusInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void Player::displayErrorMessage()
{
    setStatusInfo(player->errorString());
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
        QString format = "mm:ss";
        if (duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}

void Player::showColorDialog()
{
    if (!colorDialog) {
        QSlider *brightnessSlider = new QSlider(Qt::Horizontal);
        brightnessSlider->setRange(-100, 100);
        brightnessSlider->setValue(videoWidget->brightness());
        connect(brightnessSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setBrightness(int)));
        connect(videoWidget, SIGNAL(brightnessChanged(int)), brightnessSlider, SLOT(setValue(int)));

        QSlider *contrastSlider = new QSlider(Qt::Horizontal);
        contrastSlider->setRange(-100, 100);
        contrastSlider->setValue(videoWidget->contrast());
        connect(contrastSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setContrast(int)));
        connect(videoWidget, SIGNAL(contrastChanged(int)), contrastSlider, SLOT(setValue(int)));

        QSlider *hueSlider = new QSlider(Qt::Horizontal);
        hueSlider->setRange(-100, 100);
        hueSlider->setValue(videoWidget->hue());
        connect(hueSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setHue(int)));
        connect(videoWidget, SIGNAL(hueChanged(int)), hueSlider, SLOT(setValue(int)));

        QSlider *saturationSlider = new QSlider(Qt::Horizontal);
        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(videoWidget->saturation());
        connect(saturationSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setSaturation(int)));
        connect(videoWidget, SIGNAL(saturationChanged(int)), saturationSlider, SLOT(setValue(int)));

        QFormLayout *layout = new QFormLayout;
        layout->addRow(tr("Brightness"), brightnessSlider);
        layout->addRow(tr("Contrast"), contrastSlider);
        layout->addRow(tr("Hue"), hueSlider);
        layout->addRow(tr("Saturation"), saturationSlider);

        QPushButton *button = new QPushButton(tr("Close"));
        layout->addRow(button);

        colorDialog = new QDialog(this);
        colorDialog->setWindowTitle(tr("Color Options"));
        colorDialog->setLayout(layout);

        connect(button, SIGNAL(clicked()), colorDialog, SLOT(close()));
    }
    colorDialog->show();
}

void Player::processEvent()
{
        if(pornCheckBox->isChecked()){
            processPorn();
        }else{
            ;
        }

        if(horrCheckBox->isChecked()){
           processHorr();
        }else{
            ;
        }

        if(voilCheckBox->isChecked()){
            processVoil();
        }else{
            ;
        }
}

void Player::processPorn()
{
    QPushButton *testButton = new QPushButton(tr("test process by Porn"));
    testButton->show();
}

void Player::processHorr()
{
    horrThread.initHorr(fileName);
    if(!horrThread.isRunning()){
        horrThread.start();
        connect(&horrThread, SIGNAL(passHorrValue(const QString&)), this, SLOT(acceptHorr(const QString&)));
    }
    horrThread.quit();
}

void Player::acceptHorr(const QString &horrResult){
    int intResult = atoi(horrResult.toStdString().c_str());
    if(0 == intResult){
        outpLabel->setText(tr("<font color=red><h3>Not Horror! </h3></font>"));
    }
    else{
        outpLabel->setText(tr("<font color=red><h3>Horror Video! </h3></font>"));
    }
}

void Player::processVoil()
{
    QPushButton *testButton = new QPushButton(tr("test process by Voil"));
    testButton->show();
}
