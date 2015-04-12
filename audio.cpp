#include "audio.h"

Audio::Audio( QObject *parent ) :
    QObject( parent ), stream( 0 ), channel( 0 ) {

    if ( !BASS_Init( -1, 44100, 0, NULL, NULL ) ) {
        this->checkError();
    }
}

bool Audio::loadAudio( const QString &audioFilePath ) {
    if ( stream ) {
        this->stopAudio();
        if ( !BASS_StreamFree( stream ) ) {
            this->checkError();
            return false;
        }
        stream = 0;
    }

    stream = BASS_StreamCreateFile( false, audioFilePath.toStdString().c_str(), 0, 0, BASS_SAMPLE_FLOAT );
    channel = BASS_StreamCreateFile( false, audioFilePath.toStdString().c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE );

    if ( !stream ) {
        this->checkError();
        return false;
    }

    BASS_ChannelGetInfo( stream, &channelInfo );
    this->fillPCMData( audioFilePath );

//    this->fillOnset();

    return true;
}

bool Audio::playAudio() {
    if ( !stream && !channel ) {
        return false;
    }

    if ( !BASS_ChannelPlay( stream, false ) ) {
        this->checkError();
        return false;
    }

    return true;
}

void Audio::stopAudio() {
    if ( !stream && !channel ) {
        return;
    }

    BASS_ChannelStop( stream );

    this->seekAudio( 0 );
}

void Audio::pauseAudio() {
    if ( !stream && !channel ) {
        return;
    }

    if ( !BASS_ChannelPause( stream ) ) {
        this->checkError();
        return;
    }
}

void Audio::seekAudio( double positionSeconds ) {
    if ( !stream && !channel ) {
        return;
    }


    QWORD audioPos = BASS_ChannelSeconds2Bytes( stream, positionSeconds );
    if ( !BASS_ChannelSetPosition( stream, audioPos, BASS_POS_BYTE ) ) {
        this->checkError();
        return;
    }

}

double Audio::getAudioCurrentPositionSeconds() {
    if ( !stream && !channel ) {
        return -1.0;
    }

    QWORD audioPos;
    double seconds;

    audioPos = BASS_ChannelGetPosition( stream, BASS_POS_BYTE );

    if ( this->checkError() != 0 ) {
        return -1.0;
    }


    seconds = BASS_ChannelBytes2Seconds( stream, audioPos );


    if ( seconds < 0.0 ) {
        this->checkError();
    }

    return seconds;
}

double Audio::getAudioDuration() {
    if ( !stream && !channel ) {
        return -1.0;
    }

    QWORD audioLength;
    audioLength = BASS_ChannelGetLength( stream, BASS_POS_BYTE );

    if ( this->checkError() != 0 ) {
        return -1.0;
    }

    double duration;
    duration = BASS_ChannelBytes2Seconds( stream, audioLength );

    if ( duration < 0.0 ) {
        this->checkError();
    }

    return duration;
}

int Audio::getAudioFrequency() {
    if ( !stream && !channel ) {
        return -1;
    }

    return channelInfo.freq;
}

int Audio::getAudioChannels() {
    if ( !stream && !channel ) {
        return -1;
    }

    return channelInfo.chans;
}

int Audio::getSampleCount() {
    if ( !stream && !channel ) {
        return -1;
    }

    int frequency = this->getAudioFrequency();
    double duration = this->getAudioDuration();

    return qCeil( frequency * duration );
}

QVector<float> Audio::getPCMDataBlock( int index, int blockSize ) {
    if ( !stream && !channel ) {
        return QVector<float>();
    }

    if ( index >= this->getSampleCount() ) {
        return QVector<float>();
    }

    QVector<float> pcmPart( blockSize );
    for ( int i = 0 ; i < blockSize ; i++ ) {
        double data;
        if ( index * blockSize + i >= pcm.length() ) {
            data = 0.0;
        } else {
            data = pcm.at( index * blockSize + i );
        }
        pcmPart[i] = data;
    }
    return pcmPart;
}

