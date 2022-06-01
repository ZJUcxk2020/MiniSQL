#include "BPlusTree.h"
// B+树节点的构造函数 
BPlusTreeNode::BPlusTreeNode() {
	pre = nxt = NULL;
	isLeaf = 0;
}
// B+树的构造函数 
BPlusTree::BPlusTree() {
	root = new BPlusTreeNode();
	root->isLeaf = 2;
}
// 释放空间 
void BPlusTree::Free(BPlusTreeNode *x) {
	if (x == NULL)
		return;
	// 递归释放所有叶子 
	for (auto it : x->sons)
		Free(it);
	// 释放当前点 
	vector<node>().swap(x->keys);
	vector<bool>().swap(x->flag);
	vector<BPlusTreeNode*>().swap(x->sons);
	free(x);
}

// 节点分裂 
pair<BPlusTreeNode*, BPlusTreeNode*> BPlusTree::split(BPlusTreeNode *y) {
	assert((int)y->keys.size() == M);
	// 分裂左节点 
	BPlusTreeNode *left = new BPlusTreeNode();
	for (int j = 0; j < L; j ++) {
		left->keys.push_back(y->keys[j]), 
		left->flag.push_back(y->flag[j]); 
		if (y->isLeaf == 0)
			left->sons.push_back(y->sons[j]);
	}
	if (y->isLeaf == 0)
		left->sons.push_back(y->sons[L]);
	left->isLeaf = y->isLeaf;
	// 分裂右节点 
	BPlusTreeNode *right = new BPlusTreeNode();
	if (y->isLeaf == 0)
		right->sons.push_back(NULL);
	for (int j = L; j < M; j ++) {
		right->keys.push_back(y->keys[j]), 
		right->flag.push_back(y->flag[j]);
		if (y->isLeaf == 0)
			right->sons.push_back(y->sons[j + 1]);
	}
	right->isLeaf = y->isLeaf;
	// 左右连边 
	if (y->pre != NULL)
		y->pre->nxt = left;
	left->pre = y->pre;
	left->nxt = right;
	right->pre = left; 
	right->nxt = y->nxt;
	if (y->nxt != NULL)
		y->nxt->pre = right; 
	//left->pre = y->pre, left->nxt = right, right->pre = left, right->nxt = y->nxt;
	
	return make_pair(left, right);
}
// 插入，不考虑根分裂，递归 
BPlusTreeNode* BPlusTree::add2(BPlusTreeNode *x, node data) {
	// 找对应的值区间 
	int i = 0;
	while (i < (int)x->keys.size()) {
		if (x->keys[i] <= data)
			i ++;
		else
			break;
	}
	// 对非叶子节点，直接插入，再分裂 
	if (x->isLeaf == 0) {
		x->sons[i] = add2(x->sons[i], data);
		// 分裂，维护 keys, flag, sons 信息 
		if ((int)x->sons[i]->keys.size() == M) {
			BPlusTreeNode *y = x->sons[i];
			auto it = split(y);
			BPlusTreeNode *left = it.first, *right = it.second;
			x->keys.push_back(node()), 
			x->flag.push_back(true);
			for (int j = x->keys.size() - 1; j > i; j --)
				x->keys[j] = x->keys[j - 1], 
				x->flag[j] = x->flag[j - 1];
			x->keys[i] = right->keys[0], 
			x->flag[i] = 0;	
			x->sons.push_back(right);
			for (int j = x->sons.size() - 1; j > i + 1; j --)
				x->sons[j] = x->sons[j - 1];
			x->sons[i] = left, x->sons[i + 1] = right;
		}
	}
	// 对叶子节点，直接插入 
	else if (x->isLeaf == 1) {
		x->keys.push_back(node()), x->flag.push_back(0);
		for (int j = x->keys.size() - 1; j > i; j --)
			x->keys[j] = x->keys[j - 1], x->flag[j] = x->flag[j - 1];
		x->keys[i] = data, x->flag[i] = 0;
	}
	// 对空树的特殊处理 
	else if (x->isLeaf == 2) {
		x->isLeaf = 1;
		x->keys.push_back(data), 
		x->flag.push_back(0);
	}
	return x;
}
// 先插入，再考虑根节点分裂 
BPlusTreeNode* BPlusTree::add(BPlusTreeNode *x, node data) {
	x = add2(x, data);
	if ((int)x->keys.size() == M) {
		auto it = split(x);
		BPlusTreeNode *left = it.first, *right = it.second;
		x = new BPlusTreeNode();
		x->keys.push_back(right->keys[0]), x->flag.push_back(0);
		x->sons.push_back(left), x->sons.push_back(right);
	}
	return x;
}

