#include "Server.h"
#include "Support.h"
#include <csignal>
#include <cstddef>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

/*
원래
프로세스 통신 파이프로 하고 있었음
-> 메시지 큐로 변경함

스트림 프로세스 자원 정상회수 및 응답 확인
메인 프로레스 응답 받아서 자원회수 진행

-> 자원 회수
고스트에 죽었을 때 -> 자원회수 구현 완료
물풍선에 사망하고 시간초과로 죽음 -> 자원회수 구현 완료
캐릭터가 터치해서 죽음 -> 구현 완료

고스트 나옴 -> 직접 터치 -> 정상 확인
고스트 나옴 -> 시간 초과 -> 수정 완료



*/

Sv::Sv(Ch &ch) : ch(ch)
{
    this->prev_yx_1 = this->ch.get_yx_1();
    this->prev_yx_2 = this->ch.get_yx_2();
    int ret_ep = epoll_create1(0);
    check_err::check("Sv(Ch &ch) -> epoll_create1()", ret_ep, -1);
    this->epoll_fd = ret_ep;
    this->tid_main = gettid();

    this->st_mq_attr.mq_flags = 0;
    this->st_mq_attr.mq_maxmsg = 3;
    this->st_mq_attr.mq_msgsize = 64;

    int ret_mq_un = mq_unlink("/mq_game");
    if (ret_mq_un == 1 && errno != ENOENT)
    {
        check_err::check("Sv(Ch &ch) -> mq_unlink()", ret_mq_un, -1);
    }

    this->mq_fd_send =
        mq_open("/mq_game_send", O_CREAT | O_WRONLY, 0755, &this->st_mq_attr);
    check_err::check("Sv(Ch &ch) -> mq_open()", this->mq_fd_send, -1);

    this->mq_fd_rev =
        mq_open("/mq_game_rev", O_CREAT | O_RDONLY, 0755, &this->st_mq_attr);
    check_err::check("Sv(Ch &ch) -> mq_open()", this->mq_fd_rev, -1);
}

Sv::~Sv()
{
    //=================== epoll_fd remove ====================//
    if (this->gost_flag == true)
    {
        int ret_ep_ctl =
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, this->event_fd, NULL);
        check_err::check_end("~Sv -> epoll_ctl()", ret_ep_ctl, -1);

        int ret_cl_epfd = close(this->epoll_fd);
        check_err::check_end("~Sv -> close(this->epoll_fd)", ret_cl_epfd, -1);

        int ret_close_event_fd = close(this->event_fd);
        check_err::check_end("~Sv -> close(this->event_fd)", ret_close_event_fd,
                             -1);
    }
    else if (this->gost_flag == false)
    {
        int ret_cl_epfd = close(this->epoll_fd);
        check_err::check_end("~Sv -> close(this->epoll_fd)", ret_cl_epfd, -1);
    }
    //=================== epoll_fd remove ====================//

    //=================== mq remove ====================//
    int ret_mq_cl_send = mq_close(this->mq_fd_send);
    check_err::check_end("~Sv -> mq_close(send)", ret_mq_cl_send, -1);

    int ret_mq_un_send = mq_unlink("/mq_game_send");
    check_err::check_end("~Sv -> mq_unlink(/mq_game_send)", ret_mq_un_send, -1);

    int ret_mq_cl_rev = mq_close(this->mq_fd_rev);
    check_err::check_end("~Sv -> mq_close(rev)", ret_mq_cl_rev, -1);

    int ret_mq_un_rev = mq_unlink("/mq_game_rev");
    check_err::check_end("~Sv -> mq_unlink(/mq_game_rev)", ret_mq_un_rev, -1);

    //=================== mq remove ====================//

    //=================== sem remove ====================//
    int ret_sem_cl = sem_close(this->sem_h);
    check_err::check_end("~Sv -> sem_close()", ret_sem_cl, -1);

    int ret_sem_un = sem_unlink("/sem_game");
    check_err::check_end("~Sv -> sem_unlink()", ret_sem_un, -1);
    //=================== sem remove ====================//

    //=================== shm remove ====================//
    int ret_munmap = munmap(this->shm_p, 4096);
    check_err::check_end("~Sv -> munmap()", ret_munmap, -1);

    int ret_shm_cl = close(this->shm_fd);
    check_err::check_end("~Sv -> close(this->shm_fd)", ret_shm_cl, -1);

    int ret_shm_un = shm_unlink("/shm_map");
    check_err::check_end("~Sv -> shm_unlink()", ret_shm_un, -1);
    //=================== shm remove ====================//
}

void Sv::open_s()
{
    int shm_fd = shm_open("/shm_map", O_RDWR | O_CREAT, (mode_t)0755);
    check_err::check("open_s() -> shm_open()", shm_fd, -1);

    ftruncate(shm_fd, 4096);

    struct shm_arr *shm_p = (struct shm_arr *)mmap(
        NULL, (size_t)4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    check_err::check<void *>("UI::control() -> mmap()", (void *)shm_p,
                             (void *)-1);

    this->shm_p = shm_p;
    this->ch.set_shm_p(shm_p);

    this->village.assign(13, vector<char>(15));
    // this->set_map();
    this->set_map();

    pid_t pid = fork();
    check_err::check("Sv::open_s() -> fork()", pid, (pid_t)-1);

    if (pid == 0)
    {
        sleep(1);
        this->stream_map();
    }

    sem_t *sem_h = sem_open("/sem_game", O_CREAT, 0755, 1);
    check_err::check<sem_t *>("Sv::open_s() -> sem_open()", sem_h, (sem_t *)-1);

    this->set_sem_p(sem_h);

    int sem_cnt = 0;
    int ret_sem_getvalue = sem_getvalue(this->sem_h, &sem_cnt);
    check_err::check("open_s() -> sem_getvalue()", ret_sem_getvalue, -1);
    if (sem_cnt == 0)
    {
        cout << "sem :: 0... \n";
        sem_post(this->sem_h);
    }
    this->boom_check_map();

    // int stat;
    // int ret_waitpid = waitpid(pid, &stat, 0);
    // check_err::check("open_s -> waitpid()", ret_waitpid, -1);

    return;
}

void Sv::stream_map()
{
    /*
    char *argv[] = {
        (char *)"gnome-terminal",      (char *)"--", (char *)"bash", (char
    *)"-c", (char *)"./stream; exec bash", (char *)NULL};
    */

    char *argv[] = {
        (char *)"/home/gray/gray/mini_game/Stream/build/Desktop_Qt_6_9_3-Debug/"
                "appStream",
        (char *)NULL};
    // int ret_execvp = execvp("gnome-terminal", argv);
    int ret_execvp = execv("/home/gray/gray/mini_game/Stream/build/"
                           "Desktop_Qt_6_9_3-Debug/appStream",
                           argv);
    check_err::check("UI::stream_map() -> execv()", ret_execvp, -1);
    return;
}

void Sv::update_mv_box(const int &flag, const int &cmd,
                       const pair<int, int> &yx)
{
    int y = yx.first;
    int x = yx.second;

    if (flag == 1)
    {
        if (cmd == 119)
        {
            this->shm_p->arr[y - 1][x] = '%';
        }
        else if (cmd == 97)
        {
            this->shm_p->arr[y][x - 1] = '%';
        }
        else if (cmd == 115)
        {
            this->shm_p->arr[y + 1][x] = '%';
        }
        else if (cmd == 100)
        {
            this->shm_p->arr[y][x + 1] = '%';
        }
    }
    else if (flag == 2)
    {
        if (cmd == 259)
        {
            this->shm_p->arr[y - 1][x] = '%';
        }
        else if (cmd == 260)
        {
            this->shm_p->arr[y][x - 1] = '%';
        }
        else if (cmd == 258)
        {
            this->shm_p->arr[y + 1][x] = '%';
        }
        else if (cmd == 261)
        {
            this->shm_p->arr[y][x + 1] = '%';
        }
    }
    return;
}

void Sv::use_item_niddle(const int &flag)
{
    if (flag == 5)
    {
        this->ch.set_item_niddle_1(false);
        this->ch.set_flag_ch_1(true);
        this->ch.set_die_1(false);
        this->ch.set_die_cnt_1(1);

        pair<int, int> yx_1 = this->ch.get_yx_1();
        this->shm_p->arr[yx_1.first][yx_1.second] = '1';
        return;
    }
    else if (flag == 6)
    {
        this->ch.set_item_niddle_2(false);
        this->ch.set_flag_ch_2(true);
        this->ch.set_die_2(false);
        this->ch.set_die_cnt_2(1);

        pair<int, int> yx_2 = this->ch.get_yx_2();
        this->shm_p->arr[yx_2.first][yx_2.second] = '2';
        return;
    }
    return;
}

void Sv::start_gost()
{
    pthread_t tid;
    int ret_pth_cre = pthread_create(&tid, NULL, th_func_gost, (void *)this);
    check_err::check("start_gost() -> pthread_create()", ret_pth_cre, -1);

    this->tid_gost = tid;
    this->tid_gost_set = true;

    int ret_eventfd = eventfd(0, EFD_NONBLOCK);
    check_err::check("start_gost() -> eventfd()", ret_eventfd, -1);
    this->event_fd = ret_eventfd;

    struct epoll_event st_epoll_event = {};
    st_epoll_event.events = EPOLLIN;
    st_epoll_event.data.fd = this->event_fd;

    cout << this->epoll_fd << " epoll fd" << endl;
    int ret_epoll_ctl = epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->event_fd,
                                  &st_epoll_event);
    check_err::check("start_gost() -> epoll_ctl()", ret_epoll_ctl, -1);

    this->gost_flag = true;
    return;
}

