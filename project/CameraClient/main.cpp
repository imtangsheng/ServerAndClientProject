#include "mainwindow.h"
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/zh_CN.ttf"));
    QStringList fontFamiles = QFontDatabase::applicationFontFamilies(fontId);
    qDebug() << "fontFamiles:" <<fontFamiles[0]<< fontFamiles;
    if(fontFamiles.size() > 0){
        QFont font = app.font(); //DejaVu Sans
        qDebug() << "Browser default font:" << font.family();
        font.setFamily(fontFamiles[0]);
        app.setFont(font);
    }


    QDir appDir(QCoreApplication::applicationDirPath()); appDir.cdUp();
    qDebug() << "当前应用程序的目录：" << appDir.absolutePath();
    // gShare.InitConfigSettings(appDir.absolutePath(), "CameraClient");//初始化配置文件路径,名称

    g_language = LocalValueGet("language", "en_US");
    // g_language = gSettings->value("language").toString();
    qDebug() << "当前应用程序的语言Settings：" << g_language;
    if(g_language.isEmpty()){
        QString locale = QLocale::system().name();
        g_language = QLocale(locale).name();
    }
    if(g_language == "en_US"){
        if(g_translator.load(":/i18n/"+g_language)){
            app.installTranslator(&g_translator);
        }
    }
    gShare.session_type_ = int(SessionType::Other);
    MainWindow w;
    w.show();
    return app.exec();
}
