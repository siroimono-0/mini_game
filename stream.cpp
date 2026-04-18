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

using namespace std;

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

  ~Exception()
  {
  }

  const string &get_err_type() const
  {
    return this->err_type;
  }
  const string &get_err_name() const
  {
    return this->err_name;
  }
  const int &get_err_no() const
  {
    return this->err_no;
  }
};

class check_err
{
private:
public:
  check_err()
  {
  }

  ~check_err()
  {
  }

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
};

struct shm_arr
{
  char arr[13][15];
};

void col_compare(string &col, const char &tmp_c)
{
  string red = "\033[31m";  // 1 물풍선 겹침
  string blue = "\033[34m"; // 2 ''

  string green = "\033[32m";  // @ 수풀
  string pupple = "\033[35m"; // 못부시는 물체

  string sky = "\033[36m"; // 부실 수 있는 물체

  string reset = "\033[0m"; // 부실 수 있는 물체

  if (tmp_c == '3')
  {
    col = red;
  }
  else if (tmp_c == '4')
  {
    col = blue;
  }
  else if (tmp_c == '^' || tmp_c == '&')
  {
    col = pupple;
  }
  else if (tmp_c == '@')
  {
    col = green;
  }
  else if (tmp_c == '#' || tmp_c == '%')
  {
    col = sky;
  }
  return;
}

void mq_th_func(union sigval sv)
{
  cout << "\033[2J\033[H";
  bool *game_end = (bool *)sv.sival_ptr;
  *game_end = true;
}

