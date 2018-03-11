#pragma once
//
// Created by wuwei on 18-1-24.
//

#ifndef LASGUI_LASDANGERPOINTS_H
#define LASGUI_LASDANGERPOINTS_H

#include <vector>

#include "Geometry.h"
#include "LASPoint.h"
#include "LASReader.h"

/**
* 获取线路危险点信息
*/
class LASDangerPoints {
public:
	/**
	* 根据距离获取线路危险点信息
	* @param distance
	* @return
	*/
	long LASDangerPoints_Detect(float distance, ILASDataset* datasetLine, ILASDataset* datasetVegterain);


	/**
	* 将危险点区分出来写如文件中
	* @param datasetVegterain
	* @param pathSplit
	* @return
	*/
	long LADDangerPoints_SplitDanger(ILASDataset* datasetVegterain, const char* pathSplit);

private:
	/**
	* 检测某一个点范围
	* @param distance
	* @param pnt
	* @param datasetVegterian
	* @return
	*/
	long LASDangerPoints_PerPoint(float distance, const Point3D* pnt, ILASDataset* datasetVegterian);

	/**
	* 获取危险点可能所在的范围
	* @param pnt
	* @param distance
	* @param datasetVegterain
	* @param rectIds
	* @return
	*/
	long LASDangerPoints_Range(const Point3D *pnt, float distance, ILASDataset* datasetVegterain, std::vector<int> &rectIds);
};


#endif //LASGUI_LASDANGERPOINTS_H
