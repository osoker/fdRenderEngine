#include "stdafx.h"
#include "FindPathMap.h"
#include "../common/include/Sgdebug.h"

#include "gc_gameScene.h"
#include <math.h>
# include "time.h"

#pragma warning( disable:4244 )

//!!Ҫ���ͼ��Ե��Χ1���ڱ�������,�����������
#define FINDPATH_MINSTEP	2
#define FINDPATH_MAXSTEP	250


//! ������ṩ������������ײ��Ϣ��չ�ļ���. server collsion data
# define SERVER_COLLNAME  "map"


#define GETBAR(x,y)	m_pMapBuf[y*m_MapW+x]   //���ص�ͼ��ĳ���ֵ
#define INMAP(x,y)  (x>=0) && (x<m_MapW) &&(y>=0) &&(y<m_MapH)
//#define CHKBAR(x,y) m_pMapBuf[y*m_MapW+x]!=MAPBOO
#define SETTMP(x,y,bar)	m_maptmp[y*m_MapW+x]=bar   //���ص�ͼ��ĳ���ֵ
#define	GETDLEN(x1,y1,x2,y2) (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)


//! ��ʹ��d3dx���VectorLength
float fpm_vec2Length( const osVec2D* _v1 )
{
	return sqrt( (_v1->x*_v1->x) + (_v1->y*_v1->y) );
}



//! ��ʹ��d3dx���normalize.
void fpm_vec2Normalize( osVec2D* _res,osVec2D* _v )
{
	float t_fLength = fpm_vec2Length( _v );
	_res->x = _v->x/t_fLength;
	_res->y = _v->y/t_fLength;
}



TPoint Cross8[8]=
{
	{ 0, -1},
	{ 1, -1},
	{ 1,  0},
	{ 1,  1},
	{ 0,  1},
	{-1,  1},
	{-1,  0},
	{-1, -1}
};

//һ�����ӵ����ĵ㼰��������
TPoint  CrossBox[5]=
{
	{BOXPOINT/2,BOXPOINT/2},
	{0,0},
	{BOXPOINT-1,0},
	{0,BOXPOINT-1},
	{BOXPOINT-1,BOXPOINT-1},
};

int MyRound(double d)
{
	//guard;
	if (d == 0) return 0;
	else if (d > 0) return int(d + 0.5);
	else if (d < 0) return int(d - 0.5);
	else return 0;
	//unguard;
}

//bool CFindPathMap::FindPathMap(char * _pMapBuf, int _nMapW, int _nMapH, int _nX1, int _nY1, int _nX2, int _nY2, int &_nX, int &_nY)
bool CFindPathMap::FindPathMap( int _nX1, int _nY1, int _nX2, int _nY2, int &_nX, int &_nY)
{
	//guard;
	//�����ݴ�
	//m_pMapBuf=_pMapBuf;
	//m_MapW=_nMapW;
	//m_MapH=_nMapH;
	if( m_pMapBuf == 0 )
		return false;

	m_dlen=MAX_STEPLEN;
	//
	m_x1=_nX1;
	m_y1=_nY1;
	m_x2=_nX2;
	m_y2=_nY2;

	osDebugOut( "The src pt is<%d,%d>..dst pt is:<%d,%d>..\n",
		_nX1,_nY1,_nX2,_nY2 );

	//!�ж���㣦�յ��Ƿ����ϰ���
	// River @ 2009-7-10: ��CHKBAR���CanMove,��С��
	//if (CHKBAR(_nX1,_nY1) || CHKBAR(_nX2,_nY2)) 
	// TEST CODE:
	bool t_b1 = CanMove( _nX1,_nY1);
	bool t_b2 = CanMove( _nX2,_nY2);
	if( t_b1 )
		osDebugOut( "Src pt can pass...\n" );
	else
		osDebugOut( "Src pt can not pass...\n" );
	if( t_b2 )
		osDebugOut( "Dst pt can pass...\n" );
	else
		osDebugOut( "Dst pt can not pass...\n" );

	if( (!CanMove( _nX1,_nY1))||(!CanMove( _nX2,_nY2) ) )
		return false;


	_nX1=_nX1/BOXPOINT;
	_nY1=_nY1/BOXPOINT;
	_nX2=_nX2/BOXPOINT;
	_nY2=_nY2/BOXPOINT;
	m_bx1=_nX1;
	m_by1=_nY1;
	m_bx2=_nX2;
	m_by2=_nY2;

	m_NextPoint.x=0;
	m_NextPoint.y=0;

	
	
	//�ж��Ƿ�Խ��
	if (!INMAP(_nX1,_nY1) || !INMAP(_nX2,_nY2)) {
		//!!����Խ��
		osDebugOut( "����Խ��...\n" );
		return false;
	}

	if ((_nX1==_nX2)&&(_nY1==_nY2) ){
		m_FirstBox.x=m_bx2;
		m_FirstBox.y=m_by2;
		m_NextPoint.x=m_x2;
		m_NextPoint.y=m_y2;
		_nX=m_NextPoint.x;
		_nY=m_NextPoint.y;

		osassert(CanMove(_nX,_nY));
		return true;
	}
	

	memset(&m_maptmp[0],0,m_MapW*m_MapH);
	//��cline1������չ��ֱ���ҵ�Ŀ���
	m_Cline1Count=1;
	m_Cline1[0].x=_nX1;
	m_Cline1[0].y=_nY1;
	int step=FINDPATH_MINSTEP;//��2���� ��25
	m_maptmp[_nY1*m_MapW+_nX1]=step;
	int x,y;
	step++;
	while (true) {
		m_Cline2Count=0;
		for (int i=0;i<m_Cline1Count;i++) 
		{
			// 
			// �ڰ˸������ϳ���
			for (int j=0;j<8;j++) 
			{
				x=m_Cline1[i].x+Cross8[j].x;
				y=m_Cline1[i].y+Cross8[j].y;

				if ((!CanMoveGrid(x,y)) || (m_maptmp[y*m_MapW+x]>=FINDPATH_MINSTEP)) 
					continue;

				m_Cline2[m_Cline2Count].x=x;
				m_Cline2[m_Cline2Count].y=y;
				m_Cline2Count++;
				m_maptmp[y*m_MapW+x]=step;
				if ((x==_nX2) && (y==_nY2)) 
				{
					//�����յ�
					//������յ�
					if (!GetPathList())
					{
						osDebugOut( "�õ�path list ʧ��...\n" );
						//!!������
						GetPathList();
						osassert(false);
						return false;
					}

					GetFirstPoint();
					if( !GetStepPointEx(m_x1,m_y1) )
					{
						osDebugOut( "�õ���һ������ʧ��...\n" );
						return false;
					}

					_nX=m_NextPoint.x;
					_nY=m_NextPoint.y;

					osDebugOut("srcpt is:<%d,%d>..._nX <%d> _nY<%d>\n",
						m_x1,m_y1,_nX,_nY);
					osassert(CanMove(_nX,_nY));
					osassert(m_NextPoint.x!=0);
					return true;
				}
				
			}
		}
		step++;
		if (step>FINDPATH_MAXSTEP) {
			step=FINDPATH_MINSTEP;
		}
		//
		if (m_Cline2Count==0) 
		{	
			osDebugOut( "û���ҵ��µ��������˳�\n" );
			return false;
		}
		m_Cline1Count=m_Cline2Count;
		memcpy(&m_Cline1[0],&m_Cline2[0],m_Cline1Count*sizeof(TPoint));
	}

	osDebugOut( "��֪ԭ����˳�...\n" );
	return false;
	//unguard;
}

