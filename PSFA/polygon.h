#ifndef DATA_H_
#define DATA_H_
#include<vector>
using namespace std;
class point //点类，存储一个点的x y值
{
public:
	int x;
	int y;
};

class polygon			//多边形类，存一个多边形
{
public:
	vector<point> p; 
	int color;
};
class Edge
{
public:
	int ymax;
	float x;
	float dx;
	Edge* next;
};
#endif