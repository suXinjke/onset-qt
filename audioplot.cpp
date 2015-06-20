#include "audioplot.h"

AudioPlot::AudioPlot( QWidget *parent ) :
    QCustomPlot( parent ),
    audio( 0 ),
    seconds( 0.0 ),
    showPosition( true ),
    showAverageVolume( false ),
    meanAll( 0.0 ),
    tickTimer( 0 ) {

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

void AudioPlot::loadStress( int window, int step ) {
    audio->fillPCM( step );
    QVector<float> pcm = audio->getPCM();
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    int frequency = audio->getAudioFrequency();
    int channels = audio->getAudioChannels();

//    QVector<float> pcmShortened;
//    for ( int i = 0; i < N ; i += step ) {
//        pcmShortened.append( pcm.at( i ) );
//    }

//    N = pcmShortened.length();

    QElapsedTimer timer;
    timer.start();

    QVector<float> pcmFiltered;
    for ( int i = 0; i < N ; i ++ ) {
        int start = qMax( 0, i - window );
        int end = qMin( N - 1, i + window );
        double mean = 0;
        for ( int j = start ; j <= end ; j++ ) {
            mean += pcm.at( j ) * pcm.at( j );
        }
        mean /= ( end - start );
        mean = qSqrt( mean );
        pcmFiltered.append( mean );
    }




    N = pcmFiltered.length();

    x.resize( N );
    y.resize( N );
    for ( int i = 0, p = 0 ; i < N ; i ++ ) {
        x[p] = ( double ) ( p * step ) / frequency / channels;
        double value = pcmFiltered.at( i );
        y[p] = value;
        p++;
    }

    meanAll = 0.0;
    for ( int i = 0; i < pcm.length() ; i++ ) {
        meanAll += pcm.at( i ) * pcm.at( i );
    }
    meanAll /= ( pcm.length() );
    meanAll = qSqrt( meanAll );

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = true;
    showAverageVolume = false;

    plotInfo = "";
    this->replot();
}

void AudioPlot::loadOnset() {
    QVector<float> pcm = audio->getPeaks();

    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    int frequency = audio->getAudioFrequency();

    x.resize( N );
    y.resize( N );
    for ( int i = 0, p = 0 ; i < N ; i ++ ) {
        x[p] = i * ( 2048.0 / frequency );
        y[p] = pcm.at( i );

        p++;
    }

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = true;
    showAverageVolume = false;
    meanAll = 0.0;

    plotInfo = "";

    this->replot();
}

void AudioPlot::setPositionInSeconds( double seconds ) {
    this->seconds = seconds;
    double currentValue = this->getCurrentValue( seconds );
    double currentValueLower = this->getCurrentValue( seconds - 0.02 );
    double currentValueHigher = this->getCurrentValue( seconds + 0.02 );
    if ( currentValue > 0.01 || currentValueLower > 0.01 || currentValueHigher > 0.01 ) {
        tickTimer = 16;
    }

    if ( showAverageVolume ) {
        averageVolume = this->getAverageVolume( seconds );
    }

    this->update();
}

QString AudioPlot::getAverageVolume( double seconds ) {
    double avg = 0.0;
    int steps = 32; // 1 = 0.1 sec

    for ( int i = 0 ; i < steps / 2 ; i ++ ) {
        if ( i == 0 ) {
            avg += qAbs( this->getCurrentValue( seconds ) );
        } else {
            avg += qAbs( this->getCurrentValue( seconds + 0.1 * i ) );
            avg += qAbs( this->getCurrentValue( seconds - 0.1 * i ) );
        }
    }

    avg /= steps;

    return QString( "Average for 3 sec: %1" ).arg( QString::number( avg, 'f', 2 ) );
}

double AudioPlot::getCurrentValue( double seconds ) {
    return pcmGraph->data()->upperBound( seconds ).value().value;
}

void AudioPlot::runningAverage( int window ) {
    QVector<double> newY;

    for ( int i = 0; i < y.length() ; i++ ) {
        int start = qMax( 0, i - window );
        int end = qMin( y.length() - 1, i + window );
        double mean = 0;
        for ( int j = start ; j <= end ; j++ ) {
            mean += y.at( j ) * y.at( j );
        }
        mean /= ( end - start );
        mean = qSqrt( mean );
        newY.append( mean * 1.5 );
    }

    y = newY;

    meanAll = 0.0;
    for ( int i = 0; i < y.length() ; i++ ) {
        meanAll += y.at( i ) * y.at( i );
    }
    meanAll /= ( y.length() );
    meanAll = qSqrt( meanAll );
    meanAll += meanAll * 0.01;


    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    this->replot();
}

void AudioPlot::resetRange() {
    this->resetRangeX( false );
    this->resetRangeY( false );
    this->replot();
}

void AudioPlot::resetRangeX( bool replot ) {
    if ( x.length() <= 0 ) {
        return;
    }

    this->xAxis->setRange( 0, x.last() );
    if ( replot ) {
        this->replot();
    }
}

void AudioPlot::resetRangeY( bool replot ) {
    if ( y.length() <= 0 ) {
        return;
    }

    float max = y.at( 0 );
    float min = y.at( 0 );

    for ( int i = 1 ; i < y.length() ; i++ ) {
        if ( y.at( i ) > max ) {
            max = y.at( i );
        }
        if ( y.at( i ) < min ) {
            min = y.at( i );
        }
    }

    this->yAxis->setRange( min - 0.1, max + 0.1 );
    if ( replot ) {
        this->replot();
    }
}

void AudioPlot::paintEvent( QPaintEvent *event ) {
    QCustomPlot::paintEvent( event );

    QPainter painter( this );
    QRectF coordsRect( this->width() - 200, 0, 200, 20 );
    painter.drawText( coordsRect, Qt::AlignRight, cursorCoordinates );

//    QRectF plotInfoRect( this->width() - 200, 20, 200, 20 );
//    painter.drawText( plotInfoRect, Qt::AlignRight, plotInfo );

//    if ( showAverageVolume ) {
//        QRectF averageVolumeRect( this->width() - 200, 40, 200, 20 );
//        painter.drawText( averageVolumeRect, Qt::AlignRight, averageVolume );
//    }

    if ( showPosition ) {
        painter.setPen( Qt::red );
        double x = this->xAxis->coordToPixel( seconds );
        painter.drawLine( x, 0, x, this->height() );
    }

    if ( meanAll > 0.0 ) {
        painter.setPen( Qt::darkGreen );
        double y = this->yAxis->coordToPixel( meanAll );
        painter.drawLine( 0, y, this->width(), y );
    }

//    int redColor = ( tickTimer / 16.0 ) * 255;
//    painter.setBrush( QBrush( QColor( redColor, 0, 0 ) ) );
//    painter.setPen( QColor( redColor, 0, 0 ) );
//    painter.drawEllipse( this->width() - 120, 4, 40, 40 );
//    if ( tickTimer > 0 ) {
//        tickTimer--;
//    }
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
