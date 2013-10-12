/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
***************************************************************************/

#include <QtGui>

#define SLIDER_RANGE 8

#include "mediaplayer.h"
#include "ui_settings.h"
#include "opencv2/opencv.hpp"
#include <string>

MediaVideoWidget::MediaVideoWidget(MediaPlayer *player, QWidget *parent) :
    Phonon::VideoWidget(parent), m_player(player), m_action(this)
{
    m_action.setCheckable(true);
    m_action.setChecked(false);
    m_action.setShortcut(QKeySequence( Qt::AltModifier + Qt::Key_Return));
    m_action.setShortcutContext(Qt::WindowShortcut);
    connect(&m_action, SIGNAL(toggled(bool)), SLOT(setFullScreen(bool)));
    addAction(&m_action);
    setAcceptDrops(true);
}

void MediaVideoWidget::setFullScreen(bool enabled)
{
    Phonon::VideoWidget::setFullScreen(enabled);
    emit fullScreenChanged(enabled);
}

void MediaVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Phonon::VideoWidget::mouseDoubleClickEvent(e);
    setFullScreen(!isFullScreen());
}

void MediaVideoWidget::keyPressEvent(QKeyEvent *e)
{
    if(!e->modifiers()) {
        // On non-QWERTY Symbian key-based devices, there is no space key.
        // The zero key typically is marked with a space character.
        if (e->key() == Qt::Key_Space || e->key() == Qt::Key_0) {
            m_player->playPause();
            e->accept();
            return;
        }

        // On Symbian devices, there is no key which maps to Qt::Key_Escape
        // On devices which lack a backspace key (i.e. non-QWERTY devices),
        // the 'C' key maps to Qt::Key_Backspace
        else if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Backspace) {
            setFullScreen(false);
            e->accept();
            return;
        }
    }
    Phonon::VideoWidget::keyPressEvent(e);
}

bool MediaVideoWidget::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::Close:
        //we just ignore the cose events on the video widget
        //this prevents ALT+F4 from having an effect in fullscreen mode
        e->ignore();
        return true;
    case QEvent::MouseMove:
#ifndef QT_NO_CURSOR
        unsetCursor();
#endif
        //fall through
    case QEvent::WindowStateChange:
        {
            //we just update the state of the checkbox, in case it wasn't already
            m_action.setChecked(windowState() & Qt::WindowFullScreen);
            const Qt::WindowFlags flags = m_player->windowFlags();
            if (windowState() & Qt::WindowFullScreen) {
                m_timer.start(1000, this);
            } else {
                m_timer.stop();
#ifndef QT_NO_CURSOR
                unsetCursor();
#endif
            }
        }
        break;
    default:
        break;
    }

    return Phonon::VideoWidget::event(e);
}

void MediaVideoWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timer.timerId()) {
        //let's store the cursor shape
#ifndef QT_NO_CURSOR
        setCursor(Qt::BlankCursor);
#endif
    }
    Phonon::VideoWidget::timerEvent(e);
}

void MediaVideoWidget::dropEvent(QDropEvent *e)
{
    m_player->handleDrop(e);
}

void MediaVideoWidget::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}


