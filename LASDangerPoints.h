#pragma once
//
// Created by wuwei on 18-1-24.
// email��wuwei_cug@163.com

#ifndef LASGUI_LASDANGERPOINTS_H
#define LASGUI_LASDANGERPOINTS_H

#include <vector>

#include "../LidarAlgorithm/Geometry.h"
#include"../LidarAlgorithm/GeometryFlann.h"
#include "../LidarBase/LASPoint.h"
#include "../LidarBase/LASReader.h"
using namespace GeometryLas;

//pre define complex flann type
typedef PointCloudAdaptor<std::vector<Point3D>> PCAdaptor;
typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<double, PCAdaptor>, PCAdaptor, 3> kd_tree;


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
	long LASDangerPoints_Detect(float* distance, int dangerSectionNumber, ILASDataset* datasetLine, ILASDataset* datasetVegterain);

private:
	/**
	* ��Σ�յ����ֳ���д���ļ���
	* @param datasetVegterain
	* @param pathSplit
	* @return
	* deleted by Frank.Wu ͨ��ֱ��ͨ��д������Σ�յ�д��
	long LADDangerPoints_SplitDanger(ILASDataset* datasetVegterain, const char* pathSplit);
	*/

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
	long LASDangerPoints_PerPoint(float* distance, int dangerSectionNumber, const Point3D* pnt, ILASDataset* datasetVegterian);

protected:
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

/*
	��ȡ��·Σ�յ���Ϣ��ͨ��ANN�㷨���м���ʵ�֣�����ڼ򵥷ֿ�
	����kdtree�㷨�ܹ��������ߴ���Ч��
	***����Adaptor�Լ����������Ĺ�������ڴ����������Ҫ�ϴ��ڴ�***
	***һ����˵��Ҫ�������ƴ�С���������ڴ���ܹ����д��������ܲ����ڴ治�����***
	Created by Frank.Wu 2018-07-19
*/
class LASDangerPointsFlann
{
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
	long LASDangerPoints_Detect(float* distance, int dangerSectionNumber, ILASDataset* datasetLine, ILASDataset* datasetVegterain);

private:
	/**
	* ���ĳһ���㷶Χ
	* @param distance
	* @param pnt
	* @param treeVege
	* @return
	*/
	long LASDangerPoints_PerPoint(float distance, const Point3D* pnt, kd_tree &treeVege, ILASDataset* datasetVegterain);

	/**
	* danger level according distance pre point
	* @param distance
	* @param dangerSectionNumber
	* @param pnt
	* @param datasetVegterian
	* @return
	*/
	long LASDangerPoints_PerPoint(float* distance, int dangerSectionNumber, const Point3D* pnt, kd_tree &treeVege, ILASDataset* datasetVegterain);
};

/*
* detect falling trees
* */
class LASFallingTreesDangerPoints : public LASDangerPoints
{
public:
	/**
	* get height at the placce
	* @param dataImg: dem data
	* @param xsize :xsize of the dem image
	* @param ysize :ysize of the dem image
	* @param adfGeoTrans :geo transform of the dem image
	* @param gx :geo position x
	* @param gy :geo posision y
	* @return height
	*/
	float LASDangerPoints_Elevation(float* dataImg, int xsize, int ysize, double* adfGeoTrans, float gx, float gy);

	/**
	*
	* @param distance
	* @param dangerSectionNumber
	* @param pathDEM
	* @param datasetLine
	* @param datasetVegterain
	* @return error code
	*/
	long LASDangerPoints_FallingTree(float* distance, int dangerSectionNumber, const char* pathDEM, ILASDataset* datasetLine, ILASDataset* datasetVegterain);

private:
	/**
	*
	* @param distance
	* @param dangerSectionNumber
	* @param demData
	* @param xsize
	* @param ysize
	* @param pnt
	* @param datasetVegterian
	* @return
	*/
	long LASDangerPoints_FllingTree_PrePoint(float* distance, int dangerSectionNumber, float *demData, int xsize, int ysize,
		double *adfGeotrans, const Point3D* pnt, ILASDataset* datasetVegterian);
};


/*
	merge danger point datset to discrete points
	using dbscan algorithm to process
	get danger points and set classify discret distance
*/
class LASDangerPointsMergeArrgegate
{
public:
	/*
		extract danger points from lastadaset
		@param:asDataset:las dataset;
		@param:dangerPnts:danger points extract from dataset
	*/
	long LASDangerExtract(ILASDataset* lasDataset, Point3Ds &dangerPnts);

	/*
		extract correspond line point with the danger points
		@param:lineDataset line point set;
		@param:dangerPnts danger points;
		@linePnts:correspond line points
	*/
	long LASDangerExtractLinePoints(ILASDataset *lineDataset, Point3Ds dangerPnts, Point3Ds &linePnts);
	
	/*
		aggregate points set as a distance using knn
		@param:dangerPnts danger point set;
		@param:type aggregate type(directly using nature number)
		@param:knnRange range distance
	*/
	long LASDangerAggregate(Point3Ds dangerPnts,int *type,float knnRange);

	/*
		merge one type to one point and find the correspond line points
		@param dangerPnts:danger points
		@param type:class types
		@param typeNumbers:type numbers
		@pntDiscrete:point with the correspond line pts
		@correspond:correspond danger points and line point 
	*/
	long LASDangerMerge(Point3Ds dangerPnts, int *type, int typeNumbers, Point3Ds linePnts, int *correspondPairs);

	//for test
	long LASDangerTestFlann();
};

#endif //LASGUI_LASDANGERPOINTS_H
