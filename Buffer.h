#pragma once
#include<vector>
#include "Block.h"
#define block_num 100
//buffer里总共有16个block
using namespace std;
typedef struct buffer Buffer;
struct buffer {
	block  bufferpool[block_num];
	int cnt[block_num]{};
};
