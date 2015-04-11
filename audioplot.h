#ifndef AUDIOPLOT_H
#define AUDIOPLOT_H

#include "qcustomplot.h"

class AudioPlot : public QCustomPlot {
    Q_OBJECT
public:
    enum                                X_AXIS_INFORMATION {
        X_AXIS_INFORMATION_SAMPLE_INDEX,
        X_AXIS_INFORMATION_SECONDS,
        X_AXIS_INFORMATION_MINUTES_SECONDS
    };

    explicit                            AudioPlot( QWidget *parent = 0 );

    void                                loadPCMData( const QVector<float> pcm );
    void                                setSampleFrequency( int sampleFrequency );
    void                                setSampleChannels( int sampleChannels );
    void                                setPositionInSeconds( double seconds );

public slots:
    void                                setSampleAccuracy( int sampleAccuracy );
    void                                resetRange();

private:
    QCPGraph                            *pcmGraph;
    QCPItemStraightLine                 *position;
    X_AXIS_INFORMATION                  xAxisInformation;

    int                                 sampleAccuracy;
    int                                 sampleCount;
    int                                 sampleFrequency;
    int                                 sampleChannels;

    QVector<double>                     x;
    QVector<double>                     y;

private slots:
    void                                calculateTicks();
    void                                onRightClick( QMouseEvent *mouseEvent );

signals:
    void                                positionChanged( double positionSeconds );

};


#endif // AUDIOPLOT_H
