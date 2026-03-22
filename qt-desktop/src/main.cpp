#include <QApplication>
#include "ui/main_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ATSC 3.0 Signaling Calculator");
    MainWindow w;
    w.show();
    return app.exec();
}
