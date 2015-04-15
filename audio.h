#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QDebug>
#include <QVector>
#include "bass.h"
#include "qmath.h"
#include "transform.h"
#include "sampleprocessingdialog.h"

class Audio : public QObject {
    Q_OBJECT

public:
    enum AUDIO_TYPE {
        AUDIO_FILE,
        AUDIO_GENERATED
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

    QVector<float>                      getSampleBlock( int index, int blockSize = 1024 );
    QVector<float>                      getFFTBlock( int index, int blockSize = 1024, bool raw = false );
    QVector<float>                      getOnset() const;
    QVector<float>                      getPCM() const;
    void                                setOnsetOptions( int onsetThresholdWindowSize, float onsetMultipler, int processingSteps, bool window );
    bool                                setOnsetFilter( int lowFreq = 0, int highFreq = 0 );


private:
    HSTREAM                             stream;
    BASS_CHANNELINFO                    channelInfo;

    int                                 ONSET_PROCESSING_STEPS;
    int                                 ONSET_THRESHOLD_WINDOW_SIZE;
    float                               ONSET_MULTIPLIER;
    int                                 ONSET_WINDOW;

    int                                 ONSET_FILTER_LOW;
    int                                 ONSET_FILTER_HIGH;

    QVector<float>                      onset;
    QVector<float>                      pcm;
    QString                             audioFilePath;

    void                                fillOnset();
    void                                fillPCM();
    int                                 checkError();
};

#endif // AUDIO_H
