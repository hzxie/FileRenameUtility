#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

/*!
 * \brief main The enterance of the application
 * \param argc number of arguments
 * \param argv the list of arguments
 * \return the exit code of the application (to OS)
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator appTranslator;
    QString qmFile = ":/sca_" + QLocale::system().name() + ".qm";
    appTranslator.load(qmFile);
    app.installTranslator(&appTranslator);

    MainWindow window;
    window.show();

    return app.exec();
}
