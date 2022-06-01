#include <iostream>
#include "RecordManager.h"
#include "Table.h"
#include <vector>
#include "Condition.h"
#include "TableRow.h"
#include "Address.h"
#include "BufferManager.h"
#include <fstream>
#include "CatalogManager.h"
#include "IndexManager.h"
#include "Block.h"
#include "Attribute.h"
#include <sstream>
#include "Response.h"
using namespace std;

bool RecordManager::createTable(Table table)
{
    bool ret = true;
    ofstream file("./"+table.tablename+"_0",fstream::out);
    if(!file.is_open()) ret = false;
    else {
        char c = 1;
        file.write(&c,1);
        file.close();
    }
    return ret;
}

bool RecordManager::dropTable(Table table)
{
    int offset = 0;
    while(1) {
        string path = "./"+table.tablename+"_"+to_string(offset);
        if(remove(path.c_str()) == 0) {
            offset+=1;
        }else {
            break;
        }
    }
    BufferManager::dropTable(table.tablename);
    return true;
}

vector<TableRow> RecordManager::select(Table table,vector<string> select_attrs,vector<Cond> conditions)
{
    // cout << "Now in RM select." << endl;
    int size = 0,length = 0;
    vector< pair<int,int> > needed;
    if(select_attrs.empty()) {
        for(auto iter:table.attrs) {
            select_attrs.push_back(iter.name);
        }
    }
    for(auto iter:select_attrs) {
        int type =CatalogManager::fieldType(table.tablename,iter);
        if(type == -1) size += sizeof(int);
        else if(type == 0) size += sizeof(float);
        else size += type;
        int offset = 0;
        auto attribute = table.attrs.begin();
        for(;attribute != table.attrs.end() && attribute->name != iter;attribute++)
        {
            int type =CatalogManager::fieldType(table.tablename,attribute->name);
            if(type == -1) offset += sizeof(int);
            else if(type == 0) offset += sizeof(float);
            else offset += type;
        }
        type =CatalogManager::fieldType(table.tablename,attribute->name);
        int l = 0;
        if(type == -1) l = sizeof(int);
        else if(type == 0) l = sizeof(float);
        else l = type;
        needed.push_back(make_pair(offset,l));
    }
    for(auto iter:table.attrs) {
        int type =CatalogManager::fieldType(table.tablename,iter.name);
        if(type == -1) length += sizeof(int);
        else if(type == 0) length += sizeof(float);
        else length += type;
    }
    vector< pair<int,int> > offsets;
    for(auto iter : conditions) {
        int offset = 0;
        auto attribute = table.attrs.begin();
        for(;attribute != table.attrs.end() && attribute->name != iter.attr_name;attribute++)
        {
            int type =CatalogManager::fieldType(table.tablename,attribute->name);
            if(type == -1) offset += sizeof(int);
            else if(type == 0) offset += sizeof(float);
            else offset += type;
        }
        int type =CatalogManager::fieldType(table.tablename,attribute->name);
        offsets.push_back(make_pair(offset,type));
    }
    vector<block> bs = BufferManager::read(table);
    vector<TableRow> rows;
    // cout << "spaceused is " << data.spaceused << endl;
    for(auto data:bs) {
        if(conditions.empty()) {
            for(int i = 0;i*length<data.spaceused;i++) {
                char *p = data.data+i*length;
                if(*p == '?') continue;
                TableRow row;
                row.datasize = size;
                row.data = new char[size];
                int addr = 0;
                for(auto iter :needed) {
                    for(int x = 0;x<iter.second;x++) {
                        row.data[addr++] = p[iter.first+x];
                    }
                }
                rows.push_back(row);
            }
        }else {
            // cout << "Now here in RM select." << endl;
            for(int i = 0;i*length<data.spaceused;i++) {
                char *p = data.data+i*length;
                int flag = true;
                if(*p == '?') continue;
                for(int j = 0;j<conditions.size();j++) {
                    stringstream ss;
                    if(offsets[j].second == -1) {
                        int temp = *(int*)(p+offsets[j].first);
                        ss << conditions[j].value;
                        int value;
                        ss >> value;
                        switch(conditions[j].op)
                        {
                            case OP_EQ:
                                if(temp != value) flag = false;
                                break;
                            case OP_NEQ:
                                if(temp == value) flag = false;
                                break;
                            case OP_GEQ:
                                if(temp < value) flag = false;
                                break;
                            case OP_GT:
                                if(temp <= value) flag = false;
                                break;
                            case OP_LT:
                                if(temp >= value) flag = false;
                                break;
                            case OP_LEQ:
                                if(temp > value) flag = false;
                                break;
                        }
                    }else if(offsets[j].second == 0) {
                        float temp = *(float*)(p+offsets[j].first);
                        ss << conditions[j].value;
                        float value;
                        ss >> value;
                        switch(conditions[j].op)
                        {
                            case OP_EQ:
                                if(temp != value) flag = false;
                                break;
                            case OP_NEQ:
                                if(temp == value) flag = false;
                                break;
                            case OP_GEQ:
                                if(temp < value) flag = false;
                                break;
                            case OP_GT:
                                if(temp <= value) flag = false;
                                break;
                            case OP_LT:
                                if(temp >= value) flag = false;
                                break;
                            case OP_LEQ:
                                if(temp > value) flag = false;
                                break;
                        }
                    }else {
                        char temp[offsets[j].second+1];
                        temp[offsets[j].second] = '\0';
                        char* ptemp = p+offsets[j].first;
                        for(int k = 0;k<offsets[j].second;k++) {
                            temp[k] = ptemp[k];
                        }
                        string stemp = temp;
                        string value = conditions[j].value;
                        switch(conditions[j].op)
                        {
                            case OP_EQ:
                                if(stemp != value) flag = false;
                                break;
                            case OP_NEQ:
                                if(stemp == value) flag = false;
                                break;
                            case OP_GEQ:
                                if(stemp < value) flag = false;
                                break;
                            case OP_GT:
                                if(stemp <= value) flag = false;
                                break;
                            case OP_LT:
                                if(stemp >= value) flag = false;
                                break;
                            case OP_LEQ:
                                if(stemp > value) flag = false;
                                break;
                        }
                    }
                    if(!flag) break;
                }
                if(flag) {
                    TableRow row;
                    row.datasize = size;
                    row.data = new char[size];
                    int addr = 0;
                    for(auto iter :needed) {
                        for(int x = 0;x<iter.second;x++) {
                            row.data[addr++] = p[iter.first+x];
                        }
                    }
                    rows.push_back(row);
                }
            }
        }
    }
    return rows;
}

