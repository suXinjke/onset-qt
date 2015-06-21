#include "onset.h"
#include "ui_onset.h"

Onset::Onset( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::Onset ),
    audioDuration( 0.0 ) {

    ui->setupUi( this );

    audio = new Audio( this );
    ui->audioPlot->setAudio( audio );

    seekTimer = new QTimer( this );
    seekTimer->setInterval( 30 );
    connect( seekTimer, SIGNAL( timeout() ), this, SLOT( updateSeekInfo() ) );

    connect( ui->playButton, SIGNAL( clicked() ), this, SLOT( play() ) );
    connect( ui->pauseButton, SIGNAL( clicked() ), this, SLOT( pause() ) );
    connect( ui->stopButton, SIGNAL( clicked() ), this, SLOT( stop() ) );
    connect( ui->audioSeekSlider, SIGNAL( sliderMoved( int ) ), this, SLOT( seek( int ) ) );

    connect( ui->loadAudioFileAction, SIGNAL( triggered() ), this, SLOT( loadAudioFile() ) );
    connect( ui->resetRangeAction, SIGNAL( triggered() ), ui->audioPlot, SLOT( resetRange() ) );
    connect( ui->resetRangeXAction, SIGNAL( triggered() ), ui->audioPlot, SLOT( resetRangeX() ) );
    connect( ui->resetRangeYAction, SIGNAL( triggered() ), ui->audioPlot, SLOT( resetRangeY() ) );
    connect( ui->produceAudioInfoFileAction, SIGNAL( triggered() ), this, SLOT( updateAudioInfo() ) );

    connect( ui->audioPlot, SIGNAL( positionChanged( double ) ), this, SLOT( seek( double ) ) );

    connect( ui->onsetViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showOnset() ) );
    connect( ui->stressViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showStress() ) );
    connect( ui->stressFormattedViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showStressFormatted() ) );

    connect( ui->onsetThresholdWindowSizeSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updateAudioInfo() ) );
    connect( ui->onsetMultiplierSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateAudioInfo() ) );
    connect( ui->onsetWindowCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( updateAudioInfo() ) );

    connect( ui->waveformStepSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updateAudioInfo() ) );
    connect( ui->stressWindowSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updateAudioInfo() ) );
}

Onset::~Onset() {
    delete ui;
}

void Onset::loadAudioFile() {
    audioFilePath = QFileDialog::getOpenFileName( this, "Загрузить аудио", QString(), tr( "Аудио файлы (*.mp3 *.wav )" ) );
    this->loadAudioFile( audioFilePath );
}

void Onset::loadAudioFile( const QString &audioFilePath ) {
    if ( audioFilePath.trimmed().isEmpty() ) {
        return;
    }

    this->stop();
    if ( !audio->loadAudio( audioFilePath ) ) {
        return;
    }

    audioDuration = audio->getAudioDuration();
    if ( audioDuration < 0.0 ) {
        return;
    }

    ui->audioSeekSlider->setMaximum( audioDuration );
    this->updateSeekInfo();

    this->showAudioInfo();
    this->play();
}

void Onset::play() {
    if ( !audio->playAudio() ) {
        return;
    }
    seekTimer->start();
}

void Onset::pause() {
    seekTimer->stop();
    audio->pauseAudio();
}

void Onset::stop() {
    seekTimer->stop();
    audio->stopAudio();
    this->updateSeekInfo();
}

void Onset::seek( int seconds ) {
    this->seek( ( double ) seconds );
}

void Onset::seek( double seconds ) {
    audio->seekAudio( seconds );
    this->updateSeekInfo();
}

void Onset::updateSeekInfo() {
    double audioPosition = audio->getAudioCurrentPositionSeconds();
    if ( audioPosition < 0.0 ) {
        return;
    }

    this->updateSeekSlider( audioPosition );
    this->updateSeekLabel( audioPosition );
    ui->audioPlot->setPositionInSeconds( audioPosition );
}

void Onset::showAudioInfo() {
    QFile file( audioFilePath );
    file.open( QIODevice::ReadOnly );
    QString sha1 = QCryptographicHash::hash( file.readAll(), QCryptographicHash::Sha1 ).toHex();
    file.close();

    QString audioInfoFilePath = QString( "D:\\audios\\%1.txt" ).arg( sha1 );
    if ( !QFile::exists( audioInfoFilePath ) ) {
        int pcmStep = ui->waveformStepSpinBox->value();
        int window = ui->stressWindowSpinBox->value();
        audio->produceAudioInfoFile( pcmStep, window );
    }

    ui->audioPlot->loadAudioInfoFile( audioInfoFilePath );
}

void Onset::showOnset() {
    ui->audioPlot->setViewMode( AudioPlot::VIEW_MODE_ONSET );
}

void Onset::showStress() {
    ui->audioPlot->setViewMode( AudioPlot::VIEW_MODE_STRESS );
}

void Onset::showStressFormatted() {
    ui->audioPlot->setViewMode( AudioPlot::VIEW_MODE_STRESS_FORMATTED );
}

void Onset::updateAudioInfo() {
    int thresholdWindowSize = ui->onsetThresholdWindowSizeSpinBox->value();
    double onsetMultiplier = ui->onsetMultiplierSpinBox->value();
    bool onsetWindow = ui->onsetWindowCheckbox->isChecked();
    audio->setOnsetOptions( thresholdWindowSize, onsetMultiplier, onsetWindow );

    int pcmStep = ui->waveformStepSpinBox->value();
    int window = ui->stressWindowSpinBox->value();
    audio->produceAudioInfoFile( pcmStep, window );
    this->showAudioInfo();
}

void Onset::updateSeekSlider( double audioPosition ) {
    ui->audioSeekSlider->setValue( audioPosition );
}

void Onset::updateSeekLabel( double audioPosition ) {
    QTime time( 0, 0, 0, 0 );
    QTime durationTime = time.addSecs( audioDuration );

    QTime positionTime = time.addSecs( audioPosition );

    QString seekLabelText = QString( "%1 / %2" )
                            .arg( positionTime.toString( "mm:ss" ) )
                            .arg( durationTime.toString( "mm:ss" ) );

    ui->audioSeekLabel->setText( seekLabelText );
}
