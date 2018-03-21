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
* ��ȡ��·Σ�յ���Ϣ
*/
class LASDangerPoints {
public:
	/**
	* ���ݾ����ȡ��·Σ�յ���Ϣ
	* @param distance
	* @return
	*/
	long LASDangerPoints_Detect(float distance, ILASDataset* datasetLine, ILASDataset* datasetVegterain);


	/**
	 * danger level according distance
	 * @param distance
	 * @param dangerSectionNumber
	 * @param datasetLine
	 * @param datasetVegterain
	 * @return
	 */
	long LASDangerPoints_Detect(float* distance,int dangerSectionNumber,ILASDataset* datasetLine, ILASDataset* datasetVegterain);

	/**
	* ��Σ�յ����ֳ���д���ļ���
	* @param datasetVegterain
	* @param pathSplit
	* @return
	*/
	long LADDangerPoints_SplitDanger(ILASDataset* datasetVegterain, const char* pathSplit);

private:
	/**
	* ���ĳһ���㷶Χ
	* @param distance
	* @param pnt
	* @param datasetVegterian
	* @return
	*/
	long LASDangerPoints_PerPoint(float distance, const Point3D* pnt, ILASDataset* datasetVegterian);

	/**
	 * danger level according distance pre point
	 * @param distance
	 * @param dangerSectionNumber
	 * @param pnt
	 * @param datasetVegterian
	 * @return
	 */
	long LASDangerPoints_PerPoint(float* distance,int dangerSectionNumber, const Point3D* pnt, ILASDataset* datasetVegterian);

	/**
	* ��ȡΣ�յ�������ڵķ�Χ
	* @param pnt
	* @param distance
	* @param datasetVegterain
	* @param rectIds
	* @return
	*/
	long LASDangerPoints_Range(const Point3D *pnt, float distance, ILASDataset* datasetVegterain, std::vector<int> &rectIds);
};


#endif //LASGUI_LASDANGERPOINTS_H
