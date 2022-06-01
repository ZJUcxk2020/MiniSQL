#pragma once
#include <bits/stdc++.h>
using namespace std;
// 关键值的数据类型 
struct node {
    string value; 			// 数 
    int Type;				// 数值类型（int: -1, float: 0, char(n): n） 
    int offset, file;		// 存储信息 
    // 构造函数 
    node(string v = "", int T = 0, int o = 0) {
    	value = v, Type = T, offset = o;
	}
	// 定义全序关系 
    friend bool operator < (node A, node B) {
    	return	A.Type == -1 ?
					atoi(A.value.c_str()) < atoi(B.value.c_str())
				: A.Type == 0 ?
					atof(A.value.c_str()) < atof(B.value.c_str())
				:	A.value < B.value;
	}
	friend bool operator == (node A, node B) {
		return	A.Type == -1 ?
					atoi(A.value.c_str()) == atoi(B.value.c_str())
				: A.Type == 0 ?
					atof(A.value.c_str()) == atof(B.value.c_str())
				:	A.value == B.value;
	}
	friend bool operator > (node A, node B) {
		return	A.Type == -1 ?
					atoi(A.value.c_str()) > atoi(B.value.c_str())
				: A.Type == 0 ?
					atof(A.value.c_str()) > atof(B.value.c_str())
				:	A.value > B.value;
	}
	friend bool operator <= (node A, node B) {
		return A < B || A == B;
	}
};
// B+树节点 
struct BPlusTreeNode {
	vector<node> keys;				// 关键值 
	vector<bool> flag;				// 关键值的标记 
	vector<BPlusTreeNode*> sons;	// 儿子 
	BPlusTreeNode *pre, *nxt;		// 前一个节点、下一个节点 
	int isLeaf;						// 是否是叶子节点 
	BPlusTreeNode();				// 构造函数 
};
// B+树 
class BPlusTree {
public:
	const int M = 4;				// B+树阶数 
	const int L = M / 2;			// 分裂点 
	BPlusTreeNode *root;			// B+树根节点 
	BPlusTree();					// 构造函数 
	void Free(BPlusTreeNode *x);	// 释放空间 
	
	BPlusTreeNode* add(BPlusTreeNode *x, node data);				// 插入，不考虑根分裂 
	BPlusTreeNode* add2(BPlusTreeNode *x, node data);				// 插入，考虑根分裂 
	pair<BPlusTreeNode*, BPlusTreeNode*> split(BPlusTreeNode *x);	// 分裂 
	
	void deleteT(BPlusTreeNode *x, node data);			// 删除值为 data 的所有 key 
	vector<node> showLeaves(BPlusTreeNode *x);			// 获得所有关键值 
	
	BPlusTreeNode* search(string searchKey, int Type);	// 搜索存在值为 data 的叶子节点 
	BPlusTreeNode* Query(BPlusTreeNode *x, node data);	// 搜索存在值为 data 的叶子节点，用于递归的普遍形式 
};
