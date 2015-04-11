#include "sampleblockwidget.h"
#include "ui_sampleblockwidget.h"

SampleBlockWidget::SampleBlockWidget( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::SampleBlockWidget ),
    audio( 0 ),
    sampleBlockIndex( 0 ),
    sampleBlockCount( 0 ),
    sampleBlockSize( 512 ) {

    ui->setupUi( this );

    connect( ui->sampleBlockSizeSpinbox, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( setSampleBlockSize( QString ) ) );
    connect( ui->sampleBlockIndexSpinbox, SIGNAL( valueChanged( int ) ), this, SLOT( setSampleBlock( int ) ) );

    ui->sampleBlockPlot->setupPlot( SAMPLE_BLOCK );
    ui->realSampleBlockPlot->setupPlot( REAL_FREQ_DOMAIN );
    ui->imaginarySampleBlockPlot->setupPlot( IMAGINARY_FREQ_DOMAIN );
    ui->magnitudeSampleBlockPlot->setupPlot( MAGNITUDE_FREQ_DOMAIN );
    ui->phaseSampleBlockPlot->setupPlot( PHASE_FREQ_DOMAIN );
}

SampleBlockWidget::~SampleBlockWidget() {
    delete ui;
}

void SampleBlockWidget::setAudio( Audio *audio ) {
    this->audio = audio;
}

void SampleBlockWidget::initPlot() {
    if ( !audio ) {
        return;
    }

    sampleBlockSize = ui->sampleBlockSizeSpinbox->currentText().toInt();
    sampleBlockCount = qCeil( audio->getSampleCount() / ( double ) sampleBlockSize );

    this->setSampleBlock( 0 );
    ui->sampleBlockPlot->resetRange();

    ui->sampleBlockIndexSpinbox->setValue( sampleBlockIndex );
    ui->sampleBlockIndexSpinbox->setMaximum( sampleBlockCount - 1 );
}

void SampleBlockWidget::updateCurrentSampleBlockLabel() {
    ui->currentSampleBlockLabel->setText( QString( "%1 / %2" ).arg( sampleBlockIndex ).arg( sampleBlockCount - 1 ) );
}

void SampleBlockWidget::setSampleBlockSize( int sampleBlockSize ) {
    this->sampleBlockSize = sampleBlockSize;

    if ( audio ) {
        this->initPlot();
    }
}

void SampleBlockWidget::setSampleBlockSize( const QString &sampleBlockSizeString ) {
    int sampleBlockSize = sampleBlockSizeString.toInt();
    this->setSampleBlockSize( sampleBlockSize );
}

void SampleBlockWidget::setSampleBlock( int index ) {
    sampleBlockIndex = index;
    this->updateCurrentSampleBlockLabel();

    if ( !audio ) {
        return;
    }

    pcmBlock = audio->getPCMDataBlock( sampleBlockIndex, sampleBlockSize );

    ui->sampleBlockPlot->loadPCMData( pcmBlock );
    this->correlateDFT();
}



void SampleBlockWidget::correlateDFT() {
    int N = pcmBlock.length(); //must be a power of 2

    QVector<float> re( ( N / 2 ) + 1 );
    QVector<float> im( ( N / 2 ) + 1 );
    re.fill( 0 );
    im.fill( 0 );

    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
        for ( int i = 0 ; i < N ; i++ ) {
            re[k] = re[k] + pcmBlock[i] * qCos( 2 * M_PI * k * i / N );
            im[k] = im[k] - pcmBlock[i] * qSin( 2 * M_PI * k * i / N );
        }
    }

    ui->realSampleBlockPlot->loadPCMData( re );
    ui->realSampleBlockPlot->resetRange();
    ui->realSampleBlockPlot->resetVerticalRange();
    ui->imaginarySampleBlockPlot->loadPCMData( im );
    ui->imaginarySampleBlockPlot->resetRange();
    ui->imaginarySampleBlockPlot->resetVerticalRange();



    //polar
    QVector<float> mag( ( N / 2 ) + 1 );
    QVector<float> phase( ( N / 2 ) + 1 );

    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
        mag[k] = qSqrt( re[k] * re[k] + im[k] * im[k] );
        if ( re[k] == 0.0 ) {
            re[k] = 1e-20;
        }

        phase[k] = qAtan( im[k] / re[k] );
        if ( re[k] < 0 && im[k] < 0 ) {
            phase[k] -= M_PI;
        }
        if ( re[k] < 0 && im[k] >= 0 ) {
            phase[k] += M_PI;
        }
    }

    ui->magnitudeSampleBlockPlot->loadPCMData( mag );
    ui->magnitudeSampleBlockPlot->resetRange();
    ui->magnitudeSampleBlockPlot->resetVerticalRange();

    ui->phaseSampleBlockPlot->loadPCMData( phase );
    ui->phaseSampleBlockPlot->resetRange();
    ui->phaseSampleBlockPlot->resetVerticalRange();
}
