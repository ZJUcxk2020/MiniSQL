//
// Created by Administrator on 2021/6/2.
//
#include "API.h"
#include "CatalogManager.h"
#include "Interpreter.h"
#include "BufferManager.h"
#include "IndexManager.h"
#include <iostream>
using namespace std;
Interpreter::Interpreter() {
    command="";
    i=0;
}
Interpreter::Interpreter(string& com) {
    command=com;
    i=0;
}
void Interpreter::init(const string &str) {
    command=str;
    i=0;
    filename="";
}
int Interpreter::interpret() {
    string word= parser(command);
    if(word=="create"){ //创建语句
        word= parser(command);
        if(word=="table"){  //建表语句
            string tablename= parser(command);
            if(tablename.empty()){
                cout<<"Error:No table name"<<endl;
                return -1;
            }
            word= parser(command);
            if(word.empty()||word!="("){
                cout<<"Error:No further information"<<endl;
                return -1;
            }
            vector<Attr> Attris;  //属性集合
            word= parser(command);
            while(word!=")"&&word!="primary"&&!word.empty()){
                Attr temp;
                temp.name= word;
                word= parser(command);  //type
                if(word=="int"){
                    temp.type=-1;
                }
                else if(word=="float"){
                    temp.type=0;
                }
                else if(word=="char"){  //char 类型有括号
                    word= parser(command);
                    if(word!="("){
                        cout<<"Syntax Error"<<endl;
                        return -1;
                    }
                    word= parser(command);
                    int number=stoi(word);
                    if(number>0&&number<=255){
                        temp.type=number;
                    }
                    else{
                        cout<<"The length of char is too short or too long"<<endl;
                        return -1;
                    }
                    word= parser(command);
                    if(word!=")"){
                        cout<<"Syntax Error"<<endl;
                        return -1;
                    }
                }
                else{
                    cout<<"Illegal data type"<<endl;
                    return -1;
                }
                word= parser(command);
                if(word=="unique"){
                    temp.isunique= true;
                    word= parser(command);
                }
                else if(word==","){
                    temp.isunique= false;
                }
                else if(word==")"){
                    temp.isunique= false;
                    Attris.push_back(temp);
                    break;
                }
                else{
                    cout<<"Syntax Error"<<endl;
                    return -1;
                }

                temp.index="";
                temp.isprimarykey= false;
                Attris.push_back(temp);
                word= parser(command);
            }
            if(word=="primary"){
                word= parser(command);
                if(word=="key"){
                    word= parser(command);
                    if(word!="("){
                        cout<<"Syntax Error"<<endl;
                        return -1;
                    }
                    word= parser(command);
                    int j;
                    for( j=0;j<Attris.size();j++){
                        if(Attris[j].name==word){
                            Attris[j].isunique= true;
                            Attris[j].isprimarykey= true;
                            break;
                        }
                    }
                    if (j==Attris.size()){
                        cout<<"Error:primaryKey does not exist in attributes"<<endl;
                        return -1;
                    }
                    word= parser(command);
                    if(word!=")"){
                        cout<<"Syntax Error"<<endl;
                        return -1;
                    }
                    word= parser(command);
                    if(word!=")"){
                        cout<<"Syntax Error"<<endl;
                        return -1;
                    }
                }
                else{
                    cout<<"Syntax Error"<<endl;
                    return -1;
                }
            }
            else if(word.empty()){
                cout<<"Syntax Error"<<endl;
                return -1;
            }//  ")"的情况
//            cout<<tablename<<endl;
//            //use api to do someting
//            for(auto & Attri : Attris){
//                cout<<Attri.name<<" "<<Attri.type<<" "<<Attri.isunique<<" "<<Attri.isprimarykey<<Attri.index<<endl;
//            }
            int ret=API::createTable(tablename,Attris);
            if (ret==SUCCESS){
                return 1;
            }
            else if(ret==EXIST){
                cout<<"This table already exists"<<endl;
                return -1;
            }
            else if(ret==FAIL){
                cout<<"Fail to create table"<<endl;
                return -1;
            }
            return 1;
        }
        else if(word=="index"){
            string indexname;
            string tablename;
            string attrname;
            word= parser(command);  //index name
            if(word.empty()){
                cout<<"Error:No index name"<<endl;
                return -1;
            }
            indexname=word;
            word= parser(command);  //check on
            if(word!="on"){
                cout<<"Syntax Error"<<endl;
                return -1;
            }
            word= parser(command); //tablename
            if(word.empty()){
                cout<<"Error:No tablename"<<endl;
                return -1;
            }
            tablename=word;
            word= parser(command);
            if(word!="("){
                cout<<"Syntax Error"<<endl;
                return -1;
            }
            word= parser(command);//attribute name
            if(word.empty()){
                cout<<"No attribute name"<<endl;
                return -1;
            }

            attrname=word;
            word= parser(command);
            if(word!=")"){
                cout<<"Syntax Error"<<endl;
                return -1;
            }
            int ret=API::createIndex(tablename,indexname,attrname);
            if (ret==SUCCESS){
                return 1;
            }
            else if(ret==EXIST){
                cout<<"This index already exists"<<endl;
                return -1;
            }
            else if(ret==FAIL){
                cout<<"Fail to create index"<<endl;
                return -1;
            }
            else if(ret==NOT_EXIST){
                cout<<"The attribute is nonexistent"<<endl;
                return -1;
            }
            return 1;
//            cout<<tablename<<" "<<indexname<<" "<<attrname<<endl;
            return 1;
            //use api to do insert index
        }
        else{
            cout<<"Syntax Error"<<endl;
            return -1;
        }
    }
    else if(word=="select"){//选择语句
        word= parser(command);
        vector<string> select_attrs;
        string tablename;
        if(word!="*"){
            //word= parser(command);
            while (word!="from"){
                select_attrs.push_back(word);
                word= parser(command);
                if(word!=","&&word!="from"){
                    cout<<"Syntax Error"<<endl;
                    return -1;
                }
                if(word==","){
                    word= parser(command);
                }
            }
        }
        else {
            word= parser(command);
        }
        //word = parser(command);
        if(word!="from"){
            cout<<"Syntax Error"<<endl;
            return -1;
        }
        word= parser(command);
        if(word.empty()){
            cout<<"Error:No table name"<<endl;
            return -1;
        }
        tablename=word;
        word= parser(command);
        if(word.empty()){  //没有附加条件的查询
            //use API to do something
            vector<Cond>nothing;
                // something
                int ret=API::select(tablename,select_attrs,nothing);
                if(ret==SUCCESS){
                    return 1;
                }
                else if(ret==NOT_EXIST){
                    cout<<"Table doesn't exist"<<endl;
                    return -1;
                }
                else{
                    cout<<"selection doesn't work"<<endl;
                    return -1;
                }
        }
        else if(word=="where"){
            vector<Cond> Conds;
            while (1){
                Cond temp;
                word= parser(command);  // attr_name
                if(word.empty()||word=="and"){
                    cout<<"Condition Error:No Attribute name"<<endl;
                    return -1;
                }
                temp.attr_name=word;
                word= parser(command);  //op
                if(word.empty()||word=="and"){
                    cout<<"Condition Error:No operation"<<endl;
                    return -1;
                }
                else{
                    if(word=="="){
                        temp.op=0;
                    }
                    else if(word=="<"){
                        temp.op=1;
                    }
                    else if(word==">"){
                        temp.op=2;
                    }
                    else if(word=="<="){
                        temp.op=3;
                    }
                    else if(word==">="){
                        temp.op=4;
                    }
                    else if(word=="<>"){
                        temp.op=5;
                    }
                    else{
                        cout<<"Condition Error:No such operation"<<endl;
                        return -1;
                    }
                }
                word= parser(command); //value
                if(word.empty()||word=="and"){
                    cout<<"Condition Error:No value"<<endl;
                    return -1;
                }
                int type=CatalogManager::fieldType(tablename,temp.attr_name);
                if(type>0){
                    int size=word.size();
                    while(size<type){
                        word+=" ";
                        size++;
                    }
                }
                temp.value=word;
                word= parser(command);
                Conds.push_back(temp);
                if(word.empty()){
                    break;
                }
                else if(word!="and"){
                    cout<<"Syntax Error"<<endl;
                    return -1;
                }
            }

//            if(select_attrs.empty()){
//                cout<<"*"<<endl;
//            }
//            else{
//                for(auto j:select_attrs){
//                    cout<<j<<" "<<endl;
//                }
//            }
//            cout<<tablename<<endl;
//            for(auto j:Conds){
//                cout<<j.attr_name<<" "<<j.op<<" "<<j.value<<" "<<endl;
//            }
           int ret= API::select(tablename,select_attrs,Conds);
            if(ret==SUCCESS){
                return 1;
            }
            else{
                cout<<"selection doesn't work"<<endl;
                return -1;
            }
            //do something using API
            //return 1;
        }
        else{
            cout<<"Syntax Error"<<endl;
            return -1;
        }
    }
    else if(word=="drop"){
        word= parser(command);
        if(word=="table"){
            word= parser(command);
            if(word.empty()){
                cout<<"Error:No table name"<<endl;
                return -1;
            }
            string tablename=word;
            //cout<<tablename<<endl;
            int ret=API::dropTable(tablename);
            if(ret==SUCCESS){
                return 1;
            }
            else{
                cout<<"drop doesn't work"<<endl;
                return -1;
            }
            // use api to do drop table
        } else if(word=="index"){
            word= parser(command);
            if(word.empty()){
                cout<<"Error:No index name"<<endl;
                return -1;
            }
            string indexname=word;
           // cout<<indexname<<endl;
            // use api to do drop index
            int ret=API::dropIndex(indexname);
            if(ret==SUCCESS){
                return 1;
            }
            else{
                cout<<"drop doesn't work"<<endl;
                return -1;
            }
        }
        else{
            cout<<"Syntax Error"<<endl;
            return -1;
        }
    }
    else if(word=="insert"){
        word= parser(command);
        if(word!="into"){
            cout<<"Syntax Error"<<endl;
            return -1;
        }
        word= parser(command);
        if(word.empty()){
            cout<<"Error:No table name"<<endl;
            return -1;
        }
        string tablename=word;
        word= parser(command);
        if(word!="values"){
            cout<<"Syntax Error"<<endl;
            return -1;
        }
        word= parser(command);
        if(word!="("){
            cout<<"Syntax Error"<<endl;
            return -1;
        }
        vector<string> values;
        while(1){
            word= parser(command);
            if(word.empty()||word==")"){
                cout<<"Syntax Error"<<endl;
                return -1;
            }
            values.push_back(word);
            word= parser(command);
            if(word==","){
                continue;
            }
            else if(word==")"){
                break;
            }
            else{
                cout<<"Syntax Error"<<endl;
                return -1;
            }
        }
        //use api to insert
        bool flag=CatalogManager::tableExists(tablename);
        if(!flag){
            cout<<"table dosen't exist"<<endl;
            return -1;
        }
        int length=0;
       length=CatalogManager::rowLength(tablename);
        TABLEROW row;
        row.datasize=length;
        row.data=new char[length];
//        str.copy(row.data,length,0);
//        *(row.data+length+1)='\0';
//        for(int j=0;j<row.datasize;j++){
//            row.data[j]=str[j];
//        }
        int j=0;

        vector<string> attr_name=CatalogManager::fieldsOnTable(tablename);
        for (int k=0;k<values.size();k++){
            int type= CatalogManager::fieldType(tablename,attr_name[k]);
            if(type==-1){
                int v= stoi(values[k]);
                char * pv=(char *)&v;
                memcpy(row.data+j,&v,sizeof(int));
                j+=4;
            }
            else if(type==0){
                float v= stof(values[k]);
                char * pv=(char *)&v;
                memcpy(row.data+j,&v,sizeof(float));
                j+=4;
            }
            else{
                int z=0;
                memset(row.data+j,' ',type);
                for(z=0;z<values[k].size();z++) {
                    row.data[j+z]=(values[k].c_str())[z];
                }
                j+=type;
                // while(z<type){
                //     row.data[j+z]='.';
                //     z++;
                // }
            }
        }
        // char x[11];
        // x[10]  = '\0';
        // memcpy(x,row.data,10);
        // printf("|%s|\n",x);
        // int y;
        // memcpy(&y,row.data+10,4);
        // cout << ',' << x << ','<<y<<endl;
        // system("pause");
        int ret=API::insert(tablename,row);
        if(ret==SUCCESS){
            return 1;
        }
        else if(ret==NOT_EXIST){
            cout<<"The table is inexistant"<<endl;
            return -1;
        }
        else{
            cout<<"fail to insert"<<endl;
            return -1;
        }
        //return 1;
    }
    else if(word=="delete"){
        word= parser(command);
        if(word!="from"){
            cout<<"Syntax Error"<<endl;
            return -1;
        }
        word= parser(command);
        if(word.empty()){
            cout<<"Error:No table name"<<endl;
            return -1;
        }
        string tablename=word;
        word= parser(command);
        vector<Cond> Conds;
        if(word.empty()){  //删除整个表
           //use api to delete the whole table
           int ret=API::deleteT(tablename,Conds);
           if(ret==SUCCESS){
               return 1;
           }
           else if(ret==NOT_EXIST){
               cout<<"This table is inexistant"<<endl;
               return -1;
           }
           else{
               cout<<"Fail"<<endl;
               return -1;
           }
//           cout<<"delete whole table"<<" "<<tablename<<endl;
//            return 1;
        }
        else if(word=="where"){
            bool flag=CatalogManager::tableExists(tablename);
            if(!flag){
                cout<<"table doesn't exist"<<endl;
                return -1;
            }
            while (1){
                Cond temp;
                word= parser(command);  // attr_name
                if(word.empty()||word=="and"){
                    cout<<"Condition Error:No Attribute name"<<endl;
                    return -1;
                }
                temp.attr_name=word;
                word= parser(command);  //op
                if(word.empty()||word=="and"){
                    cout<<"Condition Error:No operation"<<endl;
                    return -1;
                }
                else{
                    if(word=="="){
                        temp.op=0;
                    }
                    else if(word=="<"){
                        temp.op=1;
                    }
                    else if(word==">"){
                        temp.op=2;
                    }
                    else if(word=="<="){
                        temp.op=3;
                    }
                    else if(word==">="){
                        temp.op=4;
                    }
                    else if(word=="<>"){
                        temp.op=5;
                    }
                    else{
                        cout<<"Condition Error:No such operation"<<endl;
                        return -1;
                    }
                }
                word= parser(command); //value
                if(word.empty()||word=="and"){
                    cout<<"Condition Error:No value"<<endl;
                    return -1;
                }
                int type=CatalogManager::fieldType(tablename,temp.attr_name);
                if(type>0){
                    int size=word.size();
                    while(size<type){
                        word+=" ";
                        size++;
                    }
                }

                temp.value=word;
                word= parser(command);
                Conds.push_back(temp);
                if(word.empty()){
                    break;
                }
                else if(word!="and"){
                    cout<<"Syntax Error"<<endl;
                    return -1;
                }
            }
            //use api to delete some turples
            int ret=API::deleteT(tablename,Conds);
            if(ret==SUCCESS){
                return 1;
            }
            else if(ret==NOT_EXIST){
                cout<<"This table is inexistant"<<endl;
                return -1;
            }
            else{
                cout<<"Fail"<<endl;
                return -1;
            }
        }
        else {
            cout<<"Syntax Error"<<endl;
            return -1;
        }
    }
    else if(word=="quit"){
        bool ret=CatalogManager::storeToFile();
        bool flag=BufferManager::writrToFileAll();
        bool flag1=IndexManager::writetoFile();
        if(!ret){
            cout<<"Fail to store catalog information into files"<<endl;
        }
        if(!flag){
            cout<<"Fail to store dirty blocks into files"<<endl;
        }
        return 2;
    }
    else if(word=="execfile"){
        word= parser(command);
        if(word.empty()){
            cout<<"Error :No file name"<<endl;
            return -1;
        }
        filename=word;
        return 0;
    }
    else{
        cout<<"No such command"<<endl;
        return -1;
    }
    //return 1;
}
string Interpreter::parser(string &subcom) {
    string word;
    int len=subcom.size()-1;
    while((subcom[i] == ' '|| subcom[i] == '\n' || subcom[i] == '\t') &&i<=len ){//处理空格和换行
        i++;
    }
    if(subcom[i]=='(' || subcom[i]==',' ||subcom[i]==')'||subcom[i]=='*'){ //直接返回这些符号
        word+=subcom[i];
        i++;
        return word;
    }
    if(subcom[i]==39){ //单引号，把引号内容返回
        i++;
        while(subcom[i]!=39 &&i<=len){
            word+=subcom[i];
            i++;
        }
        //i++;
        if(subcom[i]==39){
            i++;
            return word;
        }
        else{
            word="";
            return word;
        }
    }

    else{
        while(subcom[i]!=' '&&subcom[i]!='(' && subcom[i] !='\n' && subcom[i]!= '\t' && subcom[i] != ')' && i<=len && subcom[i]!=','){
            word+=subcom[i];
            i++;
        }
        return word;
    }
}
