#include "ck2ilogwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CK2iLogWindow w;
    w.show();

    return a.exec();
}
