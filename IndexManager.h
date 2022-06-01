#pragma once
#include <iostream>
#include <map>
#include "Address.h"
#include "Condition.h"
#include "TableRow.h"
#include "Table.h"
#include "BPlusTree.h"
#include <vector>
using namespace std;
class IndexManager {
    static map<string,BPlusTree*> trees;
    
    public:
        static bool createIndex(string table, string attribute,string idxName,int valueType);

        static bool dropIndex(string index);
        
        static vector<Address> select(string index, vector<Cond> conditions);
        
        static bool insert(string index, TableRow data, Table T, Address addr);
        
        static vector<Address> deleteT(string table, vector<Address> dels);
        
        static bool writetoFile();
        
};
