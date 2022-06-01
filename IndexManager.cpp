#include <iostream>
#include "IndexManager.h"
#include "CatalogManager.h"
#include "Table.h"
#include "Block.h"
#include "BufferManager.h"
#include "BPlusTree.h"
#include <set>
#include <vector>
#include <fstream>
#include <algorithm>
#define MAX_TREE 5
using namespace std;

map<string,BPlusTree*> IndexManager::trees;

bool write(BPlusTree &tree,string name) 
{
    vector<node> Nodes = tree.showLeaves(tree.root);
    ofstream file(name+"_idx",ios::trunc);
    for(auto n :Nodes) {
        file << n.value+"," << n.Type << "," << n.offset << "," << n.file<< endl;
    }
    file.close();
    return true;
}

bool IndexManager::createIndex(string table, string attribute,string idxName,int valueType)
{
    bool ret = true;
    Table t = CatalogManager::getTable(table);
    vector<block> bs = BufferManager::read(t);
    BPlusTree *Btree = new BPlusTree();
    FILE* fp = fopen((idxName+"_idx").c_str(), "w");
    for(auto b:bs) {
        char* data = b.data;
        int size = b.spaceused;
        pair<int,int> offset = make_pair(0,0);
        int off = 0;
        for(auto iter:t.attrs) {
            if(iter.name == attribute) {
                offset.first = off;
                if(iter.type == -1) offset.second = sizeof(int);
                else if(iter.type == 0) offset.second = sizeof(float);
                else offset.second = iter.type;
                break;
            }
            if(iter.type == -1) off += sizeof(int);
            else if(iter.type == 0) off += sizeof(float);
            else off += iter.type;
        }
        int rowlength = CatalogManager::rowLength(table);
        for(int i = 0;i*rowlength<size;i++) {
            char* p = data + i*rowlength;
            string s = "";
            if(valueType == -1) s = to_string(*(int*)(p+offset.first));
            else if(valueType == 0) s = to_string(*(float*)(p+offset.first));
            else {
                char c[offset.second+1];
                c[offset.second] = 0;
                memcpy(c,p+offset.first,offset.second);
                s = c;
            }
            node n;
            n.value = s;
            n.Type = valueType;
            n.offset = i*rowlength;
            n.file = b.fileoffset;
            Btree->root = Btree->add(Btree->root,n);
        }
    }
    if(trees.size() >= MAX_TREE) {
        write(*(trees.begin()->second),trees.begin()->first);
        trees.begin()->second->Free(Btree->root);
        delete [] trees.begin()->second;
        trees.erase(trees.begin());
    }
    trees.insert(make_pair(idxName,Btree));
    fclose(fp);
    return ret;
}

bool IndexManager::dropIndex(string index)
{
    auto iter = trees.find(index);
    if(trees.find(index) != trees.end()) {
        iter->second->Free(iter->second->root);
        trees.erase(iter);
    }
    bool ret = remove((index+"_idx").c_str());
    return (ret == 0)?true:false;
}

BPlusTree *readfromFile(string idx)
{
    ifstream file(idx+"_idx",ios::in);
    if(!file.is_open()) cout << "File open failed." << endl;
    BPlusTree *tree = new BPlusTree();
    // cout << "Tree address is "<< tree << " Tree root address is "<<tree->root << endl;
    string str;
    getline(file,str);
    while(!file.eof()) {
        // cout << "reading file..." << endl;
        node n;
        int op = str.find(",",0);
        n.value = str.substr(0,op);
        str = str.substr(op+1,str.size()-op-1); 
        op = str.find(",",0);
        n.Type = stoi(str.substr(0,op));
        str = str.substr(op+1,str.size()-op-1);
        op = str.find(",",0);
        n.offset = stoi(str.substr(0,op));
        n.file = stoi(str.substr(op+1,str.size()-op-1));
        tree->root = tree->add(tree->root,n);
        // cout << "Tree address is "<< tree << " Tree root address is "<<tree->root << endl;
        getline(file,str);
    }
    return tree;
}

