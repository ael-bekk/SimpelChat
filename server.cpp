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
#include <sys/select.h>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <signal.h>

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

int serv_sock;
int clnt_sock;
char buf;
struct sockaddr_in serv_addr;
struct sockaddr_in clnt_addr;
socklen_t clnt_addr_size;
int k = 1;

int read_len=0;

string snd;

map<string, int> users;
map<int, string> fd_list;
vector<string> list_users;

class client {
    private:
        bool    enter_name;
        string  name;
        char    msg[10000];
        int     cursor;
    public:
        client(){
            enter_name = 1;
            cursor = 0;
            memset(msg, 0, 10000);
        }

        void    get_in(int from_fd, fd_set fds, int fd_max) {
            name = string(msg).substr(0, cursor);
            cout << "\r" << name << " is connected" << endl;
            cout << BGreen << "Me -> " << Color_Off << flush;
            char c = -2;
            for (int j = 0; j < list_users.size(); j++)
                write(from_fd, list_users[j].c_str(), strlen(list_users[j].c_str())),
                write(from_fd, &c, 1);

            for (int j = 3; j <= fd_max; j++)
                if (j != from_fd && FD_ISSET(j, &fds)) {
                    write(j, name.c_str(), strlen(name.c_str()));
                    write(j, &c, 1);
                }

            users[name] = from_fd;
            fd_list[from_fd] = name;
            list_users.push_back(name);
            enter_name = 0; cursor = 0; memset(msg, 0, 10000);
        }

        void    get_out(int from_fd, fd_set fds, int fd_max) {
            char c = -4;
            for (int j = 1; j <= fd_max; j++)
            {
                 if (j != from_fd && FD_ISSET(j, &fds)) {
                    write(j, name.c_str(), cursor);
                    write(j, &c, 1);
                }

            }
            vector <string>::iterator it = find(list_users.begin(), list_users.end(),name);
            if (list_users.end() != it)
                list_users.erase(it);
            FD_CLR(from_fd, &fds);
            fd_list[from_fd] = "";
            close(from_fd);
            cout << "\r" << name << " is disconnected" << endl;
            enter_name = 1; cursor = 0; memset(msg, 0, 10000);
        }

        void    error(string err) {
            cerr << err << " error!\n";
            exit(1);
        }

        void    receive(int from_fd, fd_set fds, int fd_max) {
            int         rd;
            const char  *ToSend = snd.c_str();

            if ((rd = read(from_fd, msg + cursor, 1)) == -1)
                error("read()");
            else if (!rd)
                get_out(from_fd, fds, fd_max);
            else if (enter_name && *(msg + cursor) == '\n')
                get_in(from_fd, fds, fd_max);
            else if (*(msg + cursor++) == '\n') {
                string to = string(msg).substr(1, string(msg).find(' ') - 1);
                if (to == "everyone") {
                    for (int j = 3; j <= fd_max; j++)
                        if (FD_ISSET(j, &fds) && j != from_fd) {
                            write(j, BWhite, strlen(BWhite));
                            if (*msg == '@')
                                write(j, fd_list[from_fd].c_str(), strlen(fd_list[from_fd].c_str()));
                            else
                                write(j, "anonymous", 9);
                            write(j, BRed, strlen(BRed));
                            write(j, " --> ", 5);
                            write(j, Color_Off, strlen(Color_Off));
                            write(j, msg + 10, cursor - 10);
                        }
                    cursor = 0; memset(msg, 0, 10000); to = snd = "";
                } else if (users[to]) {
                    int go_to = users[to];
                    char c = -2;
                    for (int j = 0; j < list_users.size(); j++)
                        if (name != list_users[j])
                            write(from_fd, list_users[j].c_str(), strlen(list_users[j].c_str())),
                            write(from_fd, &c, 1);
                    cout << "\r" << BWhite << name << " -> " << Color_Off << flush;
                    write(1, msg, cursor - 1);
                    for (int k = -2; k < int(strlen(ToSend)) - int(strlen(msg)); k++)
                        write(1, " ", 1);
                    write(1, "\n", 1);
                    cout << BGreen << "Me -> " << Color_Off << flush;
                    cout << ToSend << flush;
                    if (FD_ISSET(go_to, &fds)) {
                        write(go_to, BWhite, strlen(BWhite));
                        if (*msg == '@')
                            write(go_to, fd_list[from_fd].c_str(), strlen(fd_list[from_fd].c_str()));
                        else
                            write(go_to, "anonymous", 9);
                        write(go_to, BRed, strlen(BRed));
                        write(go_to, " --> ", 5);
                        write(go_to, Color_Off, strlen(Color_Off));
                        write(go_to, msg + strlen(to.c_str()) + 2, cursor - strlen(to.c_str()) - 2);
                    }
                    cursor = 0; memset(msg, 0, 10000);
                }
            }
        }

        void    send(int from_fd, int to_fd, fd_set fds, int fd_max) {

            if (read(0, msg + cursor, 1) == -1)
                error("read()");

            snd += *(msg + cursor);
            if (*(msg + cursor++) == '\n') {
                cout << "\r" << BBlue << "Me -> " << Color_Off << msg;

                for (int j = 1; j <= fd_max; j++)
                    if (FD_ISSET(j, &fds))
                        write(j, msg, cursor);

                cout << BGreen << "Me -> " << Color_Off << flush;

                cursor = 0; memset(msg, 0, 10000); snd = "";
            } else
                write(1, (msg + cursor - 1), 1);
        }

        void set_name(string N) {
            this->name = N;
        }

        string get_name() {
            return this->name;
        }
        const char *get_msg() {
            return string(this->msg).c_str();
        }
};

int main(int argc, char *argv[])
{
    signal(SIGPIPE,SIG_IGN);
    list_users.push_back("everyone");
    fd_set fds, copy_fds;
    int result, str_len;
    struct timeval timeout;
    int fd_max, fd_num;

    struct termios old_term, new_term;
    tcgetattr(0, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_term);

    vector <client> CLNT(1024);
    if (argc!=2) {
        cerr << "usage : " << argv[0] << " <port> \n";
        exit(1);
    }

    if ((serv_sock=socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "socket() error!\n";
        exit(1);
    }

    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &k, sizeof(int)) < 0) {
        cerr << "setsockopt() error!\n";
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        cerr << "bind() error!\n";
        exit(1);
    }

    if (listen(serv_sock, 5) == -1) {
        cerr << "listen() error!\n";
        exit(1);
    }
    
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(serv_sock, &fds);
    fd_max = serv_sock;
    
    cout << BGreen << "Me -> " << Color_Off << flush;

    while (true) {
        copy_fds = fds;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;

        fd_num = select(fd_max + 1, &copy_fds, 0, 0, &timeout);
        if (!fd_num)
            continue;

        for (int i = 0; i <= fd_max; i++) {
            
            if (FD_ISSET(i, &copy_fds)) {
                if (i == serv_sock) {
                    clnt_addr_size = sizeof(clnt_addr);
                    if ((clnt_sock = accept(serv_sock, (struct sockaddr*)&serv_addr, &clnt_addr_size)) == -1) {
                        cerr << "accept() error!\n";
                        exit(1);
                    }
                    FD_SET(clnt_sock, &fds);
                    fd_max = max(clnt_sock, fd_max);
                    write(clnt_sock, "\rplease enter your name :\n", 27);
                } else if (!i)
                    CLNT[i].send(0, 0, fds, fd_max);
                else
                    CLNT[i].receive(i, fds, fd_max);
            } 
        }
    }
    close(clnt_sock);
    close(serv_sock);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    return 0;
}