QVector<float> Audio::getPCMData() const {
    return pcm;
}

QVector<float> Audio::getSpectralFlux() {
    QVector<float> spectralFlux;
    QVector<float> threshold;
    QVector<float> prunnedSpectralFlux;
    QVector<float> peaks;

//    int blockCount = qCeil( this->getSampleCount() / 1024.0 );
//    for ( int i = 1 ; i < blockCount ; i++ ) {
//        QVector<float> pcmBlock = this->getPCMDataBlock( i - 1, 1024 );
//        QVector<float> nextPcmBlock = this->getPCMDataBlock( i, 1024 );
//        spectralFlux.append( Transform::getSpectrumFlux( pcmBlock, nextPcmBlock ) );
//    }

    qDebug() << BASS_ChannelSeconds2Bytes( channel, 1.0 );

    long length = BASS_ChannelGetLength( channel, BASS_POS_BYTE );
    long step = 1024 * 4 * 2;
    for ( long i = step ; i < length; i += step ) {
        float fft[256];
        float nextFft[256];
        BASS_ChannelSetPosition( channel, i - step, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( channel, fft, BASS_DATA_FLOAT | BASS_DATA_FFT512 );

        BASS_ChannelSetPosition( channel, i, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( channel, nextFft, BASS_DATA_FLOAT | BASS_DATA_FFT512 );
        spectralFlux.append( Transform::getSpectrumFlux( fft, nextFft ) );
    }

    qDebug() << "Spec flux length: " << spectralFlux.length();
    int THRESHOLD_WINDOW_SIZE = 20;
    float MULTIPLIER = 1.5f;

    for ( int i = 0; i < spectralFlux.length() ; i++ ) {
        int start = qMax( 0, i - THRESHOLD_WINDOW_SIZE );
        int end = qMin( spectralFlux.length() - 1, i + THRESHOLD_WINDOW_SIZE );
        float mean = 0;
        for ( int j = start ; j <= end ; j++ ) {
            mean += spectralFlux.at( j );
        }
        mean /= ( end - start );
        threshold.append( mean * MULTIPLIER );
    }

    for ( int i = 0; i < threshold.size(); i++ ) {
        if ( threshold.at( i ) <= spectralFlux.at( i ) ) {
            prunnedSpectralFlux.append( spectralFlux.at( i ) - threshold.at( i ) );
        } else {
            prunnedSpectralFlux.append( 0.0 );
        }
    }

    for ( int i = 0; i < prunnedSpectralFlux.size() - 1; i++ ) {
        if ( prunnedSpectralFlux.at( i ) > prunnedSpectralFlux.at( i + 1 ) ) {
            peaks.append( prunnedSpectralFlux.at( i ) );
        } else {
            peaks.append( 0.0 );
        }
    }


    return peaks;
}

void Audio::fillPCMData( const QString &audioFilePath ) {
    if ( !stream ) {
        return;
    }

    sample = BASS_SampleLoad( false, audioFilePath.toStdString().c_str(), 0, 0, 1, BASS_SAMPLE_FLOAT );
    BASS_SAMPLE sampleInfo;
    if ( !BASS_SampleGetInfo( sample, &sampleInfo ) ) {
        this->checkError();
        return;
    }

    //freq * seconds * sampleSize * chans = length (bytes)

    pcm.resize( sampleInfo.length / sizeof( float ) );
    if ( !BASS_SampleGetData( sample, pcm.data() ) ) {
        this->checkError();
    }

    if ( !BASS_SampleFree( sample ) ) {
        this->checkError();
    }
}

void Audio::fillOnset( ) {
    if ( stream ) {
        this->stopAudio();
    } else {
        return;
    }

    long pos = 0;
    long length = BASS_ChannelGetLength( stream, BASS_POS_BYTE );

//    while ( pos < length ) {
//        BASS_ChannelSetPosition( stream, pos, BASS_POS_BYTE );
//        float fft[512];

//        BASS_ChannelGetData( stream, fft, BASS_DATA_FFT1024 );

//        for ( int i = 0 ; i < 512 ; i++ ) {
//            onset.append( fft[i] );
//        }

//        pos += 1024;
//    }
    qDebug() << BASS_ChannelSeconds2Bytes( channel, 1.0 );

    BASS_ChannelSetPosition( channel, 1024 * 300 * 4, BASS_POS_BYTE );
    float fft[1024];
    qDebug() << BASS_ChannelGetData( channel, fft, BASS_DATA_FLOAT );
    checkError();


    for ( int i = 0 ; i < 512 ; i++ ) {
        onset.append( fft[i] );
    }

//    QVector<float> fft2 = Transform::FFT( this->getPCMDataBlock( 300 ) );

    qDebug() << onset;

    qDebug() << this->getPCMDataBlock( 300 );
//    qDebug() << fft2;


}

int Audio::checkError() {
    int errorCode = BASS_ErrorGetCode();

    switch ( errorCode ) {
        case 0:
            break;

        case 1:
            qDebug() << "BASS_ERROR_MEM";
            break;

        case 2:
            qDebug() << "BASS_ERROR_FILEOPEN";
            break;

        case 3:
            qDebug() << "BASS_ERROR_DRIVER";
            break;

        case 4:
            qDebug() << "BASS_ERROR_BUFLOST";
            break;

        case 5:
            qDebug() << "BASS_ERROR_HANDLE";
            break;

        case 6:
            qDebug() << "BASS_ERROR_FORMAT";
            break;

        case 7:
            qDebug() << "BASS_ERROR_POSITION";
            break;

        case 8:
            qDebug() << "BASS_ERROR_INIT";
            break;

        case 9:
            qDebug() << "BASS_ERROR_START";
            break;

        case 14:
            qDebug() << "BASS_ERROR_ALREADY";
            break;

        case 18:
            qDebug() << "BASS_ERROR_NOCHAN";
            break;

        case 19:
            qDebug() << "BASS_ERROR_ILLTYPE";
            break;

        case 20:
            qDebug() << "BASS_ERROR_ILLPARAM";
            break;

        case 21:
            qDebug() << "BASS_ERROR_NO3D";
            break;

        case 22:
            qDebug() << "BASS_ERROR_NOEAX";
            break;

        case 23:
            qDebug() << "BASS_ERROR_DEVICE";
            break;

        case 24:
            qDebug() << "BASS_ERROR_NOPLAY";
            break;

        case 25:
            qDebug() << "BASS_ERROR_FREQ";
            break;

        case 27:
            qDebug() << "BASS_ERROR_NOTFILE";
            break;

        case 29:
            qDebug() << "BASS_ERROR_NOHW";
            break;

        case 31:
            qDebug() << "BASS_ERROR_EMPTY";
            break;

        case 32:
            qDebug() << "BASS_ERROR_NONET";
            break;

        case 33:
            qDebug() << "BASS_ERROR_CREATE";
            break;

        case 34:
            qDebug() << "BASS_ERROR_NOFX";
            break;

        case 37:
            qDebug() << "BASS_ERROR_NOTAVAIL";
            break;

        case 38:
            qDebug() << "BASS_ERROR_DECODE";
            break;

        case 39:
            qDebug() << "BASS_ERROR_DX";
            break;

        case 40:
            qDebug() << "BASS_ERROR_TIMEOUT";
            break;

        case 41:
            qDebug() << "BASS_ERROR_FILEFORM";
            break;

        case 42:
            qDebug() << "BASS_ERROR_SPEAKER";
            break;

        case 43:
            qDebug() << "BASS_ERROR_VERSION";
            break;

        case 44:
            qDebug() << "BASS_ERROR_CODEC";
            break;

        case 45:
            qDebug() << "BASS_ERROR_ENDED";
            break;

        case 46:
            qDebug() << "BASS_ERROR_BUSY";
            break;

        case -1:
            qDebug() << "BASS_ERROR_UNKNOWN";
            break;

        default:
            qDebug() << "BASS_ERROR_UNKNOWN";
            break;
    }

    return errorCode;
}
