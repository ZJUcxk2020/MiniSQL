#pragma once
#include <bits/stdc++.h>
using namespace std;
// �ؼ�ֵ���������� 
struct node {
    string value; 			// �� 
    int Type;				// ��ֵ���ͣ�int: -1, float: 0, char(n): n�� 
    int offset, file;		// �洢��Ϣ 
    // ���캯�� 
    node(string v = "", int T = 0, int o = 0) {
    	value = v, Type = T, offset = o;
	}
	// ����ȫ���ϵ 
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
// B+���ڵ� 
struct BPlusTreeNode {
	vector<node> keys;				// �ؼ�ֵ 
	vector<bool> flag;				// �ؼ�ֵ�ı�� 
	vector<BPlusTreeNode*> sons;	// ���� 
	BPlusTreeNode *pre, *nxt;		// ǰһ���ڵ㡢��һ���ڵ� 
	int isLeaf;						// �Ƿ���Ҷ�ӽڵ� 
	BPlusTreeNode();				// ���캯�� 
};
// B+�� 
class BPlusTree {
public:
	const int M = 4;				// B+������ 
	const int L = M / 2;			// ���ѵ� 
	BPlusTreeNode *root;			// B+�����ڵ� 
	BPlusTree();					// ���캯�� 
	void Free(BPlusTreeNode *x);	// �ͷſռ� 
	
	BPlusTreeNode* add(BPlusTreeNode *x, node data);				// ���룬�����Ǹ����� 
	BPlusTreeNode* add2(BPlusTreeNode *x, node data);				// ���룬���Ǹ����� 
	pair<BPlusTreeNode*, BPlusTreeNode*> split(BPlusTreeNode *x);	// ���� 
	
	void deleteT(BPlusTreeNode *x, node data);			// ɾ��ֵΪ data ������ key 
	vector<node> showLeaves(BPlusTreeNode *x);			// ������йؼ�ֵ 
	
	BPlusTreeNode* search(string searchKey, int Type);	// ��������ֵΪ data ��Ҷ�ӽڵ� 
	BPlusTreeNode* Query(BPlusTreeNode *x, node data);	// ��������ֵΪ data ��Ҷ�ӽڵ㣬���ڵݹ���ձ���ʽ 
};