vector<Address> IndexManager::select(string index, vector<Cond> conditions)
{
    auto iter = trees.find(index);
    if(iter == trees.end()) {
        if(trees.size() >= MAX_TREE) {
            write(*(trees.begin()->second),trees.begin()->first);
            trees.begin()->second->Free(trees.begin()->second->root);
            delete [] trees.begin()->second;
            trees.erase(trees.begin());
        }
        BPlusTree *tree = readfromFile(index);
        trees.insert(make_pair(index,tree));
        // vector<node> nodes=tree.showLeaves(tree.root);
        // for(auto& node : nodes) {
        //     cout << node.value << ' ' << node.Type << " " << node.offset <<endl;
        // }
    }
    BPlusTree *t = trees[index];
    // cout << "Tree address is "<< t << " Tree root address is "<<t->root << endl;
    vector<node> nodes;
    // for(auto& node : nodes) {
    //     cout << node.value << ' ' << node.Type << " " << node.offset <<endl;
    // }
    vector<Address> ret;
    string table = CatalogManager::tableIndexOn(index);
    string attribute = CatalogManager::fieldIndexOn(index);
    vector<Cond> cs;
    set<int> yes;
    if(conditions.empty()) {
        for(auto iter:nodes) {
            yes.insert(iter.offset+iter.file*BLOCK_SIZE);
        }
    }else {
        int first =true;
        for(auto it:conditions) {
            // cout << it.attr_name << " " << it.op << " "<<it.value<<endl;
            // cout << attribute << endl;
            if(it.attr_name == attribute) {
                cs.push_back(it);
            }
        }
        for(auto c:cs) {
            // cout << "Searching Value is " <<c.value<<endl;
            set<int> temp,res;
            int type = CatalogManager::fieldType(table,c.attr_name);
            node cnode;
            cnode.value = c.value;
            cnode.Type = type;
            cnode.offset = 0;
            cnode.file = 0;
            // cout << cnode.value << " " << cnode.Type << " "<<cnode.offset<<endl;
            bool find = false;
            bool flag = false;
            switch(c.op) {
                case OP_EQ:
                    {
                    auto beg = t->search(c.value,type);
                    for(auto node = beg;node != NULL;node = node->nxt) {
                        int i = 0;
                        for(auto n:node->keys) {
                            if(node->flag[i++] == 1) continue;
                            if(!find && n.value!=c.value) continue;
                            if(find && n.value!=c.value) {
                                flag = true;
                                break;
                            }
                            find = true;
                            temp.insert(n.offset+n.file*BLOCK_SIZE);
                        }
                        if(flag) break;
                    }
                    break;}
                case OP_NEQ:
                    nodes=t->showLeaves(t->root);
                    for(auto it:nodes) {
                        if(it< cnode || it> cnode) {
                            Address addr;
                            temp.insert(it.offset+it.file*BLOCK_SIZE);
                        }
                    }
                    break;
                case OP_GEQ:
                    nodes=t->showLeaves(t->root);
                    for(auto it:nodes) {
                        if(it<cnode) continue;
                        Address addr;
                        temp.insert(it.offset+it.file*BLOCK_SIZE);
                    }
                    break;
                case OP_GT:
                    nodes=t->showLeaves(t->root);
                    for(auto it:nodes) {
                        if(it<=cnode) continue;
                        temp.insert(it.offset+it.file*BLOCK_SIZE);
                    }
                    break;
                case OP_LT:
                    nodes=t->showLeaves(t->root);
                    for(auto it:nodes) {
                        if(!(it<cnode)) break;
                        temp.insert(it.offset+it.file*BLOCK_SIZE);
                    }
                    break;
                case OP_LEQ:
                    nodes=t->showLeaves(t->root);
                    for(auto it:nodes) {
                        if(it>cnode) break;
                        temp.insert(it.offset+it.file*BLOCK_SIZE);
                    }
                    break;
            }
            // for(auto iter:temp) {
            //     cout << iter << " ";
            // }
            if(first) {
                first=false;
                set_union(yes.begin(),yes.end(),temp.begin(),temp.end(),inserter(yes, yes.begin()));
            }else {
                set_intersection(yes.begin(),yes.end(),temp.begin(),temp.end(),inserter(res, res.begin()));
                yes = res;
            }
        }
    }
    for(auto iter:yes) {
        Address addr;
        addr.blockOffset = iter%BLOCK_SIZE;
        addr.file = table;
        addr.fileOffset = iter/BLOCK_SIZE;
        ret.push_back(addr);
    }                
    // cout << "Tree address is "<< t << " Tree root address is "<<t->root << endl;
    return ret;
}

