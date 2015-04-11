#include "onset.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Onset w;
    w.show();

    return a.exec();
}
