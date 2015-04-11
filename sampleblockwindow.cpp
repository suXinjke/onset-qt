#include "sampleblockwindow.h"
#include "ui_sampleblockwindow.h"

SampleBlockWindow::SampleBlockWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::SampleBlockWindow ),
    audio( 0 ),
    sampleBlockIndex( 0 ),
    sampleBlockCount( 0 ),
    sampleBlockSize( 512 ) {

    ui->setupUi( this );

    connect( ui->sampleBlockSizeSpinbox, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( setSampleBlockSize( QString ) ) );
    connect( ui->sampleBlockIndexSpinbox, SIGNAL( valueChanged( int ) ), this, SLOT( setSampleBlock( int ) ) );

    ui->sampleBlockPlot->setupPlot( SAMPLE_BLOCK );
//    ui->realSampleBlockPlot->setupPlot( REAL_FREQ_DOMAIN );
//    ui->imaginarySampleBlockPlot->setupPlot( IMAGINARY_FREQ_DOMAIN );
    ui->magnitudeSampleBlockPlot->setupPlot( MAGNITUDE_FREQ_DOMAIN );
//    ui->phaseSampleBlockPlot->setupPlot( PHASE_FREQ_DOMAIN );
}

SampleBlockWindow::~SampleBlockWindow() {
    delete ui;
}

void SampleBlockWindow::setAudio( Audio *audio ) {
    this->audio = audio;
}

void SampleBlockWindow::initPlot() {
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

void SampleBlockWindow::setSampleBlockSize( int sampleBlockSize ) {
    this->sampleBlockSize = sampleBlockSize;

    if ( audio ) {
        this->initPlot();
    }
}

void SampleBlockWindow::updateCurrentSampleBlockLabel() {
    ui->currentSampleBlockLabel->setText( QString( "%1 / %2" ).arg( sampleBlockIndex ).arg( sampleBlockCount - 1 ) );
}



void SampleBlockWindow::setSampleBlockSize( const QString &sampleBlockSizeString ) {
    int sampleBlockSize = sampleBlockSizeString.toInt();
    this->setSampleBlockSize( sampleBlockSize );
}

void SampleBlockWindow::setSampleBlock( int index ) {
    sampleBlockIndex = index;
    this->updateCurrentSampleBlockLabel();

    if ( !audio ) {
        return;
    }

    pcmBlock = audio->getPCMDataBlock( sampleBlockIndex, sampleBlockSize );

    ui->sampleBlockPlot->loadPCMData( pcmBlock );

    ui->magnitudeSampleBlockPlot->loadPCMData( Transform::correlateDFT( pcmBlock ) );
    ui->magnitudeSampleBlockPlot->resetRange();
    ui->magnitudeSampleBlockPlot->resetVerticalRange();
//    this->correlateDFT();
//    this->fastFFT();
}

//void SampleBlockWindow::correlateDFT() {
//    int N = pcmBlock.length(); //must be a power of 2

//    QVector<float> re( ( N / 2 ) + 1 );
//    QVector<float> im( ( N / 2 ) + 1 );
//    re.fill( 0.0 );
//    im.fill( 0.0 );

//    qDebug() << pcmBlock;

//    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
//        for ( int i = 0 ; i < N ; i++ ) {
//            re[k] += pcmBlock[i] * qCos( 2 * M_PI * k * i / N );
//            im[k] += pcmBlock[i] * -qSin( 2 * M_PI * k * i / N );
//        }
//    }

//    ui->realSampleBlockPlot->loadPCMData( re );
//    ui->realSampleBlockPlot->resetRange();
//    ui->realSampleBlockPlot->resetVerticalRange();
//    ui->imaginarySampleBlockPlot->loadPCMData( im );
//    ui->imaginarySampleBlockPlot->resetRange();
//    ui->imaginarySampleBlockPlot->resetVerticalRange();

//    qDebug() << re << im;


//    //polar
//    QVector<float> mag( ( N / 2 ) + 1 );
//    QVector<float> phase( ( N / 2 ) + 1 );

//    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
//        mag[k] = qSqrt( re[k] * re[k] + im[k] * im[k] );
//        if ( re[k] == 0.0 ) {
//            re[k] = 1e-20;
//        }

//        phase[k] = qAtan( im[k] / re[k] );
//        if ( re[k] < 0 && im[k] < 0 ) {
//            phase[k] -= M_PI;
//        }
//        if ( re[k] < 0 && im[k] >= 0 ) {
//            phase[k] += M_PI;
//        }
//    }

//    ui->magnitudeSampleBlockPlot->loadPCMData( mag );
//    ui->magnitudeSampleBlockPlot->resetRange();
//    ui->magnitudeSampleBlockPlot->resetVerticalRange();

//    ui->phaseSampleBlockPlot->loadPCMData( phase );
//    ui->phaseSampleBlockPlot->resetRange();
//    ui->phaseSampleBlockPlot->resetVerticalRange();
//}

//void SampleBlockWindow::fastFFT() {

//}

//void SampleBlockWindow::fastFFT() {
//    int N = pcmBlock.length();
//    int NM1 = N - 1;
//    int ND2 = N / 2;
//    int M = qLn( N ) / qLn( 2 );
//    int j = ND2;

//    QVector<float> re( pcmBlock );
//    QVector<float> im( N );
//    im.fill( 0 );

//    qDebug() << re << im;

//    for ( int i = 1 ; i < N - 2 ; i ++ ) {
//        if ( i < j ) {
//            float tr = re[j];
//            float ti = im[j];
//            re[j] = re[i];
//            im[j] = im[i];
//            re[i] = tr;
//            im[i] = ti;
//        }
//        int k = ND2;
//        while ( k <= j ) {
//            j -= k;
//            k /= 2;
//        }
//        j += k;
//    }

//    for ( int l = 1 ; l < M ; l++ ) {
//        int le = ( int ) qPow( 2, l );
//        int le2 = le / 2;
//        float ur = 1;
//        float ui = 0;
//        float sr = qCos( M_PI / le2 );
//        float si = -qSin( M_PI / le2 );
//        for ( int j = 1 ; j < le2 ; j++ ) {
//            int jm1 = j - 1;
//            for ( int i = jm1 ; i < NM1 ; i += le ) {
//                int ip = i + le2;
//                float tr = re[ip] * ur - im[ip]*ui;
//                float ti = re[ip] * ui + im[ip]*ur;
//                re[ip] = re[i] - tr;
//                im[ip] = im[i] - ti;
//                re[i] += tr;
//                im[i] += ti;
//            }
//            float tr = ur;
//            ur = tr * sr - ui * si;
//            ui = tr * si + ui * sr;
//        }
//    }

//    re.resize( re.length() / 2 + 1 );
//    im.resize( im.length() / 2 + 1 );

//    ui->realSampleBlockPlot->loadPCMData( re );
//    ui->realSampleBlockPlot->resetRange();
//    ui->realSampleBlockPlot->resetVerticalRange();
//    ui->imaginarySampleBlockPlot->loadPCMData( im );
//    ui->imaginarySampleBlockPlot->resetRange();
//    ui->imaginarySampleBlockPlot->resetVerticalRange();

//    qDebug() << re << im;
//}
