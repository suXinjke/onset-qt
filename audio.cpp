#include "audio.h"

Audio::Audio( QObject *parent ) :
    QObject( parent ), stream( 0 ),
    ONSET_PROCESSING_STEPS( 5 ), ONSET_THRESHOLD_WINDOW_SIZE( 20 ), ONSET_MULTIPLIER( 1.5f ), ONSET_WINDOW( 0 ), ONSET_FILTER_LOW( 0 ), ONSET_FILTER_HIGH( 0 ) {

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

    if ( !stream ) {
        this->checkError();
        return false;
    }

    BASS_ChannelGetInfo( stream, &channelInfo );
    this->audioFilePath = audioFilePath;

//    this->fillPCM();
    return true;
}

bool Audio::playAudio() {
    if ( !stream ) {
        return false;
    }

    if ( !BASS_ChannelPlay( stream, false ) ) {
        this->checkError();
        return false;
    }

    return true;
}

void Audio::stopAudio() {
    if ( !stream ) {
        return;
    }

    BASS_ChannelStop( stream );

    this->seekAudio( 0 );
}

void Audio::pauseAudio() {
    if ( !stream ) {
        return;
    }

    if ( !BASS_ChannelPause( stream ) ) {
        this->checkError();
        return;
    }
}

void Audio::seekAudio( double positionSeconds ) {
    if ( !stream ) {
        return;
    }


    QWORD audioPos = BASS_ChannelSeconds2Bytes( stream, positionSeconds );
    if ( !BASS_ChannelSetPosition( stream, audioPos, BASS_POS_BYTE ) ) {
        this->checkError();
        return;
    }

}

