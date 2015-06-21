#include "audioplot.h"

AudioPlot::AudioPlot( QWidget *parent ) :
    QCustomPlot( parent ),
    audio( 0 ),
    viewMode( VIEW_MODE_ONSET ),
    seconds( 0.0 ),
    showPosition( true ),
    showMean( false ),
    meanAll( 0.0 ) {

    onsetGraph = this->addGraph();
    pcmGraph = this->addGraph();
    pcmFormattedDangerGraph = this->addGraph();
    pcmFormattedSafeGraph = this->addGraph();
    pcmFormattedCautionGraph = this->addGraph();
    meanGraph = this->addGraph();
    this->xAxis->setRange( 0, 1 );
    this->yAxis->setRange( 0, 1 );
    this->setInteraction( QCP::iRangeDrag, true );
    this->setInteraction( QCP::iRangeZoom, true );
    this->axisRect()->setRangeDrag( Qt::Horizontal );
    this->axisRect()->setRangeZoom( Qt::Horizontal );

    this->setPositionInSeconds( 0.0 );

    onsetGraph->setScatterStyle( QCPScatterStyle::ssDisc );
    onsetGraph->setLineStyle( QCPGraph::lsImpulse );

    pcmGraph->setVisible( false );
    pcmGraph->setBrush( QBrush( QColor( 0, 0, 255, 90 ) ) );
    pcmGraph->setChannelFillGraph( meanGraph );

    pcmFormattedDangerGraph->setVisible( false );
    pcmFormattedDangerGraph->setBrush( QBrush( QColor( 0, 0, 255, 90 ) ) );

    pcmFormattedSafeGraph->setVisible( false );
    pcmFormattedSafeGraph->setBrush( QBrush( QColor( 0, 255, 0, 90 ) ) );
    pcmFormattedSafeGraph->setPen( QPen( Qt::green ) );

    pcmFormattedCautionGraph->setVisible( false );
    pcmFormattedCautionGraph->setBrush( QBrush( QColor( 255, 150, 0, 90 ) ) );
    pcmFormattedCautionGraph->setPen( QPen( Qt::red ) );

    meanGraph->setVisible( false );
    meanGraph->setPen( QPen( Qt::red ) );

    connect( this, SIGNAL( mousePress( QMouseEvent * ) ), this, SLOT( onRightClick( QMouseEvent * ) ) );
    connect( this, SIGNAL( mouseMove( QMouseEvent * ) ), this, SLOT( getCursorCoordinates( QMouseEvent * ) ) );
}

void AudioPlot::setAudio( Audio *audio ) {
    this->audio = audio;
}

void AudioPlot::loadAudioInfoFile( const QString &audioInfoFilePath ) {
    QFile audioInfoFile( audioInfoFilePath );
    if ( audioInfoFile.open( QIODevice::ReadOnly ) ) {
        xOnset.resize( 0 );
        yOnset.resize( 0 );
        xPCM.resize( 0 );
        yPCM.resize( 0 );
        xFormattedDangerPCM.resize( 0 );
        yFormattedDangerPCM.resize( 0 );
        xFormattedSafePCM.resize( 0 );
        yFormattedSafePCM.resize( 0 );
        xFormattedCautionPCM.resize( 0 );
        yFormattedCautionPCM.resize( 0 );

        VIEW_MODE readingMode = VIEW_MODE_ONSET;
        QTextStream inp( &audioInfoFile );
        while ( !inp.atEnd() ) {
            QString line = inp.readLine();
            if ( QString::compare( line, "PCM" ) == 0 ) {
                readingMode = VIEW_MODE_STRESS;
                inp >> meanAll;
            } else if ( QString::compare( line, "PCMFormatted" ) == 0  ) {
                readingMode = VIEW_MODE_STRESS_FORMATTED;
            }

            QStringList xy = line.split( "," );
            switch ( readingMode ) {
                case VIEW_MODE_ONSET:
                    if ( xy.size() == 2 ) {
                        xOnset.append( xy.at( 0 ).toDouble() );
                        yOnset.append( xy.at( 1 ).toDouble() );
                    }
                    break;

                case VIEW_MODE_STRESS:
                    if ( xy.size() == 2 ) {
                        xPCM.append( xy.at( 0 ).toDouble() );
                        yPCM.append( xy.at( 1 ).toDouble() );
                    }
                    break;

                case VIEW_MODE_STRESS_FORMATTED:
                    if ( xy.size() == 3 ) {
                        Audio::PERIOD_TYPE periodType = ( Audio::PERIOD_TYPE ) xy.at( 0 ).toInt();
                        double periodBegin = xy.at( 1 ).toDouble();
                        double periodEnd = xy.at( 2 ).toDouble();

                        switch ( periodType ) {
                            case Audio::PERIOD_TYPE_DANGER:
                                xFormattedDangerPCM << periodBegin - 0.00001 << periodBegin << periodEnd << periodEnd + 0.00001;
                                yFormattedDangerPCM << 0.0 << 1.0 << 1.0 << 0.0;
                                break;

                            case Audio::PERIOD_TYPE_SAFE:
                                xFormattedSafePCM << periodBegin - 0.00001 << periodBegin << periodEnd << periodEnd + 0.00001;
                                yFormattedSafePCM << 0.0 << 0.2 << 0.2 << 0.0;
                                break;

                            case Audio::PERIOD_TYPE_CAUTION:
                                xFormattedCautionPCM << periodBegin - 0.00001 << periodBegin << periodEnd << periodEnd + 0.00001;
                                yFormattedCautionPCM << 0.0 << 0.5 << 0.5 << 0.0;
                                break;

                            default:
                                break;
                        }

                    }
                    break;
            }
        }

        onsetGraph->clearData();
        onsetGraph->setData( xOnset, yOnset );

        pcmGraph->clearData();
        pcmGraph->setData( xPCM, yPCM );

        pcmFormattedDangerGraph->clearData();
        pcmFormattedDangerGraph->setData( xFormattedDangerPCM, yFormattedDangerPCM );

        pcmFormattedSafeGraph->clearData();
        pcmFormattedSafeGraph->setData( xFormattedSafePCM, yFormattedSafePCM );

        pcmFormattedCautionGraph->clearData();
        pcmFormattedCautionGraph->setData( xFormattedCautionPCM, yFormattedCautionPCM );

        QVector<double> meanX;
        QVector<double> meanY;
        meanX << 0.0 << xPCM.last();
        meanY << meanAll << meanAll;
        meanGraph->clearData();
        meanGraph->setData( meanX, meanY );

        this->replot();

        audioInfoFile.close();
    }
}

