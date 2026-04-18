#pragma once
//==========================================
#include "Server.h"

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
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> // mode_t
#include <sys/types.h>
#include <unistd.h> // write(), close()
//=====================C=====================

using namespace std;

class UI
{
private:
  Sv &sv;
  sem_t *sem_h;

public:
  UI(Sv &sv);
  ~UI();

  void run();

  void control();

  void set_character();

  void set_sem_p();
};

/*
fork로 터미널 하나 생성해서
해당 프로세스는 @초마다 clear + 공유공간 출력(map)

*/