void *Sv::th_func_gost(void *vp)
{
    try
    {
        Sv *this_p = (Sv *)vp;
        if (this_p->get_gost_set_flag() == false)
        {
            this_p->set_gost();

            unsigned seed =
                std::chrono::steady_clock::now().time_since_epoch().count();
            srand(seed);
            int ttt = rand() % 10;

            if (ttt == 0 || ttt == 1 || ttt == 8)
            {
                this_p->set_gost_prev_yx({0, 0});
            }
            else if (ttt == 2 || ttt == 3 || ttt == 9)
            {
                this_p->set_gost_prev_yx({12, 14});
            }
            else if (ttt == 4 || ttt == 5)
            {
                this_p->set_gost_prev_yx({0, 14});
            }
            else if (ttt == 6 || ttt == 7)
            {
                this_p->set_gost_prev_yx({12, 0});
            }
        }

        sem_t *sem_h = this_p->get_sem_p();
        struct epoll_event st_epoll_event = {};
        uint64_t uint_read;

        while (this_p->end_game == false)
        {
            /*
            int ret_epoll_wait = epoll_wait(this_p->epoll_fd, &st_epoll_event,
            1, -1);

            check_err::check("Sv::th_func_gost() -> epoll_wait()",
            ret_epoll_wait, -1);
            */
            epoll_wait(this_p->epoll_fd, &st_epoll_event, 1, -1);

            /*
            int ret_read = read(this_p->event_fd, &uint_read,
            sizeof(uint_read)); check_err::check("Sv::th_func_gost() -> read()",
            ret_read, -1);
            */
            read(this_p->event_fd, &uint_read, sizeof(uint_read));

            if (this_p->end_game == true)
            {
                break;
            }

            sem_wait(sem_h);
            this_p->gost_update();
            sem_post(sem_h);
        }
        if (this_p->end_game == true && this_p->tid_boom_set == true)
        {
            this_p->set_tid_gost(false);

            union sigval st_sv = {};

            int ret_sv = sigqueue(getpid(), SIGINT, st_sv);
            check_err::check("Sv::th_func_gost() -> sigqueue()", ret_sv, -1);

            cout << "gost thread -> end\n";
            pthread_exit(nullptr);
        }
        else if (this_p->end_game == true && this_p->tid_boom_set == false)
        {
            this_p->set_tid_gost(false);
            cout << "gost thread -> end\n";
            pthread_exit(nullptr);
        }
    }
    catch (Exception err)
    {
        printf("err type == %s\n"
               "err name == %s\n"
               "err no ==%d\n",
               err.get_err_type().c_str(), err.get_err_name().c_str(),
               err.get_err_no());
        return NULL;
    }
    return nullptr;
}

void Sv::gost_update()
{
    char *p_ch =
        &this->shm_p->arr[this->prev_gost.first][this->prev_gost.second];

    int gost_attack_target = this->gost.get_attack_target();

    if (this->get_gost_set_flag() == false)
    {
        if (*p_ch == 'W')
        {
            this->prev_gost_map = ' ';
        }
        else
        {
            this->prev_gost_map = *p_ch;
        }
        *p_ch = 'G';
        this->set_gost_set_flag(true);
    }
    else
    {
        pair<int, int> at_ch_yx;
        if (gost_attack_target == 1)
        {
            at_ch_yx = this->ch.get_yx_1();
        }
        else if (gost_attack_target == 2)
        {
            at_ch_yx = this->ch.get_yx_2();
        }

        const pair<int, int> &next_yx =
            this->gost.get_next_yx(this->prev_gost, at_ch_yx);

        const pair<int, int> &yx_1p = this->ch.get_yx_1();
        const pair<int, int> &yx_2p = this->ch.get_yx_2();

        cout << gost_attack_target << "..." << endl;

        *p_ch = this->prev_gost_map;
        this->prev_gost_map = this->shm_p->arr[next_yx.first][next_yx.second];
        this->shm_p->arr[next_yx.first][next_yx.second] = 'G';
        this->set_gost_prev_yx(next_yx);

        if (next_yx == yx_1p)
        {
            // this->shm_p->arr[next_yx.first][next_yx.second] = 'G';
            this->ch.set_lock_1(false);
            game_over(1);
        }
        else if (next_yx == yx_2p)
        {
            // this->shm_p->arr[next_yx.first][next_yx.second] = 'G';
            this->ch.set_lock_2(false);
            game_over(2);
        }
    }
    return;
}

void Sv::set_gost()
{
    this->gost.gost();
}

const bool Sv::get_gost_set_flag()
{
    return this->gost_set_flag;
}

