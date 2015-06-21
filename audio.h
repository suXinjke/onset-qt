#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QDebug>
#include <QCryptographicHash>
#include <QFile>
#include <QVector>
#include "bass.h"
#include "bass_fx.h"
#include "qmath.h"
#include "transform.h"
#include "sampleprocessingdialog.h"

class Audio : public QObject {
    Q_OBJECT

public:
    enum                                PERIOD_TYPE {
        PERIOD_TYPE_SAFE = 0,
        PERIOD_TYPE_CAUTION = 1,
        PERIOD_TYPE_DANGER = 2
    };

    struct                              Period {
        PERIOD_TYPE                     periodType;
        double                          periodBegin;
        double                          periodEnd;

        Period() {}
        Period( PERIOD_TYPE periodType, double periodBegin, double periodEnd ) :
            periodType( periodType ), periodBegin( periodBegin ), periodEnd( periodEnd ) {}
    };

    explicit                            Audio( QObject *parent = 0 );

    bool                                loadAudio( const QString &audioFilePath );
    bool                                playAudio();
    void                                stopAudio();
    void                                pauseAudio();
    void                                seekAudio( double positionSeconds );

    double                              getAudioCurrentPositionSeconds();
    double                              getAudioDuration();
    int                                 getAudioFrequency();
    int                                 getAudioChannels();

    int                                 getSampleCount();
    int                                 getSampleBlockCount( int sampleBlockSize = 1024 );
    QString                             getSampleBlockDuration( int index, int blockSize = 1024 );

    QVector<float>                      getPeaks();
    QVector<float>                      getPCM( int pcmStep = 512 );
    void                                setOnsetOptions( int onsetThresholdWindowSize, float onsetMultipler, bool window );

public slots:
    void                                produceAudioInfoFile( int pcmStep = 512, int window = 256 );

private:
    HSTREAM                             stream;
    BASS_CHANNELINFO                    channelInfo;

    int                                 ONSET_THRESHOLD_WINDOW_SIZE;
    double                              ONSET_MULTIPLIER;
    int                                 ONSET_WINDOW;

    int                                 pcmStep;

    QString                             audioFilePath;

    int                                 checkError();
};

#endif // AUDIO_H