bool CFindPathMap::GetPathList()
{
	int x0,y0,x,y,step;
	int mind,x2,y2,i;
	//��x2,y2���棬�ҵ�x1,y2
	//���ݾ��������ȥ��
	x0=m_bx2;
	y0=m_by2;
	step=m_maptmp[m_by2*m_MapW+m_bx2];

	m_pathcount=1;
	m_pathlist[0].x=m_bx2;
	m_pathlist[0].y=m_by2;
	while (true) {
		step--;
		if ((step<FINDPATH_MINSTEP)) {
			step=FINDPATH_MAXSTEP;
		}
		//ֱ������
		mind=-1;
		for (i=0;i<=7;i++)  {
			if (i % 2==1) continue;
			x=x0+Cross8[i].x;
			y=y0+Cross8[i].y;
			if ((m_maptmp[y*m_MapW+x]==step)) {
				mind=GETDLEN(x,y,x0,y0);
				x2=x;
				y2=y;
				break;
			}
		}
		if (mind==-1) {
			for (i=0;i<=7;i++)  {
				if (i % 2==0) continue;
				x=x0+Cross8[i].x;
				y=y0+Cross8[i].y;
				if ((m_maptmp[y*m_MapW+x]==step)) {
					mind=GETDLEN(x,y,x0,y0);
					x2=x;
					y2=y;
					break;
				}
			}
			if ((mind==-1)) {
				//!!�������������
				return false;
			}
		}



		m_pathlist[m_pathcount].x=x2;
		m_pathlist[m_pathcount].y=y2;
		m_pathcount++;
		if ((x2==m_bx1) && (y2==m_by1)) {
			return true;
		}
		x0=x2;
		y0=y2;
		if ((m_pathcount>=MAXCLINECOUNT)) {
			//!!�쳣! ·��̫������ѭ��
			return false;
		}
	}
	return false;
}

//{
//	//��x2,y2���棬�ҵ�x1,y2
//	//���ݾ��������ȥ��
//	int x0,y0,x,y;
//
//	x0=m_bx2;
//	y0=m_by2;
//	int step=m_maptmp[m_by2*m_MapW+m_bx2];
//
//	m_pathcount=1;
//	m_pathlist[0].x=m_bx2;
//	m_pathlist[0].y=m_by2;
//	int mind,d,x2,y2,fw;
//	while (true) {
//		step--;
//		if (step<FINDPATH_MINSTEP) {
//			step=FINDPATH_MAXSTEP;
//		}
//		//���ö��յ����Ϸ����������
//		fw=GetFwordNear(x0,y0,m_bx1,m_by1);
//		x=x0+Cross8[fw].x;
//		y=y0+Cross8[fw].y;
//		if (m_maptmp[y*m_MapW+x]==step) {
//			x2=x;
//			y2=y;
//		} else {
//			mind=-1;
//			for (int i=0;i<8;i++) {
//				x=x0+Cross8[i].x;
//				y=y0+Cross8[i].y;
//				if (m_maptmp[y*m_MapW+x]==step) {
//					d=(x-m_bx1)*(x-m_bx1)+(y-m_by1)*(y-m_by1);
//					if (d==0) {
//						mind=d;
//						x2=x;
//						y2=y;
//						break;
//					}
//					//Ѱ�Ҿ��������ĵ㣬������·��������ֱ��
//					if ((mind==-1) ||(d<mind)){
//						mind=d;
//						x2=x;
//						y2=y;
//					}
//				}
//			}
//			if (mind==-1) {
//				//!!�������������
//				return false;
//			}
//		}
//		m_pathlist[m_pathcount].x=x2;
//		m_pathlist[m_pathcount].y=y2;
//		m_pathcount++;
//		x0=x2;
//		y0=y2;
//		if ((x2==m_bx1) && (y2==m_by1)) {
//			return true;
//		}
//
//		if (m_pathcount>=MAXCLINECOUNT) {
//			//!!�쳣! ·��̫������ѭ��
//			return false;
//		}
//	}
//}

void CFindPathMap::GetFirstPoint()
{
	int x,y,i;
	//��m_pathlist�еĸ��㣬�ҵ���m_bX1,m_bY1����ֱ�ߵ���ĸ���
	m_FirstBox.x=m_bx1;
	m_FirstBox.y=m_by1;
	//
	//ɨ��ȫ����ֱ���ҵ���һ���ϰ���
	for (i=1;i<=m_pathcount-1;i++) {
		if (i==m_pathcount-1) {
			x=m_x2;
			y=m_y2;
		} else {
			x=m_pathlist[m_pathcount-1-i].x*BOXPOINT+BOXPOINT / 2;
			y=m_pathlist[m_pathcount-1-i].y*BOXPOINT+BOXPOINT / 2;
		}
		if ((!ChkBarLine(m_pMapBuf,m_MapH,m_MapW,m_x1,m_y1,x,y))) {
			m_FirstBox.x=x / BOXPOINT;
			m_FirstBox.y=y / BOXPOINT;
		}else
			break;
	}
}

bool CFindPathMap::GetStepPoint(int _x1,int _y1,int _x2,int _y2,int &_tx,int &_ty)
{
	int x,y,count;
	//[edit]
	x=_x1;
	y=_y1;
	count=0;
	while (true) {
		_tx=x;
		_ty=y;
		if ((x==_x2) && (y==_y2)) {
			return true;
		}
		// River @ 2009-7-10: ��CHKBAR���CanMove,��С��
		//if (CHKBAR(x / BOXPOINT,y / BOXPOINT)) 
		if( !CanMove( x,y ) )
		{
			//��һ�����ǿյ㣬�����
			//���ص�һ���ϰ���
			return false;
		}
		//CalcNextPoint(x,y,_x2,_y2,BOXPOINT-1,x,y);
		CalcNextPoint(_x1,_y1,_x2,_y2,BOXPOINT-1,x,y);
		count++;
		if (count>MAXCLINECOUNT) {
			//���ܳ�������ѭ��
			return false;
		}
	}
}


