#include "onset.h"
#include "ui_onset.h"

Onset::Onset( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::Onset ),
    lastAutoAudioPlotAccuracyIndex( 1 ),
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

    connect( ui->audioPlot, SIGNAL( positionChanged( double ) ), this, SLOT( seek( double ) ) );

    connect( ui->onsetViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showOnset() ) );
    connect( ui->sampleBlockViewModeRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( showCurrentSampleBlock() ) );

    connect( ui->sampleBlockSizeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( showCurrentSampleBlock() ) );
    connect( ui->sampleBlockIndexSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( showCurrentSampleBlock() ) );

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

    ui->sampleBlockIndexSpinBox->setValue( 0 );

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

    ui->currentSampleBlockLabel->setText( QString( "%1 / %2" ).arg( blockIndex ).arg( blockCount - 1 ) );
    ui->sampleBlockIndexSpinBox->setMaximum( blockCount - 1 );
}

void Onset::showByRadioButton() {
    if ( ui->onsetViewModeRadioButton->isChecked() ) {
        this->showOnset();
    } else if ( ui->sampleBlockViewModeRadioButton->isChecked() ) {
        this->showCurrentSampleBlock();
    }
}

void Onset::showOnset() {
    if ( !ui->onsetViewModeRadioButton->isChecked() ) {
        return;
    }
    ui->audioPlot->loadOnset();
}

void Onset::showCurrentSampleBlock() {
    if ( !ui->sampleBlockViewModeRadioButton->isChecked() ) {
        return;
    }

    int blockSize = ui->sampleBlockSizeComboBox->currentText().toInt();
    int blockIndex = ui->sampleBlockIndexSpinBox->value();

    this->updateShowControls();
    ui->audioPlot->loadPCMBlock( blockIndex, 1, blockSize );
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
