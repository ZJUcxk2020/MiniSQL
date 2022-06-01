//
// Created by Administrator on 2021/6/5.
//

#include "BufferManager.h"
#include "CatalogManager.h"
Buffer BufferManager::Buff;
void BufferManager::init() {//初始化Buffer
    for(int i=0;i<block_num;i++){
        Buff.cnt[i]=0;
        Buff.bufferpool[i].filename="";
        Buff.bufferpool[i].spaceused=0;
        Buff.bufferpool[i].isdirty= false;
        Buff.bufferpool[i].fileoffset=0;
    }
}

vector<block> BufferManager::read(const Table& t) {//读一个表数据
    vector<block> BV;
    int flag=0;
    int f_offset=0;
    while(1){//在Buffer里找属于这个表的文件，如果不在Buffer里就从文件中读取后存到Buffer
        int find=0;
        for(int i=0;i<block_num;i++){
            if(Buff.bufferpool[i].filename==t.tablename&&f_offset==Buff.bufferpool[i].fileoffset){
                Buff.cnt[i]++;
                BV.push_back(Buff.bufferpool[i]);
                find=1;
                f_offset++;
                if(Buff.bufferpool[i].isEnd){ //Buffer里有最后一个block
                    flag=1;
                    break;
                }
            }
        }
        if(flag==1){
            break;
        }
        if(find==0){
            block b= readFromFile(t.tablename,f_offset);
            f_offset++;
            addBlock(b);
            BV.push_back(b);
            if(b.isEnd){//如果拿上来的是最后的,结束循环
                break;
            }
        }
    }
    return BV;
}
TableRow BufferManager::read(const Table& t, const Address& addr) {
    TableRow TR;
    TR.datasize=CatalogManager::rowLength(addr.file);//获取一行长度
    TR.data=new char[TR.datasize];
    for(int i=0;i<block_num;i++){//找到对应位置读取一行数据
        if(Buff.bufferpool[i].filename==addr.file&&Buff.bufferpool[i].fileoffset==addr.fileOffset){
            Buff.cnt[i]++;
            for(int j=0;j<TR.datasize;j++){
                TR.data[j]=Buff.bufferpool[i].data[addr.blockOffset+j];
            }
//            strncpy(TR.data,Buff.bufferpool[i].data+addr.blockOffset,TR.datasize);
            return TR;
        }
    }
    //buffer里没找到
    block b=readFromFile(t.tablename,addr.fileOffset);
    addBlock(b);
    for(int i=0;i<TR.datasize;i++){
        TR.data[i]=b.data[addr.blockOffset+i];
    }
    return TR;
}
Address BufferManager::findFreeSpace(const string& table) {//找空闲位置
    //先打开删除地址文件，读入数据
    vector<int> FO;
    vector<int> BO;
    Address addr;
    addr.file=table;
    ifstream infile;
    string name=table+"_deladdr";
    infile.open(name,ios::in);
    if(infile.is_open()){
        int fo,bo;
        infile>>fo>>bo;
        while(!infile.eof()){
            FO.push_back(fo);
            BO.push_back(bo);
            infile>>fo>>bo;
        }
        infile.close();
    }
    if(!FO.empty()){//如果有删除文件数据
        addr.fileOffset=FO[FO.size()-1];
        addr.blockOffset=BO[BO.size()-1];
        FO.pop_back();
        BO.pop_back();
        ofstream ofile;
        string asd;
        asd=table+"_"+"deladdr";
        ofile.open(asd,ios::trunc);
        for(int i=0;i<FO.size();i++){
            ofile<<FO[i]<<" "<<BO[i]<<endl;
        }
        ofile.close();
    }
    else{//如果没有删除文件数据，说明空闲位置是块尾
        int length=CatalogManager::rowLength(table);
        int flag=0;
        for(int i=0;i<block_num;i++){
            if(Buff.bufferpool[i].filename==table&&Buff.bufferpool[i].isEnd){
                if(Buff.bufferpool[i].spaceused+length<BLOCK_SIZE){//块内能写入
                    addr.blockOffset=Buff.bufferpool[i].spaceused;
                    addr.fileOffset=Buff.bufferpool[i].fileoffset;
                    flag=1;
                    break;
                }
                else{//块满了，写不进去，新建一个块
                    Buff.bufferpool[i].isEnd= false;
                    block b;
                    b.filename=table;
                    b.spaceused=0;
                    b.fileoffset=Buff.bufferpool[i].fileoffset+1;
                    b.isdirty= false;
                    b.isEnd= true;
                    addBlock(b);
                    addr.blockOffset=0;
                    addr.fileOffset=b.fileoffset;
                    flag=1;
                    break;
                }
            }
        }
        //Buffer里找不到文件尾，从文件中找
        int i=0;
        if(!flag){
            while(1){
                block b= readFromFile(table,i);
                if(b.isEnd){
                    if(b.spaceused+length<BLOCK_SIZE){//块内能写入
                        addr.blockOffset=b.spaceused;
                        addr.fileOffset=b.fileoffset;
                        addBlock(b);
                        break;
                    }
                    else{//块内不行写入
                        b.isEnd= false;
                        block c;
                        c.filename=table;
                        c.spaceused=0;
                        c.fileoffset=b.fileoffset+1;
                        c.isdirty= false;
                        c.isEnd= true;
                        addBlock(c);
                        addr.blockOffset=0;
                        addr.fileOffset=c.fileoffset;
                        break;
                    }
                }
                else{
                    i++;
                }
            }
        }
    }
    // cout <<"BF Fileoffset is " << addr.fileOffset << " BF Blockoffset is " << addr.blockOffset << endl;
    return addr;
}
bool BufferManager::free(const vector<Address>& addrs) {
    ofstream ofile;
    ofile.open(addrs[0].file+"_deladdr",ios::app);
    // cout<<"Free addrs.size"<<" "<<addrs.size()<<endl;
    for(int z=0;z<addrs.size();z++){
        int flag=0;
        ofile<<addrs[z].fileOffset<<" "<<addrs[z].blockOffset<<endl;  //记下删除的地方
        // cout<<"Free"<<" "<<addrs[z].fileOffset<<" "<<addrs[z].blockOffset<<endl;
        int rowlength=0;
        rowlength=CatalogManager::rowLength(addrs[z].file);
        for(int i=0;i<block_num;i++){
            if(Buff.bufferpool[i].filename==addrs[z].file&&Buff.bufferpool[i].fileoffset==addrs[z].fileOffset){
                for(int j=0;j<rowlength;j++){
                    Buff.bufferpool[i].data[addrs[z].blockOffset+j]='?';//懒惰删除，全改成'?'
                }
                flag=1;
                Buff.bufferpool[i].isdirty=true;
            }
        }
        if(!flag){
            //Buffer里不存在
            block B= readFromFile(addrs[z].file,addrs[z].fileOffset);
            for(int j=0;j<rowlength;j++){
                B.data[addrs[z].blockOffset+j]='?';//懒惰删除，全改成'?'
            }
            B.isdirty = true;
            addBlock(B);
        }
    }
    ofile.close();
    return true;
}
bool BufferManager::write(const Address& addr, TableRow data) {
    // cout <<"BF Fileoffset is " << addr.fileOffset << " BF Blockoffset is " << addr.blockOffset << endl;
    for(int i=0;i<block_num;i++){
        if(Buff.bufferpool[i].filename==addr.file&&Buff.bufferpool[i].fileoffset==addr.fileOffset){
            memcpy(Buff.bufferpool[i].data+addr.blockOffset,data.data,data.datasize);
            //strcpy(Buff.bufferpool[i].data+addr.blockOffset,data);
            Buff.bufferpool[i].isdirty= true;//  写过了，变成dirty
            if(Buff.bufferpool[i].spaceused==addr.blockOffset){
                Buff.bufferpool[i].spaceused+=data.datasize;
            }

            return true;
        }
    }
    return false;
}
block BufferManager::readFromFile(const string& file,int fileoffset) {
    string name=file+ "_"+to_string(fileoffset);
    // cout << "FileName is " << name << endl;
    FILE *fp= fopen(name.c_str(),"rb");
    block B;
    B.filename=file;
    B.fileoffset=fileoffset;
    B.isdirty= false;
    char isend;
    fread(&isend,1,1,fp);
    B.isEnd= isend == 1;
    fread(B.data,8192,1,fp);
    fseek(fp,0,SEEK_END);
    B.spaceused= ftell(fp)-1;
    //B.spaceused= strlen(B.data);
    fclose(fp);
    return B;
}
bool BufferManager::writeToFile(const block &b, const string& file) {
    string name=file+"_"+ to_string(b.fileoffset);
    FILE *fp= fopen(name.c_str(),"wb");
    // if(fp) cout << "Writing open "<<name <<" success."<< endl;
    // else cout << "Writing open "<<name <<" fail."<< endl;
    if(!fp)return false;
    char isend=b.isEnd==1;
    fwrite(&isend,1,1,fp);
    fwrite(b.data,b.spaceused,1,fp);
    fclose(fp);
    return true;
}
bool BufferManager::addBlock(const block &b) {
        for(int i=0;i<block_num;i++){
            if(Buff.bufferpool[i].filename.empty()){//有空位置
                Buff.bufferpool[i]=b;
                return true;
            }
        }
    //先遍历找有没有空位,没有就LRU
        bool flag= LRUreplace(b);
    if(!flag)return false;
    return true;
}
bool BufferManager::LRUreplace(const block &b) {
        int min=Buff.cnt[0];
    int min_index=0;
    for(int i=1;i<block_num;i++){
        if(min>Buff.cnt[i]){
            min=Buff.cnt[i];
            min_index=i;
        }
    }
    if(Buff.bufferpool[min_index].isdirty){//要换的块是脏块，则要写回文件
       bool flag= writeToFile(Buff.bufferpool[min_index],Buff.bufferpool[min_index].filename);
        if(!flag)return false;
    }
    Buff.bufferpool[min_index]=b;//替换
    Buff.cnt[min_index]=1;
    return true;
}
bool BufferManager::writrToFileAll() {
    for(int i=0;i<block_num;i++){
        if(Buff.bufferpool[i].isdirty){
            if(!writeToFile(Buff.bufferpool[i],Buff.bufferpool[i].filename))return false;
        }
    }
    return true;
}

bool BufferManager::dropTable(string tableName)
{
    for(int i=0;i<block_num;i++){
        if(Buff.bufferpool[i].filename==tableName){
            Buff.bufferpool[i].filename="";
            Buff.bufferpool[i].isdirty=false;
            Buff.bufferpool[i].spaceused=0;
            Buff.bufferpool[i].fileoffset=0;
            Buff.bufferpool[i].isEnd= true;
        }
    }
    string name=tableName+"_deladdr";
    FILE *fp= fopen(name.c_str(),"r");
    if(fp){//如果有这文件就删除
        fclose(fp);
        remove(name.c_str());
    }
    return true;
}
