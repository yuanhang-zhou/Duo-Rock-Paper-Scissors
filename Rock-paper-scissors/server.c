#include "common.h"

#define UNSUCCESSFULNEWUSER '\x00'
#define SUCCESSFULNEWUSER '\x01'
#define UNSUCCESSFULNEWGAME '\x00'
#define SUCCESSFULNEWGAME '\x01'
#define REFUSE '\x00'
#define ACCEPT '\x01'
#define HASWINNER '\x00'
#define DRAW '\x01'
#define OUT '\x00'
#define IN '\x01'
#define LOSE '\x00'
#define WIN '\x01'
#define NOTATTACKED '\x00'
#define ATTACKED '\x01'

#define SORTBYNAME '\x00'
#define SORTBYSTATUS '\x01'

#define DEFAULT '\x00'

#define NAMESIZE 5
#define SIGNSIZE 10
#define USERSIZE 1024
#define BUFSIZE 4096

pthread_mutex_t g_mutex;
 
typedef struct user{
    int sock;
    char name[5];
    int score;
    bool status;    //1:in game
}user;

int usernum = 0;
user userlist[USERSIZE];

void pi(int message) {
    printf("%d\n", message);
}

void ps(char* message) {
    printf("%s\n", message);
}

void pm(char* message) {
    printf("message: \n");
    for(int i = 0; i < 200; i++){
        printf("%d ", message[i]);
    }
    printf("\n");
}

void writesign(char* message, char* sign){
    for(int i = 0; i < SIGNSIZE; i++){
        message[i] = sign[i];
    }
}

bool repeatedname(char* name, user* userlist){
    for(int i = 0; i < usernum; i++){
        if(strcmp(name, userlist[i].name) == 0){
            return true;
        }
    }
    return false;
}

int finduser(char* name, user* userlist){
    for(int i = 0; i < usernum; i++){
        if(strcmp(name, userlist[i].name) == 0){
            if(userlist[i].status == 0)
                return userlist[i].sock;
            else
                return 0;
        }
    }
    return 0;
}

void printuserlist(user* userlist){
    for(int i = 0; i < usernum; i++){
        printf("Name:%s\n", userlist[i].name);
        printf("Score:%d\n", userlist[i].score);
        printf("Status:%d\n", userlist[i].status);
        printf("Sock:%d\n\n", userlist[i].sock);
    }
}

char** updateuserlist(user* userlist, int sortby){
    if(sortby == SORTBYNAME){
        for(int i = 0; i < usernum; i++){
            for(int j = 0; j < usernum - i - 1; j++){
                if(strcmp(userlist[j].name, userlist[j+1].name) > 0){
                    user temp = userlist[j];
                    userlist[j] = userlist[j+1];
                    userlist[j+1] = temp;
                }
            }
        }
    }
    else if(sortby== SORTBYSTATUS){
        for(int i = 0; i < usernum; i++){
            for(int j = 0; j < usernum - i - 1; j++){
                if(userlist[j].status < userlist[j+1].status){
                    user temp = userlist[j];
                    userlist[j] = userlist[j+1];
                    userlist[j+1] = temp;
                }
            }
        }
    }
    else{
        perror("Problem in updateuserlist");
        exit(1);
    }
    char** buf = (char**)malloc(usernum*NAMESIZE);
    for(int i = 0; i < usernum; i++){
        buf[i] = (char*)malloc(NAMESIZE);
        strcpy(buf[i], userlist[i].name);
    }
    return buf;
}

// 0: Rock 1: Paper 2: Scissors
// 0: Draw 1: b win -1: a win
int Rock_Paper_Scissors(char a, char b){
    if(a == b){
        return 0;
    }
    else if(a == '1' && b == '1'){
        return 1;
    }
    else if(a == '0' && b == '2'){
        return -1;
    }
    else if(a == '1' && b == '0'){
        return -1;
    }
    else if(a == '1' && b == '2'){
        return 1;
    }
    else if(a == '2' && b == '0'){
        return 1;
    }
    else if(a == '2' && b == '1'){
        return -1;
    }
    else{
        perror("Problem in Rock_Paper_Scissors");
        exit(1);
    }
}

