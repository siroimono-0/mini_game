#include "Character.h"

using namespace std;

Item::Item()
{
}

Item::~Item()
{
}

Ch::Ch()
{
  this->y_1 = 0;
  this->x_1 = 0;
  this->y_2 = 12;
  this->x_2 = 14;
}
Ch::~Ch()
{
}

boom_count::boom_count()
{
}

boom_count::~boom_count()
{
  if (this->num == 1)
  {
    this->cp->up_boom_num_1();
  }
  else if (this->num == 2)
  {
    this->cp->up_boom_num_2();
  }
}

void boom_count::set_ptr(Ch *cp)
{
  this->cp = cp;
}

void boom_count::set_12(int num)
{
  this->num = num;
}
/*
  vector<pair<string, bool>> command = {
      {"up", false}, {"left", false}, {"down", false}, {"right", false}};

*/
/*
void Ch::set_yx()
{
  auto it = find_if(this->command.begin(), this->command.end(),
                    [](pair<string, bool> pa)
                    {
                      if (pa.second == true)
                      {
                        return true;
                      }
                      return false;
                    });

  if (this->over(it->first))
  {
    return;
  }
}
  */

void Ch::set_flag_ch_1(bool set_set)
{
  this->flag_ch_1 = set_set;
}

void Ch::set_flag_ch_2(bool set_set)
{
  this->flag_ch_2 = set_set;
}

bool Ch::check_map(const int &y, const int &x, const int &cmd, const int &flag)
{
  char mv_box = '%';
  char emp = ' ';
  char forest = '@';

  char item_pw = 'P';
  char item_num = '+';
  char item_life = '!';
  char item_car = '=';

  char RIP = 'X';

  if (this->shm_p->arr[y][x] == mv_box && flag == 1)
  {
    if (cmd == 119)
    {
      if ((y - 1) < 13 && (y - 1) >= 0 && this->shm_p->arr[y - 1][x] == emp)
      {
        return false;
      }
    }
    else if (cmd == 97)
    {
      if ((x - 1) < 15 && (x - 1) >= 0 && this->shm_p->arr[y][x - 1] == emp)
      {
        return false;
      }
    }
    else if (cmd == 115)
    {
      if ((y + 1) < 13 && (y + 1) >= 0 && this->shm_p->arr[y + 1][x] == emp)
      {
        return false;
      }
    }
    else if (cmd == 100)
    {
      if ((x + 1) < 15 && (x + 1) >= 0 && this->shm_p->arr[y][x + 1] == emp)
      {
        return false;
      }
    }
  }
  else if (this->shm_p->arr[y][x] == mv_box && flag == 2)
  {
    if (cmd == 259)
    {
      if ((y - 1) < 13 && (y - 1) >= 0 && this->shm_p->arr[y - 1][x] == emp)
      {
        return false;
      }
    }
    else if (cmd == 260)
    {
      if ((x - 1) < 15 && (x - 1) >= 0 && this->shm_p->arr[y][x - 1] == emp)
      {
        return false;
      }
    }
    else if (cmd == 258)
    {
      if ((y + 1) < 13 && (y + 1) >= 0 && this->shm_p->arr[y + 1][x] == emp)
      {
        return false;
      }
    }
    else if (cmd == 261)
    {
      if ((x + 1) < 15 && (x + 1) >= 0 && this->shm_p->arr[y][x + 1] == emp)
      {
        return false;
      }
    }
  }

  if (this->shm_p->arr[y][x] == emp || this->shm_p->arr[y][x] == forest ||
      this->shm_p->arr[y][x] == RIP)
  {
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_num && flag == 1)
  {
    this->boom_num_1++;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_num && flag == 2)
  {
    this->boom_num_2++;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_pw && flag == 1)
  {
    this->pw_1 += 4;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_pw && flag == 2)
  {
    this->pw_2 += 4;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_life && flag == 1)
  {
    this->item_niddle_1 = true;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_life && flag == 2)
  {
    this->item_niddle_2 = true;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_car && flag == 1)
  {
    this->item_car_1 = true;
    return false;
  }
  else if (this->shm_p->arr[y][x] == item_car && flag == 2)
  {
    this->item_car_2 = true;
    return false;
  }

  return true;
}