MediaPlayer::MediaPlayer() :
        playButton(0), nextEffect(0), settingsDialog(0), ui(0),
            m_AudioOutput(Phonon::VideoCategory),
            m_videoWidget(new MediaVideoWidget(this))
{
    setWindowTitle(tr("Video Prediction")); //设置标题
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    //打开按钮
    QPushButton *openButton = new QPushButton(this);
    openButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    QPalette bpal;
    QColor arrowcolor = bpal.buttonText().color();
    if (arrowcolor == Qt::black)
        arrowcolor = QColor(80, 80, 80);
    bpal.setBrush(QPalette::ButtonText, arrowcolor);
    openButton->setPalette(bpal);

    rewindButton = new QPushButton(this); //倒回按钮
    rewindButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    forwardButton = new QPushButton(this); //前进按钮
    forwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    forwardButton->setEnabled(false);

    playButton = new QPushButton(this); //播放按钮
    playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    playButton->setIcon(playIcon);

    slider = new Phonon::SeekSlider(this); //滑动条
    slider->setMediaObject(&m_MediaObject);
    volume = new Phonon::VolumeSlider(&m_AudioOutput);
    volume->setFixedWidth(120);

    QVBoxLayout *vLayout = new QVBoxLayout();

    QHBoxLayout *layout = new QHBoxLayout(); //控制组件框架

    //信息标签
    info = new QLabel(this);
    info->setMinimumHeight(400);
    info->setMaximumHeight(400);
    info->setMinimumWidth(720);
    info->setMaximumWidth(720);
    info->setAcceptDrops(false);
    info->setMargin(2);
    info->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    info->setLineWidth(2);
    info->setAutoFillBackground(true);

    QPalette palette;
    palette.setBrush(QPalette::WindowText, Qt::white);
    info->setStyleSheet("border-image:url(:/images/screen.png) ; border-width:3px");
    info->setPalette(palette);
    info->setText(tr("<center><font color=purple>Girls' Generation</font></center>"));

    layout->addWidget(openButton);
    layout->addWidget(rewindButton);
    layout->addWidget(playButton);
    layout->addWidget(forwardButton);

    layout->addStretch();
    layout->addWidget(volume);

    vLayout->addWidget(info);
    initVideoWindow();
    vLayout->addWidget(&m_videoWindow);
    m_videoWindow.hide();
    QVBoxLayout *buttonPanelLayout = new QVBoxLayout();
    buttonPanelLayout->addLayout(layout);
    
    timeLabel = new QLabel(this);
    progressLabel = new QLabel(this);
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(slider);
    sliderLayout->addWidget(timeLabel);    
    sliderLayout->addWidget(progressLabel);    
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *sliderPanel = new QWidget(this);
    sliderPanel->setLayout(sliderLayout);

    buttonPanelLayout->addWidget(sliderPanel);
    buttonPanelLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *buttonPanelWidget = new QWidget(this);
    buttonPanelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); 
    buttonPanelWidget->setLayout(buttonPanelLayout);    
    vLayout->addWidget(buttonPanelWidget);
 
    QHBoxLayout *labelLayout = new QHBoxLayout();

    vLayout->addLayout(labelLayout);

    /**********process layout**********/

    //类型组合框
    QGroupBox* typeGroupBox = new QGroupBox(tr("Type"));

    pornCheckBox = new QCheckBox(tr("&Pornography")); //色情
    horrCheckBox = new QCheckBox(tr("&Horror")); //恐怖
    violCheckBox = new QCheckBox(tr("&Violence")); //暴力
    horrCheckBox->setChecked(true);

    QVBoxLayout *typeBoxLayout = new QVBoxLayout; //类型框
    typeBoxLayout->addWidget(pornCheckBox);
    typeBoxLayout->addWidget(horrCheckBox);
    typeBoxLayout->addWidget(violCheckBox);
    typeGroupBox->setLayout(typeBoxLayout);

    //输出组合框
    QGroupBox* outpGroupBox = new QGroupBox(tr("Output"));

    QLabel* pornTextLabel = new QLabel(tr("Pornography: "));
    QLabel* horrTextLabel = new QLabel(tr("Horror: "));
    QLabel* violTextLabel = new QLabel(tr("Violence: "));
    pornResultLabel = new QLabel(tr("Unknown"));
    horrResultLabel = new QLabel(tr("Unknown"));
    violResultLabel = new QLabel(tr("Unknown"));

    QGridLayout* outpGridLayout = new QGridLayout;
    outpGridLayout->addWidget(pornTextLabel, 0, 0);
    outpGridLayout->addWidget(horrTextLabel, 1, 0);
    outpGridLayout->addWidget(violTextLabel, 2, 0);
    outpGridLayout->addWidget(pornResultLabel, 0, 1);
    outpGridLayout->addWidget(horrResultLabel, 1, 1);
    outpGridLayout->addWidget(violResultLabel, 2, 1);
    outpGroupBox->setLayout(outpGridLayout);


    //信息组合框
    QGroupBox* infoGroupBox = new QGroupBox(tr("Information"));

    processPushButton = new QPushButton(tr("Process"));
    processPushButton->setEnabled(false); //初始设定false
    connect(processPushButton, SIGNAL(clicked()), this, SLOT(processEvent()));

    QLabel *framesTextLabel = new QLabel(tr("Frames: ")); //帧数
    frameNumLabel = new QLabel(tr("<font color=blue>Unset</font>"));

    QGridLayout *infoGridLayout = new QGridLayout;
    infoGridLayout->addWidget(processPushButton, 0, 0, 1, 2);
    infoGridLayout->addWidget(framesTextLabel, 1, 0);
    infoGridLayout->addWidget(frameNumLabel, 1, 1);
    infoGroupBox->setLayout(infoGridLayout);

    //总框体
    QBoxLayout *prcssLayout = new QVBoxLayout;
    prcssLayout->addWidget(typeGroupBox, 1); //第二个参数为等分数
    prcssLayout->addWidget(outpGroupBox, 1);
    prcssLayout->addWidget(infoGroupBox, 1);

    QBoxLayout *playerlayout = new QHBoxLayout(this);
    playerlayout->addLayout(vLayout, 4);
    playerlayout->addLayout(prcssLayout, 1);

    this->setFixedSize(960, 480);
    //this->setFixedSize(this->width(), this->height()); //设置固定大小
    setLayout(playerlayout); //by me

    // Create menu bar:
    fileMenu = new QMenu(this);
    QAction *openFileAction = fileMenu->addAction(tr("Open &File..."));
    QAction *openUrlAction = fileMenu->addAction(tr("Open &Location..."));
    QAction *const openLinkAction = fileMenu->addAction(tr("Open &RAM File..."));

    connect(openLinkAction, SIGNAL(triggered(bool)), this, SLOT(openRamFile()));

    fileMenu->addSeparator();  
    QMenu *aspectMenu = fileMenu->addMenu(tr("&Aspect ratio"));
    QActionGroup *aspectGroup = new QActionGroup(aspectMenu);
    connect(aspectGroup, SIGNAL(triggered(QAction*)), this, SLOT(aspectChanged(QAction*)));
    aspectGroup->setExclusive(true);
    QAction *aspectActionAuto = aspectMenu->addAction(tr("Auto"));
    aspectActionAuto->setCheckable(true);
    aspectActionAuto->setChecked(true);
    aspectGroup->addAction(aspectActionAuto);
    QAction *aspectActionScale = aspectMenu->addAction(tr("Scale"));
    aspectActionScale->setCheckable(true);
    aspectGroup->addAction(aspectActionScale);
    QAction *aspectAction16_9 = aspectMenu->addAction(tr("16/9"));
    aspectAction16_9->setCheckable(true);
    aspectGroup->addAction(aspectAction16_9);
    QAction *aspectAction4_3 = aspectMenu->addAction(tr("4/3"));
    aspectAction4_3->setCheckable(true);
    aspectGroup->addAction(aspectAction4_3);

    QMenu *scaleMenu = fileMenu->addMenu(tr("&Scale mode"));
    QActionGroup *scaleGroup = new QActionGroup(scaleMenu);
    connect(scaleGroup, SIGNAL(triggered(QAction*)), this, SLOT(scaleChanged(QAction*)));
    scaleGroup->setExclusive(true);
    QAction *scaleActionFit = scaleMenu->addAction(tr("Fit in view"));
    scaleActionFit->setCheckable(true);
    scaleActionFit->setChecked(true);
    scaleGroup->addAction(scaleActionFit);
    QAction *scaleActionCrop = scaleMenu->addAction(tr("Scale and crop"));
    scaleActionCrop->setCheckable(true);
    scaleGroup->addAction(scaleActionCrop);

    m_fullScreenAction = fileMenu->addAction(tr("Full screen video"));
    m_fullScreenAction->setCheckable(true);
    m_fullScreenAction->setEnabled(false); // enabled by hasVideoChanged
    bool b = connect(m_fullScreenAction, SIGNAL(toggled(bool)), m_videoWidget, SLOT(setFullScreen(bool)));
    Q_ASSERT(b);
    b = connect(m_videoWidget, SIGNAL(fullScreenChanged(bool)), m_fullScreenAction, SLOT(setChecked(bool)));
    Q_ASSERT(b);

    fileMenu->addSeparator();
    QAction *settingsAction = fileMenu->addAction(tr("&Settings..."));

    // Setup signal connections:
    connect(rewindButton, SIGNAL(clicked()), this, SLOT(rewind()));
    //connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    openButton->setMenu(fileMenu);

    connect(playButton, SIGNAL(clicked()), this, SLOT(playPause()));
    connect(forwardButton, SIGNAL(clicked()), this, SLOT(forward()));
    //connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(settingsAction, SIGNAL(triggered(bool)), this, SLOT(showSettingsDialog()));
    connect(openUrlAction, SIGNAL(triggered(bool)), this, SLOT(openUrl()));
    connect(openFileAction, SIGNAL(triggered(bool)), this, SLOT(openFile()));

    connect(m_videoWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
    connect(&m_MediaObject, SIGNAL(metaDataChanged()), this, SLOT(updateInfo()));
    connect(&m_MediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(updateTime()));
    connect(&m_MediaObject, SIGNAL(tick(qint64)), this, SLOT(updateTime()));
    connect(&m_MediaObject, SIGNAL(finished()), this, SLOT(finished()));
    connect(&m_MediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(&m_MediaObject, SIGNAL(bufferStatus(int)), this, SLOT(bufferStatus(int)));
    connect(&m_MediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(hasVideoChanged(bool)));

    rewindButton->setEnabled(false);
    playButton->setEnabled(false);
    processPushButton->setEnabled(false);
    setAcceptDrops(true);

    m_audioOutputPath = Phonon::createPath(&m_MediaObject, &m_AudioOutput);
    Phonon::createPath(&m_MediaObject, m_videoWidget);

    resize(minimumSizeHint());
}

void MediaPlayer::stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Q_UNUSED(oldstate);

    if (oldstate == Phonon::LoadingState) {
        QRect videoHintRect = QRect(QPoint(0, 0), m_videoWindow.sizeHint());
        QRect newVideoRect = QApplication::desktop()->screenGeometry().intersected(videoHintRect);
        if (!m_smallScreen) {
            if (m_MediaObject.hasVideo()) {
                // Flush event que so that sizeHint takes the
                // recently shown/hidden m_videoWindow into account:
                qApp->processEvents();
                resize(sizeHint());
            } else
                resize(minimumSize());
        }
    }

    switch (newstate) {
        case Phonon::ErrorState:
            if (m_MediaObject.errorType() == Phonon::FatalError) {
                playButton->setEnabled(false);
                rewindButton->setEnabled(false);
                processPushButton->setEnabled(false);
            } else {
                m_MediaObject.pause();
            }
            QMessageBox::warning(this, "Phonon Mediaplayer", m_MediaObject.errorString(), QMessageBox::Close);
            break;

        case Phonon::StoppedState:
            m_videoWidget->setFullScreen(false);
            // Fall through
        case Phonon::PausedState:
            playButton->setIcon(playIcon);
            if (m_MediaObject.currentSource().type() != Phonon::MediaSource::Invalid){
                playButton->setEnabled(true);
                rewindButton->setEnabled(true);
                processPushButton->setEnabled(true);
            } else {
                playButton->setEnabled(false);
                rewindButton->setEnabled(false);
                processPushButton->setEnabled(false);
            }
            break;
        case Phonon::PlayingState:
            playButton->setEnabled(true);
            processPushButton->setEnabled(true);
            playButton->setIcon(pauseIcon);
            if (m_MediaObject.hasVideo())
                m_videoWindow.show();
            // Fall through
        case Phonon::BufferingState:
            rewindButton->setEnabled(true);
            break;
        case Phonon::LoadingState:
            rewindButton->setEnabled(false);
            break;
    }

}