void Sv::game_over(const int ch)
{
    endwin();

    if (this->tid_main == gettid())
    {
        this->game_over_main_th = true;
    }

    struct itimerval st_itval = {};
    st_itval.it_value.tv_sec = 0;
    st_itval.it_value.tv_usec = 0;
    st_itval.it_interval.tv_sec = 0;
    st_itval.it_interval.tv_usec = 0;
    int ret_setti = setitimer(ITIMER_REAL, &st_itval, nullptr);
    check_err::check("game_over() -> setitime()", ret_setti, -1);

    auto ret_sig = signal(SIGALRM, SIG_IGN);
    check_err::check("game_over() -> signal()", ret_sig, SIG_ERR);

    this->ch.set_lock_1(false);
    this->ch.set_lock_2(false);

    string buf;
    if (ch == 1)
    {
        buf += "2";
    }
    else if (ch == 2)
    {
        buf += "1";
    }
    else if (ch == 3)
    {
        buf += "3";
    }

    /*
     현재 게임오버를 어떤 쓰레드가 실행 시켯는지 알아야함

    */

    int ret_mq_se = mq_send(this->mq_fd_send, buf.c_str(), buf.size(), 0);
    check_err::check("game_over() -> mq_send()", ret_mq_se, -1);

    char mq_rev_buf[64] = {};
    int ret_mq_rev = mq_receive(this->mq_fd_rev, mq_rev_buf, 64, nullptr);
    check_err::check("game_over() -> mq_receive()", ret_mq_rev, -1);

    this->end_game = true;
    sleep(3);
    return;
}

void Sv::set_gost_prev_yx(const pair<int, int> &yx)
{
    this->prev_gost = yx;
    return;
}

void Sv::set_gost_set_flag(const bool set)
{
    this->gost_set_flag = set;
    return;
}

void Sv::set_end_game(const bool set)
{
    this->end_game = set;
}

void Sv::update_map(const int &flag, const int &cmd)
{
    if (flag == 1)
    {
        pair<int, int> yx_1 = this->ch.get_yx_1();

        char tmp_c =
            this->shm_p->arr[this->prev_yx_1.first][this->prev_yx_1.second];

        if (tmp_c == '1' ||
            (tmp_c == '3' && this->ch.get_item_car_1() == false) ||
            (tmp_c == '@' && this->ch.get_item_car_1() == false &&
             this->get_flag_ch_1_Sv() == true))
        {
            if (yx_1 != this->prev_yx_1)
            {
                char *p_c = &this->shm_p->arr[yx_1.first][yx_1.second];
                if (*p_c == '@')
                {
                    *p_c = '@';
                }
                else if (*p_c == '%')
                {
                    *p_c = '1';
                    this->update_mv_box(flag, cmd, yx_1);
                }
                else if (*p_c == '=')
                {
                    *p_c = '5';
                }
                else if (*p_c == 'X')
                {
                    *p_c = '1';
                    if (tmp_c == '3')
                    {
                        this->shm_p->arr[this->prev_yx_1.first]
                                        [this->prev_yx_1.second] = '*';
                    }
                    else if (tmp_c == '1')
                    {
                        this->shm_p->arr[this->prev_yx_1.first]
                                        [this->prev_yx_1.second] = ' ';
                    }

                    this->game_over(2);
                }
                else
                {
                    *p_c = '1';
                }

                if (tmp_c == '3')
                {
                    this->shm_p
                        ->arr[this->prev_yx_1.first][this->prev_yx_1.second] =
                        '*';
                }
                else if (tmp_c == '1')
                {
                    this->shm_p
                        ->arr[this->prev_yx_1.first][this->prev_yx_1.second] =
                        ' ';
                }

                this->prev_yx_1 = yx_1;
            }
        }
        else if (tmp_c == '5' ||
                 (tmp_c == '3' && this->ch.get_item_car_1() == true) ||
                 (tmp_c == '@' && this->ch.get_item_car_1() == true))
        {
            if (yx_1 != this->prev_yx_1)
            {
                char *p_c = &this->shm_p->arr[yx_1.first][yx_1.second];
                if (*p_c == '@')
                {
                    *p_c = '@';
                }
                else if (*p_c == '%')
                {
                    *p_c = '5';
                    this->update_mv_box(flag, cmd, yx_1);
                }
                else if (*p_c == 'X')
                {
                    *p_c = '5';
                    if (tmp_c == '3')
                    {
                        this->shm_p->arr[this->prev_yx_1.first]
                                        [this->prev_yx_1.second] = '*';
                    }
                    else if (tmp_c == '5')
                    {
                        this->shm_p->arr[this->prev_yx_1.first]
                                        [this->prev_yx_1.second] = ' ';
                    }

                    this->game_over(2);
                }
                else
                {
                    *p_c = '5';
                }

                if (tmp_c == '3')
                {
                    this->shm_p
                        ->arr[this->prev_yx_1.first][this->prev_yx_1.second] =
                        '*';
                }
                else if (tmp_c == '5')
                {
                    this->shm_p
                        ->arr[this->prev_yx_1.first][this->prev_yx_1.second] =
                        ' ';
                }

                this->prev_yx_1 = yx_1;
            }
        }
        else if (tmp_c == 'X' ||
                 (tmp_c == '@' && this->get_flag_ch_1_Sv() == false))
        {
            if (yx_1 != this->prev_yx_1)
            {
                char *p_c = &this->shm_p->arr[yx_1.first][yx_1.second];
                if (*p_c == '@')
                {
                    *p_c = '@';
                }
                else if (*p_c == '%')
                {
                    *p_c = 'X';
                    this->update_mv_box(flag, cmd, yx_1);
                }
                else
                {
                    *p_c = 'X';
                }

                /*
                char tmp_c =
                    this->shm_p->arr[this->prev_yx_1.first][this->prev_yx_1.second];
                    */

                if (tmp_c == 'X')
                {
                    this->shm_p
                        ->arr[this->prev_yx_1.first][this->prev_yx_1.second] =
                        ' ';
                }
                this->prev_yx_1 = yx_1;
            }
        }
    }
    else if (flag == 2)
    {
        pair<int, int> yx_2 = this->ch.get_yx_2();

        char tmp_c =
            this->shm_p->arr[this->prev_yx_2.first][this->prev_yx_2.second];

        if (tmp_c == '2' ||
            (tmp_c == '4' && this->ch.get_item_car_2() == false) ||
            (tmp_c == '@' && this->ch.get_item_car_2() == false &&
             this->get_flag_ch_2_Sv() == true))
        {
            if (yx_2 != this->prev_yx_2)
            {
                char *p_c = &this->shm_p->arr[yx_2.first][yx_2.second];
                if (*p_c == '@')
                {
                    *p_c = '@';
                }
                else if (*p_c == '%')
                {
                    *p_c = '2';
                    this->update_mv_box(flag, cmd, yx_2);
                }
                else if (*p_c == '=')
                {
                    *p_c = '6';
                }
                else if (*p_c == 'X')
                {
                    *p_c = '2';
                    if (tmp_c == '4')
                    {
                        this->shm_p->arr[this->prev_yx_2.first]
                                        [this->prev_yx_2.second] = '*';
                    }
                    else if (tmp_c == '2')
                    {
                        this->shm_p->arr[this->prev_yx_2.first]
                                        [this->prev_yx_2.second] = ' ';
                    }

                    this->game_over(1);
                }
                else
                {
                    *p_c = '2';
                }

                if (tmp_c == '4')
                {
                    this->shm_p
                        ->arr[this->prev_yx_2.first][this->prev_yx_2.second] =
                        '*';
                }
                else if (tmp_c == '2')
                {
                    this->shm_p
                        ->arr[this->prev_yx_2.first][this->prev_yx_2.second] =
                        ' ';
                }
                this->prev_yx_2 = yx_2;
            }
        }
        else if (tmp_c == '6' ||
                 (tmp_c == '4' && this->ch.get_item_car_2() == true) ||
                 (tmp_c == '@' && this->ch.get_item_car_2() == true))
        {
            if (yx_2 != this->prev_yx_2)
            {
                char *p_c = &this->shm_p->arr[yx_2.first][yx_2.second];
                if (*p_c == '@')
                {
                    *p_c = '@';
                }
                else if (*p_c == '%')
                {
                    *p_c = '6';
                    this->update_mv_box(flag, cmd, yx_2);
                }
                else if (*p_c == 'X')
                {
                    *p_c = '6';
                    if (tmp_c == '4')
                    {
                        this->shm_p->arr[this->prev_yx_2.first]
                                        [this->prev_yx_2.second] = '*';
                    }
                    else if (tmp_c == '6')
                    {
                        this->shm_p->arr[this->prev_yx_2.first]
                                        [this->prev_yx_2.second] = ' ';
                    }

                    this->game_over(1);
                }
                else
                {
                    *p_c = '6';
                }

                if (tmp_c == '4')
                {
                    this->shm_p
                        ->arr[this->prev_yx_2.first][this->prev_yx_2.second] =
                        '*';
                }
                else if (tmp_c == '6')
                {
                    this->shm_p
                        ->arr[this->prev_yx_2.first][this->prev_yx_2.second] =
                        ' ';
                }
                this->prev_yx_2 = yx_2;
            }
        }
        else if (tmp_c == 'X' ||
                 (tmp_c == '@' && this->get_flag_ch_2_Sv() == false))
        {
            if (yx_2 != this->prev_yx_2)
            {
                char *p_c = &this->shm_p->arr[yx_2.first][yx_2.second];
                if (*p_c == '@')
                {
                    *p_c = '@';
                }
                else if (*p_c == '%')
                {
                    *p_c = 'X';
                    this->update_mv_box(flag, cmd, yx_2);
                }
                else
                {
                    *p_c = 'X';
                }

                if (tmp_c == 'X')
                {
                    this->shm_p
                        ->arr[this->prev_yx_2.first][this->prev_yx_2.second] =
                        ' ';
                }
                this->prev_yx_2 = yx_2;
            }
        }
    }
    else if (flag == 3)
    {
        const int &b_num = this->get_boom_num_1_Sv();
        if (b_num > 0)
        {
            pair<int, int> yx_1 = this->ch.get_yx_1();
            if (this->shm_p->arr[yx_1.first][yx_1.second] == '@')
            {
                this->shm_p->arr[yx_1.first][yx_1.second] = '@';
            }
            else
            {
                this->shm_p->arr[yx_1.first][yx_1.second] = '3';
            }
            this->ch.down_boom_num_1();
        }
    }
    else if (flag == 4)
    {
        const int &b_num = this->get_boom_num_2_Sv();
        if (b_num > 0)
        {
            pair<int, int> yx_2 = this->ch.get_yx_2();
            if (this->shm_p->arr[yx_2.first][yx_2.second] == '@')
            {
                this->shm_p->arr[yx_2.first][yx_2.second] = '@';
            }
            else
            {
                this->shm_p->arr[yx_2.first][yx_2.second] = '4';
            }
            this->ch.down_boom_num_2();
        }
    }

    return;
}

