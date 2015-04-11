#include "audioplot.h"

AudioPlot::AudioPlot( QWidget *parent ) :
    QCustomPlot( parent ),
    audio( 0 ) {

    pcmGraph = this->addGraph();
    this->xAxis->setRange( 0, 1 );
//    this->xAxis->setAutoTicks( false );
//    this->xAxis->setAutoTickLabels( false );
    this->xAxis->setSubTickCount( 0 );
//    connect( this->xAxis, SIGNAL( ticksRequest() ), this, SLOT( calculateTicks() ) );
    this->yAxis->setRange( -2, 2 );
//    this->yAxis->setAutoTickStep( false );
//    this->yAxis->setTickStep( 0.5 );
//    this->yAxis->setSubTickCount( 0 );
//    this->yAxis->setAutoSubTicks( false );
    this->setInteraction( QCP::iRangeDrag, true );
    this->setInteraction( QCP::iRangeZoom, true );
    this->axisRect()->setRangeDrag( Qt::Horizontal );
    this->axisRect()->setRangeZoom( Qt::Horizontal );

    position = new QCPItemStraightLine( this );
    position->setPen( QPen( Qt::red ) );
    this->addItem( position );
    this->setPositionInSeconds( 5.0 );

    connect( this, SIGNAL( mousePress( QMouseEvent * ) ), this, SLOT( onRightClick( QMouseEvent * ) ) );
}

void AudioPlot::setAudio( Audio *audio ) {
    this->audio = audio;
}

void AudioPlot::loadPCMData( int step ) {
    QVector<float> pcm = audio->getPCMData();
    double frequency = audio->getAudioFrequency();
    int channels = audio->getAudioChannels();

    int N = pcm.size();

    x.resize( qCeil( N / ( double ) step ) );
    y.resize( qCeil( N / ( double ) step ) );
    for ( int i = 0, p = 0 ; i < N ; i += step ) {
        x[p] = i / frequency / channels;
        y[p] = pcm.at( i );
        p++;
    }

    pcmGraph->setData( x, y );

    this->replot();
}

void AudioPlot::loadPCMBlock( int index, int step, int blockSize ) {
    QVector<float> pcm = audio->getPCMDataBlock( index, blockSize );
    double frequency = audio->getAudioFrequency();
    int channels = audio->getAudioChannels();

    int N = pcm.size();

    x.resize( qCeil( N / ( double ) step ) );
    y.resize( qCeil( N / ( double ) step ) );
    for ( int i = 0, p = 0 ; i < N ; i += step ) {
        x[p] = i / frequency / channels;
        y[p] = pcm.at( i );
        p++;
    }

    pcmGraph->setData( x, y );

    this->replot();
}

void AudioPlot::setPositionInSeconds( double seconds ) {
    double x = seconds;
    position->point1->setCoords( x, 0 );
    position->point2->setCoords( x, 1 );

    this->replot();
}

void AudioPlot::resetRange() {
    this->xAxis->setRange( 0, x.last() );

    if ( y.length() <= 0 ) {
        return;
    }

    int max = y.at( 0 );
    int min = y.at( 0 );
    for ( int i = 1 ; i < y.length() ; i++ ) {
        if ( y.at( i ) > max ) {
            max = y.at( i );
        }
        if ( y.at( i ) < min ) {
            min = y.at( i );
        }
    }

    this->yAxis->setRange( min - 1, max + 1 );

    this->replot();
}

//void AudioPlot::calculateTicks() {
////    double upper = this->xAxis->range().upper;
////    double lower = this->xAxis->range().lower;
////    double range = upper - lower;
//    double increment = 0.0;
//    if ( x.size() > 0 ) {
////        increment = qMin( x.last(), range )  * 0.2;
//        increment = x.last() * 0.1;
//    } else {
//        return;
//    }

//    QVector<double> ticks;
//    QVector<QString> tickLabels;
////    for ( double i = 0.0 ; i < qMin( x.last(), range ) ; i += increment ) {
//    for ( double i = 0.0 ; i < x.last() ; i += increment ) {
//        ticks << i;

//        QString tickLabel;
//        switch ( xAxisInformation ) {
//            case X_AXIS_INFORMATION_SAMPLE_INDEX:
//                tickLabel = QString::number( ( int ) i );
//                break;

//            case X_AXIS_INFORMATION_SECONDS:
//                tickLabel = QString::number( i / sampleFrequency / sampleChannels );
//                break;

//            case X_AXIS_INFORMATION_MINUTES_SECONDS:
//                //IMPLEMENT THIS!
//                tickLabel = QString::number( i / sampleFrequency / sampleChannels );
//                break;
//        }

//        tickLabels << tickLabel;
//    }

//    this->xAxis->setTickVector( ticks );
//    this->xAxis->setTickVectorLabels( tickLabels );
//    this->replot();
//}

void AudioPlot::onRightClick( QMouseEvent *mouseEvent ) {
    if ( mouseEvent->button() != Qt::RightButton ) {
        return;
    }

    double positionSeconds = this->xAxis->pixelToCoord( mouseEvent->x() );
    emit positionChanged( positionSeconds );
}
