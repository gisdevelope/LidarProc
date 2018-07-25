#pragma once
//
// Created by wuwei on 18-1-5.
//

#ifndef LASGUI_LASMODEL_H
#define LASGUI_LASMODEL_H

#include <vector>
#include <stdio.h>
#include <memory>
#include "../LidarAlgorithm/Geometry.h"
using namespace GeometryLas;

//���ܶȺ�ģ�͵Ķ���Ķ���
struct DenseSt {
	Point3I point;
	int dense;
};

#define  LASDense std::vector<DenseSt>

class ILASDataset;

/**
* ������LAS���ݼ�������ģ��
*/
class LASModel
{
public:
	/**
	* ���ݵ������ݹ������ܶ�ģ��
	* @param dataset
	* @param cubeRange
	*/
	virtual void LASModel_PointsDense(ILASDataset* dataset, float cubeRange);

	/**
	* ���ݵ��ܶ�ģ�͹���ģ��
	*/
	void LASModel_Build(ILASDataset* dataset, float cubeRange, const char* pathExport);

private:
	LASDense m_model;
};



#endif //LASGUI_LASMODEL_H