void Sv::boom_check_map()
{
    pthread_t tid;
    int ret_th = pthread_create(&tid, NULL, th_func, (void *)this);
    check_err::check("Sv::boom_check_map() -> pthread_create()", ret_th, -1);

    this->tid_boom = tid;
    this->tid_boom_set = true;
    /*
    struct sigaction st_sigaction = {};
    st_sigaction.sa_flags = SA_SIGINFO;
    st_sigaction.sa_sigaction = sig_al_func;

    int ret_sigaction = sigaction(15, &st_sigaction, NULL);
    check_err::check("Sv::th_func() -> sigaction()", ret_sigaction, -1);
    */

    return;
}

void *Sv::th_func(void *vp)
{
    sleep(3);

    try
    {
        Sv *this_p = (Sv *)vp;

        sigset_t st_sigset = {};
        sigemptyset(&st_sigset);
        sigaddset(&st_sigset, SIGALRM);
        sigaddset(&st_sigset, SIGINT);
        int ret_p = pthread_sigmask(SIG_BLOCK, &st_sigset, NULL);
        check_err::check("Sv::th_func() -> pthread_sigmask()", ret_p, -1);

        struct itimerval st_itimerval = {};

        struct timeval st_timeval = {};
        st_timeval.tv_sec = 1;

        struct timeval st_timeval_2 = {};
        st_timeval_2.tv_sec = 1;

        st_itimerval.it_interval = st_timeval;
        st_itimerval.it_value = st_timeval_2;

        int ret_setitimer = setitimer(ITIMER_REAL, &st_itimerval, NULL);
        check_err::check("Sv::th_func() -> setitimer()", ret_setitimer, -1);
        cout << this_p->end_game << " game";
        while (this_p->end_game == false)
        {
            int sig_num = 0;
            int ret_sigwait = sigwait(&st_sigset, &sig_num);

            if (ret_sigwait != 0)
            {
                ret_sigwait = -1;
            }

            cout << "sig" << endl;
            if (this_p->end_game == true)
            {
                break;
            }
            sem_wait(this_p->sem_h);

            if (this_p->end_game == true &&
                this_p->get_game_over_main_th() == true)
            {
                sem_post(this_p->sem_h);
                sigwait(&st_sigset, &sig_num);

                break;
            }
            // check_err::check("Sv::th_func() -> sem_wait()", ret_sem_wait,
            // -1);

            this_p->update_boom_map();

            sem_post(this_p->sem_h);
            // check_err::check("Sv::th_func() -> sem_post()", ret_sem_post,
            // -1);
        }

        if (this_p->end_game == true && this_p->tid_gost_set == false)
        {
            this_p->set_tid_boom(false);
            cout << "boom thread -> end111111111111111\n";
            pthread_exit(nullptr);
        }
        else if (this_p->end_game == true && this_p->tid_gost_set == true)
        {
            this_p->set_tid_boom(false);

            if (this_p->gost_flag == true)
            {
                uint64_t uni = 1;
                int ret_write = write(this_p->event_fd, &uni, sizeof(uni));
                check_err::check("Sv::th_func() -> end -> write()", ret_write,
                                 -1);
                sleep(5);
            }
            else
            {
                this_p->set_tid_gost(false);
            }

            cout << "boom thread -> end 22222222222\n";
            pthread_exit(nullptr);
        }
    }
    catch (Exception err)
    {
        printf("err type == %s\n"
               "err name == %s\n"
               "err no ==%d\n",
               err.get_err_type().c_str(), err.get_err_name().c_str(),
               err.get_err_no());
        return NULL;
    }
    return nullptr;
}