void MediaPlayer::initSettingsDialog()
{
    settingsDialog = new QDialog(this);
    ui = new Ui_settings();
    ui->setupUi(settingsDialog);

    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));
    connect(ui->hueSlider, SIGNAL(valueChanged(int)), this, SLOT(setHue(int)));
    connect(ui->saturationSlider, SIGNAL(valueChanged(int)), this, SLOT(setSaturation(int)));
    connect(ui->contrastSlider , SIGNAL(valueChanged(int)), this, SLOT(setContrast(int)));
    connect(ui->aspectCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAspect(int)));
    connect(ui->scalemodeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setScale(int)));

    ui->brightnessSlider->setValue(int(m_videoWidget->brightness() * SLIDER_RANGE));
    ui->hueSlider->setValue(int(m_videoWidget->hue() * SLIDER_RANGE));
    ui->saturationSlider->setValue(int(m_videoWidget->saturation() * SLIDER_RANGE));
    ui->contrastSlider->setValue(int(m_videoWidget->contrast() * SLIDER_RANGE));
    ui->aspectCombo->setCurrentIndex(m_videoWidget->aspectRatio());
    ui->scalemodeCombo->setCurrentIndex(m_videoWidget->scaleMode());
    connect(ui->effectButton, SIGNAL(clicked()), this, SLOT(configureEffect()));

    ui->crossFadeSlider->setValue((int)(2 * m_MediaObject.transitionTime() / 1000.0f));
    
    // Insert audio devices:
    QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i=0; i<devices.size(); i++){
        QString itemText = devices[i].name();
        if (!devices[i].description().isEmpty()) {
            itemText += QString::fromLatin1(" (%1)").arg(devices[i].description());
        }
        ui->deviceCombo->addItem(itemText);
        if (devices[i] == m_AudioOutput.outputDevice())
            ui->deviceCombo->setCurrentIndex(i);
    }

    // Insert audio effects:
    ui->audioEffectsCombo->addItem(tr("<no effect>"));
    QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
    Phonon::Effect *currEffect = currEffects.size() ? currEffects[0] : 0;
    QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
    for (int i=0; i<availableEffects.size(); i++){
        ui->audioEffectsCombo->addItem(availableEffects[i].name());
        if (currEffect && availableEffects[i] == currEffect->description())
            ui->audioEffectsCombo->setCurrentIndex(i+1);
    }
    connect(ui->audioEffectsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(effectChanged()));

}

