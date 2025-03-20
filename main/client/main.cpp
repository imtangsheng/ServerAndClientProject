#include <QApplication>
#include "mainwindow.h"
#include <QStyleFactory>
#include <QPalette>
#include <QStyle>
void SetDarkTheme(bool dark) {
    if (dark) {
        // 设置深色主题
        QApplication::setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        QApplication::setPalette(darkPalette);
    } else {
        // 设置浅色主题
        QApplication::setStyle(QStyleFactory::create("Windows"));
        QApplication::setPalette(QApplication::style()->standardPalette());
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    gSouth.type = int(SessionType::Client);
    if(gSouth.language.isEmpty()){
        QString locale = QLocale::system().name();
        gSouth.language = QLocale(locale).name();
    }
    if(g_translator.load(":/i18n/"+gSouth.language)){
        app.installTranslator(&g_translator);
    }
    // SetDarkTheme(false);
    MainWindow window;
    window.show();

    return app.exec();
}