Address RecordManager::insert(Table table, TableRow data)
{
    Address ret;
    ret = BufferManager::findFreeSpace(table.tablename);
    // cout <<"RM Fileoffset is " << ret.fileOffset << " RM Blockoffset is " << ret.blockOffset << endl;
    char *p = data.data;
    Attr pk;
    vector<string> key;
    for(auto iter:table.attrs) {
        if(iter.isunique) {
            key.push_back(iter.name);
        }
    }
    // for(auto iter:key) {
    //     cout << "unique is " << iter << endl;
    // }
    if(!key.empty()) {
        for(auto pkey:key) {
            int offset = 0;
            // cout << "now pkey is " << pkey << endl;
            for(auto iter:table.attrs) {
                if(iter.name!=pkey) {
                    if(iter.type == -1) offset += sizeof(int);
                    else if(iter.type == 0) offset += sizeof(float);
                    else offset += iter.type;
                }else {
                    pk = iter;
                    break;
                }
            }
            vector<Cond> conds;
            Cond cond;
            cond.attr_name = pk.name;
            cond.op = OP_EQ;
            if(pk.type == -1) cond.value = to_string(*(int*)(p+offset));
            else if(pk.type == 0) cond.value = to_string(*(float*)(p+offset));
            else {
                char temp[pk.type+1];
                temp[pk.type] = '\0';
                char* ptemp = p+offset;
                for(int k = 0;k<pk.type;k++) {
                    temp[k] = ptemp[k];
                }
                cond.value = temp;
            }
            conds.push_back(cond);
            vector<Address> addr = IndexManager::select(CatalogManager::indexName(table.tablename,pkey),conds);
            if(!addr.empty()) {
                throw EXIST;
            }
        }
    } 
    // char x[11];
    // x[10]  = '\0';
    // memcpy(x,data.data,10);
    // int y;
    // memcpy(&y,data.data+10,4);
    // cout << ',' << x << ','<<y<<endl;
    // cout << "RM end offset is "<<ret.blockOffset << endl;
    BufferManager::write(ret,data);
    return ret;
}