int main()
{
  try
  {
    const char *red = "\033[31m";  // 1 물풍선 겹침
    const char *blue = "\033[34m"; // 2 ''

    const char *green = "\033[32m";  // @ 수풀
    const char *pupple = "\033[35m"; // 못부시는 물체

    const char *sky = "\033[36m"; // 부실 수 있는 물체

    const char *bk_red = "\033[41m";

    const char *white = "\033[37m"; // 부실 수 있는 물체

    const char *reset = "\033[0m"; // 부실 수 있는 물체
    int shm_fd = shm_open("/shm_map", O_RDONLY, 0);
    check_err::check("shm_open()", shm_fd, -1);

    struct shm_arr *shm_p =
        (struct shm_arr *)mmap(NULL, 4096, PROT_READ, MAP_SHARED, shm_fd, 0);
    check_err::check("mmap()", (void *)shm_p, (void *)-1);

    int fps = 30;
    int fps_ms = 1000 / fps;
    int win_who = 0;

    bool game_end = false;

    int mq_fd_rev = mq_open("/mq_game_send", O_RDWR);
    check_err::check("mq_open()", mq_fd_rev, -1);

    int mq_fd_send = mq_open("/mq_game_rev", O_WRONLY);
    check_err::check("mq_open()", mq_fd_send, -1);

    union sigval sv = {};
    sv.sival_ptr = (void *)&game_end;

    struct sigevent st_sigevn = {};
    st_sigevn.sigev_notify = SIGEV_THREAD;
    st_sigevn.sigev_value = sv;
    st_sigevn.sigev_notify_function = mq_th_func;

    int ret_mq_nf = mq_notify(mq_fd_rev, &st_sigevn);
    check_err::check("mq_notify()", ret_mq_nf, -1);

    while (!game_end)
    {
      // int ret_ep_wait = epoll_wait(fd_epoll, &st_ep_evn_wait, 1, 0);
      auto t_start = chrono::steady_clock::now();

      for (int i = 0; i < 13; i++)
      {
        for (int j = 0; j < 15; j++)
        {
          char tmp_c = (*shm_p).arr[i][j];
          // col_compare(col, tmp_c);

          if (tmp_c == '3')
          {
            printf("%s%c%s", red, tmp_c, reset);
          }
          else if (tmp_c == '4')
          {
            printf("%s%c%s", blue, tmp_c, reset);
          }
          else if (tmp_c == '^' || tmp_c == '&')
          {
            printf("%s%c%s", pupple, tmp_c, reset);
          }
          else if (tmp_c == '@')
          {
            printf("%s%c%s", green, tmp_c, reset);
          }
          else if (tmp_c == '#' || tmp_c == '%')
          {
            printf("%s%c%s", sky, tmp_c, reset);
          }
          else if (tmp_c == 'G')
          {
            printf("%s%s%c%s", bk_red, white, tmp_c, reset);
          }
          else
          {
            printf("%c", tmp_c);
          }
        }
        printf("\n");
      }

      auto t_end = chrono::steady_clock::now();
      auto off =
          chrono::duration_cast<chrono::milliseconds>(t_end - t_start).count();

      int i_sleep = fps_ms - off;
      // int i_sleep = 1000;

      if (i_sleep > 0)
      {
        this_thread::sleep_for(chrono::milliseconds(i_sleep));
      }
      cout << "\033[2J\033[H";
    }

    for (int i = 0; i < 13; i++)
    {
      for (int j = 0; j < 15; j++)
      {
        char tmp_c = (*shm_p).arr[i][j];
        // col_compare(col, tmp_c);

        if (tmp_c == '3')
        {
          printf("%s%c%s", red, tmp_c, reset);
        }
        else if (tmp_c == '4')
        {
          printf("%s%c%s", blue, tmp_c, reset);
        }
        else if (tmp_c == '^' || tmp_c == '&')
        {
          printf("%s%c%s", pupple, tmp_c, reset);
        }
        else if (tmp_c == '@')
        {
          printf("%s%c%s", green, tmp_c, reset);
        }
        else if (tmp_c == '#' || tmp_c == '%')
        {
          printf("%s%c%s", sky, tmp_c, reset);
        }
        else if (tmp_c == 'G')
        {
          printf("%s%s%c%s", bk_red, white, tmp_c, reset);
        }
        else
        {
          printf("%c", tmp_c);
        }
      }
      printf("\n");
    }

    sleep(3);

    cout << "\033[2J\033[H";

    char mq_buf[64] = {};

    int ret_mq_rev = mq_receive(mq_fd_rev, mq_buf, 64, nullptr);
    check_err::check("mq_receive()", ret_mq_rev, -1);

    string mq_buf_cnv(mq_buf, ret_mq_rev);
    cout << mq_buf_cnv << endl;

    const char *game_over = "Game Over ...\n";
    const char *win_p;

    if (mq_buf_cnv == "1")
    {
      win_p = "Win Player :: 1\n";
    }
    else if (mq_buf_cnv == "2")
    {
      win_p = "Win Player :: 2\n";
    }
    else if (mq_buf_cnv == "3")
    {
      win_p = "Draw Game \n";
    }

    for (int i = 0; i < 10; i++)
    {
      cout << win_p << endl;
      this_thread::sleep_for(chrono::milliseconds(300));
    }

    sleep(3);

    string mq_send_buf = "end";
    int ret_mq_send =
        mq_send(mq_fd_send, mq_send_buf.c_str(), mq_send_buf.size(), 0);
    check_err::check("mq_send()", ret_mq_send, -1);

    int ret_mq_cl_send = mq_close(mq_fd_send);
    check_err::check("mq_close(send)", ret_mq_cl_send, -1);

    int ret_mq_cl_rev = mq_close(mq_fd_rev);
    check_err::check("mq_close(rev)", ret_mq_cl_rev, -1);

    int ret_munmap = munmap(shm_p, 4096);
    check_err::check("munmap()", ret_munmap, -1);

    int ret_close = close(shm_fd);
    check_err::check("close()", ret_close, -1);
  }
  catch (Exception err)
  {
    printf("err type == %s\n"
           "err name == %s\n"
           "err no ==%d\n",
           err.get_err_type().c_str(), err.get_err_name().c_str(),
           err.get_err_no());
    return 0;
  }
  return 0;
}
