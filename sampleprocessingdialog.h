#ifndef SAMPLEPROCESSINGDIALOG_H
#define SAMPLEPROCESSINGDIALOG_H

#include <QDialog>
#include <QMovie>

namespace Ui {
    class SampleProcessingDialog;
}

class SampleProcessingDialog : public QDialog {
    Q_OBJECT

public:
    explicit                            SampleProcessingDialog( QWidget *parent = 0 );
    ~SampleProcessingDialog();

public slots:
    void                                setSamplesToProcess( int samples );
    void                                setSamplesProcessed( int samples );
    void                                addSampleProcessed( int sampleProcessed = 1 );

private:
    Ui::SampleProcessingDialog          *ui;
    QMovie                              *gif;
};

#endif // SAMPLEPROCESSINGDIALOG_H
