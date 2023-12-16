#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include<gl/glut.h>
#include"polygon.h"
#include <string>
#include <fstream>

static const int screenwidth = 800;  //自定义窗口宽度
static const int screenheight = 600; //自定义窗口高度
vector<point> p; //存储多边形点集
vector<polygon> s; //存储多边形
int move_x, move_y; //鼠标当前坐标
bool select = false; //当为true时即按下鼠标右键，将多边形封闭
bool Transform = false;
int angle = 0;
int direction = -1;
bool readtxt = false;
bool print_AET = false;
int polygon_color = 1;
int paint_interval = 0;
bool interval_flag = false;

Edge* ET[screenheight];
Edge* AET;

void init()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, screenwidth, 0.0, screenheight);
}

void sortAET(Edge* q)
{
	while (q->next)
	{
		Edge* p = q;
		while (p->next && p->next->next)
		{
			if (p->next->x > p->next->next->x)
			{
				int nowymax;
				float nowx;
				float nowdx;
				nowymax = p->next->ymax;
				nowx = p->next->x;
				nowdx = p->next->dx;

				p->next->ymax = p->next->next->ymax;
				p->next->x = p->next->next->x;
				p->next->dx = p->next->next->dx;

				p->next->next->ymax = nowymax;
				p->next->next->x = nowx;
				p->next->next->dx = nowdx;
			}
			else
			{
				p = p->next;
			}
		}
		q = q->next;
	}
}

// 多边形填充算法
void paint(vector<point> nowPoints, int color)
{

	//计算最高点的y坐标
	int max_Y = 0;
	for (int i = 0; i < nowPoints.size(); i++)
	{
		if (nowPoints[i].y > max_Y)
		{
			max_Y = nowPoints[i].y;
		}
	}
	// 初始化 ET
	Edge* ET[screenheight];
	for (int i = 0; i < max_Y; i++)
	{
		ET[i] = new Edge();
		ET[i]->next = nullptr;
	}
	// 初始化 AET
	AET = new Edge();
	AET->next = nullptr;

	glBegin(GL_POINTS); 

	// 建立边表ET
	for (int i = 0; i < nowPoints.size(); i++)
	{
		// 当前边的前一条边的起点
		int x0 = nowPoints[(i - 1 + nowPoints.size()) % nowPoints.size()].x;
		int y0 = nowPoints[(i - 1 + nowPoints.size()) % nowPoints.size()].y;
		// 当前边起点
		int x1 = nowPoints[i].x;
		int y1 = nowPoints[i].y;
		// 当前边终点
		int x2 = nowPoints[(i + 1) % nowPoints.size()].x;
		int y2 = nowPoints[(i + 1) % nowPoints.size()].y;
		// 当前边是水平线，不填充
		if (y1 == y2)
			continue;

		//分别计算一条边的当前y、上端点y、下端点x和斜率倒数
		int ymax;
		int nowy;//y值小的那一个
		if (y1 < y2)
		{
			nowy = y1;
			ymax = y2;
		}
		else
		{
			nowy = y2;
			ymax = y1;
		}
		float x = y1 < y2 ? x1 : x2;//低的x
		float dx = (x1 - x2) * 1.0f / (y1 - y2);


		//奇点处理
		if (((y0 < y1) && (y1 < y2)) || ((y0 > y1) && (y1 > y2)))
		{
			nowy++;
			x += dx;
		}

		// 将该边插入边表ET
		Edge* p = new Edge();
		p->ymax = ymax;
		p->x = x;
		p->dx = dx;
		p->next = ET[nowy]->next;
		ET[nowy]->next = p;
	}

	//扫描线从下往上扫描，每轮扫描 y 加 1
	for (int i = 0; i < max_Y; i++)
	{
		paint_interval++;
		//取出ET中当前扫描行的所有边并按x的递增顺序（若x相等则按dx的递增顺序）插入AET
		while (ET[i]->next)
		{
			//取出ET中当前扫描行表头位置的边
			Edge* pInsert = ET[i]->next;
			Edge* p = AET;
			//在AET中搜索合适的插入位置
			while (p->next)
			{
				if (pInsert->x > p->next->x)
				{
					p = p->next;
					continue;
				}
				if (pInsert->x == p->next->x && pInsert->dx > p->next->dx)
				{
					p = p->next;
					continue;
				}
				//找到位置
				break;
			}
			//将pInsert从ET中删除，并插入AET的当前位置
			ET[i]->next = pInsert->next;
			pInsert->next = p->next;
			p->next = pInsert;
		}

		// AET中的边两两配对并填色
		Edge* p = AET;
		while (p->next && p->next->next)
		{
			for (int x = p->next->x; x < p->next->next->x; x++)
			{
				switch (color)
				{
				case 1:
					glColor3f(1.0, 0.0, 0.0);
					break;
				case 2:
					glColor3f(0.0, 1.0, 0.0);
					break;
				case 3:
					glColor3f(0.0, 0.0, 1.0);
					break;
				}
				if (interval_flag)
				{
					if (paint_interval % 8 == 1 || paint_interval % 8 == 2 || paint_interval % 8 == 3 || paint_interval % 8 == 4)
						glVertex2i(x, i);
				}
				else
					glVertex2i(x, i);
			}
			// 打印活性边表信息
			if (print_AET)
			{
				cout << p->next->dx << " " << p->next->x << " " << p->next->ymax << endl;
				cout << p->next->next->dx << " " << p->next->next->x << " " << p->next->next->ymax << endl;
			}
			p = p->next->next; // 跳到间隔一个的交点
		}

		//删除AET中满足y=ymax的边
		p = AET;
		while (p->next)
		{
			if (p->next->ymax == i)
			{
				Edge* pDelete = p->next;
				p->next = pDelete->next;
				pDelete->next = nullptr;
				delete pDelete;
			}
			else
			{
				p = p->next;
			}
		}

		// 更新AET中边的x值
		p = AET;
		while (p->next)
		{
			p->next->x = p->next->x + p->next->dx;
			p = p->next;
		}

		// 特殊处理自相交。重新排序AET中的边，按从小到大
		Edge* q = AET;
		sortAET(q);
	}

	glEnd();
}

