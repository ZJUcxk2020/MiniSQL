//
// Created by Administrator on 2021/6/2.
//
#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H
#include <string>
#include <vector>
using namespace std;
class Interpreter{
public:
    string command;
    int i; //index
    string filename;
    void init(const string &);
    Interpreter();
    Interpreter(string &com);
    int interpret();   // 返回1代表正确执行，返回0代表读文件，返回-1代表语句错误,返回2代表quit
    string parser(string &subcom);
};
#endif //MINISQL_INTERPRETER_H