void MediaPlayer::setVolume(qreal volume)
{
    m_AudioOutput.setVolume(volume);
}

void MediaPlayer::setSmallScreen(bool smallScreen)
{
    m_smallScreen = smallScreen;
}

void MediaPlayer::effectChanged()
{
    int currentIndex = ui->audioEffectsCombo->currentIndex();
    if (currentIndex) {
        QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
        Phonon::EffectDescription chosenEffect = availableEffects[currentIndex - 1];

        QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
        Phonon::Effect *currentEffect = currEffects.size() ? currEffects[0] : 0;

        // Deleting the running effect will stop playback, it is deleted when removed from path
        if (nextEffect && !(currentEffect && (currentEffect->description().name() == nextEffect->description().name())))
            delete nextEffect;

        nextEffect = new Phonon::Effect(chosenEffect);
    }
    ui->effectButton->setEnabled(currentIndex);
}

void MediaPlayer::showSettingsDialog()
{
    const bool hasPausedForDialog = playPauseForDialog();

    if (!settingsDialog)
        initSettingsDialog();

    float oldBrightness = m_videoWidget->brightness();
    float oldHue = m_videoWidget->hue();
    float oldSaturation = m_videoWidget->saturation();
    float oldContrast = m_videoWidget->contrast();
    Phonon::VideoWidget::AspectRatio oldAspect = m_videoWidget->aspectRatio();
    Phonon::VideoWidget::ScaleMode oldScale = m_videoWidget->scaleMode();
    int currentEffect = ui->audioEffectsCombo->currentIndex();
    settingsDialog->exec();
    
    if (settingsDialog->result() == QDialog::Accepted){
        m_MediaObject.setTransitionTime((int)(1000 * float(ui->crossFadeSlider->value()) / 2.0f));
        QList<Phonon::AudioOutputDevice> devices = Phonon::BackendCapabilities::availableAudioOutputDevices();
        m_AudioOutput.setOutputDevice(devices[ui->deviceCombo->currentIndex()]);
        QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
        QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();

        if (ui->audioEffectsCombo->currentIndex() > 0){
            Phonon::Effect *currentEffect = currEffects.size() ? currEffects[0] : 0;    
            if (!currentEffect || currentEffect->description() != nextEffect->description()){
                foreach(Phonon::Effect *effect, currEffects) {
                    m_audioOutputPath.removeEffect(effect);
                    delete effect;
                }
                m_audioOutputPath.insertEffect(nextEffect);
            }
        } else {
            foreach(Phonon::Effect *effect, currEffects) {
                m_audioOutputPath.removeEffect(effect);
                delete effect;
                nextEffect = 0;
            }
        }
    } else {
        // Restore previous settings
        m_videoWidget->setBrightness(oldBrightness);
        m_videoWidget->setSaturation(oldSaturation);
        m_videoWidget->setHue(oldHue);
        m_videoWidget->setContrast(oldContrast);
        m_videoWidget->setAspectRatio(oldAspect);
        m_videoWidget->setScaleMode(oldScale);
        ui->audioEffectsCombo->setCurrentIndex(currentEffect);
    }

    if (hasPausedForDialog)
        m_MediaObject.play();
}

