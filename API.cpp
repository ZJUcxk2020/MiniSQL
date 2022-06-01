#include "API.h"
#include "CatalogManager.h"
#include "Table.h"
#include "Index.h"
#include"IndexManager.h"
#include"RecordManager.h"
#include"TableRow.h"
#include <iostream>
#include <algorithm>
using namespace std;

RESPONSE API::createTable(string tableName,vector<Attr> attrs)
{
    RESPONSE ret = SUCCESS;
    if(CatalogManager::tableExists(tableName)==true) {
        ret = EXIST;
    }else {
        Table *t = new Table;
        t->tablename = tableName;
        t->num_attr = attrs.size();
        t->attrs = attrs;
        if(CatalogManager::addTable(*t) == false) ret = FAIL;
        if(RecordManager::createTable(*t) == false) ret = FAIL;
        Index idx;
        string pkidx = "";
        for(auto iter : attrs) {
            if(iter.isunique) {
                idx.table = tableName;
                idx.attribute = iter.name;
                idx.name = tableName + "_" + iter.name;
                pkidx = idx.name;
                iter.index = idx.name;
                if(CatalogManager::addIndex(idx) == false) {
                    ret = FAIL;
                    break;
                }
                if(IndexManager::createIndex(tableName,iter.name,tableName + "_" + iter.name,iter.type) == false) {
                    CatalogManager::deleteIndex(idx.name);
                    ret = FAIL;
                    break;
                }
            }else if (iter.index != "") {
                idx.table = tableName;
                idx.attribute = iter.name;
                idx.name = iter.index;
                if(CatalogManager::addIndex(idx) == false) {
                    if(!(pkidx.empty())) CatalogManager::deleteIndex(pkidx);
                    ret = FAIL;
                    break;
                }
                if(IndexManager::createIndex(tableName,iter.name,iter.index,iter.type) == false) {
                    if(!(pkidx.empty())) CatalogManager::deleteIndex(pkidx);
                    CatalogManager::deleteIndex(idx.name);
                    ret = FAIL;
                    break;
                }
            }
        }
    }
    return ret;
}

RESPONSE API::dropTable(string tableName)
{
    RESPONSE ret = SUCCESS;
    if(CatalogManager::tableExists(tableName)==false) {
        ret = NOT_EXIST;
    }else {
        Table t = CatalogManager::getTable(tableName);
        CatalogManager::deleteTable(t.tablename);
        for(auto iter : t.attrs) {
            if(iter.isprimarykey) {
                CatalogManager::deleteIndex(t.tablename + "_" + iter.name);
                IndexManager::dropIndex(t.tablename + "_" + iter.name);
            }else if(!iter.index.empty()) {
                CatalogManager::deleteIndex(iter.index);
                IndexManager::dropIndex(iter.index);
            }
        }
        RecordManager::dropTable(t);
    }
    return ret;
}
        
RESPONSE API::createIndex(string tableName,string indexName,string attribute)
{
    RESPONSE ret = SUCCESS;
    if(!CatalogManager::tableExists(tableName)) {
        ret = NOT_EXIST;
        return ret;
    }
    Table t = CatalogManager::getTable(tableName);
    bool flag = false;
    for(auto iter : t.attrs) {
        if(iter.name == attribute) {
            flag = true;
            break;
        }
    }
    if(!flag) {
        ret = NOT_EXIST;
    }else if(CatalogManager::indexExists(indexName) || CatalogManager::hasIndex(tableName,attribute)) {
        ret = EXIST;
    }else {
        Index idx;
        idx.name = indexName;
        idx.attribute = attribute;
        idx.table = tableName;
        CatalogManager::addIndex(idx);
        IndexManager::createIndex(tableName,attribute,indexName,CatalogManager::fieldType(tableName,attribute));
    }
    return ret;
}
        
RESPONSE API::dropIndex(string indexName)
{
    RESPONSE ret = SUCCESS;
    if(!CatalogManager::indexExists(indexName)) {
        ret = NOT_EXIST;
    }else {
        IndexManager::dropIndex(indexName);
        CatalogManager::deleteIndex(indexName);
    }
    return ret;
}
        
