#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h> // header file for network-related functions and structures (e.g., getaddrinfo)
#include <netinet/in.h> // header file for internet protocol-related functions and structures (e.g., sockaddr_in)
#include <sys/socket.h> // header file for socket programming functions and structures (e.g., socket, bind, listen, accept)
#include <arpa/inet.h> // header file for internet protocol-related functions (e.g., inet_addr, inet_ntoa)
#include <fcntl.h>
#include <ncurses.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <set>
#include <string>
#include <algorithm>
#include <iomanip>
#include <string.h>

#define Color_Off "\033[0m"
#define BBlack "\033[1;30m"
#define BRed "\033[1;31m"
#define BGreen "\033[1;32m"
#define BYellow "\033[1;33m"
#define BBlue "\033[1;34m"
#define BPurple "\033[1;35m"
#define BCyan "\033[1;36m"
#define BWhite "\033[1;37m"

#define PORT "3490"
#define BUFFERSIZE 100

using namespace std;
struct sockaddr_in serv_addr;

char ToSend[10000], rcv[10000];
int inc;

class _info {
    private:
        int tab;
        int sock;
        string name;
        string msg_to;
        string old_to_send;
        set<string> list_users;
    public:
        _info()                             : tab(0), sock(0) {}
        set<string> get_list_users()        { return this->list_users; }
        string get_old_to_send()            { return this->old_to_send; }
        string get_msg_to()                 { return this->msg_to; }
        string get_name()                   { return this->name; }
        int get_sock()                      { return this->sock; }
        int get_tab()                       { return this->tab; }

        void remove_list_users(string rmv)  { this->list_users.erase(find(list_users.begin(), list_users.end(), rmv)); }
        void set_list_users(string add)     { this->list_users.insert(add); }
        void set_old_to_send(string to_send){ this->old_to_send = to_send; }
        void set_msg_to(string to)          { this->msg_to = to; }
        void set_name(string name)          { this->name = name; }
        void set_sock(int sock)             { this->sock = sock; }
        void set_tab(int tab)               { this->tab = tab; }

        void    hid_prev_msg(int hide) { for (int k = 0; k < hide; k++) write(1, " ", 1); }

        void    msg_sending(string back_stay, string color, string to_send, short pass = 0)   {
            if (pass == 2 && !name.empty())
                cout << back_stay << color << "Me -> \'" << Color_Off << to_send.substr(to_send.find(' ') + 1, to_send.length() - to_send.find(' ') - 2) << color << " \' " << BRed << "==>" << Color_Off << " : " << BPurple << "@" << BWhite << to_send.substr(1, to_send.find(' ')) << Color_Off << ": " << endl;
            else if (pass == 3)
                cout << back_stay << color << "Me -> " << Color_Off << to_send << flush;
            else if (to_send.find(" ") == string::npos || pass == 1) {
                cout << back_stay << color << "Me -> " << Color_Off << to_send << flush;
                hid_prev_msg(15);
                cout << "\r" << color << "Me -> " << Color_Off << to_send << flush;
            }
            else if (!pass) {
                cout << back_stay << color << "Me -> \'" << Color_Off << to_send.substr(to_send.find(' ') + 1) << color << " \' " << BRed << "==>" << Color_Off << " : " << BPurple << "@" << BWhite << to_send.substr(1, to_send.find(' ')) << Color_Off << ": " << flush;
                cout << "\r" << color << "Me -> \'" << Color_Off << to_send.substr(to_send.find(' ') + 1) << flush;
            }
        }

}   info;