bool CFindPathMap::GetStepPointEx(int _x1,int _y1)
{
	/*
	�㷨������
	��֪��	_x1,_y1�Ǿ�ȷ����	_boxX,_boxY��Ŀ��������꣬������Ͽ��m_dlen�ľ�ȷ���꣬��_tx,_ty�ǿյ�
	��_tx,_ty���ϰ�ʱ����Ȼ�ڵ�ǰ����Χ����һ����_boxX,_boxY����Ŀո�������ո�ΪĿ����е�����
	*/
	// River @ 2009-7-10: �Ѵ˺�����CHKBAR���CanMove,��С��

	//������յ㣬ֱ��ȡ��ȷ����
	int _boxX=m_FirstBox.x;
	int _boxY=m_FirstBox.y;
	int x2,y2;
	if ((_boxX==m_x2/BOXPOINT) &&(_boxY==m_y2/BOXPOINT)){
		x2=m_x2;
		y2=m_y2;
	}
	else
	{
		//ȡ������ӵļ�������
		x2=_boxX*BOXPOINT+BOXPOINT/2;
		y2=_boxY*BOXPOINT+BOXPOINT/2;
	}
	//
	if( !GetStepPoint(_x1,_y1,x2,y2,m_NextPoint.x,m_NextPoint.y) )
		osDebugOut( "�õ�step Point ʧ��...\n" );
	//����Ƿ����ϰ�
	if( CanMove( m_NextPoint.x,m_NextPoint.y ) )
	{
		//��һ�����ǿյ㣬�����
		return true;
	}
	//���������һ����Ϊ�ϰ�������Ҫ�������൱��С��һ��
	//��������
	int mind=-1;
	int d,x1,y1,x,y,px,py;
	x1=m_NextPoint.x/BOXPOINT;
	y1=m_NextPoint.y/BOXPOINT;
	int i;
	for (i=0;i<8;i++) {
		x=x1+Cross8[i].x;
		y=y1+Cross8[i].y;
		if (!CHKBAR(x,y)) {
			d=GETDLEN(x,y,_boxX,_boxY);
			if ((mind==-1)||(d<mind)) {
				mind=d;
				px=x;
				py=y;
			}
		}
	}
	if (mind==-1) {
		osDebugOut( "GetStepPointEx(error1): start<%d,%d>, end<%d,%d>, cur<%d,%d>\n" , m_bx1, m_by1, m_bx2, m_by2, _x1, _y1 );
		//!!������
		osassert(false);
		return false;
	}
	
	for (i=0;i<5;i++) {
		x2=px*BOXPOINT+CrossBox[i].x;
		y2=py*BOXPOINT+CrossBox[i].y;
		GetStepPoint(_x1,_y1,x2,y2,m_NextPoint.x,m_NextPoint.y);

		//����Ƿ����ϰ�
		if( CanMove( m_NextPoint.x,m_NextPoint.y ) )
			return true;

	}
	//�Է��������ɣ�����������ӵ�

	//������ֱ���ո�����
	x1=_x1 / BOXPOINT;
	y1=_y1 / BOXPOINT;
	int step=m_maptmp[y1*m_MapW+x1]+1;
	if (step>FINDPATH_MAXSTEP) 
	{
		step=FINDPATH_MINSTEP;
	};
	//mind:=GETDLEN(x1,y1,_boxX,_boxY);
	bool t_isok=false;	
	for (i=0;i<=7;i++) {
		if (i % 2==1) continue;
		x=x1+Cross8[i].x;
		y=y1+Cross8[i].y;
		if ((!CHKBAR(x,y)) && (m_maptmp[y*m_MapW+x]==step)) {
			for (int j=0;j<=m_pathcount-1;j++) {
				if ((m_pathlist[j].x==x) && (m_pathlist[j].y==y)) {
					//���ȿ���ֱ�������ϵĵ�
					x1=x;
					y1=y;
					t_isok=true;
					break;
				}
			}
			if (t_isok) break;
		}
	}
	x1=x1*BOXPOINT;
	y1=y1*BOXPOINT;
	//
	for (int j=0;j<=4;j++) {
		for (int i=0;i<=4;i++) {
			x2=px*BOXPOINT+CrossBox[i].x;
			y2=py*BOXPOINT+CrossBox[i].y;
			GetStepPoint(x1+CrossBox[j].x,y1+CrossBox[j].y,x2,y2,m_NextPoint.x,m_NextPoint.y);
			//����Ƿ����ϰ�
			if( CanMove( m_NextPoint.x,m_NextPoint.y ) ) {
				//��һ�����ǿյ㣬�����
				m_NextPoint.x=x1+CrossBox[j].x;
				m_NextPoint.y=y1+CrossBox[j].y;

				return true;
			}
		}
	}
	//!!������
	osDebugOut( "GetStepPointEx(error2): start<%d,%d>, end<%d,%d>, cur<%d,%d>\n" , m_bx1, m_by1, m_bx2, m_by2, _x1, _y1 );
	//osassert(false);
	return false;
}


int CFindPathMap::GetLine(TPoint pline[], int x1, int y1, int x2, int y2)
{
	int dx,dy,ddx,ddy,px1,py1,stepy,stepx,x,y,count;
	double xx,yy;
	int result=0;
	dx=x2-x1;
	dy=y2-y1;
	if ((dy==0) && (dx==0)) {
		pline[0].x=x1;
		pline[0].y=y1;
		result=1;
		return result;
	}
	if (dx>=0) ddx=dx; else ddx=-dx;
	if (dy>=0) ddy=dy; else ddy=-dy;
	if ((ddy<=BOXPOINT) && (ddx<=BOXPOINT)) {
		pline[0].x=x1;
		pline[0].y=y1;
		pline[1].x=x2;
		pline[1].y=y2;
		result=2;
		return result;
	}
	//
	if (ddy>ddx) {
		count=1;
		pline[0].x=x1;
		pline[0].y=y1;
		py1=y1- y1 % BOXPOINT+BOXPOINT / 2; //ȡ��ǰ����
		if (dy>=0) {
			stepy=BOXPOINT;
		} else {
			stepy=-BOXPOINT;
		}
		y=py1;
		while ((y / BOXPOINT)!=(y2 / BOXPOINT)) {
			y=y+stepy;
			xx=y-y1;
			xx=x1+xx*(x2-x1)/(y2-y1);
			x=xx;
			count++;
			pline[count-1].x=x;
			pline[count-1].y=y;
			if (count>=MAXCLINECOUNT) break;
		}
		//inc(count);
		pline[count-1].x=x2;
		pline[count-1].y=y2;
		result=count;
	} else {
		count=1;
		pline[0].x=x1;
		pline[0].y=y1;
		px1=x1- x1 % BOXPOINT+BOXPOINT / 2; //ȡ��ǰ����
		if (dx>=0) {
			stepx=BOXPOINT;
		} else {
			stepx=-BOXPOINT;
		}
		x=px1;
		while ((x / BOXPOINT)!=(x2 / BOXPOINT)) {
			x=x+stepx;
			yy=x-x1;
			yy=y1+yy*(y2-y1)/(x2-x1);
			y=yy;
			count++;
			pline[count-1].x=x;
			pline[count-1].y=y;
			if (count>=MAXCLINECOUNT) break;
		}
		//inc(count);
		pline[count-1].x=x2;
		pline[count-1].y=y2;
		result=count;
	}
	return result;
}

bool CFindPathMap::ChkBarLine(char* _pMapBuf, int _nMapH, int _nMapW, int _nX1, int _nY1, int _nX2, int _nY2)
{
	m_LineCount=GetLine(m_LinePoint,_nX1,_nY1,_nX2,_nY2);
	for (int i=0;i<m_LineCount;i++) 
	{
		if (_pMapBuf[m_LinePoint[i].y/BOXPOINT*_nMapW+m_LinePoint[i].x/BOXPOINT]!=MAPBOO) {
			return true;
		}
	}
	return false;
}

int CFindPathMap::GetFwordNear(int x1,int y1,int x2,int y2)
{
	double dx,dy,kx,ky;
	dx=x2-x1;
	dy=y2-y1;
	if (dx==0)  {
		if (dy>=0)  return 4; else return 0;
	}
	if (dy==0)  {
		if (dx>=0)  return 2; else return 6;

	}
	kx=dx;
	kx=abs(kx/dy);
	ky=dy;
	ky=abs(ky/dx);
	if ((dx<0) && (dy<0))  {
		if (kx>=3)  return 6;
		else
		if (ky>=3)  return 0;
		else
		return 7;
	}
	if ((dx>0) && (dy<0))  {
		if (kx>=3)  return 2;
		else
		if (ky>=3)  return 0;
		else
		return 1;
	}
	if ((dx>0) && (dy>0))  {
		if (kx>=3)  return 2;
		else
		if (ky>=3)  return 4;
		else
		return 3;
	}
	if ((dx<0) && (dy>0))  {
		if (kx>=3)  return 6;
		else
		if (ky>=3)  return 4;
		else
		return 5;
	}
	osassert(false);
	return 8;
}