void Sv::update_boom_map()
{
    // cout << "Sv::update_boom_map() :: use tid = " << gettid() << endl;

    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            if (this->shm_p->arr[i][j] == 'W')
            {
                this->shm_p->arr[i][j] = ' ';
            }
        }
    }

    this->gost_time++;

    if (this->gost_flag == false && this->gost_time >= 1000)
    {
        this->start_gost();
    }
    else if (this->gost_flag == true && this->gost_time >= 1000)
    {
        uint64_t uni = 1;
        /*
        int ret_write = write(this->event_fd, &uni, sizeof(uni));
        check_err::check("Sv::update_boom_map() -> ret_write()", ret_write, -1);
        */
        write(this->event_fd, &uni, sizeof(uni));
    }

    if (this->ch.get_die_1() == true)
    {
        this->ch.set_die_cnt_1(-1);
    }

    if (this->ch.get_die_2() == true)
    {
        this->ch.set_die_cnt_2(-1);
    }

    if ((this->ch.get_die_cnt_1() <= 0) && (this->ch.get_die_cnt_2() <= 0))
    {
        this->game_over(3);
        return;
    }
    else if (this->ch.get_die_cnt_1() <= 0)
    {
        this->game_over(1);
        return;
    }
    else if (this->ch.get_die_cnt_2() <= 0)
    {
        this->game_over(2);
        return;
    }

    this->gost_time++;
    this->ch.set_lock_1(false);
    this->ch.set_lock_2(false);

    vector<struct st_boom> *boom_all = this->ch.get_boom();
    if (boom_all->empty())
    {
        // cout << "if (boom_all.empty())" << endl;
        return;
    }
    else
    {
        for (auto &v : (*boom_all))
        {
            v.t--;
            if (v.t <= 0 && v.ox == true)
            {
                // 풍선 시간 0 이하이고
                // 현재 살아있는 풍선이면 상태 사망으로 변경
                v.ox = false;
                if (v.how == 1)
                {
                    this->ch.up_boom_num_1();
                }
                else if (v.how == 2)
                {
                    this->ch.up_boom_num_2();
                }
                boom_boom(&v);
                // 풍선 터지는거 구현
                // 풍선 터진거 지우면..
                // 시간 복잡도 더 안좋아 질 듯
                // 어자피 한 겜에 풍선 몇 천개 터르릴것 도 아니고 걍 검사하자
            }
        }
        if (this->ch.get_flag_ch_1() == false && this->ch.get_die_1() == false)
        {
            cout << "???1" << endl;
            pair<int, int> yx = this->ch.get_yx_1();
            this->shm_p->arr[yx.first][yx.second] = 'X';
            this->ch.set_lock_1(true);
            this->ch.set_die_1(true);
        }

        if (this->ch.get_flag_ch_2() == false && this->ch.get_die_2() == false)
        {
            cout << "???2" << endl;
            pair<int, int> yx = this->ch.get_yx_2();
            this->shm_p->arr[yx.first][yx.second] = 'X';
            this->ch.set_lock_2(true);
            this->ch.set_die_2(true);
        }

        if (this->ch.get_flag_ch_1() == true &&
            this->ch.get_item_car_1() == false &&
            this->ch.get_car_destory_1() == true)
        {
            pair<int, int> yx = this->ch.get_yx_1();
            this->shm_p->arr[yx.first][yx.second] = '1';
            this->ch.set_car_destory_1(false);
            this->ch.set_lock_1(true);
        }

        if (this->ch.get_flag_ch_2() == true &&
            this->ch.get_item_car_2() == false &&
            this->ch.get_car_destory_2() == true)
        {
            pair<int, int> yx = this->ch.get_yx_2();
            this->shm_p->arr[yx.first][yx.second] = '2';
            this->ch.set_car_destory_2(false);
            this->ch.set_lock_2(true);
        }
    }
    return;
}

bool Sv::boom_over(const int &y, const int &x)
{
    if (y < 0 || y >= 13 || x < 0 || x >= 15)
    {
        return true;
    }
    return false;
}