void MediaPlayer::initVideoWindow()
{
    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addWidget(m_videoWidget);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    m_videoWindow.setLayout(videoLayout);
    m_videoWindow.setMinimumSize(100, 100);
}

void MediaPlayer::configureEffect()
{
    if (!nextEffect)
        return;


    QList<Phonon::Effect *> currEffects = m_audioOutputPath.effects();
    const QList<Phonon::EffectDescription> availableEffects = Phonon::BackendCapabilities::availableAudioEffects();
    if (ui->audioEffectsCombo->currentIndex() > 0) {
        Phonon::EffectDescription chosenEffect = availableEffects[ui->audioEffectsCombo->currentIndex() - 1];

        QDialog effectDialog;
        effectDialog.setWindowTitle(tr("Configure effect"));
        QVBoxLayout *topLayout = new QVBoxLayout(&effectDialog);

        QLabel *description = new QLabel("<b>Description:</b><br>" + chosenEffect.description(), &effectDialog);
        description->setWordWrap(true); 
        topLayout->addWidget(description);

        QScrollArea *scrollArea = new QScrollArea(&effectDialog);
        topLayout->addWidget(scrollArea);

        QVariantList savedParamValues;
        foreach(Phonon::EffectParameter param, nextEffect->parameters()) {
            savedParamValues << nextEffect->parameterValue(param);
        }

        QWidget *scrollWidget = new Phonon::EffectWidget(nextEffect);
        scrollWidget->setMinimumWidth(320);
        scrollWidget->setContentsMargins(10, 10, 10,10);
        scrollArea->setWidget(scrollWidget);

        QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &effectDialog);
        connect(bbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &effectDialog, SLOT(accept()));
        connect(bbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &effectDialog, SLOT(reject()));
        topLayout->addWidget(bbox);

        effectDialog.exec();

        if (effectDialog.result() != QDialog::Accepted) {
            //we need to restore the parameters values
            int currentIndex = 0;
            foreach(Phonon::EffectParameter param, nextEffect->parameters()) {
                nextEffect->setParameterValue(param, savedParamValues.at(currentIndex++));
            }

        }
    }
}

