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

    pcmGraph->clearData();
    pcmGraph->setData( x, y );

    showPosition = false;
    showAverageVolume = false;
    meanAll = 0.0;

    this->replot();
}

void AudioPlot::loadWaveform( int step ) {
//    QVector<float> pcmDef = audio->getPCM();
//    int N = pcmDef.length();
//    if ( N <= 0 ) {
//        return;
//    }

    QVector<float> pcm = audio->getPCM();
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }


//    QVector<float> pcm2;
//    for ( int i = 0; i < N ; i += step ) {
//        int start = qMax( 0, i - 2048 );
//        int end = qMin( N - 1, i + 2048 );
//        float mean = 0.0f;
//        for ( int j = start ; j <= end ; j ++ ) {
//            mean += qAbs( pcmDef.at( j ) );
//        }
//        mean /= ( end - start );
//        pcm2.append( mean );
//    }
//    N = pcm2.length();
//    QVector<float> pcm;
//    for ( int i = 0; i < N ; i ++ ) {
//        int start = qMax( 0, i - 1024 );
//        int end = qMin( N - 1, i + 1024 );
//        float mean = 0.0f;
//        for ( int j = start ; j <= end ; j ++ ) {
//            mean += qAbs( pcm2.at( j ) );
//        }
//        mean /= ( end - start );
//        pcm.append( mean );
//    }


    int frequency = audio->getAudioFrequency();
    int channels = audio->getAudioChannels();

    x.resize( qCeil( ( double ) N / step ) );
    y.resize( qCeil( ( double ) N / step ) );
    for ( int i = 0, p = 0 ; i < N ; i += step ) {
        x[p] = ( double ) i / frequency / channels;
        double value = pcm.at( i );
        y[p] = value;
        p++;
    }

//    N = pcm.length();
//    x.resize( N );
//    y.resize( N );
//    for ( int i = 0; i < N ; i ++ ) {
//        x[i] = ( double ) ( i * step ) / frequency / channels;
//        y[i] = pcm.at( i );
//    }

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = true;
    showAverageVolume = true;
    meanAll = 0.0;

    plotInfo = "";

    this->replot();
}

void AudioPlot::loadStress( int window, int step ) {
    QVector<float> pcm = audio->getPCM();
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    int frequency = audio->getAudioFrequency();
    int channels = audio->getAudioChannels();

    QVector<float> pcmShortened;
    for ( int i = 0; i < N ; i += step ) {
        pcmShortened.append( pcm.at( i ) );
    }

    N = pcmShortened.length();

    QVector<float> pcmFiltered;
    for ( int i = 0; i < N ; i ++ ) {
        int start = qMax( 0, i - window );
        int end = qMin( N - 1, i + window );
        double mean = 0;
        for ( int j = start ; j <= end ; j++ ) {
            mean += pcmShortened.at( j ) * pcmShortened.at( j );
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

void AudioPlot::loadPCMBlock( int index, int blockSize ) {
    QVector<float> pcm = audio->getSampleBlock( index, blockSize );
    int N = pcm.length();
    if ( N <= 0 ) {
        return;
    }

    x.resize( N );
    y.resize( N );
    for ( int i = 0, p = 0 ; i < N ; i ++ ) {
        x[p] = i / 2.0; //2 because of float double memory usage
        y[p] = pcm.at( i );
        p++;
    }

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = false;
    showAverageVolume = false;
    meanAll = 0.0;

    plotInfo = "";

    this->replot();
}

void AudioPlot::loadFFTBlock( int index, int blockSize ) {
    QVector<float> fft = audio->getFFTBlock( index, blockSize );
    int N = fft.length();
    if ( N <= 0 ) {
        return;
    }

    int halfFrequency = audio->getAudioFrequency() / 2;

    x.resize( N );
    y.resize( N );
    for ( int i = 0, p = 0 ; i < N ; i ++ ) {
        x[p] = ( ( double ) i / N ) * halfFrequency;

//        double value = fft.at( i );
//        if ( value <= 0.0 ) {
//            y[p] = 0.0;
//        } else {
//            y[p] = 10 * std::log10( -value );
//        }
        y[p] = fft.at( i );

        p++;
    }

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = false;
    showAverageVolume = false;
    meanAll = 0.0;

    plotInfo = this->getFundamentalFrequency();

    this->replot();
}

void AudioPlot::loadFFTPhaseBlock( int index, int blockSize ) {
    QVector<float> fft = audio->getFFTBlock( index, blockSize, true );
    int N = fft.length();
    if ( N <= 0 ) {
        return;
    }

    int halfFrequency = audio->getAudioFrequency() / 2;

    x.resize( N / 4 );
    y.resize( N / 4 );
    for ( int i = 1, p = 0 ; p < N / 4 ; i += 2 ) {
        double real = fft.at( i - 1 );
        double imaginary = fft.at( i );
        x[p] = ( ( double ) p / ( N / 4 ) ) * halfFrequency;
        if ( qAbs( real ) < 0.01 ) {
            y[p] = 0.0;
        } else {
            y[p] = qAtan( imaginary / real );
        }
        p++;
    }

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = false;
    showAverageVolume = false;
    meanAll = 0.0;

    this->replot();
}

void AudioPlot::loadFFTBlockRaw( int index, int blockSize, bool imaginary ) {
    QVector<float> fft = audio->getFFTBlock( index, blockSize, true );
    int N = fft.length();
    if ( N <= 0 ) {
        return;
    }

//    int halfFrequency = audio->getAudioFrequency() / 2;

    x.resize( N / 4 );
    y.resize( N / 4 );
    for ( int i = imaginary, p = 0 ; p < N / 4 ; i += 2 ) {
//        x[p] = ( ( double ) i / N ) * halfFrequency;
        x[p] = p;
        y[p] = fft.at( i );
        p++;
    }

    pcmGraph->clearData();
    pcmGraph->setData( x, y );
    showPosition = false;
    showAverageVolume = false;
    meanAll = 0.0;

    this->replot();
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

QString AudioPlot::getFundamentalFrequency() const {
    int N = x.length();
    if ( N <= 0 ) {
        return QString();
    }

    double max = x.at( 0 );
    double fundamentalFrequency = 0.0;

    for ( int i = 1 ; i < N ; i++ ) {
        double xValue = x.at( i );
        double yValue = y.at( i );

        if ( yValue - 0.01 > max ) {
            max = yValue;
            fundamentalFrequency = xValue;
        }
    }

    return QString( "Fundamenutal frequency: %1" ).arg( fundamentalFrequency );
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

    QRectF plotInfoRect( this->width() - 200, 20, 200, 20 );
    painter.drawText( plotInfoRect, Qt::AlignRight, plotInfo );

    if ( showAverageVolume ) {
        QRectF averageVolumeRect( this->width() - 200, 40, 200, 20 );
        painter.drawText( averageVolumeRect, Qt::AlignRight, averageVolume );
    }

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
