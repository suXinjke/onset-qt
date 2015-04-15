#ifndef AUDIOPLOT_H
#define AUDIOPLOT_H

#include "qcustomplot.h"
#include "audio.h"

class AudioPlot : public QCustomPlot {
    Q_OBJECT
public:

    explicit                            AudioPlot( QWidget *parent = 0 );

    void                                setAudio( Audio *audio );
    void                                loadPCMData( const QVector<float> &pcm );
    void                                loadWaveform( int step = 4410 );
    void                                loadPCMBlock( int index, int blockSize = 1024 );
    void                                loadFFTBlock( int index, int blockSize = 1024 );
    void                                loadFFTPhaseBlock( int index, int blockSize = 1024 );
    void                                loadFFTBlockRaw(int index, int blockSize = 1024, bool imaginary = false );
    void                                loadOnset();
    void                                setPositionInSeconds( double seconds );
    QString                             getFundamentalFrequency() const;
    QString                             getAverageVolume( double seconds ) ;

    double                              getCurrentValue( double seconds );


public slots:
    void                                resetRange();
    void                                resetRangeX( bool replot = true );
    void                                resetRangeY( bool replot = true );


private:
    Audio                               *audio;
    QCPGraph                            *pcmGraph;
    QCPGraph                            *waveformGraph;

    QVector<double>                     x;
    QVector<double>                     y;
    QString                             cursorCoordinates;
    QString                             plotInfo;
    QString                             averageVolume;
    double                              seconds;
    bool                                showPosition;
    bool                                showAverageVolume;
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
