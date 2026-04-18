#pragma once
//==========================================

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>
//====================C++===================

#include <ctype.h>
#include <fcntl.h> // open()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // mode_t
#include <sys/types.h>
#include <unistd.h> // write(), close()
//=====================C=====================

using namespace std;

class Exception
{
private:
  string err_type;
  string err_name;
  int err_no;

public:
  Exception(const string &err_type, string &err_name, int err_no);
  ~Exception();

  const string &get_err_type() const;
  const string &get_err_name() const;
  const int &get_err_no() const;
};

class RAII_fd
{
private:
  int fd;
  string name;

public:
  RAII_fd(int fd, const string &name);
  ~RAII_fd();

  int get_fd();

  int set_shm_fd(int fd, const string &name);
};

class check_err
{
private:
public:
  check_err();
  ~check_err();

  template <typename T> static int check(const char *name, T ret, T f_value)
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

  template <typename T>
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

  template <typename T> static int check(const string name, T ret, T f_value)
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
  //===================== ~Class =====================//

  template <typename T> static int check_end(const char *name, T ret, T f_value)
  {
    string s_name = name;
    string err_name = strerror(errno);
    Exception err(s_name, err_name, errno);

    if (ret == f_value)
    {
      printf("err type == %s\n"
             "err name == %s\n"
             "err no ==%d\n",

             err.get_err_type().c_str(), err.get_err_name().c_str(),
             err.get_err_no());
    }
    else
    {
      printf("success %s\n", name);
    }
    return 0;
  }

  template <typename T>
  static int check_ENOENT_end(const char *name, T ret, T f_value)
  {
    string s_name = name;
    string err_name = strerror(errno);
    Exception err(s_name, err_name, errno);

    if (ret == f_value && errno != ENOENT)
    {
      printf("err type == %s\n"
             "err name == %s\n"
             "err no ==%d\n",

             err.get_err_type().c_str(), err.get_err_name().c_str(),
             err.get_err_no());
    }
    else
    {
      printf("success %s\n", name);
    }
    return 0;
  }

  template <typename T>
  static int check_end(const string name, T ret, T f_value)
  {

    string s_name = name;
    string err_name = strerror(errno);
    Exception err(s_name, err_name, errno);

    if (ret == f_value)
    {
      printf("err type == %s\n"
             "err name == %s\n"
             "err no ==%d\n",

             err.get_err_type().c_str(), err.get_err_name().c_str(),
             err.get_err_no());
    }
    else
    {
      printf("success %s\n", name.c_str());
    }
    return 0;
  }
};

class Sig_Guard
{
private:
  sigset_t sig_set_block;
  sigset_t sig_set_old;

public:
  Sig_Guard();
  ~Sig_Guard();

  void sig_lock();

  void sig_unlock();
};