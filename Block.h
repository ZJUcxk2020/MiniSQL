//
// Created by Administrator on 2021/6/10.
//

#ifndef MINISQL_BLOCK_H
#define MINISQL_BLOCK_H
#define BLOCK_SIZE 8192
#include <iostream>
using namespace std;
struct block{
    string filename;
    int fileoffset;
    int spaceused;
    bool isdirty;
    bool isEnd;
    char data[8192];
};
#endif //MINISQL_BLOCK_H
