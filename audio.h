#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QDebug>
#include <QVector>
#include "bass.h"
#include "qmath.h"
#include "transform.h"
#include <QElapsedTimer>

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

    QVector<float>                      getSampleBlock( int index, int blockSize = 1024 );
    QVector<float>                      getOnset() const;


private:
    HSTREAM                             stream;
    BASS_CHANNELINFO                    channelInfo;

    QVector<float>                      onset;
    QVector<float>                      pcm;
    QString                             audioFilePath;

    void                                fillOnset();
    void                                fillPCM();
    int                                 checkError();
};

#endif // AUDIO_H
