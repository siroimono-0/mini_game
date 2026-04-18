#ifndef WK_LOOP_H
#define WK_LOOP_H

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
using namespace std;
class Map_md;
class Cpp_Module;
struct shm_arr
{
    char arr[13][15];
};

class WK_loop : public QObject
{
    Q_OBJECT
public:
    explicit WK_loop(QObject *parent = nullptr);
    ~WK_loop();

    void col_compare(string &col, const char &tmp_c);
    static void mq_th_func(union sigval sv);
    void loop();

    void set_p_md(Map_md *set);
    void set_p_module(Cpp_Module *set);

signals:

private:
    Map_md *p_md;
    Cpp_Module *p_module;

    QVector<QVector<char>> vec_prev;
    QVector<QPair<QPair<int, int>, char>> vec_compare;
};
class Exception
{
private:
    string err_type;
    string err_name;
    int err_no;

public:
    Exception(const string &err_type, string &err_name, int err_no)
    {
        this->err_type = err_type;
        this->err_name = err_name;
        this->err_no = err_no;
    }

    ~Exception() {}

    const string &get_err_type() const { return this->err_type; }
    const string &get_err_name() const { return this->err_name; }
    const int &get_err_no() const { return this->err_no; }
};

class check_err
{
private:
public:
    check_err() {}

    ~check_err() {}

    template<typename T>
    static int check(const char *name, T ret, T f_value)
    {
        if (ret == f_value)
        {
            string s_name = name;
            string err_name = strerror(errno);
            Exception err(s_name, err_name, errno);
            throw err;
        }
        printf("success %s\n", name);
        return 0;
    }

    template<typename T>
    static int check_ENOENT(const char *name, T ret, T f_value)
    {
        if (ret == f_value && errno != ENOENT)
        {
            string s_name = name;
            string err_name = strerror(errno);
            Exception err(s_name, err_name, errno);
            throw err;
        }
        printf("success %s\n", name);
        return 0;
    }

    template<typename T>
    static int check(const string name, T ret, T f_value)
    {
        if (ret == f_value)
        {
            string s_name = name;
            string err_name = strerror(errno);
            Exception err(s_name, err_name, errno);
            throw err;
        }
        printf("success %s\n", name.c_str());
        return 0;
    }
};

#endif // WK_LOOP_H