void Ch::item_use(const char &item_c)
{
}

void Ch::set_cmd_1(const int &cmd, const int &flag)
{
  if (over_1(cmd, flag))
  {
    cout << "don't move" << endl;
    return;
  }

  if (this->cmd_1.empty())
  {
    this->cmd_1[cmd] = true;
  }
  else
  {
    this->cmd_1.clear();
    this->cmd_1[cmd] = true;
  }
  return;
}

void Ch::set_cmd_2(const int &cmd, const int &flag)
{
  if (over_2(cmd, flag))
  {
    cout << "don't move" << endl;
    return;
  }

  if (this->cmd_2.empty())
  {
    this->cmd_2[cmd] = true;
  }
  else
  {
    this->cmd_2.clear();
    this->cmd_2[cmd] = true;
  }
  return;
}

bool Ch::over_1(const int &cmd, const int &flag)
{
  if (cmd == 119) // up
  {
    int ny = this->y_1 - 1;
    if (ny >= 13 || ny < 0 || this->check_map(ny, this->x_1, cmd, flag))
    {
      return true;
    }
    else
    {
      this->y_1 = ny;
    }
  }
  else if (cmd == 97) // left
  {
    int nx = this->x_1 - 1;
    if (nx >= 15 || nx < 0 || this->check_map(this->y_1, nx, cmd, flag))
    {
      return true;
    }
    else
    {
      this->x_1 = nx;
    }
  }
  else if (cmd == 115) // down
  {
    int ny = this->y_1 + 1;
    if (ny >= 13 || ny < 0 || this->check_map(ny, this->x_1, cmd, flag))
    {
      return true;
    }
    else
    {
      this->y_1 = ny;
    }
  }
  else if (cmd == 100) // right
  {
    int nx = this->x_1 + 1;
    if (nx >= 15 || nx < 0 || this->check_map(this->y_1, nx, cmd, flag))
    {
      return true;
    }
    else
    {
      this->x_1 = nx;
    }
  }
  return false;
}

bool Ch::over_2(const int &cmd, const int &flag)
{
  if (cmd == 259) // up
  {
    int ny = this->y_2 - 1;
    if (ny >= 13 || ny < 0 || this->check_map(ny, this->x_2, cmd, flag))
    {
      return true;
    }
    else
    {
      this->y_2 = ny;
    }
  }
  else if (cmd == 260) // left
  {
    int nx = this->x_2 - 1;
    if (nx >= 15 || nx < 0 || this->check_map(this->y_2, nx, cmd, flag))
    {
      return true;
    }
    else
    {
      this->x_2 = nx;
    }
  }
  else if (cmd == 258) // down
  {
    int ny = this->y_2 + 1;
    if (ny >= 13 || ny < 0 || this->check_map(ny, this->x_2, cmd, flag))
    {
      return true;
    }
    else
    {
      this->y_2 = ny;
    }
  }
  else if (cmd == 261) // right
  {
    int nx = this->x_2 + 1;
    if (nx >= 15 || nx < 0 || this->check_map(this->y_2, nx, cmd, flag))
    {
      return true;
    }
    else
    {
      this->x_2 = nx;
    }
  }
  return false;
}

void Ch::set_boom_1(const int &cmd)
{
  if (this->boom_num_1 <= 0)
  {
    return;
  }

  struct st_boom st_boom1;
  st_boom1.pw = this->pw_1;
  st_boom1.yx.first = this->y_1;
  st_boom1.yx.second = this->x_1;
  st_boom1.ox = true;
  st_boom1.how = 1;

  // cout << this->boom_num_1 << endl;
  this->boom_all.push_back(st_boom1);
  return;
}

void Ch::set_boom_2(const int &cmd)
{
  if (this->boom_num_2 <= 0)
  {
    return;
  }

  struct st_boom st_boom2;
  st_boom2.pw = this->pw_2;
  st_boom2.yx.first = this->y_2;
  st_boom2.yx.second = this->x_2;
  st_boom2.ox = true;
  st_boom2.how = 2;

  this->boom_all.push_back(st_boom2);
  return;
}

