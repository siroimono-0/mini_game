#include <chrono>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Gost
{
private:
  map<pair<pair<int, int>, pair<int, int>>, pair<int, int>> trace;

  vector<vector<int>> visitor;
  vector<vector<int>> vmap;
  map<pair<int, int>, pair<int, int>> mp;
  vector<pair<int, int>> ret;

  pair<int, int> target;

  int attack_target;

  vector<int> dy{-1, 0, 1, 0};
  vector<int> dx{0, 1, 0, -1};

public:
  void gost();
  bool over(int y, int x);
  void print(pair<int, int> pa);
  void trace_yx(int sy, int sx);
  const int get_attack_target();
  const pair<int, int> &get_next_yx(const pair<int, int> &gost_yx,
                                    const pair<int, int> &ch_yx);
};