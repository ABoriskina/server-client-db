#include <QApplication>
#include <syslog.h>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
    openlog("practice1_client", LOG_PID | LOG_DAEMON, 0);
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    int result = app.exec();
    closelog();
    return result;
}