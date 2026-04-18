#include "Support.h"
#include <csignal>

Exception::Exception(const string &err_type, string &err_name, int err_no)
{
  this->err_type = err_type;
  this->err_name = err_name;
  this->err_no = err_no;
}

Exception::~Exception()
{
}

const string &Exception::get_err_type() const
{
  return this->err_type;
}
const string &Exception::get_err_name() const
{
  return this->err_name;
}
const int &Exception::get_err_no() const
{
  return this->err_no;
}

RAII_fd::RAII_fd(int fd, const string &name)
{
  this->fd = fd;
  this->name = name;
  check_err::check(this->name, this->fd, -1);
}

RAII_fd::~RAII_fd()
{
  int ret_close = close(fd);

  string end_msg = "close() " + this->name;
  check_err::check(end_msg, ret_close, -1);
}

int RAII_fd::get_fd()
{
  return this->fd;
}

int RAII_fd::set_shm_fd(int fd, const string &name)
{
  return 0;
}

check_err::check_err()
{
}

check_err::~check_err()
{
}

/*
template <typename T> int check_err::check(const char *name, T ret, T f_value)
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
int check_err::check_ENOENT(const char *name, T ret, T f_value)
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

template <typename T> int check_err::check(const string name, T ret, T f_value)
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
*/

Sig_Guard::Sig_Guard()
{
  sigemptyset(&this->sig_set_block);
}

Sig_Guard::~Sig_Guard()
{
  int ret_sigprocmask = sigprocmask(SIG_UNBLOCK, &this->sig_set_block, NULL);
  check_err::check("SIG_UNLOCK", ret_sigprocmask, -1);
}

void Sig_Guard::sig_lock()
{
  cout << "Lock tid : " << gettid() << endl;
  sigfillset(&this->sig_set_block);
  int ret_sigprocmask = sigprocmask(SIG_BLOCK, &this->sig_set_block, NULL);
  check_err::check("SIG_LOCK", ret_sigprocmask, -1);
}

void Sig_Guard::sig_unlock()
{
  cerr << "Un Lock tid : " << gettid() << endl;
  sigfillset(&this->sig_set_block);
  int ret_sigprocmask = sigprocmask(SIG_UNBLOCK, &this->sig_set_block, NULL);
  check_err::check("SIG_UNLOCK", ret_sigprocmask, -1);
}