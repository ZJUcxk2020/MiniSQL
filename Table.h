//
// Created by Administrator on 2021/6/10.
//

#ifndef MINISQL_TABLE_H
#define MINISQL_TABLE_H
#include <vector>
#include <iostream>
#include "Attribute.h"
using namespace std;
typedef struct table Table;
struct table{
    string tablename;
    int num_attr;
    vector<Attr> attrs;
};
#endif //MINISQL_TABLE_H
