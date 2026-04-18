#include "Cpp_Module.h"

Cpp_Module::Cpp_Module(QObject *parent)
    : QObject{parent}
{
    this->p_md = new Map_md(this);
    this->create_th();
}

Map_md *Cpp_Module::get_p_md()
{
    return this->p_md;
}

void Cpp_Module::create_th()
{
    this->p_th = new QThread();
    this->p_wk = new WK_loop();
    this->p_wk->set_p_md(this->p_md);
    this->p_wk->set_p_module(this);

    connect(this->p_th, &QThread::started, this->p_wk, &WK_loop::loop);
    connect(this->p_th, &QThread::finished, this->p_wk, &WK_loop::deleteLater);
    connect(this->p_th, &QThread::finished, this->p_th, &QThread::deleteLater);

    this->p_wk->moveToThread(p_th);
    p_th->start();
    return;
}
