//
// Created by Administrator on 2021/6/10.
//

#ifndef MINISQL_TABLEROW_H
#define MINISQL_TABLEROW_H
typedef struct TableRow TABLEROW;
struct TableRow{
    int datasize;
    char * data;
};
#endif //MINISQL_TABLEROW_H
