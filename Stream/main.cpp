#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Cpp_Module/Cpp_Module.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    Cpp_Module cpp_module;
    engine.rootContext()->setContextProperty("cpp_module", &cpp_module);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Stream", "Main");

    return app.exec();
}