void my_readtxt()
{
	int pcnt = 0;
	glutPostRedisplay(); //重绘窗口
	FILE* fp;//文件指针
	char filename[100] = "C:\\Users\\86152\\Desktop\\myPolygon.txt";
	if ((fopen(filename, "r")) == NULL)
	{   //二进制只读打开文件
		printf("fail to open file\n");
		exit(1);
	}
	fp = fopen(filename, "r");
	fscanf_s(fp, "%d", &(pcnt));
	for (int j = 0; j < pcnt; j++)
	{
		int lines;
		fscanf_s(fp, "%d", &(polygon_color));
		fscanf_s(fp, "%d", &(lines));
		for (int i = 0; i < lines; i++)
		{
			int xx, yy;
			fscanf_s(fp, "%d", &(xx));//输出数据到数组
			fscanf_s(fp, "%d", &(yy));
			point fv;
			fv.x = xx;
			fv.y = screenheight - yy;
			p.push_back(fv); //将点信息存入多边形点集向量p中
			glutPostRedisplay(); //重绘窗口
		}

		polygon sq;
		int i;
		//将封闭了的多边形保存到多边形类中
		for (i = 0; i < p.size(); i++)
			sq.p.push_back(p[i]);
		p.clear();
		sq.color = polygon_color;
		s.push_back(sq); //将绘成的多边形存入多边形类向量中
		paint(sq.p, sq.color); //给当前画完的多边形填色
		
	}

	fclose(fp);/*关闭文件*/
	//s.clear();
	readtxt = !readtxt;
}

