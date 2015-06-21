#include "onset.h"
#include "ui_onset.h"

Onset::Onset( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::Onset ),
    lastOnsetThresholdWindowSize( 20 ),
    lastOnsetMultiplier( 1.5f ),
    lastOnsetWindow( true ),
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
    connect( ui->produceAudioInfoFileAction, SIGNAL( triggered() ), this, SLOT( produceAudioInfoFile() ) );

    connect( ui->audioPlot, SIGNAL( positionChanged( double ) ), this, SLOT( seek( double ) ) );

    connect( ui->onsetViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showOnset() ) );
    connect( ui->stressViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showStress() ) );

    connect( ui->onsetThresholdWindowSizeSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );
    connect( ui->onsetMultiplierSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( showByRadioButton() ) );
    connect( ui->onsetWindowCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( showByRadioButton() ) );

    connect( ui->waveformStepSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );
    connect( ui->stressWindowSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );
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

    ui->onsetThresholdWindowSizeSpinBox->setValue( 20 );
    ui->onsetMultiplierSpinBox->setValue( 1.5 );
    ui->onsetWindowCheckbox->setChecked( true );
    audio->setOnsetOptions( 20, 1.5, true );
    audio->setOnsetFilter( 0, 0 );

    this->showByRadioButton();
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

void Onset::showByRadioButton() {
    if ( ui->onsetViewModeRadioButton->isChecked() ) {
        this->showOnset();
    } else if ( ui->stressViewModeRadioButton->isChecked() ) {
        this->showStress();
    }
}

void Onset::showOnset() {
    if ( !ui->onsetViewModeRadioButton->isChecked() ) {
        return;
    }

    int thresholdWindowSize = ui->onsetThresholdWindowSizeSpinBox->value();
    float onsetMultiplier = ui->onsetMultiplierSpinBox->value();

    bool onsetWindow = ui->onsetWindowCheckbox->isChecked();
    if ( thresholdWindowSize != lastOnsetThresholdWindowSize || lastOnsetMultiplier != onsetMultiplier
            || onsetWindow != lastOnsetWindow ) {
        lastOnsetThresholdWindowSize = thresholdWindowSize;
        lastOnsetMultiplier = onsetMultiplier;
        lastOnsetWindow = onsetWindow;

        audio->setOnsetOptions( thresholdWindowSize, onsetMultiplier, onsetWindow );
    }

    audio->fillPCM( ui->waveformStepSpinBox->value() );

    ui->audioPlot->loadOnset();
}

void Onset::showStress() {
    if ( !ui->stressViewModeRadioButton->isChecked() ) {
        return;
    }

    int step = ui->waveformStepSpinBox->value();
    int stressWindow = ui->stressWindowSpinBox->value();
    ui->audioPlot->loadStress( stressWindow, step );
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

void Onset::produceAudioInfoFile() {
    QFile file( audioFilePath );
    file.open( QIODevice::ReadOnly );
    QString sha1 = QCryptographicHash::hash( file.readAll(), QCryptographicHash::Sha1 ).toHex();
    file.close();

    QFile outFile( QString( "D:\\%1" ).arg( sha1 ) );
    if ( outFile.open( QIODevice::WriteOnly ) ) {
        QVector<float> peaks = audio->getPeaks();
        int N = peaks.length();
        if ( N <= 0 ) {
            return;
        }
        int frequency = audio->getAudioFrequency();
        QTextStream out( &outFile );

        for ( int i = 0 ; i < N ; i ++ ) {
            if ( peaks.at( i ) > 0.0 ) {
                double positionSeconds = i * ( 2048.0 / frequency );
                out << positionSeconds << ", " << audio->getLevelAtPosition( positionSeconds ) << endl;
            }
        }

        outFile.close();
    }



}
