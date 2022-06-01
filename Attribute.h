//
// Created by Administrator on 2021/6/10.
//

#ifndef MINISQL_ATTRIBUTE_H
#define MINISQL_ATTRIBUTE_H
typedef struct Attribute Attr;
#include <iostream>
using namespace std;
struct Attribute{
    string name;
    bool isunique;  // 1 is unique
    string index;  //"" is no index
    int type;   // -1 is int,0 is float , and other numbers are the length of char
    bool isprimarykey;
};
#endif //MINISQL_ATTRIBUTE_H
