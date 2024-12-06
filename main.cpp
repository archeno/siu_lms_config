#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStandardItem>
#include <QTextStream>

Q_DECLARE_METATYPE(QList<QStandardItem *>)
void loadStyleSheet(QApplication &app, QString styleSheetPath)
{
    QFile file(styleSheetPath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream outStream(&file);
        QString styleStr = outStream.readAll();
        file.close();
        app.setStyleSheet(styleStr);
    } else {
        qDebug() << "load styleSheet failed!" << endl;
    }
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loadStyleSheet(a, ":/styles/main.qss");
    qRegisterMetaType<QList<QStandardItem *>>("QList<QStandardItem*>");
    MainWindow w;
    w.resize(1200, 600);
    w.move(400, 10);
    w.setWindowTitle("SIU/LMS配置工具v1.1");
    w.show();
    return a.exec();
}
