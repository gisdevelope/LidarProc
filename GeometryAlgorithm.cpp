#include <assert.h>
#include <stdio.h>

#include "../LidarAlgorithm/GeometryAlgorithm.h"
#include "../LidarAlgorithm/GeometryAlgorithm.h"

#include "../LidarBase/tsmToUTM.h"

#pragma warning(disable:4996)

static bool Is_Equal_Eps(double v1, double v2, double epsilon)
{
	return abs(v1 - v2) <= epsilon;
}

#ifndef EPSILON
const static double EPSILON = 1.0e-8;
#endif // !EPSILON

#ifndef EPSILONF
const static float  EPSILONF = 1.0e-6F;
#endif // !EPSILONF


int GeometryRelation::IsPointOnPolyline(double x, double y, const double* polyline, int doubleArrayCnt,
	double tolerance, bool isClosed, int offset)
{
	int ptsCnt = doubleArrayCnt / offset;
	assert(ptsCnt >= 2);

	int cnt = ptsCnt - 1;
	int index, index2;
	for (int k = 0; k < cnt; k++)
	{
		index = k * offset;
		index2 = index + offset;
		if (IsPointOnLine(x, y, polyline[index], polyline[index + 1], polyline[index2],
			polyline[index2 + 1], tolerance))
		{
			return k;
		}
	}
	if (isClosed)
	{
		index = cnt * offset;
		if (IsPointOnLine(x, y, polyline[index], polyline[index + 1], polyline[0], polyline[1], tolerance))
		{
			return cnt;
		}
	}
	return -1;
}
bool GeometryRelation::IsPointOnLine(double x, double y, double x0, double y0, double x1, double y1, double tolerance)
{
	assert(tolerance >= 0);

	Rect2D rect;
	rect.minx = min(x0, x1);
	rect.maxx = max(x0, x1);
	rect.miny = min(y0, y1);
	rect.maxy = max(y0, y1);

	rect.Normalize();
	rect.minx -= tolerance;
	rect.maxx += tolerance;
	rect.miny -= tolerance;
	rect.maxy += tolerance;
	if (rect.IsInclude(x, y))
	{
		return IsPointOnBeeline(x, y, x0, y0, x1, y1, tolerance);
	}
	return false;
}
bool GeometryRelation::IsPointOnBeeline(double x, double y, double x0, double y0, double x1, double y1,
	double tolerance)
{
	assert(tolerance >= 0);

	double dissqr = DistanceComputation::SquarePointToBeeline(x, y, x0, y0, x1, y1);
	return dissqr < tolerance * tolerance;
}
bool GeometryRelation::IsPointInPolygon(double x, double y, const double* polygon, double doubleArrayCnt,
	bool allowonisin, double tolerance, int offset)
{
	int ptsCnt = int(doubleArrayCnt / offset);
	assert(ptsCnt >= 3);

	bool isin = false;
	if (allowonisin)
	{
		int index = IsPointOnPolyline(x, y, polygon, (int)doubleArrayCnt, tolerance, true, offset);
		isin = index != -1;
	}
	if (!isin)
	{
		isin = IsPointInPolygon(x, y, polygon, (int)doubleArrayCnt, offset);
	}
	return isin;
}
bool GeometryRelation::IsPointInPolygon(double x, double y, const Point2Ds& polygon)
{
	return GeometryRelation::IsPointInPolygon(x, y, (const double*)&(polygon[0].x), polygon.size() * 2, Point2D::SizeofPoint2);
}
bool GeometryRelation::IsPointInPolygon(double x, double y, const double* polygon,
	int doubleArrayCnt, int offset)
{
	int ptsCnt = doubleArrayCnt / offset;
	assert(ptsCnt >= 3);

	int wn = 0;    // the winding number counter
	double startx, starty, nextx, nexty;
	for (int i = 0; i < ptsCnt - 1; i++)
	{
		int index = i * offset;
		startx = polygon[index];
		starty = polygon[index + 1];
		nextx = polygon[index + offset];
		nexty = polygon[index + offset + 1];
		if (starty <= y) // start y <= pt->y
		{
			if (nexty > y)      // an upward crossing
				if (IsLeft(startx, starty, nextx, nexty, x, y) > 0)  // P left of edge
					++wn;            // have a valid up intersect
		}
		else// start y > P.y (no test needed)
		{
			if (nexty <= y)     // a downward crossing
				if (IsLeft(startx, starty, nextx, nexty, x, y) < 0)  // P right of edge
					--wn;            // have a valid down intersect
		}
	}
	int index = (ptsCnt - 1) * offset;
	startx = polygon[index];
	starty = polygon[index + 1];
	nextx = polygon[0];
	nexty = polygon[1];
	if (starty <= y) // start y <= pt->y
	{
		if (nexty > y)      // an upward crossing
			if (IsLeft(startx, starty, nextx, nexty, x, y) > 0)  // P left of edge
				++wn;            // have a valid up intersect
	}
	else
	{                       // start y > P.y (no test needed)
		if (nexty <= y)     // a downward crossing
			if (IsLeft(startx, starty, nextx, nexty, x, y) < 0)  // P right of edge
				--wn;            // have a valid down intersect
	}

	if (wn == 0)
		return false;
	return true;
}
double GeometryRelation::IsLeft(double startx, double starty, double endx, double endy,
	double x, double y)
{
	return ((endx - startx) * (y - starty) - (x - startx) * (endy - starty));
}
bool GeometryRelation::IsPointOnRect(double x, double y, double xmin, double ymin,
	double xmax, double ymax, double tolerance)
{
	bool b = (x >= xmin - tolerance && x <= xmax + tolerance
		&& (Is_Equal_Eps(y, ymin, tolerance) || Is_Equal_Eps(y, ymax, tolerance)))
		|| (y >= ymin - tolerance && y <= ymax + tolerance
			&& (Is_Equal_Eps(x, xmin, tolerance) || Is_Equal_Eps(x, xmax, tolerance)));
	return b;
}
bool GeometryRelation::IsPolylineInRect(const double* polyline, int doubleArrayCnt, double minx,
	double miny, double maxx, double maxy, int offset)
{
	Rect2D user(minx, miny, maxx, maxy);
	user.Normalize();

	int ptsCnt = doubleArrayCnt / offset;
	for (int i = 0; i < ptsCnt; i++)
	{
		int index = i * offset;
		double x = polyline[index];
		double y = polyline[index + 1];
		if (!user.IsInclude(x, y))
		{
			return false;
		}
	}
	return true;
}
bool GeometryRelation::IsLineIntersectRect(double x0, double y0, double x1, double y1, double xmin,
	double ymin, double xmax, double ymax)
{
	double deltax = x1 - x0;
	double deltay = y1 - y0;

	if (deltax == 0 && deltay == 0)
	{
		if (GeometryRelation::IsPointOnRect(x0, y0, xmin, ymin, xmax, ymax, EPSILON))
		{
			return true;
		}
	}
	else
	{
		if (deltax != 0)
		{
			//????
			double u = (xmin - x0) / deltax;
			if (u >= 0 && u <= 1)//???xmin???line??
			{
				double y = y0 + u * deltay;
				if (y >= ymin && y <= ymax)
				{
					return true;
				}
			}

			//????
			u = (xmax - x0) / deltax;
			if (u >= 0 && u <= 1)//???xmax???line??
			{
				double y = y0 + u * deltay;
				if (y >= ymin && y <= ymax)
				{
					return true;
				}
			}
		}
		if (deltay != 0)
		{
			//???
			double u = (ymin - y0) / deltay;
			if (u >= 0 && u <= 1)//???ymin???line??
			{
				double x = x0 + u * deltax;
				if (x >= xmin && x <= xmax)
				{
					return true;
				}
			}

			//????
			u = (ymax - y0) / deltay;
			if (u >= 0 && u <= 1)//???ymax???line??
			{
				double x = x0 + u * deltax;
				if (x >= xmin && x <= xmax)
				{
					return true;
				}
			}
		}
	}
	return false;
}
bool GeometryRelation::IsRectIntersectPolyline(double xmin, double ymin, double xmax,
	double ymax, const double* polyline, int doubleArrayCnt, bool isClosed, int offset)
{
	int ptsCnt = doubleArrayCnt / offset;
	int index = -1, index2 = -1;
	for (int k = 1; k < ptsCnt; k++)
	{
		index2 = k * offset;
		index = index2 - offset;
		double x1 = polyline[index];
		double y1 = polyline[index + 1];
		double x2 = polyline[index2];
		double y2 = polyline[index2 + 1];
		if (IsLineIntersectRect(x1, y1, x2, y2, xmin, ymin, xmax, ymax))
		{
			return true;
		}
	}

	if (isClosed)
	{
		index = (ptsCnt - 1) * offset;
		double x1 = polyline[index];
		double y1 = polyline[index + 1];
		double x2 = polyline[0];
		double y2 = polyline[1];
		if (IsLineIntersectRect(x1, y1, x2, y2, xmin, ymin, xmax, ymax))
		{
			return true;
		}
	}

	return false;
}
bool GeometryRelation::IsPointInRect(int x, int y, int rect_x1, int rect_y1, int rect_x2, int rect_y2)
{
	if (((x < rect_x1 && x > rect_x2) || (x >rect_x1 && x < rect_x2)) &&
		((y < rect_y1 && y > rect_y2) || (y >rect_y1 && y < rect_y2)))
		return true;
	return false;
}
bool GeometryRelation::IsRectIntersectSlantingRect(const Rect2D& rect, const Point2Ds& pts)
{
	assert(pts.size() == 4);
	Rect2D bound;       // ?????????

	size_t ptId = 0;
	size_t ptCnt = pts.size();
	for (ptId = 0; ptId < ptCnt; ptId++)
	{
		bound.Merge(pts[ptId].x, pts[ptId].y);
	}

	return GeometryRelation::IsRectIntersectSlantingRect(rect, pts, bound);
}
bool GeometryRelation::IsRectIntersectSlantingRect(const Rect2D& rect, const Point2Ds& pts,
	const Rect2D& slantingRect)
{
	if (!rect.IsIntersect(slantingRect))
	{
		return false;
	}

	size_t lineId = 0;
	size_t startPtId, endPtId, crossPtId;
	for (lineId = 0; lineId < 4; lineId++)
	{
		startPtId = lineId;
		endPtId = (startPtId + 1) % 4;
		crossPtId = (endPtId + 1) % 4;

		double crossDir = GeometryRelation::IsLeft(pts[startPtId].x, pts[startPtId].y,
			pts[endPtId].x, pts[endPtId].y, pts[crossPtId].x, pts[crossPtId].y);

		double curDir0 = GeometryRelation::IsLeft(pts[startPtId].x, pts[startPtId].y,
			pts[endPtId].x, pts[endPtId].y, rect.minx, rect.miny);
		if (crossDir > EPSILON && curDir0 > EPSILON ||
			crossDir < -EPSILON && curDir0 < -EPSILON)
		{
			continue;
		}

		double curDir1 = GeometryRelation::IsLeft(pts[startPtId].x, pts[startPtId].y,
			pts[endPtId].x, pts[endPtId].y, rect.minx, rect.maxy);
		if (crossDir > EPSILON && curDir1 > EPSILON ||
			crossDir < -EPSILON && curDir1 < -EPSILON)
		{
			continue;
		}

		double curDir2 = GeometryRelation::IsLeft(pts[startPtId].x, pts[startPtId].y,
			pts[endPtId].x, pts[endPtId].y, rect.maxx, rect.miny);
		if (crossDir > EPSILON && curDir2 > EPSILON ||
			crossDir < -EPSILON && curDir2 < -EPSILON)
		{
			continue;
		}

		double curDir3 = GeometryRelation::IsLeft(pts[startPtId].x, pts[startPtId].y,
			pts[endPtId].x, pts[endPtId].y, rect.maxx, rect.maxy);
		if (crossDir > EPSILON && curDir3 > EPSILON ||
			crossDir < -EPSILON && curDir3 < -EPSILON)
		{
			continue;
		}

		return false;
	}

	return true;
}
bool GeometryRelation::IsRectIntersectRect(const Rect2D& rect1, const Rect2D& rect2)
{
	//�жϾ����ཻ�Ĵ����д���
	//if (IsPointInRect(rect1.minx, rect1.miny, rect2.minx, rect2.miny, rect2.maxx, rect2.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect1.minx, rect1.maxy, rect2.minx, rect2.miny, rect2.maxx, rect2.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect1.maxx, rect1.miny, rect2.minx, rect2.miny, rect2.maxx, rect2.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect1.maxx, rect1.maxy, rect2.minx, rect2.miny, rect2.maxx, rect2.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect2.minx, rect2.miny, rect1.minx, rect1.miny, rect1.maxx, rect1.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect2.minx, rect2.maxy, rect1.minx, rect1.miny, rect1.maxx, rect1.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect2.maxx, rect2.miny, rect1.minx, rect1.miny, rect1.maxx, rect1.maxy))
	//{
	//	return true;
	//}
	//else if (IsPointInRect(rect2.maxx, rect2.maxy, rect1.minx, rect1.miny, rect1.maxx, rect1.maxy))
	//{
	//	return true;
	//}
	//else
	//	return false;

	double  minx = max(rect1.minx, rect2.minx);
	double 	miny = max(rect1.miny, rect2.miny);
	double 	maxx = min(rect1.maxx, rect2.maxx);
	double 	maxy = min(rect1.maxy, rect2.maxy);

	if (minx > maxx || miny > maxy)
	{
//#ifdef _DEBUG
//		FILE* ofs = fopen("E:\\rect.txt", "a+");
//		fprintf(ofs, "%lf,%lf,%lf,%lf;%lf,%lf,%lf,%lf;\n", rect1.minx, rect1.miny, rect1.maxx, rect1.maxy,
//			rect2.minx, rect2.miny, rect2.maxx, rect2.maxy);
//		fclose(ofs);
//#endif // DEBUG
		return false;
	}
	else
		return true;

}


