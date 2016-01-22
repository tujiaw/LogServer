#include "dialog.h"
#include <QApplication>
#include <ctime>
int main(int argc, char *argv[])
{
    time(NULL);
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