//��ֱ�ﷵ��false, ����ֱ�ﷵ��true
//bool CFindPathMap::GetNextPoint(char * _pMapBuf, int _nMapW, int _nMapH, int _nX1, int _nY1, int _nX2, int _nY2, int &_nX, int &_nY)
bool CFindPathMap::GetNextPoint( int _nX1, int _nY1, int _nX2, int _nY2, int &_nX, int &_nY)
{
	int bx1,by1,bx2,by2,dx,dy;
	int x0,y0;
	//10 �ж���Ŀ����Ƿ��ϰ�
	//20 �����ϰ���
	//     ����ˮƽ�����ֱ�������ҵ�ǽ�㣬ֹͣ��
	//     ����б���򣬿��ǲ໬����������������ϰ������ҵ��ǵ�ֹͣ��
	//     ��ˮƽ���ϰ��������ҵ���һ��ˮƽ���ĵ�(��nX2)������
	//     ����ֱ���ϰ������ҵ���һ����ֱ���ĵ�(��nY2)������
	//30 �����ϰ���
	//     ����ˮƽ�����ֱ�������ҵ���һ�����ĵ㣬����
	//     ����б�򣬿��ǿ粽�����ܷ�ֱ�ӵ�������ܣ����ҽǵ㣬����
	if ((_nX1==_nX2)&&(_nY1==_nY2)) {
		_nX=_nX2;
		_nY=_nY2;
		return false;
	}

	bx1=_nX1 / BOXPOINT;
	by1=_nY1 / BOXPOINT;
	bx2=_nX2 / BOXPOINT;
	by2=_nY2 / BOXPOINT;
	dx=bx2-bx1;
	dy=by2-by1;
	osassert((dx>=-1) && (dx<=1));
	osassert((dy>=-1) && (dy<=1));

	//��������������ĵ�
	x0=bx1*BOXPOINT+BOXPOINT / 2;
	y0=by1*BOXPOINT+BOXPOINT / 2;

	if (m_pMapBuf[by2*m_MapW+bx2]==MAP_NOT_MOVE) {
		//���ϰ���
		//    ����ˮƽ�����ֱ�������ҵ�ǽ�㣬ֹͣ��
		if ((dx==0) || (dy==0)) {
			_nX=x0+dx*(BOXPOINT / 2-BOXBORDER);
			_nY=y0+dy*(BOXPOINT / 2-BOXBORDER);
			return true;
		}
		//    ����б���򣬿��ǲ໬
		if (m_pMapBuf[by1*m_MapW+bx1+dx]==MAPBOO) {
			//��ˮƽ���ϰ��������ҵ���һ��ˮƽ���ĵ㣬����
			_nX=_nX2;
			_nY=_nY1;
			return true;
		}

		if (m_pMapBuf[(by1+dy)*m_MapW+bx1]==MAPBOO) {
			//����ֱ���ϰ������ҵ���һ����ֱ���ĵ㣬����
			_nX=_nX1;
			_nY=_nY2;
			return true;
		}
		//��������������ϰ������ҵ��ǵ�ֹͣ��
		_nX=x0+dx*(BOXPOINT / 2-BOXBORDER);
		_nY=y0+dy*(BOXPOINT / 2-BOXBORDER);
		return true;
	} else {
		//30 �����ϰ���
		//     ����ˮƽ�����ֱ�������ҵ���һ�����ĵ㣬����
		//     ����б�򣬿��ǿ粽�����ܷ�ֱ�ӵ�������ܣ����ҽǵ㣬����
		if ((dx==0) || (dy==0)) {
			_nX=_nX2;
			_nY=_nY2;
			return false;
		}
		//�����ܷ�ֱ�ӵ���   
		int xx,yy;

		//@{
		// River mod @ 2008-11-10:�˴�û�г�ʼ��xx,yy,����CalcNextPoint������ȴ��ֱ�ӵ������������
		xx = _nX1;
		yy = _nY1;
		//@}

		CalcNextPoint(_nX1,_nY1,_nX2,_nY2,BOXPOINT-1,xx,yy);
		if (m_pMapBuf[(yy / BOXPOINT)*m_MapW+xx / BOXPOINT]==MAP_NOT_MOVE) {
			//����ֱ�ӵ�����ؽǵ�
			_nX=x0+dx*(BOXPOINT / 2-BOXBORDER);
			_nY=y0+dy*(BOXPOINT / 2-BOXBORDER);
			return true;
		} else {
			_nX=_nX2;
			_nY=_nY2;
			return false;
		}
	}
}

osVec2D CFindPathMap::get_mapSize()
{
	return osVec2D( m_MapW*1.5f, m_MapH*1.5f );
}







//! �洢��������ͼ
bool CFindPathMap::saveMapInfo( const char* _filename )
{
	guard;

	osassert( m_sHeader );

	FILE*   t_file = fopen( _filename,"wb" );
	if (t_file)
	{
		DWORD   t_dw = time( NULL );

		fwrite( m_sHeader,sizeof( tagMAPFILEHEADER ),1,t_file );
		fwrite( &t_dw,sizeof(DWORD),1,t_file );
		fwrite( m_pMapBuf,
			sizeof( BYTE ),m_MapW*m_MapH,t_file );
		fclose( t_file );
	}

	
	
	return true;

	unguard;
}


//��ȡ��ͼ��ײ��Ϣ
bool CFindPathMap::LoadMapInfo( const char* _pMapFile )
{
	char file_name[128] = {0};
	sprintf( file_name, "map\\%s\\%s.%s",_pMapFile,_pMapFile,SERVER_COLLNAME );
	if( !file_exist( file_name ) )
	{
		osDebugOut( "CFindPathMap:�ļ�û�ҵ�!<%s>\n", file_name );
		return false;
	}

	int t_iLen=sizeof(MAPFILEHEADER);

	int  t_iGBufIdx;
	BYTE* file_start = START_USEGBUF( t_iGBufIdx );
	int file_size = get_fileSize( file_name );
	file_size = read_fileToBuf( file_name, file_start, TMP_BUFSIZE );
	osassert( file_size >= 0 );
	file_start[file_size] = 0;


	// �ļ�ͷ
	if( !m_sHeader )
		m_sHeader = new MAPFILEHEADER;

	READ_MEM_OFF(  m_sHeader,file_start,sizeof(MAPFILEHEADER) );
	READ_MEM_OFF(  &m_timeData,file_start,sizeof(m_timeData));

	//! �����ͼ�Ŀ�Ⱥ͸߶ȡ�
	m_MapW = m_sHeader->shWidth;
	m_MapH = m_sHeader->shHeight;

	if ( m_pMapBuf ) 
	{
		delete[] m_pMapBuf;
		m_pMapBuf = 0;
	}
	if ( !m_pMapBuf )
		m_pMapBuf = new char[m_MapW*m_MapH];
	
	// ��ͼ����Ϣ
	if( m_pMapBuf )
		READ_MEM_OFF( &m_pMapBuf[0], file_start, sizeof(BYTE)*m_MapW*m_MapH );

	END_USEGBUF( t_iGBufIdx );

	// ����128,128Ϊ����ͨ����TEST CODE��
	//m_pMapBuf[128+128*256] = 0;

	osassert( m_sHeader );

	return true;

}
bool CFindPathMap::CanMove( const osVec2D& _pPos )
{
	if( _pPos && m_pMapBuf )
	{
		float x = ( _pPos.x * 1000);
		float y = ( _pPos.y * 1000);
		return CanMove( x, y );
	}
	return false;
}

bool CFindPathMap::CanMove( const osVec3D& _pPos )
{
	if( _pPos && m_pMapBuf )
	{
		float x = ( _pPos.x * 1000);
		float y = ( _pPos.z * 1000);
		return CanMove( x, y );
	}
	return false;
}