/*****************************************************************************
* @brief : ???????
* @author : W.W.Frank
* @date : 2015/11/30 14:53
* @version : version 1.0
* @inparam :
* @outparam :
*****************************************************************************/
double DistanceComputation::SquarePointToBeeline(double x, double y, double x0, double y0,
	double x1, double y1)
{
	Point2D pt = PointJointComputation::PointJointBeeline(x, y, x0, y0, x1, y1);
	double xx = pt.x - x;
	double yy = pt.y - y;
	return xx * xx + yy * yy;
}
double DistanceComputation::Distance(Point2Ds& pts, bool isclosed)
{
	double dis = 0.0;
	for (size_t i = 0; i < pts.size() - 1; i++)
	{
		dis += pts[i].Distance(pts[i + 1]);
	}
	if (isclosed)
	{
		dis += pts[pts.size() - 1].Distance(pts[0]);
	}
	return dis;
}

/*****************************************************************************
* @brief : ?????????
* @author : W.W.Frank
* @date : 2015/11/30 14:55
* @version : version 1.0
* @inparam :
* @outparam :
*****************************************************************************/
void PointJointComputation::PointJointBeeline(double x, double y, double bln0x, double bln0y, double bln1x,
	double bln1y, double& ratio)
{
	double x0 = bln0x;
	double y0 = bln0y;
	double x1 = bln1x;
	double y1 = bln1y;
	double deltax = x1 - x0;
	double deltay = y1 - y0;
	double xp = x;
	double yp = y;

	if (deltay == 0 && deltax == 0)
		ratio = 0;
	else
		ratio = ((xp - x0) * deltax + (yp - y0) * deltay) / (deltax * deltax + deltay * deltay);
}

Point2D PointJointComputation::PointJointBeeline(double x, double y, double bln0x, double bln0y, double bln1x,
	double bln1y)
{
	double u;
	PointJointBeeline(x, y, bln0x, bln0y, bln1x, bln1y, u);
	double x0 = bln0x;
	double y0 = bln0y;
	double x1 = bln1x;
	double y1 = bln1y;
	double deltax = x1 - x0;
	double deltay = y1 - y0;

	return Point2D(x0 + u * deltax, y0 + u * deltay);
}

void PointProjection::PointProjectLatLngToUTM(double lat, double lng, int nZone, double &x, double &y)
{
	tsmLatLongToUTM(lat, lng, &nZone, &x, &y);
}