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

    QDir appDir(QCoreApplication::applicationDirPath()); appDir.cdUp();
    qDebug() << "当前应用程序的目录：" << appDir.absolutePath();
    gSouth.InitConfigSettings(appDir.absolutePath(), "client");//初始化配置文件路径,名称

    g_language = gSettings->value("language").toString();
    qDebug() << "当前应用程序的语言：" << g_language;
    if(g_language.isEmpty()){
        QString locale = QLocale::system().name();
        g_language = QLocale(locale).name();
    }
    if(g_translator.load(":/i18n/"+g_language)){
        app.installTranslator(&g_translator);
    }
    // SetDarkTheme(false);

    gSouth.sessiontype_ = int(SessionType::Client);
    MainWindow window;
    window.show();

    return app.exec();
}
