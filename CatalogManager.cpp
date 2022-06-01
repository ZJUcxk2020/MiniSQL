#include "CatalogManager.h"
#include <fstream>
using namespace std;
TabFile CatalogManager::TF;
IdxFile CatalogManager::IF;
bool CatalogManager::readFromFile() {  //从文件中读入表信息和索引信息
    ifstream tablein;
    ifstream indexin;
    tablein.open("Table_info",ios::in);
    indexin.open("Index_info",ios::in);
    if(!tablein.is_open()||!indexin.is_open()){
        return false;
    }
    string tfnum;
    getline(tablein,tfnum);
//    tablein>>TF.num;
    if(tfnum.empty()){
        TF.num=0;
    }
    else{
        TF.num=stoi(tfnum);
    }
    for(int i=0;i<TF.num;i++){
        Table T;
        getline(tablein,T.tablename);
        string num_attr;
        getline(tablein,num_attr);
//        tablein>>T.num_attr;
        T.num_attr=stoi(num_attr);
        string str_type,str_pk,str_uni;
        for(int j=0;j<T.num_attr;j++){
            Attr A;
            getline(tablein,A.name);
            getline(tablein,str_type);
            getline(tablein,str_pk);
//            tablein>>A.type;
//            tablein>>A.isprimarykey;
            getline(tablein,A.index);
            getline(tablein,str_uni);
//            tablein>>A.isunique;
            A.type=stoi(str_type);
            A.isprimarykey=stoi(str_pk);
            A.isunique= stoi(str_uni);
            T.attrs.push_back(A);
        }
        TF.tables.push_back(T);
    }
    string ifnum;
    getline(indexin,ifnum);
    //indexin>>IF.num;
    if(ifnum.empty()){
        IF.num=0;
    }
    else{
        IF.num=stoi(ifnum);
    }
    for(int i=0;i<IF.num;i++){
        Index Ind;
//        indexin>>Ind.name;
//        indexin>>Ind.table;
//        indexin>>Ind.attribute;
        getline(indexin,Ind.name);
        getline(indexin,Ind.table);
        getline(indexin,Ind.attribute);
        IF.Idxs.push_back(Ind);
    }
    return true;
}
bool CatalogManager::tableExists(const string& table) {//判断该表存不存在
    for(const auto& i:TF.tables){
        if(i.tablename==table){
            return true;
        }
    }
    return false;
}
bool CatalogManager::indexExists(const string& index) {//判断该索引存不存在
    for(const auto& i:IF.Idxs){
        if(i.name==index){
            return true;
        }
    }
    return false;
}
int CatalogManager::fieldCount(const string& table) {  //返回该表的属性数量
    int fieldcnt=0;
    for(const auto& i:TF.tables){
        if(i.tablename==table){
            fieldcnt=i.num_attr;
        }
    }
    return fieldcnt;
}
Table CatalogManager::getTable(const string& tableName) {  //返回表信息
    Table T;
    for (const auto i:TF.tables){
        if(i.tablename==tableName){
            T=i;
            break;
        }
    }
    return T;
}

vector<string> CatalogManager::fieldsOnTable(const string& table) {  //返回该表上的所有属性名
    vector<string > fields;
    for(const auto i:TF.tables){
        if(i.tablename==table){
            for(const auto j:i.attrs){
                fields.push_back(j.name);
            }
        }
    }
    return fields;
}

