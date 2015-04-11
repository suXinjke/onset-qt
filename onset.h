#ifndef ONSET_H
#define ONSET_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include "audio.h"
#include "audioplot.h"
#include "sampleblockwindow.h"

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
    SampleBlockWindow                   *sampleBlockWindow;

    Audio                               *audio;
    QTimer                              *seekTimer;
    int                                 lastAutoAudioPlotAccuracyIndex;

    double                              audioDuration;
    void                                updateSeekSlider( double audioPosition );
    void                                updateSeekLabel( double audioPosition );
    void                                updateAudioTitleLabel();
    void                                loadAudioFile( const QString &audioFilePath );

private slots:
    void                                bringSampleBlockWidget();
    void                                loadAudioFile();
    void                                generateAudio();
    void                                regenerateAudio();
    void                                play();
    void                                pause();
    void                                stop();
    void                                seek( int seconds );
    void                                seek( double seconds );
    void                                updateSeekInfo();
    void                                reloadAudioPlot();
    void                                updateAudioPlotSampleAccuracy( int accuracyIndex );
    void                                updateAudioPlotSampleAccuracy( bool autoChecked );




};

#endif // ONSET_H