void writetoall(user* userlist, char* buf, int usernum){
    for(int i = 0; i < usernum; i++){
        write(userlist[i].sock, buf, BUFSIZE);
    }
}

void* proc(void* p) {
    int sock = *(int*)p;
    char sign[SIGNSIZE];
    char message[BUFSIZE];
    memset(sign, 0, sizeof(sign));
    memset(message, 0, sizeof(message));

    char name[NAMESIZE];
    memset(name, 0, sizeof(name));
    read(sock, message, BUFSIZE);
    strncpy(name, message+SIGNSIZE, NAMESIZE);

    while(repeatedname(name, userlist)){
        sprintf(sign,"%c%c%c%c%c",UNSUCCESSFULNEWUSER,DEFAULT,DEFAULT,DEFAULT,DEFAULT);
        writesign(message,sign);
        write(sock, message, BUFSIZE);
        memset(name, 0, sizeof(name));
        read(sock, message, BUFSIZE);
        strncpy(name, message+SIGNSIZE, NAMESIZE);
    }

    strncpy(userlist[usernum].name, name, NAMESIZE);
    userlist[usernum].score = 0;
    userlist[usernum].status = 0;
    userlist[usernum].sock = sock;
    printf("Add new user: %s\n", userlist[usernum].name);
    printf("Sock:%d\n", userlist[usernum].sock);

    pthread_mutex_lock(&g_mutex);
    usernum++;
    pthread_mutex_unlock(&g_mutex);

    printf("Current user number:%d\n\n", usernum);
    printf("Current user list:\n");
    char** userarr = updateuserlist(userlist, DEFAULT);
    printuserlist(userlist);
    
    //name
    for(int i = 0; i < usernum; i++){
        strcpy(message+SIGNSIZE+NAMESIZE*i,userarr[i]);
    }
    //score
    for(int i = 0; i < usernum; i++){
        char* score = (char*)malloc(4);
        sprintf(score, "%d", userlist[i].score);
        strcpy(message+2048+NAMESIZE*i,score);
    }

    sprintf(sign,"%c%c%c%c%c",SUCCESSFULNEWUSER,DEFAULT,DEFAULT,IN,DEFAULT);
    writesign(message,sign);
        
    char* usernumarr = (char*)malloc(4);
    sprintf(usernumarr, "%d", usernum);
    strcpy(message+6,usernumarr);

    //pm(message);
    writetoall(userlist, message, usernum);

    while(true) {
        int len = read(sock, message, BUFSIZE);
        //pm(message);
        if(0 >= len) {
            // close connect
            pthread_mutex_lock(&g_mutex);
            for(int i = 0; i < usernum; i++) {
                if(userlist[i].sock == sock) {
                    usernum--;
                    while(i < usernum) {
                        userlist[i] = userlist[i+1];
                        i++;
                    }
                    break;
                }
            }
            pthread_mutex_unlock(&g_mutex);

            printf("Current user number:%d\n\n", usernum);
            printf("Current user list:\n");
            char** userarr = updateuserlist(userlist, DEFAULT);
            printuserlist(userlist);

            for(int i = 0; i < usernum; i++){
                strcpy(message+SIGNSIZE+NAMESIZE*i,userarr[i]);
            }
            sprintf(sign,"%c%c%c%c%c",SUCCESSFULNEWUSER,DEFAULT,DEFAULT,OUT,DEFAULT);
            writesign(message,sign);
            
            char* usernumarr = (char*)malloc(4);
            sprintf(usernumarr, "%d", usernum);
            strcpy(message+6,usernumarr);
            strcpy(message+SIGNSIZE,name);
            writetoall(userlist, message, usernum);

            break;
            
        } 
        else{
            if(message[0])
            {
                printf("Update userlist\n");
                char** userarr = updateuserlist(userlist, DEFAULT);
                printuserlist(userlist);

                for(int i = 0; i < usernum; i++){
                    strcpy(message+SIGNSIZE+NAMESIZE*i,userarr[i]);
                }
                sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT);
                writesign(message,sign);
                char* usernumarr = (char*)malloc(4);
                sprintf(usernumarr, "%d", usernum);
                strcpy(message+6,usernumarr);
                
                //pm(message);
                write(sock, message, BUFSIZE);
            }
            else if(message[1])
            {
                //attack
                char attackname[NAMESIZE];
                strncpy(attackname, message+SIGNSIZE, NAMESIZE);
                int attacksock = finduser(attackname, userlist);
                if(!attacksock){
                    //in game
                    sprintf(sign,"%c%c%c%c%c",DEFAULT,UNSUCCESSFULNEWGAME,DEFAULT,DEFAULT,DEFAULT);
                    writesign(message,sign);
                    write(sock, message, BUFSIZE);
                }
                else{
                    //not in game
                    sprintf(sign,"%c%c%c%c%c",DEFAULT,SUCCESSFULNEWGAME,DEFAULT,DEFAULT,DEFAULT);
                    writesign(message,sign);
                    write(sock, message, BUFSIZE);

                    sprintf(sign,"%c%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT,ATTACKED);
                    writesign(message,sign);
                    write(attacksock, message, BUFSIZE);
                    read(attacksock, message, BUFSIZE);

                    if(message[2] == '\x01'){
                        sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,ACCEPT,DEFAULT,DEFAULT);
                        writesign(message,sign);
                        write(sock, message, BUFSIZE);

                        read(sock, message, BUFSIZE);
                        int attacker = message[SIGNSIZE];
                        read(attacksock, message, BUFSIZE);
                        int attackee = message[SIGNSIZE];

                        int result = Rock_Paper_Scissors(attacker, attackee);

                        pi(result); 
                        if(result == 0){
                            sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,DRAW,DEFAULT);
                            writesign(message,sign);
                            write(sock, message, BUFSIZE);
                            write(attacksock, message, BUFSIZE);
                        }
                        else if(result == -1){
                            sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,HASWINNER,WIN);
                            writesign(message,sign);
                            write(sock, message, BUFSIZE);
                            sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,HASWINNER,LOSE);
                            writesign(message,sign);
                            write(attacksock, message, BUFSIZE);
                        }
                        else{
                            sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,HASWINNER,LOSE);
                            writesign(message,sign);
                            write(sock, message, BUFSIZE);
                            sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,DEFAULT,HASWINNER,WIN);
                            writesign(message,sign);
                            write(attacksock, message, BUFSIZE);
                        }

                    }
                    else{
                        sprintf(sign,"%c%c%c%c%c",DEFAULT,DEFAULT,REFUSE,DEFAULT,DEFAULT);
                        writesign(message,sign);
                        write(sock, message, BUFSIZE);
                    }

                }

            }
            
            
            
            memset(message, 0, sizeof(message));
        }
    }
 
    return NULL;
}
 
int main(int argc, char **argv) {
    pthread_mutex_init(&g_mutex, NULL);
 
    
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == sock){
        printf("socket() error");
        return -1;
    }
 
    struct sockaddr_in adrSock;
    adrSock.sin_family = PF_INET;
    adrSock.sin_addr.s_addr = INADDR_ANY;
    adrSock.sin_port = htons(1111);
 
    bind(sock, (struct sockaddr *)&adrSock, sizeof(adrSock));
 
    listen(sock, USERSIZE);
 
    socklen_t lenSock = sizeof(adrSock);
    while(true) {;
        int sockClnt = accept(sock, (struct sockaddr *)&adrSock, &lenSock);
        if (-1 == sockClnt) {
            printf("accept error");
            continue;
        }
 
        pthread_t thd;
        if(pthread_create(&thd, NULL, proc, (void*)&sockClnt) != 0)
        {
            perror("Failed to create thread");
            return -1;
        };
        pthread_detach(thd);
    }
 
    close(sock);

    pthread_mutex_destroy(&g_mutex);
    return 0;
}