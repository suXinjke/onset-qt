#ifndef SAMPLEBLOCKWINDOW_H
#define SAMPLEBLOCKWINDOW_H

#include <QMainWindow>
#include <complex>
#include "audio.h"
#include "sampleblockplot.h"

namespace Ui {
    class SampleBlockWindow;
}

class SampleBlockWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit                            SampleBlockWindow( QWidget *parent = 0 );
    ~SampleBlockWindow();

    void                                setAudio( Audio *audio );
    void                                initPlot();

public slots:
    void                                setSampleBlockSize( int sampleBlockSize );

private:
    Ui::SampleBlockWindow               *ui;

    Audio                               *audio;
    QVector<float>                      pcmBlock;
    int                                 sampleBlockIndex;
    int                                 sampleBlockCount;
    int                                 sampleBlockSize;

    void                                updateCurrentSampleBlockLabel();

private slots:
    void                                setSampleBlockSize( const QString &sampleBlockSizeString );
    void                                setSampleBlock( int index );
};

#endif // SAMPLEBLOCKWINDOW_H
