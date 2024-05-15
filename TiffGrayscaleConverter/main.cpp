#include "mainwindow.h"

#include <QApplication>
#include <omp.h>

int main(int argc, char *argv[])
{
    omp_set_num_threads(omp_get_num_procs());
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