// 删除 
void BPlusTree::deleteT(BPlusTreeNode *x, node data) {
	if (x == NULL)
		return;
	// 对非叶子节点，递归遍历所有满足条件的区间 
	if (x->isLeaf == 0) {
		if (data <= x->keys[0])
			deleteT(x->sons[0], data);
		for (int i = 1; i < (int)x->keys.size(); i ++)
			if (x->keys[i - 1] <= data && data <= x->keys[i])
				deleteT(x->sons[i], data);
		if (x->keys[x->keys.size() - 1] <= data)
			deleteT(x->sons[x->sons.size() - 1], data);
	}
	// 对叶子节点，逐个关键值遍历 
	else if (x->isLeaf == 1) {
		for (int i = 0; i < (int)x->keys.size(); i ++)
			if (x->keys[i] == data)
				x->flag[i] = 1;
	}
}

// 得到所有关键值 
vector<node> BPlusTree::showLeaves(BPlusTreeNode *x) {
	// 找到第一个叶子 
	while (x->isLeaf == 0) {
		int i = 0;
		while (x->sons[i] == NULL)
			i ++;
		x = x->sons[i];
	}
	// 叶子沿着右指针跳，没跳到一个点提取处所有的关键值 
	vector<node> it;
	while (x) {
	    for (int i = 0; i < (int)x->keys.size(); i ++)
			if (x->flag[i] == 0) 
	        	it.push_back(x->keys[i]);
        x = x->nxt;
    }
	return it;
}

// 查询，类似删除 
BPlusTreeNode* BPlusTree::Query(BPlusTreeNode *x, node data) {
	if (x == NULL)
		return NULL;
	if (x->isLeaf == 0) {
		if (data <= x->keys[0]) {
			auto it = Query(x->sons[0], data);
			if (it)
				return it;
		}
		for (int i = 1; i < (int)x->keys.size(); i ++)
			if (x->keys[i - 1] <= data && data <= x->keys[i]) {
				auto it = Query(x->sons[i], data);
				if (it)
					return it;
			}
		if (x->keys[x->keys.size() - 1] <= data) {
			auto it = Query(x->sons[x->sons.size() - 1], data);
			if (it)
				return it;
		}
	}
	else if (x->isLeaf == 1) { 
		for (int i = 0; i < (int)x->keys.size(); i ++)
			if (x->keys[i] == data && x->flag[i] == 0)
				return x;
	}
	return 0;
}
// 查询的传入格式 
BPlusTreeNode* BPlusTree::search(string searchKey, int Type) {
	node data = (node){searchKey, Type, 0};
	return Query(root, data);
}

// 代码调试 
void debug(BPlusTreeNode *x, int dep = 0, int single = 0) {
	if (x == NULL) {
		puts("NULL");
		return;
	}
	for (node it : x->keys)
		cout << it.value << " ";
	puts("");
	if (!single && x->isLeaf == 0)
		for (auto it : x->sons)
			debug(it, dep + 1);
	if (dep == 0)
		puts("");
}
void debug2(BPlusTree &T) {
	auto it = T.showLeaves(T.root);
	for (node x : it)
		cout << x.value << " ";
	puts("");
}
//int main() {
//	BPlusTree T = BPlusTree();
//	T.root = T.add(T.root, node("00", 10, 0));
//	T.root = T.add(T.root, node("01", 10, 0));
//	T.root = T.add(T.root, node("10", 10, 0));
//	T.root = T.add(T.root, node("11", 10, 0));
//	T.root = T.add(T.root, node("12", 10, 0));
//	T.root = T.add(T.root, node("13", 10, 0));
//	T.root = T.add(T.root, node("14", 10, 0));
//	T.root = T.add(T.root, node("15", 10, 0));
//	T.root = T.add(T.root, node("16", 10, 0));
//	T.root = T.add(T.root, node("17", 10, 0));
//	T.root = T.add(T.root, node("02", 10, 0));
//	T.root = T.add(T.root, node("03", 10, 0));
//	T.root = T.add(T.root, node("04", 10, 0));
//	debug2(T);
//	T.root = T.add(T.root, node("07", 10, 0));
//	debug(T.root);
//	debug2(T);
//	return 0;
//}
