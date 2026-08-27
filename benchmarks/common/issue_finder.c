#include "issue_finder.h"


int check_for_RAW_2(int r1[3], int r2[3]){
    return r1[0] == r2[1] || r1[0] == r2[2];
}

int check_for_WAW_2(int r1[3], int r2[3]){
    return r1[0] == r2[0];
}