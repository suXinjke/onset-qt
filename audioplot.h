#ifndef AUDIOPLOT_H
#define AUDIOPLOT_H

#include "qcustomplot.h"
#include "audio.h"

class AudioPlot : public QCustomPlot {
    Q_OBJECT
public:

    explicit                            AudioPlot( QWidget *parent = 0 );

    void                                setAudio( Audio *audio );
    void                                loadStress( int window = 1024, int step = 4410 );
    void                                loadOnset();
    void                                setPositionInSeconds( double seconds );
    QString                             getFundamentalFrequency() const;
    QString                             getAverageVolume( double seconds ) ;

    double                              getCurrentValue( double seconds );


public slots:
    void                                runningAverage( int window = 128 );
    void                                resetRange();
    void                                resetRangeX( bool replot = true );
    void                                resetRangeY( bool replot = true );


private:
    Audio                               *audio;
    QCPGraph                            *pcmGraph;

    QVector<double>                     x;
    QVector<double>                     y;
    QString                             cursorCoordinates;
    QString                             plotInfo;
    QString                             averageVolume;
    double                              seconds;
    bool                                showPosition;
    bool                                showAverageVolume;
    double                              meanAll;
    int                                 tickTimer;

    void                                paintEvent( QPaintEvent *event );

private slots:
    void                                onRightClick( QMouseEvent *mouseEvent );
    void                                getCursorCoordinates( QMouseEvent *mouseEvent );

signals:
    void                                positionChanged( double positionSeconds );
    void                                cursorCoordinatesChanged( const QString &coordinates );

};


#endif // AUDIOPLOT_H
