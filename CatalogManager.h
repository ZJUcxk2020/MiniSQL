#ifndef MINISQL_CatalogManager_H
#define MINISQL_CatalogManager_H
#include<iostream>
#include<vector>
#include"Attribute.h"
#include "Table.h"
#include "Index.h"
using namespace std;
typedef struct indexfile IdxFile;
struct indexfile{
    vector<Index> Idxs;
    int num;
};
typedef struct tablefile TabFile;
struct tablefile{
    vector<Table> tables;
    int num;
};

class CatalogManager {
private:
    static IdxFile IF;
    static TabFile TF;
public:
    static bool readFromFile();//从文件中读数据
    static bool tableExists(const string& table);//判断表是否存在
    static bool indexExists(const string& index);//判断索引是否存在
    static int fieldCount(const string& table);//返回表中属性数
    static int indexCount(const string& table);//返回表中索引数
    static Table getTable(const string& tableName);//返回表信息
    static vector<string> fieldsOnTable(const string& table);//返回表所有属性
    static vector<string> indicesOnTable(const string& table);//返回表所有索引
    static int rowLength(const string& table);//返回表中数据行长度
    static string pkOnTable(const string& table);//返回表中主键
    static int fieldType(const string& table, const string& field);//返回属性种类
    static bool isUnique(const string& table, const string& field);//判断是否唯一
    static bool isPK(const string& table, const string& field);//判断是否是主键
    static bool hasIndex(const string& table, const string& field);//判断是否存在索引
    static string indexName(const string& table, const string& field);//返回索引名
    static string tableIndexOn(const string& index);//返回索引所在表名
    static string fieldIndexOn(const string& index);//返回索引所在属性名
    static bool addTable(const Table& T);//增加表信息
    static bool deleteTable(const string& table);//删除表信息
    static bool addIndex(const Index& idx);//增加索引信息
    static bool deleteIndex(const string& index);//删除索引信息
    static bool storeToFile();//把索引信息存回文件
};
#endif