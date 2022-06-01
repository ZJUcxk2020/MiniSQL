//
// Created by Administrator on 2021/6/10.
//
#include <iostream>
using namespace std;
#ifndef MINISQL_CONDITION_H
#define MINISQL_CONDITION_H
#define OP_EQ 0
#define OP_LT 1
#define OP_GT 2
#define OP_LEQ 3
#define OP_GEQ 4
#define OP_NEQ 5

typedef struct condition Cond;
struct condition{
    string attr_name;
    int op; // 0 is =,1 is <,2 is >,3 is <=,4 is >=,5 is <>
    string value;
};
#endif //MINISQL_CONDITION_H
