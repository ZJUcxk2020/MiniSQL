//
// Created by Administrator on 2021/6/15.
//
#include <fstream>
#include "API.h"
#include "Address.h"
#include "CatalogManager.h"
#include "Interpreter.h"
#include <bits/stdc++.h>
#include "BufferManager.h"
using namespace std;

int main(){
    bool flag=CatalogManager::readFromFile();
    if(!flag){
        FILE * fp = fopen("Table_info","w");
        fclose(fp);
        fp = fopen("Index_info","w");
        fclose(fp);
        CatalogManager::readFromFile();
    }
    BufferManager::init();
    bool readfile= false;
    string command;
    ifstream inputfile;
    Interpreter inter;
    while(1){
        if(readfile){
            cout<<"Open the "+inter.filename+"..."<<endl;
            inputfile.open(inter.filename.c_str());
            if(!inputfile.is_open())
            {
                cout<<"Fail to open "<<inter.filename<<endl;
                readfile = false;
                continue;
            }
            double currentTime=clock();
            while(getline(inputfile,command,';'))
            {
                if(command.empty()){
                    continue;
                }
                // double currentTime=clock();
                inter.init(command);
                cout<<"execute command:"<<command<<endl;
                int ret=inter.interpret();
                if(ret==0){
                    readfile= true;
                }
                else if(ret==1){
                    // cout<<"successfully execute"<<endl;
                    // cout <<"Time cost:" <<(clock() - currentTime) / 1000. <<"s"<< endl;
                }
                else if(ret==-1){
                    cout << (clock() - currentTime) / 1000. <<"s"<< endl;
                    continue;
                }
                else if(ret==2){
                    cout<<"Byebye"<<endl;
                    return 0;
                    break;
                }
            }
            inputfile.close();
            cout<<"successfully execute"<<endl;
            cout <<"Time cost:" <<(clock() - currentTime) / 1000. <<"s"<< endl;
            readfile= false;
        }
        else{
            cout<<"MiniSQL->";
            getline(cin,command,';');
            double currentTime = clock();
            inter.init(command);
            int ret=inter.interpret();
            if(ret==0){
                readfile= true;
            }
            else if(ret==1){
                cout<<"successfully execute"<<endl;
                cout <<"Time cost:" <<(clock() - currentTime) / 1000. <<"s"<< endl;
            }
            else if(ret==-1){
                cout << (clock() - currentTime) / 1000. <<"s"<< endl;
                continue;
            }
            else if(ret==2){
                cout<<"Byebye"<<endl;
                break;
            }
        }
    }
    return 0;
}