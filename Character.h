#pragma once
//==========================================

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <queue>
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

class Ch;

class Item
{
private:
  vector<string> item_list;

public:
  Item();
  ~Item();
};
/*
좀 있다가
*/
struct shm_arr
{
  char arr[13][15];
};

class boom_count
{
private:
  Ch *cp = nullptr;
  int num;

public:
  boom_count();
  ~boom_count();

  void set_ptr(Ch *cp);
  void set_12(int num);
};

struct st_boom
{
  int t = 5;
  pair<int, int> yx;
  int pw;
  bool ox;
  int how;
};

class Ch
{
private:
  map<int, bool> cmd_1 = {
      {119, false}, {97, false}, {115, false}, {100, false}};

  map<int, bool> cmd_2 = {
      {259, false}, {260, false}, {258, false}, {261, false}};

  struct shm_arr *shm_p;

  // vector<struct st_boom> boom1;
  // vector<struct st_boom> boom2;
  vector<struct st_boom> boom_all;

  int y_1;
  int x_1;

  int y_2;
  int x_2;

  int pw_1 = 4;
  int pw_2 = 4;

  int boom_num_1 = 1;
  int boom_num_2 = 1;

  bool item_niddle_1 = false;
  bool item_niddle_2 = false;

  bool item_car_1 = false;
  bool item_car_2 = false;

  bool item_car_destroy_1 = false;
  bool item_car_destroy_2 = false;

  bool flag_ch_1 = true;
  bool flag_ch_2 = true;

  bool lock_1 = false;
  bool lock_2 = false;

  bool die_1 = false;
  bool die_2 = false;

  int die_cnt_1 = 5;
  int die_cnt_2 = 5;

public:
  Ch();
  ~Ch();

  void set_pw();
  void set_pw_num();
  void set_sp();
  void set_flag_ch_1(bool set_set);
  void set_flag_ch_2(bool set_set);

  void set_yx();
  bool over_1(const int &cmd, const int &flag);
  bool over_2(const int &cmd, const int &flag);

  void check_item();

  bool check_map(const int &y, const int &x, const int &cmd, const int &flag);
  void item_use(const char &item_c);

  void set_cmd_1(const int &cmd, const int &flag);
  void set_cmd_2(const int &cmd, const int &flag);

  void set_boom_1(const int &cmd);
  void set_boom_2(const int &cmd);

  void set_car_destory_1(const bool set);
  void set_car_destory_2(const bool set);

  void set_lock_1(const bool set);
  void set_lock_2(const bool set);

  void set_item_niddle_1(const bool set);
  void set_item_niddle_2(const bool set);

  void set_die_1(const bool set);
  void set_die_2(const bool set);

  void set_die_cnt_1(const int set);
  void set_die_cnt_2(const int set);

  void up_boom_num_1();
  void down_boom_num_1();
  void up_boom_num_2();
  void down_boom_num_2();

  vector<struct st_boom> *get_boom();

  const int &get_pw_1();
  const int &get_pw_2();
  const bool get_flag_ch_1();
  const bool get_flag_ch_2();

  const int &get_boom_num_1();
  const int &get_boom_num_2();

  bool &get_item_niddle_1();
  bool &get_item_niddle_2();

  bool &get_item_car_1();
  bool &get_item_car_2();

  const bool get_car_destory_1();
  const bool get_car_destory_2();

  const bool get_lock_1();
  const bool get_lock_2();

  const bool get_die_1();
  const bool get_die_2();

  const int get_die_cnt_1();
  const int get_die_cnt_2();

  void set_shm_p(struct shm_arr *shm_p);

  const pair<int, int> get_yx_1();
  const pair<int, int> get_yx_2();
};

/*
1. 이동
-> 방향키(1p, 2p 기준 다름)으로 캐릭터가 이동
-> 속도 아이템에 영향을 받음
-> 기본 속도 1칸당 0.5초 소모
-> 속도 아이템 1개당 최대치 기준 10% 속도업 -> 최대 5개 중첩 가능
-> 최종 1칸당 0.25초

1-1 느린 거북이 획득 후 이동

1-2 해적 거북이 획득 후 이동

1-3 우주선 획득 후 이동

2 물풍선
-> 기본 물풍선은 가운대 + 상하좌우
-> 아이템 먹을시 상하좌우 1칸씩 증가 최대 ~중첩

-> 물풍선 최대 놓을 수 있는 것 1개 ~ 개

2-1 신발 아이템 획득 후 물풍선 넘기기

3 피격시
-> 피격시 물풍선에 갇히고 ~초 기다린 후 사망

3-1 바늘 아이템 쓰면 물풍선 빠져나오기 구현


*/