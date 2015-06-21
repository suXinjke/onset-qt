#include "audio.h"

Audio::Audio( QObject *parent ) :
    QObject( parent ), stream( 0 ),
    ONSET_THRESHOLD_WINDOW_SIZE( 20 ), ONSET_MULTIPLIER( 1.5 ), ONSET_WINDOW( 0 ) {

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

QVector<float> Audio::getPeaks() {
    HSTREAM decodeChannel = BASS_StreamCreateFile( false, audioFilePath.toStdString().c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE );

    if ( !decodeChannel ) {
        this->checkError();
        return QVector<float>();
    }

    long length = BASS_ChannelGetLength( decodeChannel, BASS_POS_BYTE );
    int channels = this->getAudioChannels();
    long step = 2048 * sizeof( float ) * channels;

    QVector<float> peaks;

    for ( long i = step ; i < length; i += step ) {
        float fft[512];
        float nextFft[512];
        BASS_ChannelSetPosition( decodeChannel, i - step, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( decodeChannel, fft, BASS_DATA_FLOAT | BASS_DATA_FFT1024 | ONSET_WINDOW );

        BASS_ChannelSetPosition( decodeChannel, i, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( decodeChannel, nextFft, BASS_DATA_FLOAT | BASS_DATA_FFT1024 | ONSET_WINDOW );

        float flux = 0.0;
        for ( int i = 0 ; i < 512 ; i++ ) {
            float value = nextFft[i] - fft[i];
            flux += value < 0 ? 0 : value;
        }

        peaks.append( flux );
    }

    BASS_StreamFree( decodeChannel );


    QVector<float> threshold;

    for ( int i = 0; i < peaks.length() ; i++ ) {
        int start = qMax( 0, i - ONSET_THRESHOLD_WINDOW_SIZE );
        int end = qMin( peaks.length() - 1, i + ONSET_THRESHOLD_WINDOW_SIZE );
        float mean = 0;
        for ( int j = start ; j <= end ; j++ ) {
            mean += peaks.at( j );
        }
        mean /= ( end - start );
        threshold.append( mean * ONSET_MULTIPLIER );
    }


    for ( int i = 0; i < threshold.size(); i++ ) {
        if ( threshold.at( i ) <= peaks.at( i ) ) {
            peaks[i] = peaks.at( i ) - threshold.at( i );
        } else {
            peaks[i] = 0.0;
        }
    }

    for ( int i = 0; i < peaks.size() - 1; i++ ) {
        if ( peaks.at( i ) <= peaks.at( i + 1 ) ) {
            peaks[i] = 0.0;
        }
    }

    double max = peaks.at( 0 );
    for ( int i = 0; i < peaks.size(); i++ ) {
        if ( peaks.at( i ) > max ) {
            max = peaks.at( i );
        }
    }
    for ( int i = 0; i < peaks.size(); i++ ) {
        peaks[i] = peaks.at( i ) / max;
    }

    return peaks;

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
}

QVector<float> Audio::getPCM( int pcmStep ) {
    if ( !stream ) {
        return QVector<float>();
    }

    QVector<float> pcm;
    HSTREAM decodeChannel = BASS_StreamCreateFile( false, audioFilePath.toStdString().c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE );

    if ( !decodeChannel ) {
        this->checkError();
        return QVector<float>();
    }

    long length = BASS_ChannelGetLength( decodeChannel, BASS_POS_BYTE );
    int channels = this->getAudioChannels();
    long step = 1024 * sizeof( float ) * channels;

    for ( long i = 0 ; i < length; i += step ) {
        float piece[1024 * channels];

        BASS_ChannelSetPosition( decodeChannel, i, BASS_POS_BYTE | BASS_POS_DECODETO );
        BASS_ChannelGetData( decodeChannel, piece, step );

        for ( int j = 0 ; j < 1024 * channels ; j += pcmStep ) {
            pcm.append( piece[j] );
        }
    }

    BASS_StreamFree( decodeChannel );

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
}

void Audio::produceAudioInfoFile( int pcmStep, int window ) {
    QFile file( audioFilePath );
    file.open( QIODevice::ReadOnly );
    QString sha1 = QCryptographicHash::hash( file.readAll(), QCryptographicHash::Sha1 ).toHex();
    file.close();

    QFile outFile( QString( "D:\\audios\\%1.txt" ).arg( sha1 ) );
    if ( outFile.open( QIODevice::WriteOnly ) ) {
        int frequency = this->getAudioFrequency();
        int channels = this->getAudioChannels();

        //output onsets
        QVector<float> peaks = this->getPeaks();
        int N = peaks.length();
        if ( N <= 0 ) {
            return;
        }

        QTextStream out( &outFile );
        for ( int i = 0; i < N ; i++ ) {
            double positionSeconds = i * ( 2048.0 / frequency );
            if ( peaks.at( i ) > 0.0 ) {
                out << positionSeconds << ", " << peaks.at( i ) << endl;
            }
        }

        //output avg pcm
        QVector<float> rawPCM = this->getPCM( pcmStep );
        QVector<float> avgPCM;

        N = rawPCM.length();
        for ( int i = 0; i < N ; i ++ ) {
            int start = qMax( 0, i - window );
            int end = qMin( N - 1, i + window );
            double mean = 0;
            for ( int j = start ; j <= end ; j++ ) {
                mean += rawPCM.at( j ) * rawPCM.at( j );
            }
            mean /= ( end - start );
            mean = qSqrt( mean );
            avgPCM.append( mean );
        }

        double meanAll = 0.0;
        for ( int i = 0; i < N ; i++ ) {
            meanAll += rawPCM.at( i ) * rawPCM.at( i );
        }
        meanAll /= ( rawPCM.length() );
        meanAll = qSqrt( meanAll );

        N = avgPCM.length();
        if ( N <= 0 ) {
            return;
        }

        out << "PCM" << endl;
        out << meanAll << endl;
        for ( int i = 0; i < N ; i++ ) {
            double positionSeconds = ( double ) ( i * pcmStep ) / frequency / channels;
            out << positionSeconds << ", " << avgPCM.at( i ) << endl;
        }

        out << "PCMFormatted" << endl;


        bool onPeriod = false;
        double periodBegin = 0.0;
        double periodEnd = 0.0;
        QVector< Period > periods;
        for ( int i = 0 ; i < N ; i++ ) {
            double val = avgPCM.at( i );
            if ( !onPeriod ) {
                if ( val >= meanAll ) {
                    onPeriod = true;
                    periodBegin = ( double ) ( i * pcmStep ) / frequency / channels;
                }
            } else {
                if ( val < meanAll ) {
                    onPeriod = false;
                    periodEnd = ( double ) ( i * pcmStep ) / frequency / channels;
                    periods.append( Period( PERIOD_TYPE_DANGER, periodBegin, periodEnd ) );
                }
            }
        }

        bool dirty = true;
        while ( dirty ) {
            bool foundPeriod = false;
            for ( int i = 0 ; i < periods.length() - 1 ; i++ ) {
                Period *currentPeriod = &periods[i];
                Period *nextPeriod = &periods[i + 1];

                if ( ( nextPeriod->periodBegin - currentPeriod->periodEnd ) < 3.0 ) {
                    foundPeriod = true;
                    currentPeriod->periodEnd = nextPeriod->periodEnd;
                    periods.removeAt( i + 1 );
                    i--;
                }
            }

            if ( !foundPeriod ) {
                dirty = false;
            }
        }

        for ( int i = 0 ; i < periods.length() ; i++ ) {
            Period *currentPeriod = &periods[i];
            if ( i == 0 ) {
                if ( currentPeriod->periodBegin > 0.0 ) {
                    if ( currentPeriod->periodBegin > 15.0 ) {
                        periods.prepend( Period( PERIOD_TYPE_CAUTION, 15.0, currentPeriod->periodBegin ) );
                        periods.prepend( Period( PERIOD_TYPE_SAFE, 0.0, 15.0 ) );
                        i++;
                        continue;
                    } else {
                        periods.prepend( Period( PERIOD_TYPE_CAUTION, 0.0, currentPeriod->periodBegin ) );
                        continue;
                    }
                }
            }

            if ( i < periods.length() - 1 ) {
                Period *nextPeriod = &periods[i + 1];
                if ( nextPeriod->periodBegin - currentPeriod->periodEnd >= 15.0 ) {
                    periods.insert( i + 1, Period( PERIOD_TYPE_CAUTION, currentPeriod->periodEnd + 15.0, nextPeriod->periodBegin ) );
                    periods.insert( i + 2, Period( PERIOD_TYPE_SAFE, currentPeriod->periodEnd, currentPeriod->periodEnd + 15.0 ) );
                    i += 2;
                    continue;
                } else {
                    periods.insert( i + 1, Period( PERIOD_TYPE_CAUTION, currentPeriod->periodEnd, nextPeriod->periodBegin ) );
                    i++;
                    continue;
                }
            }
        }

        if ( periods.length() > 0 ) {
            double audioDuration = this->getAudioDuration();
            if ( periods.last().periodEnd < audioDuration ) {
                periods.append( Period( PERIOD_TYPE_SAFE, periods.last().periodEnd, audioDuration ) );
            }
        }


        for ( int i = 0 ; i < periods.length() ; i++ ) {
            Period period = periods.at( i );
            out << period.periodType << ", " << period.periodBegin << ", " << period.periodEnd << endl;
        }

        outFile.close();
    }
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
