#ifndef CPP_MODULE_H
#define CPP_MODULE_H

#include <chrono>
#include <csignal>
#include <sys/mman.h>
#include <thread>

//==========================================

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
//====================C++===================

#include <ctype.h>
#include <fcntl.h> // open()
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h> // mode_t
#include <sys/types.h>
#include <unistd.h> // write(), close()
//=====================C=====================

#include <QObject>
#include <QThread>
#include "Map_md.h"
#include "WK_loop.h"

using namespace std;

class Cpp_Module : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Map_md *p_md READ get_p_md CONSTANT)
public:
    explicit Cpp_Module(QObject *parent = nullptr);

    Map_md *get_p_md();
    void create_th();

public slots:

signals:

private:
    Map_md *p_md;
    QThread *p_th;
    WK_loop *p_wk;
};

#endif // CPP_MODULE_H
