#ifndef AUDIOPLOT_H
#define AUDIOPLOT_H

#include "qcustomplot.h"
#include "audio.h"

class AudioPlot : public QCustomPlot {
    Q_OBJECT
public:

    enum                                VIEW_MODE {
        VIEW_MODE_ONSET,
        VIEW_MODE_STRESS,
        VIEW_MODE_STRESS_FORMATTED
    };

    explicit                            AudioPlot( QWidget *parent = 0 );

    void                                setAudio( Audio *audio );
    void                                setPositionInSeconds( double seconds );
    void                                setViewMode( VIEW_MODE viewMode );
    QString                             getFundamentalFrequency() const;
    QString                             getAverageVolume( double seconds ) ;

    double                              getCurrentValue( double seconds );


public slots:
    void                                loadAudioInfoFile( const QString &audioInfoFilePath );

    void                                resetRange();
    void                                resetRangeX( bool replot = true );
    void                                resetRangeY( bool replot = true );


private:
    Audio                               *audio;
    VIEW_MODE                           viewMode;

    QCPGraph                            *onsetGraph;
    QVector<double>                     xOnset;
    QVector<double>                     yOnset;

    QCPGraph                            *pcmGraph;
    QVector<double>                     xPCM;
    QVector<double>                     yPCM;

    QCPGraph                            *pcmFormattedDangerGraph;
    QVector<double>                     xFormattedDangerPCM;
    QVector<double>                     yFormattedDangerPCM;

    QCPGraph                            *pcmFormattedSafeGraph;
    QVector<double>                     xFormattedSafePCM;
    QVector<double>                     yFormattedSafePCM;

    QCPGraph                            *pcmFormattedCautionGraph;
    QVector<double>                     xFormattedCautionPCM;
    QVector<double>                     yFormattedCautionPCM;

    QCPGraph                            *meanGraph;

    QString                             cursorCoordinates;

    double                              seconds;
    bool                                showPosition;
    bool                                showMean;
    double                              meanAll;

    void                                paintEvent( QPaintEvent *event );

private slots:
    void                                onRightClick( QMouseEvent *mouseEvent );
    void                                getCursorCoordinates( QMouseEvent *mouseEvent );

signals:
    void                                positionChanged( double positionSeconds );
    void                                cursorCoordinatesChanged( const QString &coordinates );

};


#endif // AUDIOPLOT_H
