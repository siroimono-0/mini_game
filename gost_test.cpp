#include <bits/stdc++.h>

using namespace std;

vector<vector<int>> visitor;
vector<vector<int>> vmap;
map<pair<int, int>, pair<int, int>> mp;
vector<pair<int, int>> ret;

pair<int, int> target;
map<pair<pair<int, int>, pair<int, int>>, pair<int, int>> trace;

vector<int> dy{-1, 0, 1, 0};
vector<int> dx{0, 1, 0, -1};

bool over(int y, int x)
{
  if (y < 0 || y >= 13 || x < 0 || x >= 15)
  {
    return true;
  }
  return false;
}

void trace_yx(int sy, int sx)
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
        // 타겟 위치를 입력하면 고스트 ~ 초기 고스트 위치 나옴
        if (target == pair<int, int>({ny, nx}))
        {
          return;
        }
      }
    }
  }
  return;
}

void print(pair<int, int> pa)
{
  if (mp.find(pa) != mp.end())
  {
    // cout << "y : " << pa.first << "   x : " << pa.second << endl;
    ret.push_back(pa);
    print(mp[pa]);
  }
}

int main()
{
  vmap.assign(13, vector<int>(15, 0));
  visitor.assign(13, vector<int>(15, -1));

  for (int i = 0; i < 13; i++)
  {
    for (int j = 0; j < 15; j++)
    {
      target.first = i;
      target.second = j;

      string buf_1;
      getline(cin, buf_1);

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

              cout << "===================" << endl;
              cout << "Gost ";
              cout << "y : " << target.first << "   x : " << target.second
                   << endl;

              cout << "y : " << (ret[1].first) << "   x : " << (ret[1].second)
                   << endl;
              cout << "Char ";
              cout << "y : " << z << "   x : " << v << endl;
              cout << "===================" << endl;

              pair<int, int> tmp_pa = {z, v};
              trace[{target, tmp_pa}] = {ret[1].first, ret[1].second};
            }
            else if (ret.size() == 1)
            {
              /*
                            cout << "===================" << endl;
                            cout << "Gost ";
                            cout << "y : " << target.first << "   x : " <<
                 target.second
                                 << endl;

                            cout << "y : " << z << "   x : " << v << endl;
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
  /*
  while (1)
  {
    string buf_1;
    getline(cin, buf_1);
    string buf_2;
    getline(cin, buf_2);
    int i_1 = stoi(buf_1);
    int i_2 = stoi(buf_2);

    pair<int, int> pa = trace[{{0, 0}, {i_1, i_2}}];
    cout << pa.first << " :: " << pa.second << endl;
  }
    */
  return 0;
}

const pair<int, int> &get_next_yx(const pair<int, int> &ch_yx)
{
  return mp[ch_yx];
}
