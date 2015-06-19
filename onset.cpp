#include "onset.h"
#include "ui_onset.h"

Onset::Onset( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::Onset ),
    lastOnsetProcessingSteps( 5 ),
    lastOnsetThresholdWindowSize( 20 ),
    lastOnsetMultiplier( 1.5f ),
    lastOnsetLowFreqFilter( 0 ),
    lastOnsetHighFreqFilter( 0 ),
    lastOnsetWindow( true ),
    lastOnsetViewMode( 0 ),
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

    connect( ui->audioPlot, SIGNAL( positionChanged( double ) ), this, SLOT( seek( double ) ) );

    connect( ui->onsetViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showByComboBox() ) );
//    connect( ui->waveformViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showWaveform() ) );
    connect( ui->stressViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showStress() ) );
    connect( ui->sampleBlockViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showCurrentSampleBlock() ) );
    connect( ui->fftViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showFFT() ) );
    connect( ui->fftPhaseViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showFFTPhase() ) );
    connect( ui->fftRawRealViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showFFTRawReal() ) );
    connect( ui->fftRawImaginaryViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showFFTRawImaginary() ) );

    connect( ui->onsetProcessingStepComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( showByComboBox() ) );

    connect( ui->onsetThresholdWindowSizeSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );
    connect( ui->onsetMultiplierSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( showByRadioButton() ) );
//    connect( ui->onsetLowFreqFilterSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updateOnsetFilter() ) );
//    connect( ui->onsetHighFreqFilterSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updateOnsetFilter() ) );
    connect( ui->onsetWindowCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( showByRadioButton() ) );

    connect( ui->waveformStepSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );
    connect( ui->stressWindowSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );

    connect( ui->sampleBlockSizeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( showByRadioButton() ) );
    connect( ui->sampleBlockIndexSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showByRadioButton() ) );

//    QVector<float> pcm;
//    pcm << 1.0 << 1.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;

//    ui->audioPlot->loadPCMData( Transform::FFT( pcm ) );
}

Onset::~Onset() {
    delete ui;
}

