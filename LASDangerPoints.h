#pragma once
//
// Created by wuwei on 18-1-24.
//

#ifndef LASGUI_LASDANGERPOINTS_H
#define LASGUI_LASDANGERPOINTS_H

#include <vector>
#include "gdal/gdal_priv.h"

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
	 * danger level according distance
	 * @param distance
	 * @param dangerSectionNumber
	 * @param datasetLine
	 * @param datasetVegterain
	 * @return
	 */
	long LASDangerPoints_Detect(float* distance,int dangerSectionNumber,ILASDataset* datasetLine, ILASDataset* datasetVegterain);

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
	 * danger level according distance pre point
	 * @param distance
	 * @param dangerSectionNumber
	 * @param pnt
	 * @param datasetVegterian
	 * @return
	 */
	long LASDangerPoints_PerPoint(float* distance,int dangerSectionNumber, const Point3D* pnt, ILASDataset* datasetVegterian);

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

/*
 * detect falling trees
 * */
class LASFallingTreesDangerPoints: public LASDangerPoints
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
    float LASDangerPoints_Elevation(float* dataImg,int xsize,int ysize,double* adfGeoTrans,float gx,float gy);

    /**
     *
     * @param distance
     * @param dangerSectionNumber
     * @param pathDEM
     * @param datasetLine
     * @param datasetVegterain
     * @return
     */
    long LASDangerPoints_FallingTree(float* distance,int dangerSectionNumber,const char* pathDEM,ILASDataset* datasetLine, ILASDataset* datasetVegterain);

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
    long LASDangerPoints_FallingTree_PrePoint(float* distance,int dangerSectionNumber,float *demData,int xsize,int ysize,double *adfGeotrans, const Point3D* pnt, ILASDataset* datasetVegterian);

};

#endif //LASGUI_LASDANGERPOINTS_H