void Sv::boom_boom(struct st_boom *boom)
{
    cout << "Sv::boom_boom()" << endl;
    this->visitor.assign(13, vector<int>(15, -1));
    queue<struct st_boom *> q;
    q.push(boom);

    visitor[q.front()->yx.first][q.front()->yx.second] = 1;

    if (this->shm_p->arr[q.front()->yx.first][q.front()->yx.second] == '1' ||
        this->shm_p->arr[q.front()->yx.first][q.front()->yx.second] == '3')
    {
        if (this->ch.get_item_car_1() == true)
        {
            this->ch.get_item_car_1() = false;
            this->ch.set_car_destory_1(true);
        }
        else if (this->ch.get_item_car_1() == false)
        {
            this->ch.set_flag_ch_1(false);
        }
    }
    else if (this->shm_p->arr[q.front()->yx.first][q.front()->yx.second] ==
                 '2' ||
             this->shm_p->arr[q.front()->yx.first][q.front()->yx.second] == '4')
    {
        if (this->ch.get_item_car_2() == true)
        {
            this->ch.get_item_car_2() = false;
            this->ch.set_car_destory_2(true);
        }
        else if (this->ch.get_item_car_2() == false)
        {
            this->ch.set_flag_ch_2(false);
        }
    }

    this->shm_p->arr[q.front()->yx.first][q.front()->yx.second] = 'W';

    while (!q.empty())
    {
        unordered_map<int, bool> un_mp = {
            {8, false}, {6, false}, {2, false}, {4, false}};
        int cy = q.front()->yx.first;
        int cx = q.front()->yx.second;
        int c_pw = q.front()->pw;
        if (c_pw > 20)
        {
            c_pw = 20;
        }
        q.pop();

        for (int i = 0; i < c_pw; i++)
        {
            int ny = cy + dy[i];
            int nx = cx + dx[i];

            if (dy[i] < 0 && un_mp[8] == true)
            {
                continue;
            }
            else if (dx[i] >= 1 && un_mp[6] == true)
            {
                continue;
            }
            else if (dy[i] >= 1 && un_mp[2] == true)
            {
                continue;
            }
            else if (dx[i] < 0 && un_mp[4] == true)
            {
                continue;
            }

            if (this->boom_over(ny, nx) || this->shm_p->arr[ny][nx] == '^' ||
                this->shm_p->arr[ny][nx] == '&' ||
                this->shm_p->arr[ny][nx] == '#' ||
                this->shm_p->arr[ny][nx] == '%')
            {
                if (dy[i] < 0 && dx[i] == 0)
                {
                    un_mp[8] = true;
                }
                else if (dy[i] == 0 && dx[i] >= 1)
                {
                    un_mp[6] = true;
                }
                else if (dy[i] >= 1 && dx[i] == 0)
                {
                    un_mp[2] = true;
                }
                else if (dy[i] == 0 && dx[i] < 0)
                {
                    un_mp[4] = true;
                }

                if (this->shm_p->arr[ny][nx] == '#' ||
                    this->shm_p->arr[ny][nx] == '%')
                {
                    this->make_item(ny, nx);
                }
            }
            else if (this->shm_p->arr[ny][nx] == 'W')
            {
                continue;
            }
            else if (this->shm_p->arr[ny][nx] == '*')
            {
                pair<int, int> yx;
                yx.first = ny;
                yx.second = nx;

                vector<struct st_boom> *boom_all = this->ch.get_boom();
                for (auto &v : (*boom_all))
                {
                    if (v.yx == yx && v.ox == true)
                    {
                        if (v.how == 1)
                        {
                            this->ch.up_boom_num_1();
                        }
                        if (v.how == 2)
                        {
                            this->ch.up_boom_num_2();
                        }
                        v.ox = false;
                        q.push(&v);

                        break;
                    }
                }

                visitor[ny][nx] = 1;
                this->shm_p->arr[ny][nx] = 'W';
            }
            else if (this->shm_p->arr[ny][nx] == '3')
            {
                if (this->ch.get_item_car_1() == true)
                {
                    this->ch.get_item_car_1() = false;
                    this->ch.set_car_destory_1(true);
                }
                else if (this->ch.get_item_car_1() == false)
                {
                    this->ch.set_flag_ch_1(false);
                }

                pair<int, int> yx;
                yx.first = ny;
                yx.second = nx;

                vector<struct st_boom> *boom_all = this->ch.get_boom();
                for (auto &v : (*boom_all))
                {
                    if (v.yx == yx && v.ox == true)
                    {
                        if (v.how == 1)
                        {
                            this->ch.up_boom_num_1();
                        }
                        if (v.how == 2)
                        {
                            this->ch.up_boom_num_2();
                        }
                        v.ox = false;
                        q.push(&v);

                        break;
                    }
                }
                visitor[ny][nx] = 1;
                this->shm_p->arr[ny][nx] = 'W';
            }
            else if (this->shm_p->arr[ny][nx] == '4')
            {
                if (this->ch.get_item_car_2() == true)
                {
                    this->ch.get_item_car_2() = false;
                    this->ch.set_car_destory_2(true);
                }
                else if (this->ch.get_item_car_2() == false)
                {
                    this->ch.set_flag_ch_2(false);
                }

                pair<int, int> yx;
                yx.first = ny;
                yx.second = nx;

                vector<struct st_boom> *boom_all = this->ch.get_boom();
                for (auto &v : (*boom_all))
                {
                    if (v.yx == yx && v.ox == true)
                    {
                        if (v.how == 1)
                        {
                            this->ch.up_boom_num_1();
                        }
                        if (v.how == 2)
                        {
                            this->ch.up_boom_num_2();
                        }
                        v.ox = false;
                        q.push(&v);

                        break;
                    }
                }

                visitor[ny][nx] = 1;
                this->shm_p->arr[ny][nx] = 'W';
            }
            else if (this->shm_p->arr[ny][nx] == ' ' ||
                     this->shm_p->arr[ny][nx] == '@' ||
                     this->shm_p->arr[ny][nx] == '+' ||
                     this->shm_p->arr[ny][nx] == 'P' ||
                     this->shm_p->arr[ny][nx] == '!' ||
                     this->shm_p->arr[ny][nx] == '=')
            {
                if (this->shm_p->arr[ny][nx] == '@' &&
                    (this->ch.get_yx_1().first == ny &&
                     this->ch.get_yx_1().second == nx) &&
                    this->ch.get_item_car_1() == false)
                {
                    this->ch.set_flag_ch_1(false);

                    visitor[ny][nx] = 1;
                    this->shm_p->arr[ny][nx] = 'W';
                }
                else if (this->shm_p->arr[ny][nx] == '@' &&
                         (this->ch.get_yx_1().first == ny &&
                          this->ch.get_yx_1().second == nx) &&
                         this->ch.get_item_car_1() == true)
                {
                    this->ch.get_item_car_1() = false;
                    this->ch.set_car_destory_1(true);
                }
                else if (this->shm_p->arr[ny][nx] == '@' &&
                         (this->ch.get_yx_2().first == ny &&
                          this->ch.get_yx_2().second == nx) &&
                         this->ch.get_item_car_2() == false)
                {
                    this->ch.set_flag_ch_2(false);
                    visitor[ny][nx] = 1;

                    this->shm_p->arr[ny][nx] = 'W';
                }
                else if (this->shm_p->arr[ny][nx] == '@' &&
                         (this->ch.get_yx_2().first == ny &&
                          this->ch.get_yx_2().second == nx) &&
                         this->ch.get_item_car_2() == true)
                {
                    this->ch.get_item_car_2() = false;
                    this->ch.set_car_destory_2(true);
                }
                else
                {
                    visitor[ny][nx] = 1;
                    this->shm_p->arr[ny][nx] = 'W';
                }
            }
            else if (this->shm_p->arr[ny][nx] == '1')
            {
                this->ch.set_flag_ch_1(false);

                visitor[ny][nx] = 1;
                this->shm_p->arr[ny][nx] = 'W';
            }
            else if (this->shm_p->arr[ny][nx] == '5')
            {
                this->ch.get_item_car_1() = false;
                this->ch.set_car_destory_1(true);
            }
            else if (this->shm_p->arr[ny][nx] == '2')
            {
                this->ch.set_flag_ch_2(false);
                visitor[ny][nx] = 1;

                this->shm_p->arr[ny][nx] = 'W';
            }
            else if (this->shm_p->arr[ny][nx] == '6')
            {
                this->ch.get_item_car_2() = false;
                this->ch.set_car_destory_2(true);
            }
        }
    }
    /*
    cout << "bb end    ````\n";
    vector<struct st_boom> *boom111 = this->ch.get_boom();
    cout << "================" << endl;
    for (auto v : (*boom111))
    {
      cout << "addr: " << &v << " | yx: (" << v.yx.first << "," << v.yx.second
           << ") | ox: " << v.ox << endl;
    }
    cout << "================" << endl;
    */
}

void Sv::make_item(const int &y, const int &x)
{
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    srand(seed);
    int tmpi = rand() % 10;

    if (tmpi == 0 || tmpi == 1 || tmpi == 2)
    {
        this->shm_p->arr[y][x] = '+';
    }
    else if (tmpi == 3 || tmpi == 4 || tmpi == 5)
    {
        this->shm_p->arr[y][x] = 'P';
    }
    else if (tmpi == 6)
    {
        this->shm_p->arr[y][x] = '!';
    }
    else if (tmpi == 7)
    {
        this->shm_p->arr[y][x] = '=';
    }
    else
    {
        this->shm_p->arr[y][x] = ' ';
    }
    return;
}

void Sv::set_cmd_1_Sv(const int &cmd, const int &flag)
{
    this->ch.set_cmd_1(cmd, flag);
    return;
}

void Sv::set_cmd_2_Sv(const int &cmd, const int &flag)
{
    this->ch.set_cmd_2(cmd, flag);
    return;
}

void Sv::set_boom_1_Sv(const int &cmd)
{
    this->ch.set_boom_1(cmd);
    return;
}

void Sv::set_boom_2_Sv(const int &cmd)
{
    this->ch.set_boom_2(cmd);
    return;
}

void Sv::set_item_niddle_1_SV(const bool set)
{
    this->ch.set_item_niddle_1(set);
    return;
}

void Sv::set_item_niddle_2_Sv(const bool set)
{
    this->ch.set_item_niddle_2(set);
    return;
}

void Sv::set_sem_p(sem_t *sem_h)
{
    this->sem_h = sem_h;
    return;
}

void Sv::set_tid_main(const bool set)
{
    this->tid_main_set = set;
}

void Sv::set_tid_boom(const bool set)
{
    this->tid_boom_set = set;
}

void Sv::set_tid_gost(const bool set)
{
    this->tid_gost_set = set;
}

const bool Sv::get_tid_set_main()
{
    return this->tid_main_set;
}

const bool Sv::get_tid_set_boom()
{
    return this->tid_boom_set;
}

const bool Sv::get_tid_set_gost()
{
    return this->tid_gost_set;
}

const pthread_t Sv::get_tid_boom()
{
    return this->tid_boom;
}

const pthread_t Sv::get_tid_gost()
{
    return this->tid_gost;
}

const int &Sv::get_boom_num_1_Sv()
{
    return this->ch.get_boom_num_1();
}

const int &Sv::get_boom_num_2_Sv()
{
    return this->ch.get_boom_num_2();
}

