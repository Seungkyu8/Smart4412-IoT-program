#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <asm/ioctls.h>

#define dot_d "/dev/dot"
#define tact_d "/dev/tactsw"
#define clcd "dev/clcd"

unsigned char In;
int dot, tact;

unsigned char map[8][8];
unsigned char a[8];
int count = 0;

//x y 토글
char flag = 1;

int player =1;
int x, y;

int chance_1p = 1;
int chance_2p = 1;

void config_input();
void input_vector(int scal);
void set_rock();
void light(unsigned char matrix[8][8], int id);
int check();
int win();

int main(){

    tact = open(tact_d,O_RDWR);
    if(tact < 0){
        printf("open failed! \n");
        return 0;
    }
    while(1){
        while(1){
            read(tact,&In,sizeof(In));
            usleep(100000);
            if(In) {
                break;
            }
        }
        if(In > 0 && In < 9)
            input_vector(In - 1);
        if(In == 9)
            config_input();
        if(In == 12){
            if(map[x][y]){
                printf("Overlap\n");
                dot = open(dot_d,O_RDWR);
                unsigned char fail[8] = { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 };
                write(dot, &fail, sizeof(fail));
                usleep(100000);
                close(dot);    
            }
            else{
                set_rock();
                light(map,3);
                if(check() == 0){
                    win();
                    return 0;
                }
            }
        } 
        if(In == 11 && chance_2p){
	  light(map,2);
	  chance_2p--;
	}
        if(In == 10 && chance_1p){
	  light(map,1);
	  chance_1p--;
	}
    }
    return 0;
}

void light(unsigned char matrix[8][8], int id) {
    dot = open(dot_d,O_RDWR);
    unsigned char value = 1;
    unsigned char temp = 0;

    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 8 - 1; j >= 0; j--) {
            //카운트보다 작은것만 빛나도록
            if (matrix[i][j] == id) temp += value;
            if(id == 3){
                if(matrix[i][j] > 0 && matrix[i][j] < 3)
                    temp += value;
            }
            value = value * 2;
        }
        a[i] = temp;
        temp = 0;
        value = 1;
    }
    write(dot, &a, sizeof(a));
    usleep(300000);
    close(dot);
}

void input_vector(int scal){
    if(flag == 1){
        x = scal;
    }if(flag == 2){
        y = scal;
    }
    count++;
}
void config_input(){
    flag = 2;
}

void set_rock(){
    if(count > 1){
        map[x][y] = player;
    if(player == 1)
        player = 2;
    else
        player = 1;
    
    flag = 1;

    count = 0;

    chance_1p = 1;
    chance_2p = 1;
    }
}

int check() {
    int dy[8] = { 0,1,0,-1,1,-1,1,-1 };
    int dx[8] = { 1,0,-1,0,1,-1,-1,1 };
    int i, j;
    int yy, xx;
    int cnt[8] ={0,0,0,0,0,0,0,0};
    
    for (i = 0; i < 8; i++) {
        for (j = 1; j < 5; j++) {
            xx = x + dx[i] * j;
            yy = y + dy[i] * j;

            if (yy < 0 || xx < 0 || xx >= 8 || yy >= 8) continue;
	    if (map[xx][yy] != map[x][y]) break;
            if (map[xx][yy] == map[x][y]) cnt[i]++;
        }
    }
    for (i = 0; i < 8; i++) {
	printf("%d",cnt[i]);
    }
    printf("\n");
    if ((cnt[1] + cnt[3]) == 4)return 0;
    if ((cnt[0] + cnt[2]) == 4)return 0;
    if ((cnt[4] + cnt[5]) == 4)return 0;
    if ((cnt[6] + cnt[7]) == 4)return 0;

    

    return 1;
}

int win(){
    int clcd_d;
    clcd_d = open(clcd, O_RDWR);
    if(clcd_d<0){
        printf("Open fail");
        return 0;
    }
    if(player == 2)
        write(clcd_d, "1p victory", strlen("1p victory"));
    if(player == 1)
        write(clcd_d, "2p victory", strlen("2p victory"));
    usleep(1000000);
    close(clcd_d);
    return 0;
}