void *receive(void *t) {
    while (true)
    {
        int i = 0;
        memset(rcv, 0, 10000);

        while(read(info.get_sock(), rcv + i, 1) > 0)
            if (*(rcv + i) == -2 || *(rcv + i) == -4) {
                
                int sav = *(rcv + i); *(rcv + i) = 0;

                if (sav == -2 && *rcv != ' ')       info.set_list_users(rcv);
                else if (sav == -4 && i)  info.remove_list_users(rcv);

                memset(rcv, 0, 10000); i = 0;
            } else if (*(rcv + i++) == '\n') break;
        info.set_tab(0);
        cout << "\r" << rcv << flush;
        info.hid_prev_msg(int(strlen(ToSend)) - int(strlen(rcv)));
        info.msg_sending("", BGreen, ToSend);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    struct termios old_term, new_term;
    tcgetattr(0, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_term);

    if (argc!=3) {
        cerr << "usage : " << argv[0] << " <IP> <port> \n";
        exit(1);
    }
    int sock;
    if ((sock=socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "socket() error!\n";
        exit(1);
    }
    info.set_sock(sock);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        cerr << "connect() error!\n";
        exit(1);
    }
    pthread_t trd;
    pthread_create(&trd, NULL, &receive, NULL);

    while (true) {
        inc = 0;
        memset(ToSend, 0, 10000);

        cout << BGreen << "Me -> " << Color_Off << flush;
        char c = 0;
        while(read(0, ToSend + inc, 1) > 0)
        {  
            c = *(ToSend + inc);
            if (c == 27) {
                getchar();
                char key = getchar();
                if (key == 'A' || key == 'B' || key == 'C' || key == 'D') 
                    continue;
            }
            else if (*(ToSend + inc++) == '\n') {
                if ((string(ToSend).find(" ") == string::npos && !info.get_name().empty()) || inc == 1)
                    *(ToSend + --inc) = 0;
                else
                    break;
            } else if (*ToSend != '@' && !info.get_name().empty()) {
                *(ToSend + --inc) = 0;
            } else if (*ToSend == '@' && (inc == 1 || *(ToSend + inc - 1) == '\t')) {
                if (*(ToSend + inc - 1) == '\t') *(ToSend + --inc) = 0;
                string to, crnt;
                int pos = 0, users = 0;
                set<string> list = info.get_list_users();
                for (auto &x : list) {
                    if (info.get_tab() && (pos = x.find(info.get_old_to_send().substr(1))) != string::npos && !x.empty()) {
                        if (users++ == (info.get_tab() - 1) % list.size())
                            crnt = x;
                    } else if (!info.get_tab() && (pos = x.find(string(ToSend).substr(1))) != string::npos && !x.empty())
                        to = x, users++;
                }
                if (!info.get_tab())
                {
                    info.set_old_to_send(ToSend);
                    if (users > 1) {
                        cout << "\r" << flush;
                        for (int x = 0; x < 30; x++) cout << "          " << flush;
                        cout << "\rusers : " << flush;
                        for (auto &x : list)
                            if ((pos = x.find(string(ToSend).substr(1))) != string::npos && !x.empty())
                                cout << "@" << x.substr(0, pos) << BRed << x.substr(pos, inc - 1) << Color_Off << x.substr(pos+inc - 1) << "    " << flush;
                    }
                    if (users > 1)
                        cout << endl;
                    else if (users == 1)
                        inc = strlen(string("@" + to + " ").c_str()), memcpy(ToSend, string("@" + to + " ").c_str(), inc);
                    info.msg_sending("\r", BGreen, ToSend);
                } else {
                    memset(ToSend, 0, 10000);
                    inc = strlen(string("@" + crnt).c_str()), memcpy(ToSend, string("@" + crnt).c_str(), inc);
                    info.msg_sending("\r", BGreen, ToSend);
                }
                info.set_tab(info.get_tab() + 1);
            }
            else if (*(ToSend + inc - 1) == 127) {
                *(ToSend + --inc) = 0;
                if (inc) {
                    *(ToSend + --inc) = 0;
                    info.msg_sending("\r", BGreen, string(ToSend) + " ", 1);
                    info.msg_sending("\r", BGreen, ToSend);
                }
                info.set_tab(0);
                info.set_old_to_send(ToSend);
            } else
                info.set_tab(0),
                info.msg_sending("\r", BGreen, ToSend);
        }
        if (info.get_name().empty())
            info.msg_sending("\r", BBlue, ToSend, 3),
            info.set_name(ToSend);
        else
            info.set_old_to_send(ToSend),
            info.msg_sending("\r", BBlue, ToSend, 2);
        info.set_tab(0);
        write(sock, ToSend, inc);
    }
    close(sock);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    return 0;
}


 