void MediaPlayer::handleDrop(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if (e->proposedAction() == Qt::MoveAction){
        // Just add to the queue:
        for (int i=0; i<urls.size(); i++)
            m_MediaObject.enqueue(Phonon::MediaSource(urls[i].toLocalFile()));
    } else {
        // Create new queue:
        m_MediaObject.clearQueue();
        if (urls.size() > 0) {
            QString fileName = urls[0].toLocalFile();
            QDir dir(fileName);
            if (dir.exists()) {
                dir.setFilter(QDir::Files);            
                QStringList entries = dir.entryList();
                if (entries.size() > 0) {
                    setFile(fileName + QDir::separator() +  entries[0]);
                    for (int i=1; i< entries.size(); ++i)
                        m_MediaObject.enqueue(fileName + QDir::separator() + entries[i]);
                }
            } else {
                setFile(fileName);
                for (int i=1; i<urls.size(); i++)
                    m_MediaObject.enqueue(Phonon::MediaSource(urls[i].toLocalFile()));
            }        
        }
    }
    forwardButton->setEnabled(m_MediaObject.queue().size() > 0);
    m_MediaObject.play();
}

void MediaPlayer::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls() && e->proposedAction() != Qt::LinkAction) {
        e->acceptProposedAction();
        handleDrop(e);
    } else {
        e->ignore(); 
    }
}

void MediaPlayer::dragEnterEvent(QDragEnterEvent *e)
{
    dragMoveEvent(e);
}

void MediaPlayer::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        if (e->proposedAction() == Qt::CopyAction || e->proposedAction() == Qt::MoveAction){
            e->acceptProposedAction();
        }
    }
}

void MediaPlayer::playPause()
{
    if (m_MediaObject.state() == Phonon::PlayingState)
        m_MediaObject.pause();
    else {
        if (m_MediaObject.currentTime() == m_MediaObject.totalTime())
            m_MediaObject.seek(0);
        m_MediaObject.play();
    }
}

void MediaPlayer::setFile(const QString &fileName)
{
    setWindowTitle(fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1));
    m_MediaObject.setCurrentSource(Phonon::MediaSource(fileName));
    m_MediaObject.play();
}

void MediaPlayer::setLocation(const QString& location)
{
    setWindowTitle(location.right(location.length() - location.lastIndexOf('/') - 1));
    m_MediaObject.setCurrentSource(Phonon::MediaSource(QUrl::fromEncoded(location.toUtf8())));
    m_MediaObject.play();
}

bool MediaPlayer::playPauseForDialog()
{
    // If we're running on a small screen, we want to pause the video when
    // popping up dialogs. We neither want to tamper with the state if the
    // user has paused.
    if (m_smallScreen && m_MediaObject.hasVideo()) {
        if (Phonon::PlayingState == m_MediaObject.state()) {
            m_MediaObject.pause();
            return true;
        }
    }
    return false;
}

void MediaPlayer::openFile()
{
    const bool hasPausedForDialog = playPauseForDialog();

    QStringList fileNames = QFileDialog::getOpenFileNames(this, QString(),
                                                          QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    if (hasPausedForDialog)
        m_MediaObject.play();

    m_MediaObject.clearQueue();
    if (fileNames.size() > 0) {
        QString fileName = fileNames[0];

        /*by me*/
        prcssfileName = fileName;
        setFile(fileName);
        loadVideoInformation();
        pornResultLabel->setText(tr("Unknown"));
        horrResultLabel->setText(tr("Unknown"));
        violResultLabel->setText(tr("Unknown"));

        for (int i=1; i<fileNames.size(); i++)
            m_MediaObject.enqueue(Phonon::MediaSource(fileNames[i]));
    }
    forwardButton->setEnabled(m_MediaObject.queue().size() > 0);
}

void MediaPlayer::bufferStatus(int percent)
{
    if (percent == 100)
        progressLabel->setText(QString());
    else {
        QString str = QString::fromLatin1("(%1%)").arg(percent);
        progressLabel->setText(str);
    }
}

void MediaPlayer::setSaturation(int val)
{
    m_videoWidget->setSaturation(val / qreal(SLIDER_RANGE));
}

void MediaPlayer::setHue(int val)
{
    m_videoWidget->setHue(val / qreal(SLIDER_RANGE));
}

void MediaPlayer::setAspect(int val)
{
    m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio(val));
}

