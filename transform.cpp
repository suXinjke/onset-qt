#include "transform.h"

QVector<float> Transform::correlateDFT( const QVector<float> &pcmBlock ) {
    int N = pcmBlock.length(); //must be a power of 2

    QVector<float> re( ( N / 2 ) + 1 );
    QVector<float> im( ( N / 2 ) + 1 );
    re.fill( 0.0 );
    im.fill( 0.0 );

    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
        for ( int i = 0 ; i < N ; i++ ) {
            re[k] += pcmBlock[i] * qCos( 2 * M_PI * k * i / N );
            im[k] += pcmBlock[i] * -qSin( 2 * M_PI * k * i / N );
        }
    }

    //polar
    QVector<float> mag( ( N / 2 ) + 1 );

    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
        mag[k] = qSqrt( re[k] * re[k] + im[k] * im[k] );
        if ( re[k] == 0.0 ) {
            re[k] = 1e-20;
        }
    }

    return mag;
}

QVector<float> Transform::FFT( const QVector<float> &pcmBlock ) {
    int N = pcmBlock.length();
    if ( N <= 1 ) {
        return pcmBlock;
    }

    QElapsedTimer timer;
    timer.start();

    QVector< std::complex<float> > x;
    for ( int i = 0 ; i < pcmBlock.size() ; i++ ) {
        x.append( std::complex<float>( pcmBlock.at( i ) ) );
    }

    FFT( x );

    QVector<float> mag( ( N / 2 ) + 1 );

    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
        mag[k] = qSqrt( x[k].real() * x[k].real() + x[k].imag() * x[k].imag() );
        if ( x[k].real() == 0.0 ) {
            x[k].real() = 1e-20;
        }
    }

    qDebug() << timer.elapsed();

    return mag;
}

void Transform::FFT( QVector<std::complex<float> > &x ) {
    int N = x.length();
    if ( N <= 1 ) {
        return;
    }

    QVector< std::complex<float> > even;
    QVector< std::complex<float> > odd;
    for ( int i = 0 ; i < x.length() ; i++ ) {
        if ( i % 2 == 0 ) {
            even.append( x.at( i ) );
        } else {
            odd.append( x.at( i ) );
        }
    }

    FFT( even );
    FFT( odd );

    for ( int k = 0 ; k < N / 2 ; k++ ) {
        std::complex<float> t = ( std::complex<float> ) std::polar( 1.0, -2 * M_PI * k / N ) * odd.at( k );
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}

//QVector<float> Transform::FFT( const QVector<float> &pcmBlock ) {
//    int N = pcmBlock.length();
//    int NM1 = N - 1;
//    int ND2 = N / 2;
//    int M = qLn( N ) / qLn( 2 );
//    int j = ND2;

//    QVector<float> re( pcmBlock );
//    QVector<float> im( N );
//    im.fill( 0 );

//    for ( int i = 1 ; i < N - 2 ; i ++ ) {
//        if ( i < j ) {
//            float tr = re[j];
//            float ti = im[j];
//            re[j] = re[i];
//            im[j] = im[i];
//            re[i] = tr;
//            im[i] = ti;
//        }
//        int k = ND2;
//        while ( k <= j ) {
//            j -= k;
//            k /= 2;
//        }
//        j += k;
//    }

//    for ( int l = 1 ; l < M ; l++ ) {
//        int le = ( int ) qPow( 2, l );
//        int le2 = le / 2;
//        float ur = 1;
//        float ui = 0;
//        float sr = qCos( M_PI / le2 );
//        float si = -qSin( M_PI / le2 );
//        for ( int j = 1 ; j < le2 ; j++ ) {
//            int jm1 = j - 1;
//            for ( int i = jm1 ; i < NM1 ; i += le ) {
//                int ip = i + le2;
//                float tr = re[ip] * ur - im[ip] * ui;
//                float ti = re[ip] * ui + im[ip] * ur;
//                re[ip] = re[i] - tr;
//                im[ip] = im[i] - ti;
//                re[i] += tr;
//                im[i] += ti;
//            }
//            float tr = ur;
//            ur = tr * sr - ui * si;
//            ui = tr * si + ui * sr;
//        }
//    }

//    re.resize( re.length() / 2 + 1 );
//    im.resize( im.length() / 2 + 1 );

//    //polar
//    QVector<float> mag( ( N / 2 ) + 1 );

//    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
//        mag[k] = qSqrt( re[k] * re[k] + im[k] * im[k] );
//        if ( re[k] == 0.0 ) {
//            re[k] = 1e-20;
//        }
//    }

//    return mag;
//}

float Transform::getSpectrumFlux( QVector<float> &pcmBlock, QVector<float> &nextPcmBlock ) {

    pcmBlock = FFT( pcmBlock );
    nextPcmBlock = FFT( nextPcmBlock );

    //hamming
    hamming( pcmBlock );
    hamming( nextPcmBlock );

    float flux = 0.0;
    for ( int i = 0 ; i < nextPcmBlock.length() ; i++ ) {
        float value = nextPcmBlock.at( i ) - pcmBlock.at( i );
        flux += value < 0 ? 0 : value;
    }

    return flux;
}

void Transform::hamming( QVector<float> &pcmBlock ) {
    for ( int i = 0 ; i < pcmBlock.length() ; i++ ) {
        pcmBlock[i] *= ( 0.54f - 0.46f * qCos( 2 * M_PI * i / ( pcmBlock.length() - 1 ) ) );
    }
}