void lineSegment()
{

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);   //设定颜色

	int i, j;                 //两个循环控制变量，在下面的向量循环和数组循环中将被多次调用。
	if (!s.empty())            //看多边形类向量是否为空，即判断除了当前正在画的多边形是否还有曾经已经画好的多边形
	{
		for (i = 0; i < s.size(); i++)   //对多边形类向量循环，该向量中的每个元素代表一个多边形
		{
			int k = s[i].p.size();  //将一个多边形的点的个数，后面划线会用到
//			s[i].line(); //生成多边形的边
			for (j = 0; j < s[i].p.size(); j++) //画多边形
			{
				glBegin(GL_LINES); //将当前的点与后一个点连线
				glVertex2i(s[i].p[j].x, s[i].p[j].y);	//确定前一个点
				glVertex2i(s[i].p[(j + 1) % k].x, s[i].p[(j + 1) % k].y);//确定后一个点，通过取模操作来避免越界问题（循环队列）
				glEnd();
			}
			if (Transform)
			{
				// 旋转
				glRotatef(angle, 0, 0, direction);

				Transform = !Transform;
			}

			paint(s[i].p, s[i].color);  //为当前的多边形填充颜色
			glColor3f(0.0, 0.0, 0.0);
		}
	}
	i = 0;
	j = p.size() - 1;
	while (i < j)           //循环画图，画当前正在画的多边形
	{
		glBegin(GL_LINES); //将已经确定的点连接起来
		glVertex2i(p[i].x, p[i].y);
		glVertex2i(p[i + 1].x, p[i + 1].y);
		glEnd();
		i++;
	}
	if (!p.empty())//画当前点到鼠标点的线段（变化）
	{
		//	int i = p.size() - 1; //将确定的最后一个点与当前鼠标所在位置连线，即动态画线
		glBegin(GL_LINES);
		glVertex2i(p[j].x, p[j].y);
		glVertex2i(move_x, move_y);
		glEnd();
	}

	// 读多边形文件
	if (readtxt)
		my_readtxt();

	// 保存多边形
	if (select) //判断右键是否被点下
	{
		select = false; //将状态值置为假
		if (!p.empty())
		{
			glBegin(GL_LINES); //自动将当前点和第一点连接起来封闭多边形
			glVertex2i(p[j].x, p[j].y);
			glVertex2i(p[0].x, p[0].y);
			glEnd();
			polygon sq;
			//将封闭了的多边形保存到多边形类中
			for (i = 0; i < p.size(); i++)
				sq.p.push_back(p[i]);
			sq.color = polygon_color;
			s.push_back(sq); //将绘成的多边形存入多边形类向量中
			paint(sq.p, sq.color); //给当前画完的多边形填色
		}
		p.clear();
	}

	glFlush();
}

void myMouse(int button, int state, int x, int y) //鼠标点击事件响应函数
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)//当鼠标左键被点击
	{
		point v;  //申请一个点类变量，点类为自定义类，在zl.h中定义
		v.x = x; //将点击处的点坐标，即x和y的值存入v中
		v.y = screenheight - y;
		cout << "该点坐标  x:" << v.x << ",  y:" << v.y << '\n';
		p.push_back(v); //将点信息存入多边形点集向量p中
		glutPostRedisplay(); //重绘窗口
	}

	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) //当鼠标右键被点击
	{
		select = true;
		print_AET = true;
		Transform = true;
		glutPostRedisplay();
	}
}

void mykeyboard(unsigned char key, int x, int y)//键盘交互函数
{
	switch (key)
	{
	case'1':   //返回上一步
		p.pop_back();
		break;
	case'2':   //删除正在绘制的多边形
		p.clear();
		break;
	}
}

void myPassiveMotion(int x, int y) //鼠标移动事件响应函数
{
	move_x = x; //保存当前鼠标所在的坐标的值
	move_y = screenheight - y;
	print_AET = false;
	glutPostRedisplay();
}

void mymenuOption(GLint option)
{
	switch (option)
	{
	case 1:// 删除上一个已经绘制好的多边形
		s.pop_back();
		break;
	case 2:// 清空画布
		p.clear();
		s.clear();
		glutPostRedisplay();
		break;
	case 3://颜色变换
		polygon_color = (polygon_color + 1) % 4;
		break;
	case 4://旋转开关
		if (angle == 0)
			angle = 10;
		else
			angle = 0;
		break;
	case 5:// 旋转方向
		if (direction == 1)
			direction = -1;
		else if (direction == -1)
			direction = 1;
		break;
	case 6:
		interval_flag = !interval_flag;
		break;
	case 7:
		readtxt = !readtxt;
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(50, 100);
	glutInitWindowSize(screenwidth, screenheight);
	glutCreateWindow("多边形扫描填充算法");
	init();
	glutMouseFunc(myMouse); //鼠标点击消息监控，即监控鼠标是否被点击，若被点击就调用myMouse函数
	glutDisplayFunc(lineSegment);

	glutCreateMenu(mymenuOption);
	glutAddMenuEntry("delete previous polygon", 1);
	glutAddMenuEntry("Clear screen", 2);
	glutAddMenuEntry("Color transformation", 3);
	glutAddMenuEntry("Rotate switch", 4);
	glutAddMenuEntry("Rotation direction transformation", 5);
	glutAddMenuEntry("Interval scanning", 6);
	glutAddMenuEntry("Read polygon-file", 7);

	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glutKeyboardFunc(mykeyboard);
	glutPassiveMotionFunc(myPassiveMotion); //鼠标移动消息监控，即监控鼠标是否移动，若移动就调用myPassiveMotion函数
	glutMainLoop();

	return 0;
}