void MediaPlayer::setScale(int val)
{
    m_videoWidget->setScaleMode(Phonon::VideoWidget::ScaleMode(val));
}

void MediaPlayer::setBrightness(int val)
{
    m_videoWidget->setBrightness(val / qreal(SLIDER_RANGE));
}

void MediaPlayer::setContrast(int val)
{
    m_videoWidget->setContrast(val / qreal(SLIDER_RANGE));
}

void MediaPlayer::updateInfo()
{
    int maxLength = 30;
    QString font = "<font color=#ffeeaa>";
    QString fontmono = "<font family=\"monospace,courier new\" color=#ffeeaa>";

    QMap <QString, QString> metaData = m_MediaObject.metaData();
    QString trackArtist = metaData.value("ARTIST");
    if (trackArtist.length() > maxLength)
        trackArtist = trackArtist.left(maxLength) + "...";
    
    QString trackTitle = metaData.value("TITLE");
    int trackBitrate = metaData.value("BITRATE").toInt();

    QString fileName;
    if (m_MediaObject.currentSource().type() == Phonon::MediaSource::Url) {
        fileName = m_MediaObject.currentSource().url().toString();
    } else {
        fileName = m_MediaObject.currentSource().fileName();
        fileName = fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1);
        if (fileName.length() > maxLength)
            fileName = fileName.left(maxLength) + "...";
    }
    
    QString title;    
    if (!trackTitle.isEmpty()) {
        if (trackTitle.length() > maxLength)
            trackTitle = trackTitle.left(maxLength) + "...";
        title = "Title: " + font + trackTitle + "<br></font>";
    } else if (!fileName.isEmpty()) {
        if (fileName.length() > maxLength)
            fileName = fileName.left(maxLength) + "...";
        title = font + fileName + "</font>";
        if (m_MediaObject.currentSource().type() == Phonon::MediaSource::Url) {
            title.prepend("Url: ");
        } else {
            title.prepend("File: ");
        }
    }
        
    QString artist;
    if (!trackArtist.isEmpty())
        artist = "Artist:  " + font + trackArtist + "</font>";

    QString bitrate;
    if (trackBitrate != 0)
        bitrate = "<br>Bitrate:  " + font + QString::number(trackBitrate/1000) + "kbit</font>";

    info->setText(title + artist + bitrate);
}

void MediaPlayer::updateTime()
{
    long len = m_MediaObject.totalTime();
    long pos = m_MediaObject.currentTime();
    QString timeString;    
    if (pos || len)
    {
        int sec = pos/1000;
        int min = sec/60;
        int hour = min/60;
        int msec = pos;

        QTime playTime(hour%60, min%60, sec%60, msec%1000);
        sec = len / 1000;
        min = sec / 60;
        hour = min / 60;
        msec = len;

        QTime stopTime(hour%60, min%60, sec%60, msec%1000);
        QString timeFormat = "m:ss";
        if (hour > 0)
            timeFormat = "h:mm:ss";        
        timeString = playTime.toString(timeFormat);
        if (len)
            timeString += " / " + stopTime.toString(timeFormat);
    }
    timeLabel->setText(timeString);
}

void MediaPlayer::rewind()
{
    m_MediaObject.seek(0);
}

void MediaPlayer::forward()
{
    QList<Phonon::MediaSource> queue = m_MediaObject.queue();
    if (queue.size() > 0) {
        m_MediaObject.setCurrentSource(queue[0]);
        forwardButton->setEnabled(queue.size() > 1);
        m_MediaObject.play();
    }
}

void MediaPlayer::openUrl()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));
    QString sourceURL = settings.value("location").toString();
    bool ok = false;
    sourceURL = QInputDialog::getText(this, tr("Open Location"), tr("Please enter a valid address here:"), QLineEdit::Normal, sourceURL, &ok);
    if (ok && !sourceURL.isEmpty()) {
        setLocation(sourceURL);
        settings.setValue("location", sourceURL);
    }
}

/*!
 \since 4.6
 */
