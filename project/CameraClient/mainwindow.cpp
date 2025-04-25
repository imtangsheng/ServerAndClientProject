#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <emscripten/emscripten.h>
#include <QByteArray>

void WasmSettings::setValue(const QString &key, const QString &value) {
    QByteArray js = QString("localStorage.setItem('%1', '%2');")
    .arg(key.toHtmlEscaped(), value.toHtmlEscaped()).toUtf8();
    emscripten_run_script(js.constData());
}

QString WasmSettings::getValue(const QString &key, const QString &defaultValue) {
    QByteArray js = QString(R"(
        (function(){
            var val = localStorage.getItem('%1');
            return val === null ? '%2' : val;
        })()
    )").arg(key.toHtmlEscaped(), defaultValue.toHtmlEscaped()).toUtf8();

    char *result = emscripten_run_script_string(js.constData());
    return QString::fromUtf8(result);
}

void WasmSettings::remove(const QString &key) {
    QByteArray js = QString("localStorage.removeItem('%1');").arg(key.toHtmlEscaped()).toUtf8();
    emscripten_run_script(js.constData());
}

void WasmSettings::clear() {
    emscripten_run_script("localStorage.clear();");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(g_language == en_US)
    {
        qApp->removeTranslator(&g_translator);
        g_language = zh_CN;
        qDebug()<<"当前显示语言Languages："<<g_language;
    }else{
        if(g_translator.load(QString(":/i18n/%1").arg(en_US))){
            // ui->pushButton_language_switch->setText("English");
            g_language = en_US;
            qApp->installTranslator(&g_translator);
            qDebug()<<"当前显示语言Languages："<<g_language;
        }else{
            // show_message(tr("fail language switch:%1").arg(en_US), LogLevel::Warning);
        }
    }
    // gSettings->setValue("language",g_language);
    WasmSettings::setValue("language",g_language);
    ui->retranslateUi(this);
    // qDebug() << "读取配置中的语言"<<gSettings->value("language").toString();
}

