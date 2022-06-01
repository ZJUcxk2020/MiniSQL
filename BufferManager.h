#pragma once
#include "Address.h"
#include "Block.h"
#include "Table.h"
#include "TableRow.h"
#include "Buffer.h"
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;
class BufferManager {
private:
    static Buffer Buff;
	public:
        static void init();//初始化Buffer

        static vector<block> read(const Table& t); //读表格所有数据

        static TableRow read(const Table& t,const Address& addr);//读表的一行数据

        static bool write(const Address& addr, TableRow data);//往表内写入一行数据

        static Address findFreeSpace(const string& table);//找到表的能够写的地方

        static bool free(const vector<Address>& addrs);//删除一行数据

        static block readFromFile(const string& file,int fileoffset);//从文件中读取表数据

        static bool writeToFile(const block &b, const string& file);//把块数据写回文件

        static bool addBlock(const block &b);//往Buffer中新增一个块

        static bool LRUreplace(const block &b);//LRU替换接口

        static bool writrToFileAll();//把Buffer中的所有脏块写回文件

        static bool dropTable(string tableName);//删除Buffer和文件中的这个表的所有数据
}; 