vector<Address> RecordManager::deleteT(Table table, vector<Cond> conditions)
{
    vector<Address> ret;
    int length = CatalogManager::rowLength(table.tablename);
    vector<block> bs = BufferManager::read(table);
    vector< pair<int,int> > offsets;
    if(!conditions.empty()) {
        for(auto iter : conditions) {
            int offset = 0;
            auto attribute = table.attrs.begin();
            for(;attribute != table.attrs.end() && attribute->name != iter.attr_name;attribute++)
            {
                int type =CatalogManager::fieldType(table.tablename,attribute->name);
                if(type == -1) offset += sizeof(int);
                else if(type == 0) offset += sizeof(float);
                else offset += type;
            }
            int type =CatalogManager::fieldType(table.tablename,attribute->name);
            offsets.push_back(make_pair(offset,type));
        }
    }
    while(!bs.empty()){
        vector<Address> addrs;
        auto pdata = bs.begin();
        auto data = *pdata;
        if(conditions.empty()) {
            for(int i = 0;i*length<data.spaceused;i++) {
                char *p = data.data+i*length;
                if(*p == '?') continue;
                Address addr;
                addr.file = table.tablename;
                addr.blockOffset = i*length;
                addr.fileOffset = data.fileoffset;
                addrs.push_back(addr);
            }
        }else {
            for(int i = 0;i*length<data.spaceused;i++) {
                // cout << "Now file "<<data.fileoffset << " in offset " << i*length << " spaceused is " << data.spaceused << endl;
                char *p = data.data+i*length;
                int flag = true;
                if(*p == '?') continue;
                for(int j = 0;j<conditions.size();j++) {
                    stringstream ss;
                    if(offsets[j].second == -1) {
                        int temp = *(int*)(p+offsets[j].first);
                        ss << conditions[j].value;
                        int value;
                        ss >> value;
                        switch(conditions[j].op)
                        {
                            case OP_EQ:
                                if(temp != value) flag = false;
                                break;
                            case OP_NEQ:
                                if(temp == value) flag = false;
                                break;
                            case OP_GEQ:
                                if(temp < value) flag = false;
                                break;
                            case OP_GT:
                                if(temp <= value) flag = false;
                                break;
                            case OP_LT:
                                if(temp >= value) flag = false;
                                break;
                            case OP_LEQ:
                                if(temp > value) flag = false;
                                break;
                        }
                    }else if(offsets[j].second == 0) {
                        float temp = *(float*)(p+offsets[j].first);
                        ss << conditions[j].value;
                        float value;
                        ss >> value;
                        switch(conditions[j].op)
                        {
                            case OP_EQ:
                                if(temp != value) flag = false;
                                break;
                            case OP_NEQ:
                                if(temp == value) flag = false;
                                break;
                            case OP_GEQ:
                                if(temp < value) flag = false;
                                break;
                            case OP_GT:
                                if(temp <= value) flag = false;
                                break;
                            case OP_LT:
                                if(temp >= value) flag = false;
                                break;
                            case OP_LEQ:
                                if(temp > value) flag = false;
                                break;
                        }
                    }else {
                        char temp[offsets[j].second+1];
                        temp[offsets[j].second] = '\0';
                        char* ptemp = p+offsets[j].first;
                        for(int k = 0;k<offsets[j].second;k++) {
                            temp[k] = ptemp[k];
                        }
                        string stemp = temp;
                        string value = conditions[j].value;
                        switch(conditions[j].op)
                        {
                            case OP_EQ:
                                if(stemp != value) flag = false;
                                break;
                            case OP_NEQ:
                                if(stemp == value) flag = false;
                                break;
                            case OP_GEQ:
                                if(stemp < value) flag = false;
                                break;
                            case OP_GT:
                                if(stemp <= value) flag = false;
                                break;
                            case OP_LT:
                                if(stemp >= value) flag = false;
                                break;
                            case OP_LEQ:
                                if(stemp > value) flag = false;
                                break;
                        }
                    }
                    if(!flag) break;
                }
                // cout << "Now select end." << endl;
                // if(flag) cout << "Flag is true." << endl;
                // else cout << "Flag is false." << endl;
                if(flag) {
                    Address addr;
                    addr.file = table.tablename;
                    addr.blockOffset = i*length;
                    addr.fileOffset = data.fileoffset;
                    addrs.push_back(addr);
                    // cout << "AddrFile is " << addr.fileOffset << " AddrBlock is " << addr.blockOffset <<endl;
                }
            }
        }
        // for(auto& addr : addrs) {
        //     cout << "AddrFile is " << addr.fileOffset << " AddrBlock is " << addr.blockOffset <<endl;
        // } 
        ret.insert(ret.end(),addrs.begin(),addrs.end());
        bs.erase(pdata);
    }
    // for(auto addr : ret) {
    //         cout << "AddrFile is " << addr.fileOffset << " AddrBlock is " << addr.blockOffset <<endl;
    // } 
    BufferManager::free(ret);
    return ret;
}