bool IndexManager::insert(string index, TableRow data, Table T, Address addr)
{
    auto iter = trees.find(index);
    if(iter == trees.end()) {
        if(trees.size() >= MAX_TREE) {
            write(*(trees.begin()->second),trees.begin()->first);
            trees.begin()->second->Free(trees.begin()->second->root);
            delete [] trees.begin()->second;
            trees.erase(trees.begin());
        }
        BPlusTree *tree = readfromFile(index);
        // cout << "Tree address is "<< tree << " Tree root address is "<<tree->root << endl;
        trees.insert(make_pair(index,tree));
    }
    BPlusTree *t = trees[index];

    // cout << "Tree address is "<< t << " Tree root address is "<<t->root << endl;
    string attribute = CatalogManager::fieldIndexOn(index);
    pair<int,int> offset;
    int off = 0;
    for(auto iter:T.attrs) {
        if(iter.name == attribute) {
            offset.first = off;
            if(iter.type == -1) offset.second = iter.type;
            else if(iter.type == 0) offset.second = iter.type;
            else offset.second = iter.type;
            break;
        }
        if(iter.type == -1) off += sizeof(int);
        else if(iter.type == 0) off += sizeof(float);
        else off += iter.type;
    }
    char* p = data.data;
    string s = "";
    if(offset.second == -1) {s = to_string(*(int*)(p+offset.first));}
    else if(offset.second == 0) s = to_string(*(float*)(p+offset.first));
    else {
        char c[offset.second+1];
        c[offset.second] = 0;
        memcpy(c,p+offset.first,offset.second);
        s = c;
    }
    node n;
    n.value = s;
    n.Type = offset.second;
    n.offset = addr.blockOffset;
    n.file = addr.fileOffset;
    // cout << n.value << ',' << n.Type << ','<<n.offset << endl;
    // cout << "Tree address is "<< t << " Tree root address is "<<t->root << endl;
    // cout << "Inserting Value is " <<n.value<<endl;
    t->root = t->add(t->root,n);
    return true;
}

vector<Address> IndexManager::deleteT(string table, vector<Address> dels)
{
    vector<string> indices = CatalogManager::indicesOnTable(table);
    for(auto idx : indices) {
        auto iter = trees.find(idx);
        if(iter == trees.end()) {
            if(trees.size() >= MAX_TREE) {
                write(*(trees.begin()->second),trees.begin()->first);
                trees.begin()->second->Free(trees.begin()->second->root);
                delete [] trees.begin()->second;
                trees.erase(trees.begin());
            }
            BPlusTree *tree = readfromFile(idx);
            trees.insert(make_pair(idx,tree));
        }
        BPlusTree *t = trees[idx];
        int type = CatalogManager::fieldType(table,CatalogManager::fieldIndexOn(idx));
        vector<node> nodes = t->showLeaves(t->root);
        set<string> deletesAlr;
        for(auto n:nodes) {
            if(deletesAlr.find(n.value) != deletesAlr.end()) continue;
            auto x = dels.begin();
            for(;x!=dels.end() && (x->blockOffset!=n.offset || x->fileOffset!=n.file);x++);
            if(x!=dels.end()) {
                t->deleteT(t->root,n);
                deletesAlr.insert(n.value);
            }
        }
    }
    return dels;
}

bool IndexManager::writetoFile() 
{
    for(auto iter:trees) {
        write(*(iter.second),iter.first);
    }
    return true;
}
