#pragma once
#include <bits/stdc++.h>
using namespace std;
struct node {
    string value; 
    int Type;
    int offset;
	int file;
    friend bool operator < (node A, node B) {
    	assert(A.Type == B.Type);
    	return	A.Type == -1 ?
					atoi(A.value.c_str()) < atoi(B.value.c_str())
				: A.Type == 0 ?
					atof(A.value.c_str()) < atof(B.value.c_str())
				:	A.value < B.value;
	}
	friend bool operator == (node A, node B) {
		assert(A.Type == B.Type);
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
		return A <= B;
	}
};