sem_t *Sv::get_sem_p()
{
    return this->sem_h;
}

struct shm_arr *Sv::get_shm_p()
{
    return this->shm_p;
}

const bool Sv::get_game_over_main_th()
{
    return this->game_over_main_th;
}

const bool Sv::get_flag_ch_1_Sv()
{
    return this->ch.get_flag_ch_1();
}

const bool Sv::get_flag_ch_2_Sv()
{
    return this->ch.get_flag_ch_2();
}

const bool Sv::get_lock_1()
{
    return this->ch.get_lock_1();
}

const bool Sv::get_lock_2()
{
    return this->ch.get_lock_2();
}

bool &Sv::get_item_niddle_1_Sv()
{
    return this->ch.get_item_niddle_1();
}

bool &Sv::get_item_niddle_2_Sv()
{
    return this->ch.get_item_niddle_2();
}
const bool Sv::get_end_game()
{
    return this->end_game;
}

const bool Sv::get_gost_flag()
{
    return this->gost_flag;
}

void Sv::set_map()
{

    /*
    빈 공간 : 그냥 비워둠
    집(못부심) : ^
    레고블럭 : #
    밀리는 상자 : %
    나무(못부심) : &
    수풀 : @
    */

    this->village.assign(13, vector<char>(15));
    //========================================
    village[0][0] = '1';
    village[0][1] = '#';
    village[0][2] = '#';
    village[0][3] = '#';
    village[0][4] = '#';
    village[0][5] = '@';
    village[0][6] = ' ';
    village[0][7] = ' ';
    village[0][8] = '%';
    village[0][9] = '@';
    village[0][10] = '^';
    village[0][11] = '#';
    village[0][12] = '^';
    village[0][13] = ' ';
    village[0][14] = '^';
    //========================================
    // village[1][0] = ' ';
    village[1][0] = '@';
    village[1][1] = '^';
    village[1][2] = '%';
    village[1][3] = '^';
    village[1][4] = '%';
    village[1][5] = '&';
    village[1][6] = '%';
    village[1][7] = ' ';
    village[1][8] = ' ';
    village[1][9] = '&';
    village[1][10] = '#';
    village[1][11] = '#';
    village[1][12] = ' ';
    village[1][13] = ' ';
    village[1][14] = ' ';
    //========================================
    village[2][0] = ' ';
    village[2][1] = ' ';
    village[2][2] = '#';
    village[2][3] = '#';
    village[2][4] = '#';
    village[2][5] = '@';
    village[2][6] = ' ';
    village[2][7] = '%';
    village[2][8] = '%';
    village[2][9] = '@';
    village[2][10] = '^';
    village[2][11] = '%';
    village[2][12] = '^';
    village[2][13] = '%';
    village[2][14] = '^';
    //========================================
    //========================================
    village[3][0] = '%';
    village[3][1] = '^';
    village[3][2] = '%';
    village[3][3] = '^';
    village[3][4] = '%';
    village[3][5] = '&';
    village[3][6] = '%';
    village[3][7] = ' ';
    village[3][8] = ' ';
    village[3][9] = '&';
    village[3][10] = '#';
    village[3][11] = '#';
    village[3][12] = '#';
    village[3][13] = '#';
    village[3][14] = '#';
    //========================================
    //========================================
    village[4][0] = '#';
    village[4][1] = '#';
    village[4][2] = '#';
    village[4][3] = '#';
    village[4][4] = '#';
    village[4][5] = '@';
    village[4][6] = ' ';
    village[4][7] = ' ';
    village[4][8] = '%';
    village[4][9] = '@';
    village[4][10] = '^';
    village[4][11] = '%';
    village[4][12] = '^';
    village[4][13] = '%';
    village[4][14] = '^';
    //========================================
    //========================================
    village[5][0] = '#';
    village[5][1] = '^';
    village[5][2] = '#';
    village[5][3] = '^';
    village[5][4] = '#';
    village[5][5] = '&';
    village[5][6] = '%';
    village[5][7] = '%';
    village[5][8] = ' ';
    village[5][9] = ' ';
    village[5][10] = '#';
    village[5][11] = '#';
    village[5][12] = '#';
    village[5][13] = '#';
    village[5][14] = '#';
    //========================================
    //========================================
    village[6][0] = '&';
    village[6][1] = '@';
    village[6][2] = '&';
    village[6][3] = '@';
    village[6][4] = '&';
    village[6][5] = '@';
    village[6][6] = ' ';
    village[6][7] = ' ';
    village[6][8] = '%';
    village[6][9] = '@';
    village[6][10] = '&';
    village[6][11] = '@';
    village[6][12] = '&';
    village[6][13] = '@';
    village[6][14] = '&';
    //========================================
    //========================================
    village[7][0] = '#';
    village[7][1] = '#';
    village[7][2] = '#';
    village[7][3] = '#';
    village[7][4] = '#';
    village[7][5] = ' ';
    village[7][6] = '%';
    village[7][7] = ' ';
    village[7][8] = ' ';
    village[7][9] = '&';
    village[7][10] = '#';
    village[7][11] = '^';
    village[7][12] = '#';
    village[7][13] = '^';
    village[7][14] = '#';
    //========================================
    //========================================
    village[8][0] = '^';
    village[8][1] = '%';
    village[8][2] = '^';
    village[8][3] = '%';
    village[8][4] = '^';
    village[8][5] = '@';
    village[8][6] = ' ';
    village[8][7] = '%';
    village[8][8] = '%';
    village[8][9] = '@';
    village[8][10] = '#';
    village[8][11] = '#';
    village[8][12] = '#';
    village[8][13] = '#';
    village[8][14] = '#';
    //========================================
    //========================================
    village[9][0] = '#';
    village[9][1] = '#';
    village[9][2] = '#';
    village[9][3] = '#';
    village[9][4] = '#';
    village[9][5] = '&';
    village[9][6] = '%';
    village[9][7] = ' ';
    village[9][8] = ' ';
    village[9][9] = '&';
    village[9][10] = '%';
    village[9][11] = '^';
    village[9][12] = '%';
    village[9][13] = '^';
    village[9][14] = '%';
    //========================================
    //========================================
    village[10][0] = '^';
    village[10][1] = ' ';
    village[10][2] = '^';
    village[10][3] = '%';
    village[10][4] = '^';
    village[10][5] = '@';
    village[10][6] = ' ';
    village[10][7] = ' ';
    village[10][8] = '%';
    village[10][9] = '@';
    village[10][10] = '#';
    village[10][11] = '#';
    village[10][12] = '#';
    village[10][13] = '#';
    village[10][14] = ' ';
    //========================================
    //========================================
    village[11][0] = ' ';
    village[11][1] = ' ';
    village[11][2] = '#';
    village[11][3] = '#';
    village[11][4] = '#';
    village[11][5] = '&';
    village[11][6] = '%';
    village[11][7] = '%';
    village[11][8] = ' ';
    village[11][9] = '&';
    village[11][10] = '%';
    village[11][11] = '^';
    village[11][12] = '%';
    village[11][13] = '^';
    // village[11][14] = ' ';
    village[11][14] = '%';
    //========================================
    //========================================
    village[12][0] = '^';
    village[12][1] = ' ';
    village[12][2] = '^';
    village[12][3] = '#';
    village[12][4] = '^';
    village[12][5] = '@';
    village[12][6] = ' ';
    village[12][7] = ' ';
    village[12][8] = '%';
    village[12][9] = '@';
    village[12][10] = '#';
    village[12][11] = '#';
    village[12][12] = '#';
    village[12][13] = ' ';
    village[12][14] = '2';
    //========================================
    //========================================
    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            this->shm_p->arr[i][j] = this->village[i][j];
        }
    }
    return;
}
void Sv::set_map_2()
{

    /*
    빈 공간 : 그냥 비워둠
    집(못부심) : ^
    레고블럭 : #
    밀리는 상자 : %
    나무(못부심) : &
    수풀 : @
    */

    this->village.assign(13, vector<char>(15));
    //========================================
    village[0][0] = '1';
    village[0][1] = ' ';
    village[0][2] = ' ';
    village[0][3] = ' ';
    village[0][4] = ' ';
    village[0][5] = ' ';
    village[0][6] = ' ';
    village[0][7] = ' ';
    village[0][8] = ' ';
    village[0][9] = ' ';
    village[0][10] = ' ';
    village[0][11] = ' ';
    village[0][12] = ' ';
    village[0][13] = ' ';
    village[0][14] = ' ';
    //========================================
    // village[1][0] = ' ';
    village[1][0] = ' ';
    village[1][1] = ' ';
    village[1][2] = ' ';
    village[1][3] = ' ';
    village[1][4] = ' ';
    village[1][5] = ' ';
    village[1][6] = ' ';
    village[1][7] = ' ';
    village[1][8] = ' ';
    village[1][9] = ' ';
    village[1][10] = ' ';
    village[1][11] = ' ';
    village[1][12] = ' ';
    village[1][13] = ' ';
    village[1][14] = ' ';
    //========================================
    village[2][0] = ' ';
    village[2][1] = ' ';
    village[2][2] = ' ';
    village[2][3] = ' ';
    village[2][4] = ' ';
    village[2][5] = ' ';
    village[2][6] = ' ';
    village[2][7] = ' ';
    village[2][8] = ' ';
    village[2][9] = ' ';
    village[2][10] = ' ';
    village[2][11] = ' ';
    village[2][12] = ' ';
    village[2][13] = ' ';
    village[2][14] = ' ';
    //========================================
    //========================================
    village[3][0] = ' ';
    village[3][1] = ' ';
    village[3][2] = ' ';
    village[3][3] = ' ';
    village[3][4] = ' ';
    village[3][5] = ' ';
    village[3][6] = ' ';
    village[3][7] = ' ';
    village[3][8] = ' ';
    village[3][9] = ' ';
    village[3][10] = ' ';
    village[3][11] = ' ';
    village[3][12] = ' ';
    village[3][13] = ' ';
    village[3][14] = ' ';
    //========================================
    //========================================
    village[4][0] = ' ';
    village[4][1] = ' ';
    village[4][2] = ' ';
    village[4][3] = ' ';
    village[4][4] = ' ';
    village[4][5] = ' ';
    village[4][6] = ' ';
    village[4][7] = ' ';
    village[4][8] = ' ';
    village[4][9] = ' ';
    village[4][10] = ' ';
    village[4][11] = ' ';
    village[4][12] = ' ';
    village[4][13] = ' ';
    village[4][14] = ' ';
    //========================================
    //========================================
    village[5][0] = ' ';
    village[5][1] = ' ';
    village[5][2] = ' ';
    village[5][3] = ' ';
    village[5][4] = ' ';
    village[5][5] = ' ';
    village[5][6] = ' ';
    village[5][7] = ' ';
    village[5][8] = ' ';
    village[5][9] = ' ';
    village[5][10] = ' ';
    village[5][11] = ' ';
    village[5][12] = ' ';
    village[5][13] = ' ';
    village[5][14] = ' ';
    //========================================
    //========================================
    village[6][0] = ' ';
    village[6][1] = ' ';
    village[6][2] = ' ';
    village[6][3] = ' ';
    village[6][4] = ' ';
    village[6][5] = ' ';
    village[6][6] = ' ';
    village[6][7] = ' ';
    village[6][8] = ' ';
    village[6][9] = ' ';
    village[6][10] = ' ';
    village[6][11] = ' ';
    village[6][12] = ' ';
    village[6][13] = ' ';
    village[6][14] = ' ';
    //========================================
    //========================================
    village[7][0] = ' ';
    village[7][1] = ' ';
    village[7][2] = ' ';
    village[7][3] = ' ';
    village[7][4] = ' ';
    village[7][5] = ' ';
    village[7][6] = ' ';
    village[7][7] = ' ';
    village[7][8] = ' ';
    village[7][9] = ' ';
    village[7][10] = ' ';
    village[7][11] = ' ';
    village[7][12] = ' ';
    village[7][13] = ' ';
    village[7][14] = ' ';
    //========================================
    //========================================
    village[8][0] = ' ';
    village[8][1] = ' ';
    village[8][2] = ' ';
    village[8][3] = ' ';
    village[8][4] = ' ';
    village[8][5] = ' ';
    village[8][6] = ' ';
    village[8][7] = ' ';
    village[8][8] = ' ';
    village[8][9] = ' ';
    village[8][10] = ' ';
    village[8][11] = ' ';
    village[8][12] = ' ';
    village[8][13] = ' ';
    village[8][14] = ' ';
    //========================================
    //========================================
    village[9][0] = ' ';
    village[9][1] = ' ';
    village[9][2] = ' ';
    village[9][3] = ' ';
    village[9][4] = ' ';
    village[9][5] = ' ';
    village[9][6] = ' ';
    village[9][7] = ' ';
    village[9][8] = ' ';
    village[9][9] = ' ';
    village[9][10] = ' ';
    village[9][11] = ' ';
    village[9][12] = ' ';
    village[9][13] = ' ';
    village[9][14] = ' ';
    //========================================
    //========================================
    village[10][0] = ' ';
    village[10][1] = ' ';
    village[10][2] = ' ';
    village[10][3] = ' ';
    village[10][4] = ' ';
    village[10][5] = ' ';
    village[10][6] = ' ';
    village[10][7] = ' ';
    village[10][8] = ' ';
    village[10][9] = ' ';
    village[10][10] = ' ';
    village[10][11] = ' ';
    village[10][12] = ' ';
    village[10][13] = ' ';
    village[10][14] = ' ';
    //========================================
    //========================================
    village[11][0] = ' ';
    village[11][1] = ' ';
    village[11][2] = ' ';
    village[11][3] = ' ';
    village[11][4] = ' ';
    village[11][5] = ' ';
    village[11][6] = ' ';
    village[11][7] = ' ';
    village[11][8] = ' ';
    village[11][9] = ' ';
    village[11][10] = ' ';
    village[11][11] = ' ';
    village[11][12] = ' ';
    village[11][13] = ' ';
    // village[11][14] = ' ';
    village[11][14] = ' ';
    //========================================
    //========================================
    village[12][0] = ' ';
    village[12][1] = ' ';
    village[12][2] = ' ';
    village[12][3] = ' ';
    village[12][4] = ' ';
    village[12][5] = ' ';
    village[12][6] = ' ';
    village[12][7] = ' ';
    village[12][8] = ' ';
    village[12][9] = ' ';
    village[12][10] = ' ';
    village[12][11] = ' ';
    village[12][12] = ' ';
    village[12][13] = ' ';
    village[12][14] = '2';
    //========================================
    //========================================
    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            this->shm_p->arr[i][j] = this->village[i][j];
        }
    }
    return;
}