void Onset::loadAudioFile() {
    QString audioFilePath = QFileDialog::getOpenFileName( this, "Загрузить аудио", QString(), tr( "Аудио файлы (*.mp3 *.wav )" ) );
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

    int halfFrequency = audio->getAudioFrequency() / 2;

    ui->sampleBlockIndexSpinBox->setValue( 0 );
//    ui->onsetProcessingStepsSpinBox->setValue( 5 );
    ui->onsetThresholdWindowSizeSpinBox->setValue( 20 );
    ui->onsetMultiplierSpinBox->setValue( 1.5 );
//    ui->onsetLowFreqFilterSpinBox->setMaximum( halfFrequency );
//    ui->onsetLowFreqFilterSpinBox->setValue( 0 );
//    ui->onsetHighFreqFilterSpinBox->setMaximum( halfFrequency );
//    ui->onsetHighFreqFilterSpinBox->setValue( 0 );
    ui->onsetWindowCheckbox->setChecked( true );
    audio->setOnsetOptions( 20, 1.5, true );
    audio->setOnsetFilter( 0, 0 );

    this->updateShowControls();
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

void Onset::updateShowControls() {
    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();
    int blockCount = audio->getSampleBlockCount( blockSize );

    QString blockDuration = audio->getSampleBlockDuration( blockIndex, blockSize );

    ui->currentSampleBlockLabel->setText( QString( "%1 / %2" ).arg( blockIndex ).arg( blockCount - 1 ) );
    ui->currentSampleBlockDurationLabel->setText( blockDuration );
    ui->sampleBlockIndexSpinBox->setMaximum( blockCount - 1 );
}

void Onset::updateOnsetFilter() {
//    int onsetLowFreqFilter = ui->onsetLowFreqFilterSpinBox->value();
//    int onsetHighFreqFilter = ui->onsetHighFreqFilterSpinBox->value();

//    if ( audio->setOnsetFilter( onsetLowFreqFilter, onsetHighFreqFilter ) ) {
//        this->showByRadioButton();
//    }
}

void Onset::showByRadioButton() {
    if ( ui->onsetViewModeRadioButton->isChecked() ) {
        this->showByComboBox();
    }
//    else if ( ui->waveformViewModeRadioButton->isChecked() ) {
//        this->showWaveform();
//    }
    else if ( ui->stressViewModeRadioButton->isChecked() ) {
        this->showStress();
    } else if ( ui->sampleBlockViewModeRadioButton->isChecked() ) {
        this->showCurrentSampleBlock();
    } else if ( ui->fftViewModeRadioButton->isChecked() ) {
        this->showFFT();
    } else if ( ui->fftPhaseViewModeRadioButton->isChecked() ) {
        this->showFFTPhase();
    } else if ( ui->fftRawRealViewModeRadioButton->isChecked() ) {
        this->showFFTRawReal();
    } else if ( ui->fftRawImaginaryViewModeRadioButton->isChecked() ) {
        this->showFFTRawImaginary();
    }
}

void Onset::showByComboBox() {
    int index = ui->onsetProcessingStepComboBox->currentIndex();
    switch ( index ) {
        case 0:
            this->showWaveform();
            break;
        case 1:
            this->showWaveform( true ); //windowed
            break;
        case 2:
            this->showOnset( 1 );
            break;
        case 3:
            this->showOnset( 2 );
            break;
        case 4:
            this->showOnset( 3 );
            break;
        case 5:
            this->showOnset( 4 );
            break;

        default:
            break;
    }
}

void Onset::showOnset( int processingSteps ) {
    if ( !ui->onsetViewModeRadioButton->isChecked() ) {
        return;
    }

//    int processingSteps = ui->onsetProcessingStepsSpinBox->value();
    int thresholdWindowSize = ui->onsetThresholdWindowSizeSpinBox->value();
//    int onsetLowFreqFilter = ui->onsetLowFreqFilterSpinBox->value();
//    int onsetHighFreqFilter = ui->onsetHighFreqFilterSpinBox->value();
    float onsetMultiplier = ui->onsetMultiplierSpinBox->value();

    bool onsetWindow = ui->onsetWindowCheckbox->isChecked();
    if ( thresholdWindowSize != lastOnsetThresholdWindowSize || lastOnsetMultiplier != onsetMultiplier
//         || processingSteps != lastOnsetProcessingSteps
            || onsetWindow != lastOnsetWindow
//         || onsetLowFreqFilter != lastOnsetLowFreqFilter || onsetHighFreqFilter != lastOnsetHighFreqFilter
       ) {
//        lastOnsetProcessingSteps = processingSteps;
        lastOnsetThresholdWindowSize = thresholdWindowSize;
        lastOnsetMultiplier = onsetMultiplier;
//        lastOnsetLowFreqFilter = onsetLowFreqFilter;
//        lastOnsetHighFreqFilter = onsetHighFreqFilter;
        lastOnsetWindow = onsetWindow;

        audio->setOnsetOptions( thresholdWindowSize, onsetMultiplier, onsetWindow );
    }

    ui->audioPlot->loadOnset( processingSteps );
}

void Onset::showWaveform( bool applyWindow ) {
    if ( !ui->onsetViewModeRadioButton->isChecked() ) {
        return;
    }

    int step = ui->waveformStepSpinBox->value();
    ui->audioPlot->loadWaveform( step, applyWindow );
}

void Onset::showStress() {
    if ( !ui->stressViewModeRadioButton->isChecked() ) {
        return;
    }

    int step = ui->waveformStepSpinBox->value();
    int stressWindow = ui->stressWindowSpinBox->value();
    ui->audioPlot->loadStress( stressWindow, step );
}

void Onset::showCurrentSampleBlock() {
    if ( !ui->sampleBlockViewModeRadioButton->isChecked() ) {
        return;
    }

    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();

    this->updateShowControls();
    ui->audioPlot->loadPCMBlock( blockIndex, blockSize );

}

void Onset::showFFT() {
    if ( !ui->fftViewModeRadioButton->isChecked() ) {
        return;
    }

    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();

    this->updateShowControls();
    ui->audioPlot->loadFFTBlock( blockIndex, blockSize );
}

void Onset::showFFTPhase() {
    if ( !ui->fftPhaseViewModeRadioButton->isChecked() ) {
        return;
    }

    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();

    this->updateShowControls();
    ui->audioPlot->loadFFTPhaseBlock( blockIndex, blockSize );
}

void Onset::showFFTRawReal() {
    if ( !ui->fftRawRealViewModeRadioButton->isChecked() ) {
        return;
    }

    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();

    this->updateShowControls();
    ui->audioPlot->loadFFTBlockRaw( blockIndex, blockSize, false );
}

void Onset::showFFTRawImaginary() {
    if ( !ui->fftRawImaginaryViewModeRadioButton->isChecked() ) {
        return;
    }

    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();

    this->updateShowControls();
    ui->audioPlot->loadFFTBlockRaw( blockIndex, blockSize, true );
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
