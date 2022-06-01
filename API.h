#pragma once
#include <vector>
#include <iostream>
#include "Response.h"
#include "Attribute.h"
#include "Condition.h"
#include "TableRow.h"
using namespace std;
class API {
    public:
        static RESPONSE createTable(string tableName,vector<Attr> attrs);
        
        static RESPONSE dropTable(string tableName);
        
        static RESPONSE createIndex(string tableName,string indexName,string attribute);
        
        static RESPONSE dropIndex(string indexName);
        
        static RESPONSE select(string tableName, vector<string> select_attrs,vector<Cond> conds);
        
        static RESPONSE insert(string tableName,TableRow row);
        
        static RESPONSE deleteT(string tableName, vector<Cond> conds);
};