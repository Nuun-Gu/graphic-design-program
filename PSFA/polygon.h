#ifndef DATA_H_
#define DATA_H_
#include<vector>
using namespace std;
class point //���࣬�洢һ�����x yֵ
{
public:
	int x;
	int y;
};

class polygon			//������࣬��һ�������
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