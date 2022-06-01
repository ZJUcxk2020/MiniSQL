#pragma once
#include "Table.h"
#include "TableRow.h"
#include "Address.h"
#include "Condition.h"
#include <iostream>
#include <vector>
using namespace std;
class RecordManager {
    public:
        static bool createTable(Table table);
        
        static bool dropTable(Table table);
        
        static vector<TableRow> select(Table table,vector<string> select_attrs,vector<Cond> conditions);
        
        static Address insert(Table table, TableRow data);
        
        static vector<Address> deleteT(Table table, vector<Cond> conditions);
        
        static vector<TableRow> select(vector<Address> addr,vector<string> select_attrs,vector<Cond> conditions);
        
        static vector<Address> deleteT(vector<Address> addr,vector<Cond> conds);
};
