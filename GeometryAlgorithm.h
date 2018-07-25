#pragma once
#ifndef _GEOMETRY_ALGORITHMS_H_
#define _GEOMETRY_ALGORITHMS_H_

#include "Geometry.h"
using namespace GeometryLas;

class  GeometryRelation
{
public:
	static int IsPointOnPolyline(double x, double y, const double* polyline, int doubleArrayCnt,
		double tolerance, bool isClosed, int offset = sizeof(Point3D) / sizeof(double));
	/*
	* �жϵ��Ƿ����߶��ϣ��ռ��ϵ��line contains point��
	*/
	static bool IsPointOnLine(double x, double y, double x0, double y0, double x1, double y1,
		double tolerance);
	/*
	* �жϵ��Ƿ���ֱ���ϣ��ռ��ϵ��line contains point��
	*/
	static bool IsPointOnBeeline(double x, double y, double x0, double y0, double x1, double y1,
		double tolerance);
	/*
	* �жϵ��Ƿ��ڶ����������ڱ߽��ϣ��ռ��ϵ��region contains point��
	* ˳ʱ�����ÿ����
	* @param allowonisin : �Ƿ���Ϊ�ڱ߽���Ҳ���ڱ߽��ڣ�Ϊtrue����tolerance��Ч��������Ч
	*/
	static bool IsPointInPolygon(double x, double y, const double* polygon, double doubleArrayCnt,
		bool allowonisin, double tolerance, int offset = sizeof(Point3D) / sizeof(double));
	/*
	* �жϵ��Ƿ��ڶ������
	*/
	static bool IsPointInPolygon(double x, double y, const double* polygon,
		int doubleArrayCnt, int offset = sizeof(Point3D) / sizeof(double));
	static bool IsPointInPolygon(double x, double y, const Point2Ds& polygon);
	/*
	* used for IsPointInPolygon follows
	*/
	static double IsLeft(double startx, double starty, double endx, double endy, double x, double y);
	/*
	* ���Ƿ��ھ��α߽��ϣ��ռ��ϵ��lines contains point��
	*/
	static bool IsPointOnRect(double x, double y, double minx, double miny,
		double maxx, double maxy, double tolerance);
	/*
	* �ж����ߡ�������Ƿ��ھ�������������ھ��α߽��ϣ��ռ��ϵ��region contains region, region contains lines��
	*/
	static bool IsPolylineInRect(const double* polyline, int doubleArrayCnt, double minx,
		double miny, double maxx, double maxy, int offset = sizeof(Point3D) / sizeof(double));
	/*
	* �ж��߶�����α߽��Ƿ��ཻ�������߽�㣨�ռ��ϵ��line crosses line��
	*/
	static bool IsLineIntersectRect(double x0, double y0, double x1, double y1, double xmin,
		double ymin, double xmax, double ymax);
	/*
	* �жϾ��α߽��������Ƿ��ཻ�������߽�㣨�ռ��ϵ��line crosses line��
	*/
	static bool IsRectIntersectPolyline(double xmin, double ymin, double xmax,
		double ymax, const double* polyline, int doubleArrayCnt, bool isClosed,
		int offset = sizeof(Point3D) / sizeof(double));
	static bool IsPointInRect(int x, int y, int rect_x1, int rect_y1, int rect_x2, int rect_y2);
	/*
	* �ж���������б�����Ƿ��ཻ
	*/
	static bool IsRectIntersectRect(const Rect2D& rect1, const Rect2D& rect2);
	static bool IsRectIntersectSlantingRect(const Rect2D& rect, const Point2Ds& pts);
	static bool IsRectIntersectSlantingRect(const Rect2D& rect, const Point2Ds& pts,
		const Rect2D& slantingRect);
};

/*������㺯��*/
class  DistanceComputation
{
public:
	/*
	* �㵽ֱ�ߵľ����ƽ��
	*/
	static double SquarePointToBeeline(double x, double y, double x0, double y0, double x1, double y1);
	/*
	* ����㼯�����е��ľ���֮�ͣ�ŷ�Ͼ���
	*/
	static double Distance(Point2Ds& pts, bool isclosed);
};

/*�����ཻ����*/
class  PointJointComputation
{
public:
	/*
	* ��(x, y)��ֱ��(bln0x, bln0y)(bln1x, bln1y)�Ĵ���ռ�߶εı���
	*/
	static void PointJointBeeline(double x, double y, double bln0x, double bln0y, double bln1x,
		double bln1y, double& ratio);
	/*
	* ��(x, y)��ֱ��(bln0x, bln0y)(bln1x, bln1y)�Ĵ���
	*/
	static Point2D PointJointBeeline(double x, double y, double bln0x, double bln0y, double bln1x,
		double bln1y);
};

/*ͶӰ����ת��*/
class  PointProjection
{
public:
	/*��γ������ת����ͶӰ����*/
	static void PointProjectLatLngToUTM(double lat, double lng, int nZone, double &x, double &y);
};

#endif