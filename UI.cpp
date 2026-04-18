#include "UI.h"
#include "Server.h"
#include "Support.h"
#include <ncurses.h>
#include <pthread.h>

using namespace std;

UI::UI(Sv &sv) : sv(sv)
{
}

UI::~UI()
{
}

void UI::run()
{
  Sig_Guard sg;

  sg.sig_lock();

  this->set_sem_p();

  printf("=============== mini game manual ===============\n");
  printf("                1p                              2p\n"
         "move         w s a d                          ↑ ↓ ← →\n"
         "attack          [g]                             [']\n"
         "niddle          [q]                             [/]\n\n");

  printf("Start ? [Y]\n");

  string buf;
  getline(cin, buf);

  if (buf == "y" || buf == "Y" || buf.empty())
  {
    // this->sv.set_map();
    // p_map();

    this->control();
  }
  else
  {
    printf("unavailable value\n");
  }
  return;
}

void UI::control()
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  timeout(500);

  while (1)
  {
    int flag = 0;
    int ch = getch();

    sem_wait(this->sem_h);

    if (ch == 119 || ch == 97 || ch == 115 || ch == 100)
    {
      if (this->sv.get_lock_1() != true)
      {
        flag = 1;
        this->sv.set_cmd_1_Sv(ch, flag);
      }
    }
    else if (ch == 259 || ch == 260 || ch == 258 || ch == 261)
    {
      if (this->sv.get_lock_2() != true)
      {
        flag = 2;
        this->sv.set_cmd_2_Sv(ch, flag);
      }
    }
    else if (ch == 103 && this->sv.get_flag_ch_1_Sv() == true)
    {
      if (this->sv.get_lock_1() != true)
      {
        flag = 3;
        this->sv.set_boom_1_Sv(ch);
      }
    }
    else if (ch == 39 && this->sv.get_flag_ch_2_Sv() == true)
    {
      if (this->sv.get_lock_2() != true)
      {
        flag = 4;
        this->sv.set_boom_2_Sv(ch);
      }
    }
    else if (ch == 113 && this->sv.get_flag_ch_1_Sv() == false &&
             this->sv.get_item_niddle_1_Sv() == true)
    {
      if (this->sv.get_lock_1() != true)
      {
        flag = 5;
        this->sv.use_item_niddle(5);
      }
    }
    else if (ch == 47 && this->sv.get_flag_ch_2_Sv() == false &&
             this->sv.get_item_niddle_2_Sv() == true)
    {
      if (this->sv.get_lock_2() != true)
      {
        flag = 6;
        this->sv.use_item_niddle(6);
      }
    }

    this->sv.update_map(flag, ch);
    sem_post(this->sem_h);
    if (this->sv.get_end_game() == true &&
        this->sv.get_game_over_main_th() == true)
    {
      union sigval st_sv = {};

      int ret_sv = sigqueue(getpid(), SIGINT, st_sv);
      check_err::check("UI::control -> sigqueue()", ret_sv, -1);

      break;
    }
    else if (this->sv.get_end_game() == true &&
             this->sv.get_game_over_main_th() == false)
    {
      break;
    }
  }

  cout << "pthread ... ... ...\n";
  int ret_pth_j_1 = pthread_join(this->sv.get_tid_boom(), nullptr);
  check_err::check("run() -> pthread_join() -> boom", ret_pth_j_1, -1);

  if (this->sv.get_gost_flag() == true)
  {
    int ret_pth_j_2 = pthread_join(this->sv.get_tid_gost(), nullptr);
    check_err::check("run() -> pthread_join() -> gost", ret_pth_j_2, -1);
  }

  return;
}

void UI::set_character()
{

  return;
}

void UI::set_sem_p()
{
  this->sem_h = this->sv.get_sem_p();
  return;
}