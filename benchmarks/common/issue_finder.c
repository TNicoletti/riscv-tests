#include "issue_finder.h"


int check_for_RAW_2(int r1[3], int r2[3]){
    return r1[0] == r2[1] || r1[0] == r2[2];
}

int check_for_RAW_3(int r1[3], int r2[3], int r3[3]){
    return check_for_RAW_2(r1, r2) || check_for_RAW_2(r1, r3) || check_for_RAW_2(r2, r3);
}

int check_for_RAW_4(int r1[3], int r2[3], int r3[3], int r4[3]){
    return check_for_RAW_2(r1, r2) || check_for_RAW_2(r1, r3) || check_for_RAW_2(r1, r4) \
        || check_for_RAW_2(r2, r3) || check_for_RAW_2(r2, r4) || check_for_RAW_2(r3, r4);
}

int check_for_WAW_2(int r1[3], int r2[3]){
    return r1[0] == r2[0];
}

int check_for_WAW_3(int r1[3], int r2[3], int r3[3], int r4[3]){
    return check_for_WAW_2(r1, r2) || check_for_WAW_2(r1, r3) || check_for_WAW_2(r2, r3);
}

int check_for_WAW_4(int r1[3], int r2[3], int r3[3], int r4[3]){
    return check_for_WAW_2(r1, r2) || check_for_WAW_2(r1, r3) || check_for_WAW_2(r1, r4) \
        || check_for_WAW_2(r2, r3) || check_for_WAW_2(r2, r4) || check_for_WAW_2(r3, r4);
}