#include "gost.h"

using namespace std;

bool Gost::over(int y, int x)
{
  if (y < 0 || y >= 13 || x < 0 || x >= 15)
  {
    return true;
  }
  return false;
}

void Gost::trace_yx(int sy, int sx)
{
  queue<pair<int, int>> q;
  q.push({sy, sx});
  visitor[sy][sx] = 1;

  while (!q.empty())
  {
    int cy = q.front().first;
    int cx = q.front().second;
    q.pop();

    for (int i = 0; i < 4; i++)
    {
      int ny = cy + dy[i];
      int nx = cx + dx[i];

      if (over(ny, nx) || visitor[ny][nx] != -1)
      {
        continue;
      }
      else
      {
        visitor[ny][nx] = 1;
        q.push({ny, nx});
        mp[{ny, nx}] = {cy, cx};
        if (target == pair<int, int>({ny, nx}))
        {
          return;
        }
      }
    }
  }
  return;
}

void Gost::print(pair<int, int> pa)
{
  if (mp.find(pa) != mp.end())
  {
    // cout << "y : " << pa.first << "   x : " << pa.second << endl;
    ret.push_back(pa);
    print(mp[pa]);
  }
}

void Gost::gost()
{
  vmap.assign(13, vector<int>(15, 0));
  visitor.assign(13, vector<int>(15, -1));

  for (int i = 0; i < 13; i++)
  {
    for (int j = 0; j < 15; j++)
    {
      target.first = i;
      target.second = j;

      for (int z = 0; z < 13; z++)
      {
        for (int v = 0; v < 15; v++)
        {
          if (pair<int, int>({z, v}) != target)
          {
            visitor.assign(13, vector<int>(15, -1));
            ret.clear();
            trace_yx(z, v);
            print(target);
            if (ret.size() > 1)
            {
              /*
              cout << "===================" << endl;
              cout << "Gost ";
              cout << "y : " << target.first << "   x : " << target.second
                   << endl;

              cout << "y : " << (ret[1].first) << "   x : " << (ret[1].second)
                   << endl;
              cout << "Char ";
              cout << "y : " << z << "   x : " << v << endl;
              cout << "===================" << endl;
              */

              pair<int, int> tmp_pa = {z, v};
              trace[{target, tmp_pa}] = {ret[1].first, ret[1].second};
            }
            else if (ret.size() == 1)
            {
              /*
              cout << "===================" << endl;
              cout << "Gost ";
              cout << "y : " << target.first << "   x : " << target.second
                   << endl;

              cout << "y : " << (ret[0].first) << "   x : " << (ret[0].second)
                   << endl;
              cout << "Char ";
              cout << "y : " << z << "   x : " << v << endl;
              cout << "===================" << endl;
              */

              pair<int, int> tmp_pa = {z, v};
              trace[{target, tmp_pa}] = {tmp_pa};
            }
          }
          mp.clear();
        }
      }
    }
  }

  unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
  srand(seed);
  int ttt = rand() % 10;

  if (ttt == 0 || ttt == 1 || ttt == 8)
  {
    this->attack_target = 1;
  }
  else if (ttt == 2 || ttt == 3 || ttt == 9)
  {
    this->attack_target = 2;
  }
  else if (ttt == 4 || ttt == 5)
  {
    this->attack_target = 1;
  }
  else if (ttt == 6 || ttt == 7)
  {
    this->attack_target = 2;
  }
  return;
}

const pair<int, int> &Gost::get_next_yx(const pair<int, int> &gost_yx,
                                        const pair<int, int> &ch_yx)
{
  return this->trace[{gost_yx, ch_yx}];
}

const int Gost::get_attack_target()
{
  return this->attack_target;
}