RESPONSE API::select(string tableName, vector<string> select_attrs,vector<Cond> conds)
{
    RESPONSE ret = SUCCESS;
    if(!CatalogManager::tableExists(tableName)) {
        ret = NOT_EXIST;
        return ret;
    }
    Table t = CatalogManager::getTable(tableName);
    if(select_attrs.empty()) {
        for(auto iter:t.attrs) {
            select_attrs.push_back(iter.name);
        }
    }
    bool flag_a = true;
    for(auto iter_a : select_attrs) {
        bool flag_b = false;
        for(auto iter_b : t.attrs) {
            if(iter_a == iter_b.name) {
                flag_b = true;
                break;
            }
        }
        if(!flag_b) {
            flag_a = false;
            break;
        }
    }
    if(!flag_a) {
        ret = NOT_EXIST;
    }else {
        vector<string> indices = CatalogManager::indicesOnTable(tableName);
        // for(auto iter:indices) {
        //     cout << "Index Name is " <<iter << endl;
        // }
        vector<TableRow> rows;
        bool flag = false;
        if(!indices.empty()) {
            for(auto iter:conds) {
                while(!indices.empty()) {
                    auto idx = indices.begin();
                    if(iter.attr_name == CatalogManager::fieldIndexOn(*idx)) {
                        // cout << "Attr Name is " <<iter.attr_name << endl;
                        flag = true;
                        break;
                    }else {
                        indices.erase(idx);
                    }
                }
            }
        }
        // cout << "yes!" << endl;
        if(!conds.empty() && !indices.empty() && flag) {
                bool first = true;
                auto iter = indices.begin();
                vector<Address> res = IndexManager::select(*iter,conds);
                string attribute = CatalogManager::fieldIndexOn(*iter);
                while(1) {
                    auto it = conds.begin();
                    for(; it != conds.end() && it->attr_name != attribute; ++it);
                    if(it == conds.end()) break;
                    conds.erase(it);
                }
                rows = RecordManager::select(res,select_attrs,conds);
            }else rows = RecordManager::select(t,select_attrs,conds);
        // cout << "rowSize = "<<rows.size() << endl;
        for(auto iter : select_attrs) {
            cout << "| " << iter << " ";
        }
        cout << '|' << endl;
        if(rows.size()) {
            for(auto &iter: rows) {
                char* p = iter.data;
                int offset = 0;
                bool first = true;
                bool flag = true;
                cout << "(";
                for(auto attr : select_attrs) {
                    if(offset >= iter.datasize) {
                        cout << "Out of Bounds." << endl;
                        flag = false;
                        ret = FAIL;
                        break;
                    }
                    if(first) {
                        first = false;
                    }else {
                        cout << ",";
                    }
                    int type = CatalogManager::fieldType(tableName,attr);
                    if(type==-1) {
                        int temp = *(int*)(p+offset);
                        offset += sizeof(int);
                        cout << temp;
                    }else if(type == 0) {
                        float temp = *(float*)(p+offset);
                        offset += sizeof(float);
                        cout << temp;
                    }else if(type>0){
                        int size = type;
                        char temp[size+1];
                        temp[size] = '\0';
                        for(int i = 0; i < size;i++) {
                            temp[i] = (p+offset)[i];
                        }
                        offset += size;
                        cout << temp;
                    }else {
                        cout << "Illegal type." << endl;
                        ret = ILLEGAL;
                        flag = false;
                        break;
                    }
                }
                if(!flag) break;
                cout << ")" << endl;
                delete [] p;
            }
            cout << rows.size()  << " rows found."<<endl;
        }
    }
    return ret;
}
        
RESPONSE API::insert(string tableName,TableRow row)
{
    // char x[11];
    // x[10]  = '\0';
    // memcpy(x,row.data,10);
    // int y;
    // memcpy(&y,row.data+10,4);
    // cout << ',' << x << ','<<y<<endl;
    Table t = CatalogManager::getTable(tableName);
    RESPONSE ret = SUCCESS;
    if(!CatalogManager::tableExists(tableName)) {
        ret = NOT_EXIST;
        return ret;
    }
    int size = 0;
    int flag = true;
    for(auto iter: t.attrs) {
        if(iter.type==-1) size += sizeof(int);
        else if(iter.type==0) size += sizeof(float);
        else if(iter.type>0) size += iter.type;
        else {
            cout << "Illegal type." << endl;
            flag = false;
            ret = ILLEGAL;
            break;
        }
    }
    if(size!=row.datasize) flag = false;
    if(!flag) {
        ret = ILLEGAL;
    }else {
        Address addr;
        try {
            addr = RecordManager::insert(t,row);
        }catch(RESPONSE msg) {
            // cout << "Value exists." << endl;
            ret = EXIST;
            return ret;
        }catch (...) {
            cout << "catch." << endl;
            return ILLEGAL;
        }
        vector<string> indices = CatalogManager::indicesOnTable(tableName);
        for(auto iter:indices) {
            IndexManager::insert(iter,row,t,addr);
        }
    }
    return ret;
}
        
RESPONSE API::deleteT(string tableName, vector<Cond> conds)
{
    RESPONSE ret = SUCCESS;
    if(!CatalogManager::tableExists(tableName)) {
        ret = NOT_EXIST;
        return ret;
    }
    Table t = CatalogManager::getTable(tableName);
    vector<string> attrs = CatalogManager::fieldsOnTable(tableName);
    bool flag = true;
    for(auto iter : conds) {
        if(find(attrs.begin(),attrs.end(),iter.attr_name) == attrs.end()) {
            flag = false;
            break;
        }
    }
    if(!flag) {
        ret = NOT_EXIST;
    }else {
        bool has_id = false;
        vector<string> indices = CatalogManager::indicesOnTable(tableName);
        vector<Address> delRes;
        bool flag = false;
        if(!indices.empty()) {
            for(auto iter:conds) {
                while(!indices.empty()) {
                    auto idx = indices.begin();
                    if(iter.attr_name == CatalogManager::fieldIndexOn(*idx)) {
                        // cout << "Attr Name is " <<iter.attr_name << endl;
                        flag = true;
                        break;
                    }else {
                        indices.erase(idx);
                    }
                }
            }
        }
        if(!conds.empty() &&!indices.empty() && flag) {
            bool first = true;
            auto iter = indices.begin();
            vector<Address> res = IndexManager::select(*iter,conds);
            string attribute = CatalogManager::fieldIndexOn(*iter);
            while(1) {
                    auto it = conds.begin();
                    for(; it != conds.end() && it->attr_name != attribute; ++it);
                    if(it == conds.end()) break;
                    conds.erase(it);
            }
            delRes = RecordManager::deleteT(res,conds);
            IndexManager::deleteT(tableName,delRes);
        }else {
            // for(auto it :conds) {
            //     cout << it.attr_name << " "<<it.op << " " << it.value << endl;
            // }
            // system("pause");
            delRes = RecordManager::deleteT(t,conds);
            IndexManager::deleteT(tableName,delRes);
        }
        cout << delRes.size() << " rows deleted."<<endl;
    }
    return ret;
}