double Audio::getAudioCurrentPositionSeconds() {
    if ( !stream ) {
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
    if ( !stream ) {
        return -1.0;
    }

    QWORD audioLength = BASS_ChannelGetLength( stream, BASS_POS_BYTE );

    if ( this->checkError() != 0 ) {
        return -1.0;
    }

    double duration = BASS_ChannelBytes2Seconds( stream, audioLength );

    if ( duration < 0.0 ) {
        this->checkError();
    }

    return duration;
}

int Audio::getAudioFrequency() {
    if ( !stream ) {
        return -1;
    }

    return channelInfo.freq;
}

int Audio::getAudioChannels() {
    if ( !stream ) {
        return -1;
    }

    return channelInfo.chans;
}

int Audio::getSampleCount() {
    if ( !stream ) {
        return -1;
    }

    QWORD length = BASS_ChannelGetLength( stream, BASS_POS_BYTE );
    int channels = this->getAudioChannels();
    length /= channels * sizeof( float );

    return length;
}

int Audio::getSampleBlockCount( int sampleBlockSize ) {
    double sampleCount = this->getSampleCount();

    return qCeil( sampleCount / sampleBlockSize );
}

QString Audio::getSampleBlockDuration( int index, int blockSize ) {
    if ( index < 0 || index >= this->getSampleBlockCount( blockSize ) ) {
        return QString();
    }

    double frequency = this->getAudioFrequency();
    int sampleCount = this->getSampleCount();
    int sampleBegin = index * blockSize;
    int sampleEnd = qMin( sampleBegin + blockSize, sampleCount );

    double beginSeconds = sampleBegin / frequency;
    double endSeconds = sampleEnd / frequency;

    return QString( "%1 / %2" ).arg( QString::number( beginSeconds, 'f', 3 ) ).arg( QString::number( endSeconds, 'f', 3 ) );
}

QVector<float> Audio::getSampleBlock( int index, int blockSize ) {
    if ( index < 0 || index >= this->getSampleBlockCount( blockSize ) ) {
        return QVector<float>();
    }

    int actualBlockSize = blockSize * 2; //2 because float doubles memory usage

    QVector<float> sampleBlock( actualBlockSize );
    sampleBlock.fill( 0 );

    int beginPos = index * actualBlockSize;
    int endPos = qMin( ( index + 1 ) * actualBlockSize, pcm.length() );

    for ( int i = beginPos, p = 0 ; i < endPos ; i++ ) {
        sampleBlock[p] = pcm.at( i );
        p++;
    }

    return sampleBlock;
}

QVector<float> Audio::getFFTBlock( int index, int blockSize , bool raw ) {
    int FFT_MODE;
    switch ( blockSize ) {
        case 256:
            FFT_MODE = BASS_DATA_FFT256;
            break;
        case 512:
            FFT_MODE = BASS_DATA_FFT512;
            break;
        case 1024:
            FFT_MODE = BASS_DATA_FFT1024;
            break;
        case 2048:
            FFT_MODE = BASS_DATA_FFT2048;
            break;
        case 4096:
            FFT_MODE = BASS_DATA_FFT4096;
            break;
        case 8192:
            FFT_MODE = BASS_DATA_FFT8192;
            break;
        default:
            return QVector<float>();
    }

    QVector<float> sampleBlock = this->getSampleBlock( index, blockSize );
    int channels = this->getAudioChannels();
    int actualBlockSize = sampleBlock.length() / 2;
    int frequency = this->getAudioFrequency();

    HSAMPLE sample = BASS_SampleCreate( actualBlockSize * channels * sizeof( float ), frequency, channels, 1, BASS_SAMPLE_FLOAT );
    BASS_SampleSetData( sample, sampleBlock.data() );

    HCHANNEL sampleChannel = BASS_SampleGetChannel( sample, true );

    int fftSize = raw ? blockSize * 2 : blockSize / 2;
    int RAW_DATA = raw ? BASS_DATA_FFT_COMPLEX : 0;
    QVector<float> fft( fftSize );
    BASS_ChannelGetData( sampleChannel, fft.data(), BASS_DATA_FLOAT | FFT_MODE | RAW_DATA | ONSET_WINDOW );

    BASS_SampleFree( sample );

    if ( ONSET_FILTER_LOW != 0 || ONSET_FILTER_HIGH != 0 ) {
        int lowFreqIndex = ( ( double ) ONSET_FILTER_LOW / ( frequency / 2.0 ) ) * fftSize;;
        int highFreqIndex = ( ( double ) ONSET_FILTER_HIGH / ( frequency / 2.0 ) ) * fftSize;

        for ( int i = lowFreqIndex ; i >= 0 ; i-- ) {
            fft[i] = 0.0f;
        }
        for ( int i = highFreqIndex ; i < fftSize ; i++ ) {
            fft[i] = 0.0f;
        }
    }

    return fft;
}

QVector<float> Audio::getSpectralFlux() const {
    return spectralFlux;
}

QVector<float> Audio::getThreshold() const {
    return threshold;
}

QVector<float> Audio::getPrunnedSpectralFlux() const {
    return prunnedSpectralFlux;
}

QVector<float> Audio::getPeaks() const {
    return peaks;
}
QVector<float> Audio::getPCM() const {
    return pcm;
}

void Audio::setOnsetOptions( int onsetThresholdWindowSize, float onsetMultipler, bool window ) {
    if ( onsetThresholdWindowSize < 1 ||
            onsetMultipler < 1.0 || onsetMultipler > 2.0 ) {
        return;
    }
    ONSET_THRESHOLD_WINDOW_SIZE = onsetThresholdWindowSize;
    ONSET_MULTIPLIER = onsetMultipler;
    ONSET_WINDOW = window ? 0 : BASS_DATA_FFT_NOWINDOW;
    this->fillOnset();
}

bool Audio::setOnsetFilter( int lowFreq, int highFreq ) {
    int audioFreq = this->getAudioFrequency();
    if ( lowFreq < 0 || highFreq < 0 || lowFreq > highFreq || highFreq > audioFreq ) {
        return false;
    }

    ONSET_FILTER_LOW = lowFreq;
    ONSET_FILTER_HIGH = highFreq;

    return true;
}

void Audio::fillPCM( int pcmStep ) {
    if ( !stream ) {
        return;
    }

    HSTREAM decodeChannel = BASS_StreamCreateFile( false, audioFilePath.toStdString().c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE );

    if ( !decodeChannel ) {
        this->checkError();
        return;
    }

    pcm.resize( 0 );

    long length = BASS_ChannelGetLength( decodeChannel, BASS_POS_BYTE );
    int channels = this->getAudioChannels();
    long step = 1024 * sizeof( float ) * channels;

    for ( long i = 0 ; i < length; i += step ) {
        float piece[2048];

        BASS_ChannelSetPosition( decodeChannel, i, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( decodeChannel, piece, step );

        for ( int j = 0 ; j < 2048 ; j += pcmStep ) {
            pcm.append( piece[j] );
        }
    }


    BASS_StreamFree( decodeChannel );
}

void Audio::fillOnset() {

    HSTREAM decodeChannel = BASS_StreamCreateFile( false, audioFilePath.toStdString().c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE );

    if ( !decodeChannel ) {
        this->checkError();
        return;
    }

    spectralFlux.resize( 0 );

    long length = BASS_ChannelGetLength( decodeChannel, BASS_POS_BYTE );
    int channels = this->getAudioChannels();
    int frequency = this->getAudioFrequency();
    long step = 2048 * sizeof( float ) * channels;

    SampleProcessingDialog sampleProcessingDialog;
    sampleProcessingDialog.setWindowModality( Qt::WindowModal );
    sampleProcessingDialog.setSamplesToProcess( ( length / step ) * ( ONSET_PROCESSING_STEPS ) );
    sampleProcessingDialog.show();


    for ( long i = step ; i < length; i += step ) {
        float fft[512];
        float nextFft[512];
        BASS_ChannelSetPosition( decodeChannel, i - step, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( decodeChannel, fft, BASS_DATA_FLOAT | BASS_DATA_FFT1024 | ONSET_WINDOW );

        BASS_ChannelSetPosition( decodeChannel, i, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( decodeChannel, nextFft, BASS_DATA_FLOAT | BASS_DATA_FFT1024 | ONSET_WINDOW );

        if ( ONSET_FILTER_LOW != 0 || ONSET_FILTER_HIGH != 0 ) {
            int lowFreqIndex = ( ( double ) ONSET_FILTER_LOW / ( frequency / 2.0 ) ) * 512;
            int highFreqIndex = ( ( double ) ONSET_FILTER_HIGH / ( frequency / 2.0 ) ) * 512;

            for ( int i = lowFreqIndex ; i >= 0 ; i-- ) {
                fft[i] = 0.0;
                nextFft[i] = 0.0;
            }
            for ( int i = highFreqIndex ; i < 512 ; i++ ) {
                fft[i] = 0.0;
                nextFft[i] = 0.0;
            }
        }

        float flux = 0.0;
        for ( int i = 0 ; i < 512 ; i++ ) {
            float value = nextFft[i] - fft[i];
            flux += value < 0 ? 0 : value;
        }

        spectralFlux.append( flux );
        sampleProcessingDialog.addSampleProcessed();

    }

    BASS_StreamFree( decodeChannel );


    threshold.resize( 0 );

    for ( int i = 0; i < spectralFlux.length() ; i++ ) {
        int start = qMax( 0, i - ONSET_THRESHOLD_WINDOW_SIZE );
        int end = qMin( spectralFlux.length() - 1, i + ONSET_THRESHOLD_WINDOW_SIZE );
        float mean = 0;
        for ( int j = start ; j <= end ; j++ ) {
            mean += spectralFlux.at( j );
        }
        mean /= ( end - start );
        threshold.append( mean * ONSET_MULTIPLIER );
        sampleProcessingDialog.addSampleProcessed();
    }


    prunnedSpectralFlux.resize( 0 );

    for ( int i = 0; i < threshold.size(); i++ ) {
        if ( threshold.at( i ) <= spectralFlux.at( i ) ) {
            prunnedSpectralFlux.append( spectralFlux.at( i ) - threshold.at( i ) );
        } else {
            prunnedSpectralFlux.append( 0.0 );
        }
        sampleProcessingDialog.addSampleProcessed();
    }

    peaks.resize( 0 );

    for ( int i = 0; i < prunnedSpectralFlux.size() - 1; i++ ) {
        if ( prunnedSpectralFlux.at( i ) > prunnedSpectralFlux.at( i + 1 ) ) {
            peaks.append( prunnedSpectralFlux.at( i ) );
        } else {
            peaks.append( 0.0 );
        }
        sampleProcessingDialog.addSampleProcessed();
    }

//    int PEAK_CLEAN_WINDOW = 4;
//    for ( int i = 0 ; i < peaks.size() - PEAK_CLEAN_WINDOW ; i++ ) {
//        int peakCount = 0;
//        for ( int j = 0 ; j < PEAK_CLEAN_WINDOW ; j++ ) {
//            if ( peaks.at( i + j ) > 0.0 ) {
//                peakCount++;
//            }
//        }
//        int mid = peakCount % 2 == 0 ? i + peakCount / 2 : i + peakCount / 2 + 1;
//        for ( int j = 0 ; j < peakCount / 2 ; j++ ) {
//            peaks[mid + j] = 0.0;
//            peaks[mid - j] = 0.0;
//        }
//        sampleProcessingDialog.addSampleProcessed();
//    }

//    int PEAK_CLEAN_WINDOW = 3;
//    for ( int i = 0 ; i < peaks.size() - PEAK_CLEAN_WINDOW ; i++ ) {
//        int peakCount = 0;
//        for ( int j = 0 ; j < PEAK_CLEAN_WINDOW ; j++ ) {
//            if ( peaks.at( i + j ) > 0.0 ) {
//                peakCount++;
//            }
//        }
//        if ( peakCount > 1 ) {
//            for ( int j = 1 ; j < peakCount ; j++ ) {
//                peaks[i + j] = 0.0;
//            }
//        }
//        sampleProcessingDialog.addSampleProcessed();
//    }

    sampleProcessingDialog.accept();
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
