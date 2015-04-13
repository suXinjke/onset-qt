#include "audioplot.h"

AudioPlot::AudioPlot( QWidget *parent ) :
    QCustomPlot( parent ),
    audio( 0 ),
    seconds( 0.0 ) {

    pcmGraph = this->addGraph();
    this->xAxis->setRange( 0, 1 );
    this->yAxis->setRange( 0, 1 );
    this->setInteraction( QCP::iRangeDrag, true );
    this->setInteraction( QCP::iRangeZoom, true );
    this->axisRect()->setRangeDrag( Qt::Horizontal );
    this->axisRect()->setRangeZoom( Qt::Horizontal );

    this->setPositionInSeconds( 0.0 );

    pcmGraph->setScatterStyle( QCPScatterStyle::ssDisc );
    pcmGraph->setLineStyle( QCPGraph::lsImpulse );

    connect( this, SIGNAL( mousePress( QMouseEvent * ) ), this, SLOT( onRightClick( QMouseEvent * ) ) );
    connect( this, SIGNAL( mouseMove( QMouseEvent * ) ), this, SLOT( getCursorCoordinates( QMouseEvent * ) ) );
}

void AudioPlot::setAudio( Audio *audio ) {
    this->audio = audio;
}

void AudioPlot::loadPCMData( const QVector<float> &pcm ) {
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    x.resize( N );
    y.resize( N );
    for ( int i = 0; i < N ; i ++ ) {
        x[i] = i;
        y[i] = pcm.at( i );
    }

    pcmGraph->setData( x, y );

    this->resetRange();
}

void AudioPlot::loadPCMBlock( int index, int step, int blockSize ) {
    QVector<float> pcm = audio->getSampleBlock( index, blockSize );
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    x.resize( N );
    y.resize( N );
    for ( int i = 0, p = 0 ; i < N ; i += step ) {
        x[p] = i / 2.0; //2 because of float double memory usage
        y[p] = pcm.at( i );
        p++;
    }

    pcmGraph->setData( x, y );

    this->resetRange( false );
}

void AudioPlot::loadOnset() {
    QVector<float> pcm = audio->getOnset();
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    int frequency = audio->getAudioFrequency();

    x.resize( N );
    y.resize( N );
    for ( int i = 0, p = 0 ; i < N ; i ++ ) {
        x[p] = i * ( 1024.0 / frequency );
        y[p] = pcm.at( i );
        p++;
    }

    pcmGraph->setData( x, y );

    this->resetRange();
}

void AudioPlot::setPositionInSeconds( double seconds ) {
    this->seconds = seconds;
    this->repaint();
}

void AudioPlot::resetRange( bool detectMinMaxY ) {
    this->xAxis->setRange( 0, x.last() );

    if ( y.length() <= 0 ) {
        return;
    }

    float max;
    float min;

    if ( detectMinMaxY ) {
        max = y.at( 0 );
        min = y.at( 0 );
        for ( int i = 1 ; i < y.length() ; i++ ) {
            if ( y.at( i ) > max ) {
                max = y.at( i );
            }
            if ( y.at( i ) < min ) {
                min = y.at( i );
            }
        }
    } else {
        max = 1.0;
        min = -1.0;
    }

    this->yAxis->setRange( min - 0.1, max + 0.1 );
    this->replot();
}

void AudioPlot::paintEvent( QPaintEvent *event ) {
    QCustomPlot::paintEvent( event );

    QPainter painter( this );
    QRectF rect( this->width() - 70, 0, 70, 20 );
    painter.drawText( rect, Qt::AlignRight, cursorCoordinates );

    painter.setPen( Qt::red );
    double x = this->xAxis->coordToPixel( seconds );
    painter.drawLine( x, 0, x, this->height() );
}

void AudioPlot::onRightClick( QMouseEvent *mouseEvent ) {
    if ( mouseEvent->button() != Qt::RightButton ) {
        return;
    }

    double positionSeconds = this->xAxis->pixelToCoord( mouseEvent->x() );
    emit positionChanged( positionSeconds );
}

void AudioPlot::getCursorCoordinates( QMouseEvent *mouseEvent ) {
    if ( mouseEvent ) {
        QString x = QString::number( this->xAxis->pixelToCoord( mouseEvent->x() ), 'f', 2 );
        QString y = QString::number( this->yAxis->pixelToCoord( mouseEvent->y() ), 'f', 2 );
        cursorCoordinates = QString( "%1, %2" ).arg( x ).arg( y );

        this->repaint();
    }

}