bool CFindPathMap::CanMove( int _nX, int _nY )
{
	if( m_pMapBuf )
	{
		int x = _nX / 1500;
		int y = _nY / 1500;
		if( x < 0 || y < 0 || x >= m_MapW || y >= m_MapH )
			return false;

		osDebugOut( "The Final x,y is:<%d,%d>...\n",x,y );

		return ( m_pMapBuf[ x + y * m_MapW ] == MAP_CAN_MOVE );
	}
	return false;
}

bool CFindPathMap::CanMoveGrid( int _nX, int _nY ) //��������
{
	if( m_pMapBuf )
	{
		if( _nX < 0 || _nY < 0 || _nX >= m_MapW || _nY >= m_MapH )
			return false;
		return !( m_pMapBuf[ _nY * m_MapW + _nX ] == MAP_NOT_MOVE ) ;
	}
	return false;
}

bool CFindPathMap::CHKBAR( int x, int y )
{
	return !CanMove( x * 1500, y * 1500 );
}
/*

�Ż�NoObstacle���㷨������
1: �����ʼ�����ڸ��ӣ�������������ڵĸ��ӡ�
2: �ӿ�ʼ��ĸ�������ÿ���ཻһ�����ӣ��õ��˸����Ƿ�����ײ��Ϣ��
3: �Ż��������߶���X��ĽǶȣ�ÿ���ཻ��ֻ������ӵ��������ཻ������ֻ��Ҫ
����ǰ���߶��Ƿ��������ı��߶��ཻ��
4: ������ʼ������ӵ��ı��߶��ཻ���õ�������һ������ĸ���....ѭ����ǰ����
����ֱ�ӵ������յĸ��ӡ�

����м�û�и�������ײ��Ϣ���򷵻�true,���򷵻�false.

ʹ��galaxy�ڵ��߶��Ƿ��ཻ�Ŀ�

*/
BOOL CFindPathMap::NoObstacle(float x0,float y0,float x1, float y1)
{
	guard;

	// 
	//
	const float collWidth	= 1500.0f;
	const float collHeight	= 1500.0f;

	const float div_collWidth = collWidth / 2.0f;
	const float div_collHeight = collHeight / 2.0f;

	// get the greater point by the x axes
	//
	if(x0 > x1){
		register float tmp = x0;
		x0 = x1;
		x1 = tmp;

		tmp = y0;
		y0	= y1;
		y1	= tmp;
	}


	if(y0 < y1){

		// compute the slope
		//
		const float slope = (y1 - y0) / (x1 - x0);

		// get these rectangles range
		//
		const float startRow = (float)((int)(x0 / collWidth)) * collWidth ;
		const float endRow	 = (float)((int)(x1 / collWidth)) * collWidth + collWidth;
		const float startCol = (float)((int)(y0 / collHeight)) * collHeight;
		const float endCol	 = (float)((int)(y1 / collHeight)) * collHeight + collHeight;

		// interate these rectangls
		//
		for(float col =  startCol;col < endCol;col += collHeight){

			for(float row = startRow;row < endRow; row += collWidth){

				const bool bottomLeft		= slope * (row				- x0) - col					+ y0 > 0.0f; 
				const bool bottomRight		= slope * (row + collWidth	- x0) - col					+ y0 > 0.0f; 
				const bool topLeft			= slope * (row				- x0) - (col + collHeight)	+ y0 > 0.0f; 
				const bool topRight			= slope * (row + collWidth	- x0) - (col + collHeight)	+ y0 > 0.0f;

				if(bottomLeft){
					if(!(bottomRight && topLeft && topRight)){
						// intersect with the line
						//
						if(!CanMove(row + div_collWidth,col + div_collHeight)){
							return FALSE;
						}
					}
				}else{
					if(!(!bottomRight && !topLeft && !topRight)){
						// intersect with the line
						//
						if(!CanMove(row + div_collWidth,col + div_collHeight)){
							return FALSE;
						}
					}
				}
			}
		}

	}else{

		// compute the slope
		//
		const float slope = (y1 - y0) / (x1 - x0);

		// get these rectangles range
		//
		const float startRow = (float)((int)(x0 / collWidth)) * collWidth ;
		const float endRow	 = (float)((int)(x1 / collWidth)) * collWidth + collWidth;
		const float startCol = (float)((int)(y1 / collHeight)) * collHeight;
		const float endCol	 = (float)((int)(y0 / collHeight)) * collHeight + collHeight;

		// interate these rectangls
		//
		for(float col =  startCol;col < endCol;col += collHeight){	// this is diff from the up part

			for(float row = startRow;row < endRow; row += collWidth){
				const bool bottomLeft		= slope * (row				- x0) - col					+ y0 > 0.0f; 
				const bool bottomRight		= slope * (row + collWidth	- x0) - col					+ y0 > 0.0f; 
				const bool topLeft			= slope * (row				- x0) - (col + collHeight)	+ y0 > 0.0f; 
				const bool topRight			= slope * (row + collWidth	- x0) - (col + collHeight)	+ y0 > 0.0f;

				if(bottomLeft){
					if(!(bottomRight && topLeft && topRight)){
						// intersect with the line
						//
						if(!CanMove(row + div_collWidth,col + div_collHeight)){
							return FALSE;
						}
					}
				}else{
					if(!(!bottomRight && !topLeft && !topRight)){
						// intersect with the line
						//
						if(!CanMove(row + div_collWidth,col + div_collHeight)){
							return FALSE;
						}
					}
				}
			}
		}

	}

	return TRUE;
	unguard;

}