void Ch::set_car_destory_1(const bool set)
{
  this->item_car_destroy_1 = set;
  return;
}

void Ch::set_car_destory_2(const bool set)
{
  this->item_car_destroy_2 = set;
  return;
}

void Ch::set_lock_1(const bool set)
{
  this->lock_1 = set;
  return;
  // 똑같은 쓰레드가 변경함
  // 해당 쓰레드는 1번 함수만 호출했는대 이 함수는 두번 호출댐
  // 즉 알람 받는 쓰레드 함수의 어딘가에서 코드가 꼬임
  // 아니 왜 ㅡㅡ;
}

void Ch::set_lock_2(const bool set)
{
  this->lock_2 = set;
  return;
}

void Ch::set_item_niddle_1(const bool set)
{
  this->item_niddle_1 = set;
}

void Ch::set_item_niddle_2(const bool set)
{
  this->item_niddle_2 = set;
}

void Ch::up_boom_num_1()
{
  this->boom_num_1++;
}

void Ch::down_boom_num_1()
{
  this->boom_num_1--;
}

void Ch::up_boom_num_2()
{
  this->boom_num_2++;
}

void Ch::down_boom_num_2()
{
  this->boom_num_2--;
}

vector<struct st_boom> *Ch::get_boom()
{
  if (!this->boom_all.empty())
  {
    return &this->boom_all;
  }

  static vector<struct st_boom> void_v;
  return &void_v;
}

const int &Ch::get_pw_1()
{
  return this->pw_1;
}

const int &Ch::get_pw_2()
{
  return this->pw_2;
}

const bool Ch::get_flag_ch_1()
{
  return this->flag_ch_1;
}

const bool Ch::get_flag_ch_2()
{
  return this->flag_ch_2;
}

bool &Ch::get_item_niddle_1()
{
  return this->item_niddle_1;
}

bool &Ch::get_item_niddle_2()
{
  return this->item_niddle_2;
}

void Ch::set_die_1(const bool set)
{
  this->die_1 = set;
}

void Ch::set_die_2(const bool set)
{
  this->die_2 = set;
}

void Ch::set_die_cnt_1(const int set)
{
  if (set == 1)
  {
    this->die_cnt_1 = 5;
  }
  else if (set == -1)
  {
    this->die_cnt_1--;
  }
}

void Ch::set_die_cnt_2(const int set)
{
  if (set == 1)
  {
    this->die_cnt_2 = 5;
  }
  else if (set == -1)
  {
    this->die_cnt_2--;
  }
}

bool &Ch::get_item_car_1()
{
  return this->item_car_1;
}

bool &Ch::get_item_car_2()
{
  return this->item_car_2;
}

const bool Ch::get_car_destory_1()
{
  return this->item_car_destroy_1;
}

const bool Ch::get_car_destory_2()
{
  return this->item_car_destroy_2;
}

const bool Ch::get_lock_1()
{
  return this->lock_1;
}

const bool Ch::get_lock_2()
{
  return this->lock_2;
}

const bool Ch::get_die_1()
{
  return this->die_1;
}

const bool Ch::get_die_2()
{

  return this->die_2;
}

const int Ch::get_die_cnt_1()
{
  return this->die_cnt_1;
}

const int Ch::get_die_cnt_2()
{
  return this->die_cnt_2;
}

void Ch::set_shm_p(struct shm_arr *shm_p)
{
  this->shm_p = shm_p;
}

const int &Ch::get_boom_num_1()
{
  return this->boom_num_1;
}

const int &Ch::get_boom_num_2()
{
  return this->boom_num_2;
}
const pair<int, int> Ch::get_yx_1()
{
  pair<int, int> yx_1;
  yx_1.first = this->y_1;
  yx_1.second = this->x_1;

  return yx_1;
}

const pair<int, int> Ch::get_yx_2()
{
  pair<int, int> yx_2;
  yx_2.first = this->y_2;
  yx_2.second = this->x_2;

  return yx_2;
}

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