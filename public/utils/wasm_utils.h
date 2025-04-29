#ifndef WASM_UTILS_H
#define WASM_UTILS_H
#include <emscripten/emscripten.h>
#include <QByteArray>

inline void LocalValueSet(const QString& key,const QString& value){
    QByteArray js = QString("localStorage.setItem('%1', '%2');")
    .arg(key.toHtmlEscaped(), value.toHtmlEscaped()).toUtf8();
    emscripten_run_script(js.constData());
}
inline QString LocalValueGet(const QString &key, const QString &defaultValue = QString()){
    QByteArray js = QString(R"(
        (function(){
            var val = localStorage.getItem('%1');
            return val === null ? '%2' : val;
        })()
    )").arg(key.toHtmlEscaped(), defaultValue.toHtmlEscaped()).toUtf8();

    char *result = emscripten_run_script_string(js.constData());
    return QString::fromUtf8(result);
}
inline void LocalValueRemove(const QString& key){
    QByteArray js = QString("localStorage.removeItem('%1');").arg(key.toHtmlEscaped()).toUtf8();
    emscripten_run_script(js.constData());
}
inline void LocalValueClear(){
    emscripten_run_script("localStorage.clear();");
}
#endif // WASM_UTILS_H