vector<TableRow> RecordManager::select(vector<Address> addr,vector<string> select_attrs,vector<Cond> conditions)
{
    // cout << "addrSize = "<<addr.size() << endl;
    // cout << "conditions = "<<conditions.size() << endl;
    vector<TableRow> ret;
    if(addr.empty()) return ret;
    Table t = CatalogManager::getTable(addr.begin()->file);
    int size = 0,length = 0;
    vector< pair<int,int> > needed;
    table table = CatalogManager::getTable(addr.begin()->file);
    if(select_attrs.empty()) {
        for(auto iter:table.attrs) {
            select_attrs.push_back(iter.name);
        }
    }
    for(auto iter:select_attrs) {
        int type =CatalogManager::fieldType(table.tablename,iter);
        if(type == -1) size += sizeof(int);
        else if(type == 0) size += sizeof(float);
        else size += type;
        int offset = 0;
        auto attribute = table.attrs.begin();
        for(;attribute != table.attrs.end() && attribute->name != iter;attribute++)
        {
            int type =CatalogManager::fieldType(table.tablename,attribute->name);
            if(type == -1) offset += sizeof(int);
            else if(type == 0) offset += sizeof(float);
            else offset += type;
        }
        type =CatalogManager::fieldType(table.tablename,attribute->name);
        int l = 0;
        if(type == -1) l = sizeof(int);
        else if(type == 0) l = sizeof(float);
        else l = type;
        needed.push_back(make_pair(offset,l));
    }
    for(auto iter:table.attrs) {
        int type =CatalogManager::fieldType(table.tablename,iter.name);
        if(type == -1) length += sizeof(int);
        else if(type == 0) length += sizeof(float);
        else length += type;
    }
    vector< pair<int,int> > offsets;
    for(auto iter : conditions) {
        int offset = 0;
        auto attribute = table.attrs.begin();
        for(;attribute != table.attrs.end() && attribute->name != iter.attr_name;attribute++)
        {
            int type =CatalogManager::fieldType(table.tablename,attribute->name);
            if(type == -1) offset += sizeof(int);
            else if(type == 0) offset += sizeof(float);
            else offset += type;
        }
        int type =CatalogManager::fieldType(table.tablename,attribute->name);
        offsets.push_back(make_pair(offset,type));
    }
    for(auto iter:addr) {
        TableRow row = BufferManager::read(table,iter);
        char* p = row.data;
        int flag = true;
        if(*p == '?') continue;
        for(int j = 0;j<conditions.size();j++) {
            stringstream ss;
            if(offsets[j].second == -1) {
                int temp = *(int*)(p+offsets[j].first);
                ss << conditions[j].value;
                int value;
                ss >> value;
                switch(conditions[j].op)
                {
                    case OP_EQ:
                        if(temp != value) flag = false;
                        break;
                    case OP_NEQ:
                        if(temp == value) flag = false;
                        break;
                    case OP_GEQ:
                        if(temp < value) flag = false;
                        break;
                    case OP_GT:
                        if(temp <= value) flag = false;
                        break;
                    case OP_LT:
                        if(temp >= value) flag = false;
                        break;
                    case OP_LEQ:
                        if(temp > value) flag = false;
                        break;
                }
            }else if(offsets[j].second == 0) {
                float temp = *(float*)(p+offsets[j].first);
                ss << conditions[j].value;
                float value;
                ss >> value;
                switch(conditions[j].op)
                {
                    case OP_EQ:
                        if(temp != value) flag = false;
                        break;
                    case OP_NEQ:
                        if(temp == value) flag = false;
                        break;
                    case OP_GEQ:
                        if(temp < value) flag = false;
                        break;
                    case OP_GT:
                        if(temp <= value) flag = false;
                        break;
                    case OP_LT:
                        if(temp >= value) flag = false;
                        break;
                    case OP_LEQ:
                        if(temp > value) flag = false;
                        break;
                }
            }else {
                char temp[offsets[j].second+1];
                temp[offsets[j].second] = '\0';
                char* ptemp = p+offsets[j].first;
                for(int k = 0;k<offsets[j].second;k++) {
                    temp[k] = ptemp[k];
                }
                string stemp = temp;
                string value = conditions[j].value;
                switch(conditions[j].op)
                {
                    case OP_EQ:
                        if(stemp != value) flag = false;
                        break;
                    case OP_NEQ:
                        if(stemp == value) flag = false;
                        break;
                    case OP_GEQ:
                        if(stemp < value) flag = false;
                        break;
                    case OP_GT:
                        if(stemp <= value) flag = false;
                        break;
                    case OP_LT:
                        if(stemp >= value) flag = false;
                        break;
                    case OP_LEQ:
                        if(stemp > value) flag = false;
                        break;
                }
            }
            if(!flag) break;
        }
        if(flag) {
            TableRow row;
            row.datasize = size;
            row.data = new char[size];
            int addr = 0;
            for(auto iter :needed) {
                for(int x = 0;x<iter.second;x++) {
                    row.data[addr++] = p[iter.first+x];
                }
            }
            ret.push_back(row);
        }
        delete [] p;   
    }
    return ret;
}

