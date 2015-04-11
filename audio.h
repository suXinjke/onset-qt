#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QDebug>
#include <QVector>
#include "bass.h"
#include "qmath.h"
#include "transform.h"

class Audio : public QObject {
    Q_OBJECT

public:
    enum AUDIO_TYPE {
        AUDIO_FILE,
        AUDIO_GENERATED
    };

    explicit                            Audio( QObject *parent = 0 );

    bool                                loadAudio( const QString &audioFilePath );
    bool                                loadGeneratedAudio( double frequency, double samplingRate, double duration );
    bool                                loadRawAudio( const QVector<float> pcm, double samplingRate = 44100.0 );
    bool                                playAudio();
    void                                stopAudio();
    void                                pauseAudio();
    void                                seekAudio( double positionSeconds );

    double                              getAudioCurrentPositionSeconds();
    double                              getAudioDuration();
    int                                 getAudioFrequency();
    int                                 getAudioChannels();
    QString                             getAudioTitle();

    int                                 getSampleCount();
    QVector<float>                      getPCMDataBlock( int index , int blockSize = 512 );

    QVector<float>                      getPCMData() const;
    QVector<float>                      getSpectralFlux();


private:
    AUDIO_TYPE                          audioType;

    HSTREAM                             stream;
    HCHANNEL                            channel;
    BASS_CHANNELINFO                    channelInfo;
    HSAMPLE                             sample;
    QVector<float>                      pcm;


    void                                fillPCMData( const QString &audioFilePath );
    int                                 checkError();
};

#endif // AUDIO_H
