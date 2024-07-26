#include "common.h"
#define CLEAR_BUF \
    {\
    int ch; \
    while ((ch = getchar()) != EOF && ch != '\n')\
        {\
        ; \
        }\
    }
#define NOTUPDATAUSERLIST '\x00'
#define UPDATAUSERLIST '\x01'
#define NOTATTACK '\x00'
#define ATTACK '\x01'
#define REFUSE '\x00'
#define ACCEPT '\x01'


#define DEFAULT '\x00'

#define NAMESIZE 5
#define SIGNSIZE 10
#define USERSIZE 1024
#define BUFSIZE 4096
 
char sign[SIGNSIZE] = {0};
char message[BUFSIZE] = {0};

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

void flush()
{
    system("clear");
    printf("Welcome to Rock-Paper-Scissors Game! \n**************************************\n");
}

void printuserlist(char* message){
    //pm(message);
    char str[4]={0};
    {
        str[0] = message[6];
        str[1] = message[7];
        str[2] = message[8];
        str[3] = message[9];
    }
    int num,score;
    sscanf(str, "%d", &num);
    printf("Current user number: %d\n", num);
    printf("Current userlist:\n");
    for(int i = 0; i < num; i++){
    printf("%s\n", message+SIGNSIZE+NAMESIZE*i);
}

int main(int argc, char **argv) {
    
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == sock) {
        printf("socket error");
        return -1;
    }
 
    struct sockaddr_in sockAdr;
    memset(&sockAdr, 0, sizeof(sockAdr));
    sockAdr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &sockAdr.sin_addr);
    sockAdr.sin_port = htons(1111);
 
    if(connect(sock, (struct sockaddr *)&sockAdr, sizeof(sockAdr))) {
        printf("connect error");
         return -1;
    }
    
    char name[5];
    memset(name, 0, sizeof(name));
    flush();
    printf("Please enter your name(less than 4 characters):");
    scanf("%s", name);
    
    while(strlen(name) > 4){
        printf("Your name is too long, please enter again(less than 4 characters):\n");
        scanf("%s", name);
    }
    
    strcpy(message+10, name);

    if( 0 >= write(sock, message, BUFSIZE)) {
        printf("write error");
        return -1;
    }

    while (true) {
        memset(message, 0, sizeof(message));
        read(sock, message, BUFSIZE);
        pm(message);
        if(message[0] == '\x01') {
            flush();
            printf("Successfully enter the game!\n");
            printuserlist(message);
            break;
        }
        else {
            printf("Your name is already used, please enter again(less than 4 characters):\n");
            scanf("%s", name);
            while(strlen(name) > 4){
                printf("Your name is too long, please enter again(less than 4 characters):\n");
                scanf("%s", name);
            }
            strcpy(message+10, name);
            if( 0 >= write(sock, message, BUFSIZE)) {
                printf("write error");
            }
        }
        memset(message, 0, sizeof(message));
    }

    while (true) {
        memset(message, 0, BUFSIZE);
        memset(sign, 0, SIGNSIZE);


        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;

        int retval = select(sock + 1, &readfds, NULL, NULL, &timeout);
        if(retval == -1) {
            printf("select error");
            return -1;
        }

        if(FD_ISSET(sock, &readfds)) {
            read(sock, message, BUFSIZE);
            //pm(message);
            if(message[0] == '\x01')
            {
                if(message[3] == '\x00'){
                    char leavename[4];
                    for(int i = 0;i<4;i++){
                            leavename[i] = message[i+SIGNSIZE];
                    }
                    printf("%s leaves the game!\n", leavename);
                    continue;
                }
                else
                {
                    char entername[4];
                    for(int i = 0;i<4;i++){
                            entername[i] = message[i+SIGNSIZE];
                    }
                    printf("%s enters the game!\n", entername);
                    continue;
                }
            }
            if (message[5] == '\x01')
            {   
                char* attackername = message+SIGNSIZE;
                
                printf("You are invited to a game by %s!\n", attackername);
                printf("Do you want to accept the game?\n");
                printf("1. Yes\n2. No\n");
                reply:
                    char accept;
                    while((accept=getchar()) == '\n');
                    pi(accept); 
                    if(accept == '1')
                    {
                        sprintf(sign,"%c%c%c", DEFAULT, DEFAULT, ACCEPT);
                        writesign(message,sign);
                        write(sock, message, BUFSIZE);
                    }
                    else if(accept != '2'){
                        sprintf(sign,"%c%c%c", DEFAULT, DEFAULT, REFUSE);
                        writesign(message,sign);
                        write(sock, message, BUFSIZE);
                        printf("You have refused the game!\n");
                        memset(message, 0, BUFSIZE);
                        memset(sign, 0, SIGNSIZE);
                        continue;
                    }
                    else{
                        printf("Wrong input!\n");
                        memset(message, 0, BUFSIZE);
                        memset(sign, 0, SIGNSIZE);
                        goto reply;
                    }
                flush();
                printf("Please choose your attack:\n");
                printf("1. Rock\n2. Paper\n3. Scissors\n");
                char attack[1];
                getchar();
                scanf("%c", &attack[0]);
                strcpy(message+SIGNSIZE, attack);
                sprintf(sign,"%c%c%c", DEFAULT, DEFAULT, DEFAULT);
                writesign(message,sign);
                write(sock, message, BUFSIZE);
                read(sock, message, BUFSIZE);
                if(message[3] == '\x01'){
                    printf("The game is a draw!\n");
                }
                else if(message[4] == '\x00'){
                    printf("You have lost the game!\n");
                }
                else{
                    printf("You have won the game!\n");
                }
                printf("Press any key to continue...\n");
                memset(message, 0, BUFSIZE);
                memset(sign, 0, SIGNSIZE);
                continue;
            }
        }
        
        printf("Input\n(# to quit; a to attack; f to update the userlist):\n");

        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        retval = select(1, &readfds, NULL, NULL, &timeout);
        if(retval == -1) {
            printf("select error");
            return -1;
        }
        else if(retval == 0) {
            flush();
            continue;
        }
        char t = 0;
        while((t=getchar()) == '\n');
        printf("%d",t);
        if(t == '#') {
            shutdown(sock, SHUT_WR);
            break;
        }
        else if(t == 'f') {
            sprintf(sign,"%c%c%c",UPDATAUSERLIST, DEFAULT, DEFAULT);
            writesign(message,sign);
            write(sock, message, BUFSIZE);
            read(sock, message, BUFSIZE);
            flush();
            printuserlist(message);
        }
        else if(t == 'a'){
            sprintf(sign,"%c%c%c",UPDATAUSERLIST, DEFAULT, DEFAULT);
            writesign(message,sign);
            write(sock, message, BUFSIZE);
            read(sock, message, BUFSIZE);
            flush();
            printuserlist(message);

            printf("Choose a user to attack:\n");
            char attackname[NAMESIZE];
            scanf("%s", attackname);
            strcpy(message+SIGNSIZE, attackname);
            sprintf(sign,"%c%c%c", DEFAULT, ATTACK, DEFAULT);
            writesign(message,sign);
            strcpy(message+SIGNSIZE+NAMESIZE, name);
            write(sock, message, BUFSIZE);
            
            read(sock, message, BUFSIZE);
            while(message[1] == '\x00'){
                printf("The user is not available, please choose another user to attack:\n");
                scanf("%s", attackname);
                strcpy(message+SIGNSIZE, attackname);
                sprintf(sign,"%c%c%c", DEFAULT, ATTACK, DEFAULT);
                writesign(message,sign);
                strcpy(message+SIGNSIZE+NAMESIZE, name);;
                write(sock, message, BUFSIZE);
                read(sock, message, BUFSIZE);
            }
            printf("You have successfully sent the invitation to %s,\n", attackname);
            printf("Waiting for the response...\n");
            read(sock, message, BUFSIZE);
            if(message[2] == '\x01'){
                printf("%s has accepted the game!\n", attackname);
                printf("Please choose your attack:\n");
                printf("1. Rock\n2. Paper\n3. Scissors\n");
                char attack[1];
                getchar();
                scanf("%c", &attack[0]);
                strcpy(message+SIGNSIZE, attack);
                sprintf(sign,"%c%c%c", DEFAULT, DEFAULT, DEFAULT);
                writesign(message,sign);
                write(sock, message, BUFSIZE);
                read(sock, message, BUFSIZE);
                if(message[3] == '\x01'){
                    printf("The game is a draw!\n");
                }
                else if(message[4] == '\x00'){
                    printf("You have lost the game!\n");
                }
                else{
                    printf("You have won the game!\n");
                }
                printf("Press any key to continue...\n");
            }
            else{
                printf("%s has refused the game!", attackname);
                printf("Try another one the attack!\n");
                printf("Press any key to continue...\n");
                getchar();
            }   
        }
        else{
            flush();
            printf("Wrong input!\n");
        }

    }
    close(sock);
 
    return 0;
}