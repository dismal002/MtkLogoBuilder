#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QResource>
#include "mainwindow.h"
#include "logoproject.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Register qt-material resources
    QString rccPath = QCoreApplication::applicationDirPath() + "/resources.rcc";
    if (!QFile::exists(rccPath)) {
        rccPath = QCoreApplication::applicationDirPath() + "/../resources.rcc";
    }
    if (QFile::exists(rccPath)) {
        QResource::registerResource(rccPath);
    }

    // Load stylesheet
    QFile styleFile(QCoreApplication::applicationDirPath() + "/style.qss");
    if (!styleFile.exists()) {
        styleFile.setFileName(QCoreApplication::applicationDirPath() + "/lang/style.qss");
    }
    if (!styleFile.exists()) {
        // Look up one level (e.g. if running from build/ subdirectory)
        styleFile.setFileName(QCoreApplication::applicationDirPath() + "/../style.qss");
    }
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        a.setStyleSheet(styleFile.readAll());
    }
    
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("mode", "unpack or pack");
    parser.addPositionalArgument("arg1", "logo.bin or project_dir");
    parser.addPositionalArgument("arg2", "out_dir or out_logo.bin");
    
    parser.process(a);
    const QStringList args = parser.positionalArguments();
    
    if (args.size() == 3) {
        QString mode = args.at(0);
        QString arg1 = args.at(1);
        QString arg2 = args.at(2);
        
        LogoProject proj;
        if (mode == "unpack") {
            if (proj.loadProject(arg1, arg2)) {
                qDebug() << "Successfully unpacked to" << arg2;
                return 0;
            } else {
                qDebug() << "Error unpacking:" << proj.getError();
                return 1;
            }
        } else if (mode == "pack") {
            if (proj.packProject(arg1, arg2)) {
                qDebug() << "Successfully packed to" << arg2;
                return 0;
            } else {
                qDebug() << "Error packing:" << proj.getError();
                return 1;
            }
        }
    }
    
    MainWindow w;
    w.show();
    return a.exec();
}