//���ֱ������û����ײ��
bool CFindPathMap::CheckLine( int _nBeginX,int _nBeginY , int _nTargetX ,  
							 int _nTargetY,bool _accu/* = false*/ )
{
	guard;
	if ( !CanMove( _nBeginX,_nBeginY )) 
	{
		//Outstr_tolog("���Ϊ�ϰ�");
		return false;
	}
	if (!CanMove( _nTargetX,_nTargetY ) )
	{
		//Outstr_tolog("�յ�Ϊ�ϰ�");
		return false;
	}

	int t_nDx	=	_nTargetX-_nBeginX;
	int t_nDy	=	_nTargetY-_nBeginY;
	int t_nDDx	=	abs(t_nDx);
	int	t_nDDy	=	abs(t_nDy);


	if ( t_nDDx<=_MAP_FINESIZE && t_nDDy<=_MAP_FINESIZE )
	{
		return true;
	}

	if ( t_nDDy>t_nDDx )
	{

		//�����һ��Y����ʼ��

		int	t_nFirstY	=	_nBeginY-_nBeginY % _MAP_FINESIZE;

		int	t_nStepY	=	0;

		if ( t_nDy > 0 )
		{
			t_nFirstY	+=	(_MAP_FINESIZE + _MAP_FINESIZE/2);
			t_nStepY	=	_MAP_FINESIZE;

		}
		else
		{
			t_nFirstY	-=	_MAP_FINESIZE/2;
			t_nStepY	=	-_MAP_FINESIZE;
		}

		//!����X���б��
		double t_dStep	=	double(t_nDDx) / double(t_nDDy);
		if ( t_nDx < 0 ) 
		{
			t_dStep	=	-t_dStep;
		}
		double t_dStepX	=	t_dStep * abs(t_nFirstY - _nBeginY) ;
		int		t_nFirstX	=	_nBeginX + t_dStepX;

		//!ѭ����������
		t_dStepX	=	t_dStep*_MAP_FINESIZE;

		//!�յ�Y�ĸ�������
		int		t_nTargetCellY	=	_nTargetY / _MAP_FINESIZE;

		int	t_nCurY	=	t_nFirstY;
		int	t_nCurX	=	t_nFirstX;

		int t_nStepCnt	=	1;

		// River added @ 2008-11-11:���Y��ݽ�����һ�����ӣ�����
		if( _accu )
		{
			if( abs( t_nCurX/_MAP_FINESIZE - _nBeginX/_MAP_FINESIZE ) > 1 )
			{
				if( t_dStepX > 0 )
				{
					if( (!CanMove( _nBeginX+_MAP_FINESIZE,t_nCurY) )||
						(!CanMove( _nBeginX+_MAP_FINESIZE,_nBeginY) ) )
						return false;
				}
				else
				{
					if( (!CanMove( _nBeginX-_MAP_FINESIZE,t_nCurY))||
						(!CanMove( _nBeginX-_MAP_FINESIZE,_nBeginY) ) )
						return false;
				}
			}
		}

		while ( t_nCurY / _MAP_FINESIZE !=  t_nTargetCellY )  
		{
			if ( !CanMove( t_nCurX , t_nCurY )
				&& !CanMove( t_nCurX + PATH_TOLERANCE, t_nCurY )
				&& !CanMove( t_nCurX - PATH_TOLERANCE, t_nCurY )
				) 
			{
				return false;
			}
			t_nCurY	=	t_nFirstY+t_nStepCnt*t_nStepY;
			t_nCurX	=	t_nFirstX+double(t_nStepCnt*t_dStepX);
			++t_nStepCnt;
		}

		// River mod @ 2008-11-10:
		if( _accu )
		{
			if ( !CanMove( t_nCurX , t_nCurY )
				&& !CanMove( t_nCurX + PATH_TOLERANCE, t_nCurY )
				&& !CanMove( t_nCurX - PATH_TOLERANCE, t_nCurY )
				) 
				return false;
		}


		return true;

	}
	else
	{
		//�����һ��X����ʼ��

		int	t_nFirstX	=	_nBeginX-_nBeginX % _MAP_FINESIZE;

		int	t_nStepX	=	0;

		if ( t_nDx > 0 )
		{
			t_nFirstX	+=	(_MAP_FINESIZE + _MAP_FINESIZE/2);
			t_nStepX	=	_MAP_FINESIZE;

		}
		else
		{
			t_nFirstX	-=	_MAP_FINESIZE/2;
			t_nStepX	=	-_MAP_FINESIZE;
		}

		//!����X���б��
		double t_dStep	=	double(t_nDDy) / double(t_nDDx);
		if ( t_nDy < 0 ) 
		{
			t_dStep	=	-t_dStep;
		}
		double t_dStepY	=	t_dStep * abs(t_nFirstX - _nBeginX) ;

		int		t_nFirstY	=	_nBeginY + t_dStepY;

		//!ѭ����������
		t_dStepY	=	t_dStep*_MAP_FINESIZE;

		//!�յ�Y�ĸ�������
		int		t_nTargetCellX	=	_nTargetX / _MAP_FINESIZE;

		int	t_nCurY	=	t_nFirstY;
		int	t_nCurX	=	t_nFirstX;

		int t_nStepCnt	=	1;

		// River added @ 2008-11-11:���Y��ݽ�����һ�����ӣ�����
		if( _accu )
		{
			if( abs( t_nCurY/_MAP_FINESIZE - _nBeginY/_MAP_FINESIZE ) > 1 )
			{
				if( t_dStepY > 0 )
				{
					if( (!CanMove( t_nCurX,_nBeginY+_MAP_FINESIZE))||
						(!CanMove( _nBeginX,_nBeginY+_MAP_FINESIZE) ) )
						return false;
				}
				else
				{
					if( (!CanMove( t_nCurX,_nBeginY-_MAP_FINESIZE))||
						(!CanMove( _nBeginX,_nBeginY-_MAP_FINESIZE) ) )
						return false;
				}
			}
		}

		// TEST CODE:
		while ( t_nCurX / _MAP_FINESIZE !=  t_nTargetCellX ) 
		{
			if ( !CanMove( t_nCurX , t_nCurY )
				&& !CanMove( t_nCurX, t_nCurY + PATH_TOLERANCE )
				&& !CanMove( t_nCurX, t_nCurY - PATH_TOLERANCE )
				)
			{
				//Outstr_tolog("�ϰ����<X>: ���<%d,%d>, �յ�<%d,%d>, �ϰ���<%d,%d>"
				//	, _nBeginX, _nBeginY, _nTargetX, _nTargetY, t_nCurX, t_nCurY );
				return false;
			}
			t_nCurX	=	t_nFirstX+t_nStepCnt*t_nStepX;
			t_nCurY	=	t_nFirstY+double(t_nStepCnt*t_dStepY);
			++t_nStepCnt;
		}

		// river added @ 2008-11-10:
		if( _accu )
		{
			if ( !CanMove( t_nCurX , t_nCurY )
				&& !CanMove( t_nCurX, t_nCurY + PATH_TOLERANCE )
				&& !CanMove( t_nCurX, t_nCurY - PATH_TOLERANCE )
				)
				return false;
		}

		return true;

	}
	return true;
	unguard;
}

bool CFindPathMap::CheckLine( osVec3D *_pStartPos, osVec3D *_pEndPos )
{
	int x1 = int( _pStartPos->x * 1000.f );
	int y1 = int( _pStartPos->z * 1000.f );
	int x2 = int( _pEndPos->x * 1000.f );
	int y2 = int( _pEndPos->z * 1000.f );
	return NoObstacle( x1, y1, x2, y2 );
}

CFindPathMap::CFindPathMap()
{
	m_pMapBuf = 0;
	m_sHeader = NULL;
}

CFindPathMap::~CFindPathMap()
{
	SAFE_DELETE( m_sHeader );
	SAFE_DELETE_ARRAY( m_pMapBuf );
}

void CFindPathMap::CalcNextPoint(int _wSx,int _wSy,int _wDx,int _wDy, int _wSpace, int &_wX, int &_wY)
{
	int dx,dy;
	double d1;
	//_wX,_wY���α����ʼ��Ϊ_wSx,_wSy
	//Ϊ��߾���ȷ��ʼ����_wsx,_wsy,_wDx,_wDy��ʾ����ֹ�� _wX,_wY��ʾ��ǰ��
	//�����нϴ��һ������ȣ�������ֹ�޹�
	dx=abs(_wDx-_wSx);
	dy=abs(_wDy-_wSy);
	if ((dx<BOXPOINT) && (dy<BOXPOINT)) {
		_wX=_wDx;
		_wY=_wDy;
		return;
	}
	//[edit]
	if ((abs(_wDx-_wX)<BOXPOINT) && (abs(_wDy-_wY)<BOXPOINT)) {
		_wX=_wDx;
		_wY=_wDy;
		return;
	}
	//
	if (dy>dx) 
	{
		//y�ȷ��е�
		if (_wY % BOXPOINT != BOXPOINT / 2) 
		{
			//ȡ���¸��е�
			_wY=(_wY -_wY % BOXPOINT)+BOXPOINT / 2;
			if (_wDy>=_wY) _wY=_wY+BOXPOINT; else _wY=_wY-BOXPOINT;
		} 
		else 
		{
			if (_wDy>=_wY) _wY=_wY+BOXPOINT; else _wY=_wY-BOXPOINT;
		}
		if (abs(_wY-_wSy)>abs(_wY-_wDy)) 
		{
			//��ʽ1
			d1=(_wY-_wSy);
			_wX=_wSx+d1*(_wDx-_wSx) / (_wDy-_wSy);
		} 
		else 
		{
			//��ʽ2
			d1=(_wY-_wDy);
			_wX=_wDx+d1*(_wSx-_wDx) / (_wSy-_wDy);
		}
	} 
	else 
	{
		//x�ȷ��е�
		//y�ȷ��е�
		if (_wX % BOXPOINT != BOXPOINT / 2) 
		{
			//ȡ���¸��е�
			_wX=(_wSx -_wSx % BOXPOINT)+BOXPOINT / 2;
			if (_wDx>=_wX) _wX=_wX+BOXPOINT; else _wX=_wX-BOXPOINT;
		} 
		else 
		{
			if (_wDx>=_wX) _wX=_wX+BOXPOINT; else _wX=_wX-BOXPOINT;
		}
		//�����Ӧ��_wX;
		if (abs(_wX-_wSx)>abs(_wX-_wDx)) 
		{
			d1=(_wX-_wSx);
			_wY=_wSy+d1*(_wDy-_wSy) / (_wDx-_wSx);
		} 
		else 
		{
			d1=(_wX-_wDx);
			_wY=_wDy+d1*(_wSy-_wDy) / (_wSx-_wDx);
		}
	}
}

