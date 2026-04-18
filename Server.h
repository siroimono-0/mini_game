#pragma once
//==========================================
#include "Character.h"
#include "gost.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <ncurses.h>
#include <stack>
#include <string>
#include <vector>
//====================C++===================

#include <chrono>
#include <cstdlib>
#include <ctype.h>
#include <fcntl.h> // open()
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/stat.h> // mode_t
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h> // write(), close()
#include <unordered_map>
//=====================C=====================

using namespace std;

class Sv
{
private:
  struct mq_attr st_mq_attr = {};

  vector<vector<char>> village;
  int shm_fd;
  struct shm_arr *shm_p;
  int epoll_fd;
  int event_fd;
  mqd_t mq_fd_send;
  mqd_t mq_fd_rev;

  pid_t tid_main;
  pthread_t tid_boom;
  pthread_t tid_gost;
  pthread_t tid_last;

  bool tid_main_set = true;
  bool tid_boom_set = true;
  bool tid_gost_set = true;

  bool game_over_main_th = false;

  Ch &ch;
  Gost gost;

  int gost_time = 0;
  bool gost_flag = false;
  bool gost_set_flag = false;

  bool end_game = false;

  string th1_end;
  string th2_end;

  pair<int, int> prev_yx_1;
  pair<int, int> prev_yx_2;
  pair<int, int> prev_gost;
  char prev_gost_map;

  vector<vector<int>> visitor;
  vector<int> dy = {-1, 0, 1,  0, -2, 0, 2,  0, -3, 0,
                    3,  0, -4, 0, 4,  0, -5, 0, 5,  0};
  vector<int> dx = {0, 1,  0, -1, 0, 2,  0, -2, 0, 3,
                    0, -3, 0, 4,  0, -4, 0, 5,  0, -5};

  sem_t *sem_h;

public:
  Sv(Ch &ch);
  ~Sv();

  void open_s();

  void stream_map();

  void boom_check_map();
  static void *th_func(void *vp);
  // static void sig_al_func(int sig, siginfo_t *info, void *vp);

  void update_boom_map();
  void boom_boom(struct st_boom *boom);
  bool boom_over(const int &y, const int &x);
  void make_item(const int &y, const int &x);

  void update_map(const int &flag, const int &cmd);
  void update_mv_box(const int &flag, const int &cmd, const pair<int, int> &yx);

  void use_item_niddle(const int &flag);

  void start_gost();
  static void *th_func_gost(void *vp);
  void gost_update();
  void game_over(const int ch);

  //============================== SET ==============================//
  void set_map();
  void set_map_2();

  void set_cmd_1_Sv(const int &cmd, const int &flag);
  void set_cmd_2_Sv(const int &cmd, const int &flag);

  void set_boom_1_Sv(const int &cmd);
  void set_boom_2_Sv(const int &cmd);

  void set_item_niddle_1_SV(const bool set);
  void set_item_niddle_2_Sv(const bool set);

  void set_sem_p(sem_t *sem_h);

  void set_tid_main(const bool set);
  void set_tid_boom(const bool set);
  void set_tid_gost(const bool set);

  void set_gost();
  void set_gost_prev_yx(const pair<int, int> &yx);

  void set_gost_set_flag(const bool set);

  void set_end_game(const bool set);

  //============================== SET ==============================//

  //============================== GET ==============================//
  const bool get_gost_set_flag();

  const bool get_tid_set_main();
  const bool get_tid_set_boom();
  const bool get_tid_set_gost();

  const pthread_t get_tid_boom();
  const pthread_t get_tid_gost();

  sem_t *get_sem_p();

  const bool get_flag_ch_1_Sv();
  const bool get_flag_ch_2_Sv();

  const int &get_boom_num_1_Sv();
  const int &get_boom_num_2_Sv();

  const bool get_lock_1();
  const bool get_lock_2();

  bool &get_item_niddle_1_Sv();
  bool &get_item_niddle_2_Sv();

  const bool get_end_game();

  const bool get_gost_flag();

  const bool get_game_over_main_th();

  struct shm_arr *get_shm_p();
  //============================== GET ==============================//
};