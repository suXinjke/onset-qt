#include "onset.h"
#include "ui_onset.h"

Onset::Onset( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::Onset ),
    sampleBlockWindow( 0 ),
    lastAutoAudioPlotAccuracyIndex( 1 ),
    audioDuration( 0.0 ) {

    ui->setupUi( this );

    audio = new Audio( this );
    sampleBlockWindow = new SampleBlockWindow();
    sampleBlockWindow->setAudio( audio );

    seekTimer = new QTimer( this );
    seekTimer->setInterval( 50 );
    connect( seekTimer, SIGNAL( timeout() ), this, SLOT( updateSeekInfo() ) );

    connect( ui->playButton, SIGNAL( clicked() ), this, SLOT( play() ) );
    connect( ui->pauseButton, SIGNAL( clicked() ), this, SLOT( pause() ) );
    connect( ui->stopButton, SIGNAL( clicked() ), this, SLOT( stop() ) );
    connect( ui->audioSeekSlider, SIGNAL( sliderMoved( int ) ), this, SLOT( seek( int ) ) );
    connect( ui->loadAudioFileAction, SIGNAL( triggered() ), this, SLOT( loadAudioFile() ) );
    connect( ui->bringSampleBlockDialogAction, SIGNAL( triggered() ), this, SLOT( bringSampleBlockWidget() ) );

    connect( ui->audioPlotAccuracyComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateAudioPlotSampleAccuracy( int ) ) );
    connect( ui->autoReloadCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( updateAudioPlotSampleAccuracy( bool ) ) );
    connect( ui->reloadAudioPlotButton, SIGNAL( clicked() ), this, SLOT( reloadAudioPlot() ) );
    connect( ui->resetZoomButton, SIGNAL( clicked() ), ui->audioPlot, SLOT( resetRange() ) );

    connect( ui->generatedAudioFrequencySpinbox, SIGNAL( valueChanged( QString ) ), this, SLOT( regenerateAudio() ) );
    connect( ui->generatedAudioSamplingRateSpinbox, SIGNAL( valueChanged( QString ) ), this, SLOT( regenerateAudio() ) );
    connect( ui->generatedAudioLengthSpinbox, SIGNAL( valueChanged( QString ) ), this, SLOT( regenerateAudio() ) );
    connect( ui->generateAudioButton, SIGNAL( clicked() ), this, SLOT( generateAudio() ) );

    connect( ui->audioPlot, SIGNAL( positionChanged( double ) ), this, SLOT( seek( double ) ) );
//    this->loadAudioFile( "test-sample.mp3" );

//    QVector<float> pcm;
////    pcm << 32.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 <<  0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 <<  0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 <<  0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 ;
//    //pcm << 0.0 << 1.0 << 2.0 << 3.0 << 4.0 << 5.0 << 6.0 << 7.0;
////    pcm << 1.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
//    pcm << 1.0 << 1.0 << 1.0 << 1.0 << 0.0 << 0.0 << 0.0 << 0.0;
//    audio->loadRawAudio( pcm );
//    ui->audioPlot->setSampleFrequency( audio->getAudioFrequency() );
//    ui->audioPlot->setSampleChannels( audio->getAudioChannels() );
//    this->reloadAudioPlot();
//    ui->audioPlot->resetRange();


    sampleBlockWindow->initPlot();
}

Onset::~Onset() {
    delete ui;
}

void Onset::loadAudioFile() {
    QString audioFilePath = QFileDialog::getOpenFileName( this, "Загрузить аудио", QString(), tr( "Аудио файлы (*.mp3 *.wav )" ) );
    this->loadAudioFile( audioFilePath );
}

void Onset::generateAudio() {
    double frequency = ui->generatedAudioFrequencySpinbox->value();
    double samplingRate = ui->generatedAudioSamplingRateSpinbox->value();
    double length = ui->generatedAudioLengthSpinbox->value();

    this->stop();
    if ( !audio->loadGeneratedAudio( frequency, samplingRate, length ) ) {
        return;
    }

    ui->audioPlot->setSampleFrequency( audio->getAudioFrequency() );
    ui->audioPlot->setSampleChannels( audio->getAudioChannels() );
    this->reloadAudioPlot();
    ui->audioPlot->resetRange();

    audioDuration = audio->getAudioDuration();

    if ( audioDuration < 0.0 ) {
        return;
    }

    ui->audioSeekSlider->setMaximum( audioDuration );
    this->updateSeekInfo();
    this->updateAudioTitleLabel();

    sampleBlockWindow->initPlot();

    this->play();
}

//only for auto generate
void Onset::regenerateAudio() {
    if ( !ui->autoGenerateAudioCheckBox->isChecked() ) {
        return;
    }

    this->generateAudio();
}

void Onset::loadAudioFile( const QString &audioFilePath ) {
    if ( audioFilePath.trimmed().isEmpty() ) {
        return;
    }

    this->stop();
    if ( !audio->loadAudio( audioFilePath ) ) {
        return;
    }

    ui->audioPlot->setSampleFrequency( audio->getAudioFrequency() );
    ui->audioPlot->setSampleChannels( audio->getAudioChannels() );
    this->reloadAudioPlot();
    ui->audioPlot->resetRange();

    audioDuration = audio->getAudioDuration();
    if ( audioDuration < 0.0 ) {
        return;
    }

    ui->audioSeekSlider->setMaximum( audioDuration );
    this->updateSeekInfo();
    this->updateAudioTitleLabel();

    sampleBlockWindow->initPlot();

    this->play();
}

void Onset::bringSampleBlockWidget() {
    sampleBlockWindow->show();
    sampleBlockWindow->raise();
//    sampleBlockWindow->activateWindow();
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

void Onset::reloadAudioPlot() {
    this->updateAudioPlotSampleAccuracy( ui->audioPlotAccuracyComboBox->currentIndex() );
//    ui->audioPlot->loadPCMData( audio->getPCMData() );
    ui->audioPlot->loadPCMData( audio->getSpectralFlux() );
}

void Onset::updateAudioPlotSampleAccuracy( int accuracyIndex ) {
    int frequency = audio->getAudioFrequency();
    if ( frequency < 0 ) {
        return;
    }

    int sampleAccuracy = 0;

    switch ( accuracyIndex ) {
        case 0:
            sampleAccuracy = frequency / 10;
            break;

        case 1:
            sampleAccuracy = frequency / 100;
            break;

        case 2:
            sampleAccuracy = frequency / 1000;
            break;

        case 3:
            sampleAccuracy = frequency / 5000;
            break;

        default:
            return;
    }

    ui->audioPlot->setSampleAccuracy( sampleAccuracy );
    if ( ui->autoReloadCheckBox->isChecked() ) {
//        ui->audioPlot->loadPCMData( audio->getPCMData() );
        ui->audioPlot->loadPCMData( audio->getSpectralFlux() );
        lastAutoAudioPlotAccuracyIndex = accuracyIndex;
    }
}

void Onset::updateAudioPlotSampleAccuracy( bool autoChecked ) {
    if ( !autoChecked ) {
        return;
    }

    int accuracyIndex = ui->audioPlotAccuracyComboBox->currentIndex();
    if ( accuracyIndex != lastAutoAudioPlotAccuracyIndex ) {
        this->reloadAudioPlot();
    }
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

void Onset::updateAudioTitleLabel() {
    ui->audioTitleLabel->setText( audio->getAudioTitle() );
}
