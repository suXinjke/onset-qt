#include "sampleprocessingdialog.h"
#include "ui_sampleprocessingdialog.h"

SampleProcessingDialog::SampleProcessingDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::SampleProcessingDialog ) {

    ui->setupUi( this );

    gif = new QMovie( ":/dance.gif", "GIF", this );
    ui->label->setMovie( gif );
    gif->start();
}

SampleProcessingDialog::~SampleProcessingDialog() {
    delete ui;
}

void SampleProcessingDialog::setSamplesToProcess( int samples ) {
    ui->progressBar->setMaximum( samples );
}

void SampleProcessingDialog::setSamplesProcessed( int samples ) {
    ui->progressBar->setValue( samples );

    QApplication::processEvents();
}

void SampleProcessingDialog::addSampleProcessed( int sampleProcessed ) {
    int value = ui->progressBar->value() + sampleProcessed;
    this->setSamplesProcessed( value );
}