int CatalogManager::fieldType(const string& table, const string& field) {  //返回表中某个属性的类型
    int type=0;
    for(const auto i:TF.tables){
        if(i.tablename==table){
            for(const auto j:i.attrs){
                if(j.name==field){
                    type=j.type;
                    break;
                }
            }
        }
    }
    return type;
}
bool CatalogManager::isUnique(const string& table, const string& field) {  //判断表内某个属性是否唯一
    bool uni= false;
    for(const auto i:TF.tables){
        if(i.tablename==table){
            for(const auto j:i.attrs){
                if(j.name==field){
                    uni=j.isunique;
                    break;
                }
            }
        }
    }
    return uni;
}
bool CatalogManager::hasIndex(const string& table, const string& field) {  //判断表内某属性是否有索引
    bool hasindex=false;
    for(const auto& i:TF.tables){
        if(i.tablename==table){
            for(const auto& j:i.attrs){
                if(j.name==field){
                    if(j.index.empty()){
                        hasindex= false;
                    }
                    else{
                        hasindex= true;
                    }
                    break;
                }
            }
        }
    }
    return hasindex;
}
string CatalogManager::indexName(const string& table, const string& field) {//返回表中某字段上的索引名
    string indexname;
    for(const auto i:TF.tables){
        if(i.tablename==table){
            for(const auto j:i.attrs){
                if(j.name==field){
                    indexname=j.index;
                    break;
                }
            }
        }
    }
    return indexname;
}
bool CatalogManager::addTable(const Table& T) {//添加一个表信息
    if(tableExists(T.tablename))return false;
    TF.tables.push_back(T);
    TF.num++;
    return true;
}
bool CatalogManager::deleteTable(const string& table) {//删除一个表信息
    if(!tableExists(table))return false;
    for( auto i=TF.tables.begin();i!=TF.tables.end();i++){//先找到这个表
        if((*i).tablename==table){
            TF.tables.erase(i);//删除
            break;
        }
    }
    TF.num--;
    return true;
}
int CatalogManager::rowLength(const string& table) {//返回表内一行的长度，单位是字节
    int rowlength=0;
    for(const auto i:TF.tables){
        if(i.tablename==table){
            for(const auto j:i.attrs){
                if(j.type==-1||j.type==0){//  int or float  4字节
                    rowlength+=4;
                }
                else{
                    rowlength+=j.type;   // char
                }
            }
        }
    }

    return rowlength;
}
bool CatalogManager::addIndex(const Index& idx) {//添加一个索引信息
    if(indexExists(idx.name))return false;
    IF.Idxs.push_back(idx);
    IF.num++;
    for(auto &i:TF.tables){
        bool flag = false;
        if(i.tablename==idx.table){
            for(auto &j:i.attrs){
                if(j.name==idx.attribute){
                    j.index=idx.name;
                    flag = true;
                    break;
                }
            }
        }
        if(flag) break;
    }
    return true;
}
bool CatalogManager::deleteIndex(const string& index) {//删除一个索引信息
    if(!indexExists(index))return false;
    string tablename;
    for(auto i=IF.Idxs.begin();i!=IF.Idxs.end();i++){
        if(i->name==index){//找到该索引
            tablename=i->table;
            IF.Idxs.erase(i);//删除信息
            break;
        }
    }
    IF.num--;
    bool flag=false;
    for(int i=0; i<TF.num; i++){
        if(TF.tables[i].tablename==tablename){
                for(int j=0;j<TF.tables[i].attrs.size();j++){
                    if(TF.tables[i].attrs[j].index==index){
                        TF.tables[i].attrs[j].index="";
                        flag=true;
                        break;
                    }
                }
        }
        if(flag)break;
    }
    return true;
}
int CatalogManager::indexCount(const string& table) {//计算该表上索引的数量
    int cnt=0;
    for(auto i:IF.Idxs){
        if(i.table==table){
            cnt++;
        }
    }
    return cnt;
}
vector<string> CatalogManager::indicesOnTable(const string& table) {//返回该表上的所有索引名
    vector<string>inds;
    for(auto i:IF.Idxs){
        if(i.table==table){//找到表名相同的
            inds.push_back(i.name);
        }
    }
    return inds;
}
string CatalogManager::tableIndexOn(const string& index) {//返回改索引位于哪个表
    for(auto i:IF.Idxs){
        if(i.name==index){
            return i.table;
        }
    }
    return "";
}
string CatalogManager::fieldIndexOn(const string& index) {//返回索引位于哪个字段
    for(auto i:IF.Idxs){
        if(i.name==index){
            return i.attribute;
        }
    }
    return "";
}
bool CatalogManager::isPK(const string& table, const string& field) {//判断表中某个字段是否是primaryKey
    for(auto i:TF.tables){
        if(i.tablename==table){
            for(auto j:i.attrs){
                if(j.name==field){
                    return j.isprimarykey;
                }
            }
        }
    }
    return false;
}
string CatalogManager::pkOnTable(const string& table) {//返回表中的primarykey，如果没有返回空字符串
    for(auto i:TF.tables){
        if(i.tablename==table){//找到表
            for(auto j:i.attrs){//遍历属性
                if(j.isprimarykey){
                    return j.name;
                }
            }
        }
    }
    return "";
}
bool CatalogManager::storeToFile() {//把信息存入文件中
    ofstream tableout;
    ofstream indexout;
    tableout.open("Table_info",ios::trunc);
    indexout.open("Index_info",ios::trunc);
    if(!tableout.is_open()||!indexout.is_open()){
        cout<<"Fail to store catalog information into file"<<endl;
        return false;
    }
    if(TF.num==0){
        tableout.close();
    }
    else{
        tableout<<TF.num<<endl;
        for(int i=0;i<TF.num;i++){
            tableout<<TF.tables[i].tablename<<endl;
            tableout<<TF.tables[i].num_attr<<endl;
            for(auto j:TF.tables[i].attrs){
                tableout<<j.name<<endl;
                tableout<<j.type<<endl;
                tableout<<j.isprimarykey<<endl;
                tableout<<j.index<<endl;
                tableout<<j.isunique<<endl;
            }
        }
        tableout.close();
    }
    if(IF.num==0){
        indexout.close();
    }
    else{
        indexout<<IF.num<<endl;
        for(auto i:IF.Idxs){
            indexout<<i.name<<endl;
            indexout<<i.table<<endl;
            indexout<<i.attribute<<endl;
        }
        indexout.close();
    }
    return true;
}
