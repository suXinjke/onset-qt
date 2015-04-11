#ifndef SAMPLEBLOCKPLOT_H
#define SAMPLEBLOCKPLOT_H

#include "qcustomplot.h"

enum SAMPLE_BLOCK_PLOT_SETUP {
    SAMPLE_BLOCK,
    REAL_FREQ_DOMAIN,
    IMAGINARY_FREQ_DOMAIN,
    MAGNITUDE_FREQ_DOMAIN,
    PHASE_FREQ_DOMAIN
};

class SampleBlockPlot : public QCustomPlot {
    Q_OBJECT

public:

    explicit                            SampleBlockPlot( QWidget *parent = 0 );

    void                                loadPCMData( const QVector<float> pcm );
    void                                setupPlot( SAMPLE_BLOCK_PLOT_SETUP setup );

public slots:
    void                                resetRange();
    void                                resetVerticalRange();

private:
    QCPGraph                            *pcmGraph;

    int                                 sampleCount;

    QVector<double>                     x;
    QVector<double>                     y;

private slots:
    void                                calculateFrequencyTicks();
    void                                calculateReadableFrequencyTicks();



};

#endif // SAMPLEBLOCKPLOT_H