/**\brief
* ����������������ָ�������һ���Ϸ���
* \param    
* \return   
*/
void CalcPoint(int _nBeginX,int _nBeginY,int _nTargetX,int _nTargetY,int _nSpace,int &_nX,int &_nY)
{
	//guard;
	if( _nBeginX < 0 || _nBeginY < 0 || _nTargetX < 0 || _nTargetY < 0 || _nSpace < 0 )
		return;
	//�����֮��ľ���
	double	X = _nTargetX - _nBeginX;
	double	Y = _nTargetY - _nBeginY;
	double s = sqrt(X*X + Y*Y);
	if (s<=_nSpace) 
	{
		_nX = _nTargetX;
		_nY = _nTargetY;
	}
	else
	{
		//!�������������� ,���������һ����
		_nX=double(_nBeginX) +(X * (double(_nSpace)/s));
		_nY=double(_nBeginY) +(Y * (double(_nSpace)/s));
	}
	//unguard;
}


/** \brief
* ������ʼ����յ�ķ��򣬼������յ�����Ŀ�ͨ��������㡣 by river@ 2008-11-10:
*
* ����ʹ����CalcNextPoint������ͬ������ֵ����ϸ��(mm��λ)���ӵ����ꡣ
*/
bool CFindPathMap::FindNearestMovePt( int _wSx, int _wSy, int _wDx, int _wDy,int &_wX, int &_wY )
{
	guard;

	int   t_iResx,t_iResy;
	bool  t_bCanMove;

	if( !CanMove( _wSx,_wSy ) )
		return false;

	int testx,testy,testx1,testy1;

	testx = _wSx/1500;
	testy = _wSy/1500;

	do 
	{
		t_iResx = _wDx;
		t_iResy = _wDy;
		CalcNextPoint( _wDx,_wDy,_wSx,_wSy,0,t_iResx,t_iResy );
		
		t_bCanMove = CanMove( t_iResx,t_iResy );

		_wDx = t_iResx;
		_wDy = t_iResy;
		
		testx1 = _wDx/1500;
		testy1 = _wDy/1500;

		if( (testx1==testx)&&(testy1==testy) )
			break;

	}while( !t_bCanMove );

	_wX = t_iResx;
	_wY = t_iResy;

	return true;

	unguard;
}


/**\brief
* ����ָ������Ѱ��ֱ����Զ������ by sisun @ 2007-06-25
* \param    _sStartPos: ��ʼ����
* \param	_sDir:		�ƶ�����
*/
void CFindPathMap::FindLinePath( osVec2D& _sPos, osVec2D _sDir )
{
	POINT myPos, ptDir, ptPos;
	myPos.x = int( _sPos.x * 1000.f );
	myPos.y = int( _sPos.y * 1000.f );
	if( !CanMove( myPos.x, myPos.y ) )
		return;
	ptDir.x = int( _sDir.x * 1000.f );
	ptDir.y = int( _sDir.y * 1000.f );
	if( ptDir.x == 0 && ptDir.y == 0 )
		return;

	//! ���
	float fStepX = 0;	//! X�Ĳ���ֵ
	float fStepY = 0;	//! Y�Ĳ���ֵ

	if( abs(ptDir.y) > abs(ptDir.x) )
	{
		if( ptDir.y > 0 )
		{
			fStepY = 1;
		}
		else
		{
			fStepY = -1;
		}
		fStepX = abs( fStepY / _sDir.y ) * _sDir.x;
	}
	else
	{
		if( ptDir.x > 0 )
		{
			fStepX = 1;
		}
		else
		{
			fStepX = -1;
		}
		fStepY = abs( fStepX / _sDir.x ) * _sDir.y;
	}
	ptPos.x = myPos.x;
	ptPos.y = myPos.y;

	//! Ŀ���
	POINT tgPos;
	if( CanMove( ptPos.x, ptPos.y ) )
	{
		for( int i = 0; ; i++ )
		{
			int x = ptPos.x + fStepX * i;
			int y = ptPos.y + fStepY * i;

			if( CanMove( x, y ) )
			{
				tgPos.x = x;
				tgPos.y = y;
			}
			else {
				break;
			}
		}
	}
	else
	{
		tgPos.x = myPos.x;
		tgPos.y = myPos.y;
	}

	//! �յ�
	POINT ptTmp, ptEnd;
	ptTmp.x = tgPos.x - tgPos.x % _MAP_FINESIZE;
	ptTmp.x += ptDir.x > 0 ? _MAP_FINESIZE - 1 : 1;
	ptTmp.y = tgPos.y - tgPos.y % _MAP_FINESIZE;
	ptTmp.y += ptDir.y > 0 ? _MAP_FINESIZE - 1 : 1;
	fStepX = abs( float( ptTmp.y - tgPos.y ) / _sDir.y ) * _sDir.x;
	fStepY = abs( float( ptTmp.x - tgPos.x ) / _sDir.x ) * _sDir.y;
	if( abs(ptDir.y) > abs(ptDir.x) )
	{
		ptEnd.y = ptTmp.y;
		ptEnd.x = tgPos.x + fStepX;
		if( !CanMove( ptEnd.x, ptEnd.y ) )
		{
			ptEnd.x = ptTmp.x;
			ptEnd.y = tgPos.y + fStepY;
		}
	}
	else
	{
		ptEnd.x = ptTmp.x;
		ptEnd.y = tgPos.y + fStepY;
		if( !CanMove( ptEnd.x, ptEnd.y ) )
		{
			ptEnd.y = ptTmp.y;
			ptEnd.x = tgPos.x + fStepX;
		}
	}

	//! ���
	_sPos.x = float(ptEnd.x) / 1000.f;
	_sPos.y = float(ptEnd.y) / 1000.f;
	osDebugOut( "ֱ��Ѱ·<�ɹ�>: ��ǰλ��<%d,%d>, Ŀ��λ��<%d,%d>\n"
		, myPos.x, myPos.y, ptEnd.x, ptEnd.y );
	
}