vector<Address> RecordManager::deleteT(vector<Address> addr,vector<Cond> conds)
{
    vector<Address> ret;
    if(addr.empty()) return ret;
    if(conds.empty()) {
        ret = addr;
        BufferManager::free(addr);
        return ret;
    }else {
        vector< pair<int,int> > offsets;
        table table = CatalogManager::getTable(addr.begin()->file);
        for(auto iter : conds) {
        int offset = 0;
        auto attribute = table.attrs.begin();
        for(;attribute != table.attrs.end() && attribute->name != iter.attr_name;attribute++)
        {
            int type =CatalogManager::fieldType(table.tablename,attribute->name);
            if(type == -1) offset += sizeof(int);
            else if(type == 0) offset += sizeof(float);
            else offset += type;
        }
        int type =CatalogManager::fieldType(table.tablename,attribute->name);
        offsets.push_back(make_pair(offset,type));
        }
        for(auto iter:addr) {
            TableRow row = BufferManager::read(table,iter);
            char* p = row.data;
            int flag = true;
            if(*p == '?') continue;
            for(int j = 0;j<conds.size();j++) {
                stringstream ss;
                if(offsets[j].second == -1) {
                    int temp = *(int*)(p+offsets[j].first);
                    ss << conds[j].value;
                    int value;
                    ss >> value;
                    switch(conds[j].op)
                    {
                        case OP_EQ:
                            if(temp != value) flag = false;
                            break;
                        case OP_NEQ:
                            if(temp == value) flag = false;
                            break;
                        case OP_GEQ:
                            if(temp < value) flag = false;
                            break;
                        case OP_GT:
                            if(temp <= value) flag = false;
                            break;
                        case OP_LT:
                            if(temp >= value) flag = false;
                            break;
                        case OP_LEQ:
                            if(temp > value) flag = false;
                            break;
                    }
                }else if(offsets[j].second == 0) {
                    float temp = *(float*)(p+offsets[j].first);
                    ss << conds[j].value;
                    float value;
                    ss >> value;
                    switch(conds[j].op)
                    {
                        case OP_EQ:
                            if(temp != value) flag = false;
                            break;
                        case OP_NEQ:
                            if(temp == value) flag = false;
                            break;
                        case OP_GEQ:
                            if(temp < value) flag = false;
                            break;
                        case OP_GT:
                            if(temp <= value) flag = false;
                            break;
                        case OP_LT:
                            if(temp >= value) flag = false;
                            break;
                        case OP_LEQ:
                            if(temp > value) flag = false;
                            break;
                    }
                }else {
                    char temp[offsets[j].second+1];
                    temp[offsets[j].second] = '\0';
                    char* ptemp = p+offsets[j].first;
                    for(int k = 0;k<offsets[j].second;k++) {
                        temp[k] = ptemp[k];
                    }
                    string stemp = temp;
                    string value = conds[j].value;
                    switch(conds[j].op)
                    {
                        case OP_EQ:
                            if(stemp != value) flag = false;
                            break;
                        case OP_NEQ:
                            if(stemp == value) flag = false;
                            break;
                        case OP_GEQ:
                            if(stemp < value) flag = false;
                            break;
                        case OP_GT:
                            if(stemp <= value) flag = false;
                            break;
                        case OP_LT:
                            if(stemp >= value) flag = false;
                            break;
                        case OP_LEQ:
                            if(stemp > value) flag = false;
                            break;
                    }
                }
                if(!flag) break;
            }
            if(flag) {
                ret.push_back(iter);
                vector<Address> addrs;
                addrs.push_back(iter);
                BufferManager::free(addrs);
            }   
        }
        return ret; 
    }
}