void AudioPlot::setPositionInSeconds( double seconds ) {
    this->seconds = seconds;

    this->update();
}

void AudioPlot::setViewMode( VIEW_MODE viewMode ) {
    this->viewMode = viewMode;

    switch ( viewMode ) {
        case VIEW_MODE_ONSET:
            onsetGraph->setVisible( true );
            pcmGraph->setVisible( false );
            pcmFormattedDangerGraph->setVisible( false );
            pcmFormattedSafeGraph->setVisible( false );
            pcmFormattedCautionGraph->setVisible( false );
            meanGraph->setVisible( false );
            break;
        case VIEW_MODE_STRESS:
            onsetGraph->setVisible( false );
            pcmGraph->setVisible( true );
            pcmFormattedDangerGraph->setVisible( false );
            pcmFormattedSafeGraph->setVisible( false );
            pcmFormattedCautionGraph->setVisible( false );
            meanGraph->setVisible( true );
            break;
        case VIEW_MODE_STRESS_FORMATTED:
            onsetGraph->setVisible( false );
            pcmGraph->setVisible( false );
            pcmFormattedDangerGraph->setVisible( true);
            pcmFormattedSafeGraph->setVisible( true );
            pcmFormattedCautionGraph->setVisible( true );
            meanGraph->setVisible( false );
            break;
    }

    this->resetRangeY();
}

double AudioPlot::getCurrentValue( double seconds ) {
    return onsetGraph->data()->upperBound( seconds ).value().value;
}

void AudioPlot::resetRangeX( bool replot ) {
    QVector<double> x;
    switch ( viewMode ) {
        case VIEW_MODE_ONSET:
            x = xOnset;
            break;

        case VIEW_MODE_STRESS:
            x = xPCM;
            break;

        case VIEW_MODE_STRESS_FORMATTED:
            x = xFormattedDangerPCM;
            break;
    }

    if ( x.length() <= 0 ) {
        return;
    }

    this->xAxis->setRange( 0, x.last() );

    if ( replot ) {
        this->replot();
    }
}

void AudioPlot::resetRangeY( bool replot ) {
    QVector<double> y;
    switch ( viewMode ) {
        case VIEW_MODE_ONSET:
            y = yOnset;
            break;

        case VIEW_MODE_STRESS:
            y = yPCM;
            break;

        case VIEW_MODE_STRESS_FORMATTED:
            y = yFormattedDangerPCM;
            break;
    }

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

void AudioPlot::resetRange() {
    this->resetRangeX( false );
    this->resetRangeY( false );
    this->replot();
}

void AudioPlot::paintEvent( QPaintEvent *event ) {
    QCustomPlot::paintEvent( event );

    QPainter painter( this );
    QRectF coordsRect( this->width() - 200, 0, 200, 20 );
    painter.drawText( coordsRect, Qt::AlignRight, cursorCoordinates );

    if ( showPosition ) {
        painter.setPen( Qt::red );
        double x = this->xAxis->coordToPixel( seconds );
        painter.drawLine( x, 0, x, this->height() );
    }
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
