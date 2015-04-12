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

    std::valarray< std::complex<float> > x( N );
    for ( int i = 0 ; i < pcmBlock.size() ; i++ ) {
        x[i] = pcmBlock.at( i );
    }

    FFT( x );

    QVector<float> mag( ( N / 2 ) + 1 );

    for ( int k = 0 ; k < ( N / 2 ) + 1 ; k++ ) {
        mag[k] = qSqrt( x[k].real() * x[k].real() + x[k].imag() * x[k].imag() );
        if ( x[k].real() == 0.0 ) {
            x[k].real() = 1e-20;
        }
    }

    return mag;
}

void Transform::FFT( std::valarray< std::complex<float> > &x ) {
    int N = x.size();
    if ( N <= 1 ) {
        return;
    }

    std::valarray< std::complex<float> > even = x[ std::slice( 0, N / 2, 2 ) ];
    std::valarray< std::complex<float> > odd = x[ std::slice( 1, N / 2, 2 ) ];

    FFT( even );
    FFT( odd );

    for ( int k = 0 ; k < N / 2 ; k++ ) {
        std::complex<float> t = ( std::complex<float> ) std::polar( 1.0, -2 * M_PI * k / N ) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}

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

float Transform::getSpectrumFlux( float *block, float *nextBlock ) {
    float flux = 0.0;
    for ( int i = 0 ; i < 256 ; i++ ) {
        float value = nextBlock[i] - block[i];
        flux += value < 0 ? 0 : value;
    }

    return flux;
}

void Transform::hamming( QVector<float> &pcmBlock ) {
    for ( int i = 0 ; i < pcmBlock.length() ; i++ ) {
        pcmBlock[i] *= ( 0.54f - 0.46f * qCos( 2 * M_PI * i / ( pcmBlock.length() - 1 ) ) );
    }
}