void MediaPlayer::openRamFile()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));

    const QStringList fileNameList(QFileDialog::getOpenFileNames(this,
                                                                  QString(),
                                                                  settings.value("openRamFile").toString(),
                                                                  QLatin1String("RAM files (*.ram)")));

    if (fileNameList.isEmpty())
        return;

    QFile linkFile;
    QList<QUrl> list;
    QByteArray sourceURL;
    for (int i = 0; i < fileNameList.count(); i++ ) {
        linkFile.setFileName(fileNameList[i]);
        if (linkFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!linkFile.atEnd()) {
                sourceURL = linkFile.readLine().trimmed();
                if (!sourceURL.isEmpty()) {
                    const QUrl url(QUrl::fromEncoded(sourceURL));
                    if (url.isValid())
                        list.append(url);
                }
            }
            linkFile.close();
        }
    }

    if (!list.isEmpty()) {
        m_MediaObject.clearQueue();
        setLocation(list[0].toString());
        for (int i = 1; i < list.count(); i++)
            m_MediaObject.enqueue(Phonon::MediaSource(list[i]));
        m_MediaObject.play();
    }

    forwardButton->setEnabled(!m_MediaObject.queue().isEmpty());
    settings.setValue("openRamFile", fileNameList[0]);
}

void MediaPlayer::finished()
{
}

void MediaPlayer::showContextMenu(const QPoint &p)
{
    fileMenu->popup(m_videoWidget->isFullScreen() ? p : mapToGlobal(p));
}

void MediaPlayer::scaleChanged(QAction *act)
{
    if (act->text() == tr("Scale and crop"))
        m_videoWidget->setScaleMode(Phonon::VideoWidget::ScaleAndCrop);
    else 
        m_videoWidget->setScaleMode(Phonon::VideoWidget::FitInView);    
}

void MediaPlayer::aspectChanged(QAction *act)
{
    if (act->text() == tr("16/9"))
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio16_9);
    else if (act->text() == tr("Scale"))
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);
    else if (act->text() == tr("4/3"))
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio4_3);
    else
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatioAuto);    
}

void MediaPlayer::hasVideoChanged(bool bHasVideo)
{
    info->setVisible(!bHasVideo);
    m_videoWindow.setVisible(bHasVideo);
    m_fullScreenAction->setEnabled(bHasVideo);
}

/*process video*/

void MediaPlayer::processEvent()
{
        pornResultLabel->setText(tr("Unknown"));
        horrResultLabel->setText(tr("Unknown"));
        violResultLabel->setText(tr("Unknown"));
        if(pornCheckBox->isChecked()){
            processPorn();
        }else{
            pornResultLabel->setText(tr("Unknown"));
        }

        if(horrCheckBox->isChecked()){
           processHorr();
        }else{
            horrResultLabel->setText(tr("Unknown"));
        }

        if(violCheckBox->isChecked()){
            processViol();
        }else{
            violResultLabel->setText(tr("Unknown"));
        }
}

void MediaPlayer::processPorn()
{
    pornThread.initPorn(prcssfileName);
    if(!pornThread.isRunning()){
        pornThread.start();
        connect(&pornThread, SIGNAL(passPornValue(const QString&)), this, SLOT(acceptPorn(const QString&)));
    }
    pornThread.quit();
}

void MediaPlayer::acceptPorn(const QString &pornResult){
    int intResult = atoi(pornResult.toStdString().c_str());
    if(0 == intResult){
        pornResultLabel->setText(tr("<font color=blue><h4>Not Porn.! </h4></font>"));
    }
    else if(1 == intResult){
        pornResultLabel->setText(tr("<font color=red><h4>Porn. Video! </h4></font>"));
    }else{
        pornResultLabel->setText(tr("<font color=green><h4>Error Video! </h4></font>"));
    }
}


void MediaPlayer::processHorr()
{
    horrThread.initHorr(prcssfileName);
    if(!horrThread.isRunning()){
        horrThread.start();
        connect(&horrThread, SIGNAL(passHorrValue(const QString&)), this, SLOT(acceptHorr(const QString&)));
    }
    horrThread.quit();
}

void MediaPlayer::processViol()
{
     violResultLabel->setText(tr("<font color=blue><h4>Prcss Viol</h4></font>"));
}

void MediaPlayer::acceptHorr(const QString &horrResult){
    int intResult = atoi(horrResult.toStdString().c_str());
    if(0 == intResult){
        horrResultLabel->setText(tr("<font color=blue><h4>Not Horr.! </h4></font>"));
    }
    else if(1 == intResult){
        horrResultLabel->setText(tr("<font color=red><h4>Horr. Video! </h4></font>"));
    }else{
        horrResultLabel->setText(tr("<font color=red><h4>Error Video! </h4></font>"));
    }
}

void MediaPlayer::loadVideoInformation(){
    std::string filename = prcssfileName.toStdString();
    std::cout << filename << std::endl;
    cv::VideoCapture vc;
    vc.open(filename);
    if(!vc.isOpened())
        std::cout << "Error!" << std::endl;
    int frames = vc.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout << "frames = " << frames << std::endl;
    frameNumLabel->setText("<font color=red>"+QString::number(frames)+"</font>");
}
