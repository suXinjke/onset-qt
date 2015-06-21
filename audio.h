#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QDebug>
#include <QVector>
#include "bass.h"
#include "bass_fx.h"
#include "qmath.h"
#include "transform.h"
#include "sampleprocessingdialog.h"

class Audio : public QObject {
    Q_OBJECT

public:
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
    double                              getAudioBPM();
    double                              getLevelAtPosition( double positionSeconds, int pcmStep = 512 );

    int                                 getSampleCount();
    int                                 getSampleBlockCount( int sampleBlockSize = 1024 );
    QString                             getSampleBlockDuration( int index, int blockSize = 1024 );

    QVector<float>                      getSampleBlock( int index, int blockSize = 1024 );
    QVector<float>                      getFFTBlock( int index, int blockSize = 1024, bool raw = false );
    QVector<float>                      getSpectralFlux() const;
    QVector<float>                      getThreshold() const;
    QVector<float>                      getPrunnedSpectralFlux() const;
    QVector<float>                      getPeaks() const;
    QVector<float>                      getPCM() const;
    void                                setOnsetOptions( int onsetThresholdWindowSize, float onsetMultipler, bool window );
    bool                                setOnsetFilter( int lowFreq = 0, int highFreq = 0 );

    void                                fillPCM( int pcmStep = 512 );

private:
    HSTREAM                             stream;
    BASS_CHANNELINFO                    channelInfo;

    int                                 ONSET_PROCESSING_STEPS;
    int                                 ONSET_THRESHOLD_WINDOW_SIZE;
    float                               ONSET_MULTIPLIER;
    int                                 ONSET_WINDOW;

    int                                 ONSET_FILTER_LOW;
    int                                 ONSET_FILTER_HIGH;

    int                                 pcmStep;

    QVector<float>                      pcm;
    QString                             audioFilePath;

    QVector<float>                      spectralFlux;
    QVector<float>                      threshold;
    QVector<float>                      prunnedSpectralFlux;
    QVector<float>                      peaks;

    void                                fillOnset();
    int                                 checkError();
};

#endif // AUDIO_H
