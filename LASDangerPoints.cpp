//
// Created by wuwei on 18-1-24.
//

#include "LASDangerPoints.h"

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

	for (int i = 0; i<rectRangeIdx.size(); ++i)
	{
		int ind = rectRangeIdx[i];
		for (int j = 0; j <datasetVegterian->m_lasRectangles[ind].m_lasPoints_numbers; ++j) {
            const Point3D tmpPnt = datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_vec3d;
            if(datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_classify==elcDanger)
                continue;
            if(pnt->Distance(tmpPnt)<distance[0])
            {
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_classify = elcDanger;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Red=255;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Green=0;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Blue=0;
            }
            else if(pnt->Distance(tmpPnt)<distance[1])
            {
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_classify = elcDanger;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Red=255;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Green=255;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Blue=0;
            }
            else if(pnt->Distance(tmpPnt)<distance[2])
            {
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_classify = elcDanger;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Red=0;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Green=0;
                datasetVegterian->m_lasRectangles[ind].m_lasPoints[j].m_colorExt.Blue=255;
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
    return rs;
}

long LASDangerPoints::LADDangerPoints_SplitDanger(ILASDataset *datasetVegterain, const char *pathSplit)
{
	LidarMemReader lidarOpt;
	lidarOpt.LidarReader_Export(pathSplit, datasetVegterain, (int)elcDanger);
	return 0;
}