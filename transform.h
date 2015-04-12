#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include "qmath.h"
#include <complex>
#include <QElapsedTimer>

class Transform : public QObject {

    Q_OBJECT
public:

    static QVector<float>               correlateDFT( const QVector<float> &pcmBlock );
    static QVector<float>               FFT( const QVector<float> &pcmBlock );
    static void                         FFT( QVector<std::complex< float > > &x );
    static float                        getSpectrumFlux( QVector<float> &pcmBlock , QVector<float> &nextPcmBlock );
    static void                         hamming( QVector<float> &pcmBlock );

};

#endif // TRANSFORM_H