//! River added @ 2008-11-11:�ѳ����·���������ָ�ɶ�һЩ��·���㡣
void divide_wayPoint( osVec2D& _src,osVec2D* _arrayWayPoint,int& _num,int _maxWayPoint,float _currSpeed )
{
	guard;

	if(_num <= 0){ return;}

	// ������������д���ͼ����ӳ٣�������Ҫ��·��ļ����ȥ��ǰ���ٶȣ���ǰ���ٶ�Խ��
	// ·��ļ��С��
	//
	const float t_maxIntervalLength = MAX_WAYLENGTH - _currSpeed;

	const int t_maxWayPoint = 300;
	osassert(_num < t_maxWayPoint);

	osVec2D t_tmpWayPoint[t_maxWayPoint];

	int t_tmpIndex = 0;

	// divide the from the src point to first point 
	//
	osVec2D t_dir = _arrayWayPoint[0] - _src;
	int t_insertPoint = osVec2Length(&t_dir) / t_maxIntervalLength;
	osVec2Normalize(&t_dir,&t_dir);

	while(t_tmpIndex < t_insertPoint){
		t_tmpWayPoint[t_tmpIndex] = _src + t_dir * t_maxIntervalLength * (t_tmpIndex + 1);
		t_tmpIndex++;
	}

	t_tmpWayPoint[t_tmpIndex] = _arrayWayPoint[0];
	t_tmpIndex++;

	osassert(t_tmpIndex < t_maxWayPoint);

	// divide the remain points
	//
	for(int i = 1;i< _num;i++){
		t_dir = _arrayWayPoint[i] - _arrayWayPoint[i -1];
		t_insertPoint = osVec2Length(&t_dir) / t_maxIntervalLength;
		osVec2Normalize(&t_dir,&t_dir);

		int t_insertIndex = 0;
		while(t_insertIndex < t_insertPoint){
			t_tmpWayPoint[t_tmpIndex + t_insertIndex] = _arrayWayPoint[i - 1] + t_dir * t_maxIntervalLength * (t_insertIndex + 1);
			t_insertIndex++;
		}	
		t_tmpIndex += t_insertIndex;
		t_tmpWayPoint[t_tmpIndex] = _arrayWayPoint[i];

		t_tmpIndex++;

		osassert(t_tmpIndex < t_maxWayPoint);
	}

	_num = t_tmpIndex;
	CopyMemory(_arrayWayPoint,t_tmpWayPoint,sizeof(t_tmpWayPoint[0]) * t_tmpIndex);

	unguard;
}



/** \brief
*
* \param osVec2D& _src Ѱ·��ʼ���λ�á�
* \param osVec2D& _dest Ѱ·�������λ�á�
* \param osVec2D* _arrayWayPoint Ѱ·���صĹؼ�·��
* \param int& _num          Ѱ·���صĹؼ�·��λ��.
* \param int _maxWayPoint   ���������,�����Է��ص�·��λ��
* \param _currSpeed		��ǰ������ٶ�
*/
bool CFindPathMap::ComputeWayPoint(osVec2D& _src,
								   osVec2D& _dest,osVec2D* _arrayWayPoint,int& _num,int _maxWayPoint,float _currSpeed)
{
	guard;

	osassert( _arrayWayPoint );

	int  t_iSrcPtx,t_iSrcPtz;
	int  t_iDstPtx,t_iDstPtz;
	int  t_iResPtx,t_iResPtz;

	bool      t_bTarget = true;
	osVec3D   t_vec3Src,t_vec3RealTarget;
	

	_num = 0;

	t_iSrcPtx = _src.x * 1000;
	t_iSrcPtz = _src.y * 1000;
	t_iDstPtx = _dest.x * 1000;
	t_iDstPtz = _dest.y * 1000;


	//! �����ǰ���������ƶ�������ͨ����λ�ã���˴�Ѱ·ʧ�ܣ����ء�
	if( !CanMove( t_iSrcPtx,t_iSrcPtz ) )
		return false;

	// ���Ŀ��㲻��ͨ��,���ҵ���Ŀ�������Ŀ�ͨ���ĸ��ӵ���ΪĿ���.
	if( !CanMove( t_iDstPtx,t_iDstPtz ) )
	{
		if( !FindNearestMovePt( 
			t_iSrcPtx,t_iSrcPtz,t_iDstPtx,t_iDstPtz,t_iResPtx,t_iResPtz ) )
			return false;

		t_iDstPtx = t_iResPtx;
		t_iDstPtz = t_iResPtz;
	}

	// ����Ŀ������Ѱ·��Ϣ,���Ŀ��㲻�ɵ���,����������Ĵ���.
	if( !NoObstacle( t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz)  )
	{
		t_bTarget = FindPathMap( t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz,t_iResPtx,t_iResPtz );
		
		osassert(CanMove(t_iResPtx,t_iResPtz));

		while( t_bTarget && _num < _maxWayPoint)
		{
			_arrayWayPoint[_num].x = t_iResPtx / 1000.0f;
			_arrayWayPoint[_num].y = t_iResPtz / 1000.0f;
			osDebugOut("\narrayWayPoint [%f,%f]\n",_arrayWayPoint[_num].x,_arrayWayPoint[_num].y);

			osassert(CanMove(t_iResPtx,t_iResPtz));
			osassert(CanMove(_arrayWayPoint[_num]));

			_num ++;

			t_iSrcPtx = t_iResPtx;
			t_iSrcPtz = t_iResPtz;

			// �������ֱ�ߵ���
			if( NoObstacle( t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz ) )
			{
				_arrayWayPoint[_num].x = t_iDstPtx / 1000.0f;
				_arrayWayPoint[_num].y = t_iDstPtz / 1000.0f;
				
				osassert(CanMove(_arrayWayPoint[_num]));

				_num ++;

				if( _num >= _maxWayPoint )
					osassert(false);

				break;
			}
			
			// ATTENTION TO OPT: ���β��ҵ�ͼ��·��,����,�Ժ����һ����Ѱ�ҵ�ͼ,�Ż�.
			t_bTarget = FindPathMap( t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz,t_iResPtx,t_iResPtz );
			osassert(CanMove(t_iResPtx,t_iResPtz));
			if( t_bTarget )
				osassert( NoObstacle( t_iSrcPtx, t_iSrcPtz, t_iResPtx, t_iResPtz) );


			if(!t_bTarget){
				osDebugOut("����Ԥ���ִ��·��...\n");
				_num = 0;
				return false;
			}
		}

	}
	else
	{
// 		osDebugOut( "Check Line success<%d,%d,%d,%d>....\n",
// 			t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz );
		_num = 1;
		_arrayWayPoint[0].x = t_iDstPtx/1000.0f;
		_arrayWayPoint[0].y = t_iDstPtz/1000.0f;

		osassert(CanMove(_arrayWayPoint[0]));
	}


	// ����ʼ����Ŀ�����,�ߵ�����ͨ���ĸ���.
	if( !t_bTarget )
	{
		osVec2D   t_vec3Src,t_vec3Dir;
		
		t_vec3Src.x = _src.x;
		t_vec3Src.y = _src.y;

		t_vec3Dir.x = _dest.x - _src.x;
		t_vec3Dir.y = _dest.y - _src.y;
		fpm_vec2Normalize( &t_vec3Dir,&t_vec3Dir );

		FindLinePath( t_vec3Src,t_vec3Dir );

		_num = 1;
		_arrayWayPoint[0].x = t_vec3Src.x;
		_arrayWayPoint[0].y = t_vec3Src.y;

		osassert(CanMove(_arrayWayPoint[0]));
	}

	osassert( _num <= _maxWayPoint );

	// 
	// ÿһ��·���㲻�ܹ���
	divide_wayPoint( _src,_arrayWayPoint,_num,_maxWayPoint,_currSpeed );

#ifdef OUTPUT_WALKPOS_INFO
	osDebugOut("\n---->Compute walk Point (src position <%f,%f>) :\n",_src.x,_src.y);
	for(int i = 0;i<_num;i++){
		osDebugOut("----><%f,%f>\n",_arrayWayPoint[i].x,_arrayWayPoint[i].y);
	}
	osDebugOut("---->Dest position:<%f,%f>:",_dest.x,_dest.y);
#endif

	// ����
	return true;

	unguard;
}


