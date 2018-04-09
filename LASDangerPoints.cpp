//
// Created by wuwei on 18-1-24.
//

#include "LASDangerPoints.h"

static void SetPointColor(LASPoint &pnt, short r,short g,short b)
{
    pnt.m_colorExt.Red = r;
    pnt.m_colorExt.Green=g;
    pnt.m_colorExt.Blue =b;
}

long LASDangerPoints::LASDangerPoints_Range(const Point3D *pnt, float distance, ILASDataset* datasetVegterain, vector<int> &rectIds)
{
	Rect2D rect;
	rect.minx = pnt->x - distance; rect.maxx = pnt->x + distance;
	rect.miny = pnt->y - distance; rect.maxy = pnt->y + distance;
	datasetVegterain->LASDataset_Search(0, rect, rectIds);
	return 0;
}

long LASDangerPoints::LASDangerPoints_PerPoint(float distance, const Point3D *pnt, ILASDataset *datasetVegterian)
{
	vector<int> rectRangeIdx;
	LASDangerPoints_Range(pnt, distance, datasetVegterian, rectRangeIdx);

	for (int i = 0; i<rectRangeIdx.size(); ++i)
	{
		int ind = rectRangeIdx[i];
		for (int j = 0; j <datasetVegterian->m_lasRectangles[ind].m_lasPoints_numbers; ++j) {
			const Point3D tmpPnt = datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_vec3d;
			if (pnt->Distance(tmpPnt)<distance) {
				datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_classify = elcDanger;
			}
		}
	}

	return 0;
}

long LASDangerPoints::LASDangerPoints_PerPoint(float *distance, int dangerSectionNumber, const Point3D *pnt,
											   ILASDataset *datasetVegterian) {
	vector<int> rectRangeIdx;
	LASDangerPoints_Range(pnt, distance[dangerSectionNumber-1], datasetVegterian, rectRangeIdx);
    if(dangerSectionNumber!=3)
    {
        printf("plz input 3 number range\n");
        return -1;
    }

    //only consider 3 type of classes
	for (int i = 0; i<rectRangeIdx.size(); ++i)
	{
		int ind = rectRangeIdx[i];
		for (int j = 0; j <datasetVegterian->m_lasRectangles[ind].m_lasPoints_numbers; ++j) {

            LASPoint &laspnt = datasetVegterian->m_lasRectangles[ind].m_lasPoints[j];
            const Point3D tmpPnt = laspnt.m_vec3d;
            int classType = laspnt.m_classify;

            if(classType>elcDanger&&classType<elcDangerEnd)
            {
                if(pnt->Distance(tmpPnt)<distance[0]&&classType>elcDanger+1)
                {
                    laspnt.m_classify = elcDangerLevel1;
                    SetPointColor(laspnt,255,0,0);
                }
                else if(pnt->Distance(tmpPnt)<distance[1]&&classType>elcDanger+2)
                {
                    laspnt.m_classify = elcDangerLevel2;
                    SetPointColor(laspnt,255,255,0);
                }
                else if(pnt->Distance(tmpPnt)<distance[2]&&classType>elcDanger+3)
                {
                    laspnt.m_classify = elcDanger;
                    SetPointColor(laspnt,0,0,255);
                }
            }
            else
            {
                if(pnt->Distance(tmpPnt)<distance[0])
                {
                    laspnt.m_classify = elcDangerLevel1;
                    SetPointColor(laspnt,255,0,0);
                }
                else if(pnt->Distance(tmpPnt)<distance[1])
                {
                    laspnt.m_classify = elcDangerLevel2;
                    SetPointColor(laspnt,255,255,0);
                }
                else if(pnt->Distance(tmpPnt)<distance[2])
                {
                    laspnt.m_classify = elcDanger;
                    SetPointColor(laspnt,0,0,255);
                }
            }
        }
    }
    return 0;
}

long LASDangerPoints::LASDangerPoints_Detect(float distance, ILASDataset *datasetLine, ILASDataset *datasetVegterain)
{
	int rs = 0;
	int numLineRects = datasetLine->m_numRectangles;
	for (int i = 0; i<numLineRects; ++i)
	{
		int numPntInRect = datasetLine->m_lasRectangles[i].m_lasPoints_numbers;
		for (int j = 0; j<numPntInRect; ++j)
		{
			printf("\r%d-%d", numPntInRect, j);
			const Point3D linePnt = datasetLine->m_lasRectangles[i].m_lasPoints[j].m_vec3d;
			rs = rs | LASDangerPoints_PerPoint(distance, &linePnt, datasetVegterain);
		}
		printf("\n");
	}
	return rs;
}

/*
在检测的过程中存在一个问题：
对于每一个植被点，可能存在多个导线点与其距离小于最大的阈值
但是不小于最小的阈值，但是在判断过程中随着导线点的变化，对
同一个植被点的判断可能出现变化，由此导致判断出错，因此在这
需要进行改进
*/

long LASDangerPoints::LASDangerPoints_Detect(float* distance,int dangerSectionNumber,ILASDataset* datasetLine, ILASDataset* datasetVegterain){
    int rs = 0;
    int numLineRects = datasetLine->m_numRectangles;
    for (int i = 0; i<numLineRects; ++i)
    {
        int numPntInRect = datasetLine->m_lasRectangles[i].m_lasPoints_numbers;
        for (int j = 0; j<numPntInRect; ++j)
        {
            printf("\r%d-%d", numPntInRect, j);
            const Point3D linePnt = datasetLine->m_lasRectangles[i].m_lasPoints[j].m_vec3d;
            rs = rs | LASDangerPoints_PerPoint(distance,dangerSectionNumber, &linePnt, datasetVegterain);
        }
        printf("\n");
    }
    //trim to elcDanger to export and process
    for(int i=0;i<datasetVegterain->m_numRectangles;++i)
    {
        int numPntInRect = datasetVegterain->m_lasRectangles[i].m_lasPoints_numbers;
        for (int j = 0; j<numPntInRect; ++j)
        {
            LASPoint &pnt = datasetVegterain->m_lasRectangles[i].m_lasPoints[j];
            int classType = pnt.m_classify;
            if(classType>elcDanger&&classType<elcDangerEnd){
                pnt.m_classify = elcDanger;
            }
        }
    }

    return rs;
}

long LASDangerPoints::LADDangerPoints_SplitDanger(ILASDataset *datasetVegterain, const char *pathSplit)
{
	LidarMemReader lidarOpt;
	lidarOpt.LidarReader_Export(pathSplit, datasetVegterain, (int)elcDanger);
	return 0;
}