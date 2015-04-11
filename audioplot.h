#ifndef AUDIOPLOT_H
#define AUDIOPLOT_H

#include "qcustomplot.h"
#include "audio.h"

class AudioPlot : public QCustomPlot {
    Q_OBJECT
public:
    enum                                X_AXIS_INFORMATION {
        X_AXIS_INFORMATION_SAMPLE_INDEX,
        X_AXIS_INFORMATION_SECONDS,
        X_AXIS_INFORMATION_MINUTES_SECONDS
    };

    explicit                            AudioPlot( QWidget *parent = 0 );

    void                                setAudio( Audio *audio );
    void                                loadPCMData( int step = 1 );
    void                                loadPCMBlock( int index, int step = 1, int blockSize = 1024 );
    void                                setPositionInSeconds( double seconds );

public slots:
    void                                resetRange();

private:
    Audio                               *audio;
    QCPGraph                            *pcmGraph;
    QCPItemStraightLine                 *position;

    QVector<double>                     x;
    QVector<double>                     y;

private slots:
    void                                onRightClick( QMouseEvent *mouseEvent );

signals:
    void                                positionChanged( double positionSeconds );

};


#endif // AUDIOPLOT_H
