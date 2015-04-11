#ifndef SAMPLEBLOCKWIDGET_H
#define SAMPLEBLOCKWIDGET_H

#include <QMainWindow>
#include "audio.h"
#include "sampleblockplot.h"

namespace Ui {
    class SampleBlockWidget;
}

class SampleBlockWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit                            SampleBlockWidget( QWidget *parent = 0 );
    ~SampleBlockWidget();

    void                                setAudio( Audio *audio );
    void                                initPlot();

public slots:
    void                                setSampleBlockSize( int sampleBlockSize );

private:
    Ui::SampleBlockWidget               *ui;

    Audio                               *audio;
    QVector<float>                      pcmBlock;
    int                                 sampleBlockIndex;
    int                                 sampleBlockCount;
    int                                 sampleBlockSize;

    void                                updateCurrentSampleBlockLabel();


    void                                correlateDFT();

private slots:
    void                                setSampleBlockSize( const QString &sampleBlockSizeString );
    void                                setSampleBlock( int index );

};

#endif // SAMPLEBLOCKWIDGET_H
