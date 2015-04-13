#ifndef ONSET_H
#define ONSET_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include "audio.h"
#include "audioplot.h"
#include "transform.h"

namespace Ui {
    class Onset;
}

class Onset : public QMainWindow {
    Q_OBJECT

public:
    explicit                            Onset( QWidget *parent = 0 );
    ~Onset();

private:
    Ui::Onset                           *ui;

    Audio                               *audio;
    QTimer                              *seekTimer;
    int                                 lastAutoAudioPlotAccuracyIndex;

    double                              audioDuration;
    void                                updateSeekSlider( double audioPosition );
    void                                updateSeekLabel( double audioPosition );
    void                                updateAudioTitleLabel();
    void                                loadAudioFile( const QString &audioFilePath );

private slots:
    void                                loadAudioFile();
    void                                play();
    void                                pause();
    void                                stop();
    void                                seek( int seconds );
    void                                seek( double seconds );
    void                                updateSeekInfo();

    //plot
    void                                updateShowControls();
    void                                showByRadioButton();
    void                                showOnset();
    void                                showCurrentSampleBlock();




};

#endif // ONSET_H
