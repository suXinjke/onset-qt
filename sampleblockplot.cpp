#include "sampleblockplot.h"

SampleBlockPlot::SampleBlockPlot( QWidget *parent ) :
    QCustomPlot( parent ),
    sampleCount( 0 ) {

    pcmGraph = this->addGraph();
//    this->xAxis->setRange( 0, 1 );
//    this->xAxis->setAutoTicks( false );
//    this->xAxis->setAutoTickLabels( false );
//    this->xAxis->setSubTickCount( 0 );
//    this->xAxis->setTickStep( 1.0 );

//    this->yAxis->setRange( -2, 2 );
//    this->yAxis->setAutoTickStep( false );
//    this->yAxis->setTickStep( 0.5 );
//    this->yAxis->setSubTickCount( 0 );
//    this->yAxis->setAutoSubTicks( false );
//    this->setInteraction( QCP::iRangeDrag, true );
//    this->setInteraction( QCP::iRangeZoom, true );
//    this->axisRect()->setRangeDrag( Qt::Horizontal );
//    this->axisRect()->setRangeZoom( Qt::Horizontal );

    pcmGraph->setLineStyle( QCPGraph::lsImpulse );
//    pcmGraph->setLineStyle( QCPGraph::lsNone );
    pcmGraph->setScatterStyle( QCPScatterStyle::ssDisc );

}

void SampleBlockPlot::setupPlot( SAMPLE_BLOCK_PLOT_SETUP setup ) {
    switch ( setup ) {
        case SAMPLE_BLOCK:
            this->xAxis->setRange( 0, 1 );
            this->xAxis->setSubTickCount( 0 );
            this->xAxis->setTickStep( 1.0 );

            this->yAxis->setRange( -2, 2 );
            this->yAxis->setAutoTickStep( false );
            this->yAxis->setTickStep( 0.5 );
            this->yAxis->setSubTickCount( 0 );
            this->yAxis->setAutoSubTicks( false );
            this->setInteraction( QCP::iRangeDrag, true );
            this->setInteraction( QCP::iRangeZoom, true );
            this->axisRect()->setRangeDrag( Qt::Horizontal );
            this->axisRect()->setRangeZoom( Qt::Horizontal );
            break;

        case REAL_FREQ_DOMAIN:
        case IMAGINARY_FREQ_DOMAIN:
        case MAGNITUDE_FREQ_DOMAIN:
        case PHASE_FREQ_DOMAIN:
            this->xAxis->setRange( 0, 1 );
            this->xAxis->setSubTickCount( 0 );
            this->xAxis->setTickStep( 1.0 );
            this->xAxis->setAutoTickStep( false );

//            this->xAxis->setAutoTicks( false );
//            this->xAxis->setAutoTickLabels( false );
//            connect( this->xAxis, SIGNAL( ticksRequest() ), this, SLOT( calculateFrequencyTicks() ) );
//            connect( this->xAxis, SIGNAL( ticksRequest() ), this, SLOT( calculateReadableFrequencyTicks() ) );

            this->yAxis->setRange( -2, 2 );
            this->setInteraction( QCP::iRangeDrag, true );
            this->setInteraction( QCP::iRangeZoom, true );
            this->axisRect()->setRangeDrag( Qt::Horizontal );
            this->axisRect()->setRangeZoom( Qt::Horizontal );
            break;

    }
}

void SampleBlockPlot::loadPCMData( const QVector<float> pcm ) {
    sampleCount = pcm.length();

    x.resize( sampleCount );
    y.resize( sampleCount );
    for ( int i = 0; i < sampleCount ; i++ ) {
        x[i] = i;
        y[i] = pcm.at( i );
    }

    pcmGraph->setData( x, y );

    this->replot();
}


void SampleBlockPlot::resetRange() {
    this->xAxis->setRange( 0, sampleCount );
    this->replot();
}

void SampleBlockPlot::resetVerticalRange() {
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

void SampleBlockPlot::calculateFrequencyTicks() {

    double increment = 0.1;
    if ( x.size() > 0 ) {
        increment = x.last() * 0.1;
    } else {
        return;
    }

    QVector<double> ticks;
    QVector<QString> tickLabels;
    for ( double i = 0.0 ; i < x.length() ; i += increment ) {
        ticks << i;

        QString tickLabel;
        tickLabel = QString::number( i / x.length() / 2, 'g', 2 );

        tickLabels << tickLabel;
    }

    this->xAxis->setTickVector( ticks );
    this->xAxis->setTickVectorLabels( tickLabels );
    this->replot();
}

void SampleBlockPlot::calculateReadableFrequencyTicks() {
    double increment = 0.1;
    if ( x.size() > 0 ) {
        increment = x.last() * 0.1;
    } else {
        return;
    }

    QVector<double> ticks;
    QVector<QString> tickLabels;
    for ( double i = 0.0 ; i < x.length() ; i += increment ) {
        ticks << i;

        QString tickLabel;
        tickLabel = QString::number( ( int ) ( i / x.length() * 44100 / 2 ) );

        tickLabels << tickLabel;
    }

    this->xAxis->setTickVector( ticks );
    this->xAxis->setTickVectorLabels( tickLabels );
    this->replot();
}
