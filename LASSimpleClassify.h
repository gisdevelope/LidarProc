#pragma once
//
// Created by wuwei on 18-1-14.
//

#ifndef LASGUI_LASSIMPLECLASSIFY_H
#define LASGUI_LASSIMPLECLASSIFY_H

#include <vector>
#include "LASPoint.h"

#ifndef ColorClassInfo
typedef struct ColorClassInfo
{
	int red;
	int green;
	int blue;
	eLASClassification classType;
}ColoInfo;
#endif

class LASSimpleClassify {
public:
	/**
	* classify the point cloud by number of echo
	* @param dataset
	* @return
	*/
	long LASVegetationByEcho(ILASDataset* dataset);

	/**
	* classify data by elevation
	* @param dataset :lidar dataset
	* @param elevation :input elevation
	* @param direct :above or below the elevation(true above false below)
	* @param eclass :input the type
	* @return
	*/
	long LASClassifyByElevation(ILASDataset* dataset, float elevation, bool direct, eLASClassification eclass);


	/**
	* classufy data by intensity
	* @param dataset
	* @param elevation
	* @param direct
	* @param eclass
	* @return
	*/
	long LASClassifyByIntensity(ILASDataset* dataset, float intensity, bool direct, eLASClassification eclass);

	/**
	* classify data by RGB color
	* @param dataset :lidar dataset(in & out)
	* @param colorInfo : classify color info (in)
	* @return
	*/
	long LASClassifyByColor(ILASDataset* dataset, std::vector<ColoInfo> colorInfo);

    /**
     * export points by type
     * @param dataset
     * @param classType
     * @param pathOut
     * @return
     * TODO:
     */
    long LASClassifyByType(ILASDataset* dataset,eLASClassification classType,const char* pathOut);
};


class LASClassifyTower :public LASSimpleClassify
{
public:
	/*
	* get tower position
	* @param pathLocate :path
	*/
	long LASGetTowerLocation(const char* pathLocate);

	/**
	* tower range
	* @param dataset :lidar dataset(in & out)
	* @param range : classify color info (in)
	* @param rectTowers: rect towers
	* @return
	*/
	long LASTowerRect(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers);

	/**
	* classify tower roughly
	* @param dataset :lidar dataset(in & out)
	* @param rectTowers : range tower
	* @param heightThreshod: height thresthod
	* @return
	*/
	long LASTowerRough(ILASDataset* dataset, vector<Rect2D> rectTowers, float heightThreshod = 10);


	/**
	* classify tower refine
	* @param dataset :lidar dataset(in & out)
	* @param range : range tower
	* @param cubeDis : range cube
	* @param cubePoints: point thresthod
	* @return
	*/
	long LASTowerRefine(ILASDataset* dataset, double range, float cubeDis = 0.5, float cubePoints = 20);

protected:
	//tower position
	vector<Point2D> m_towerLocate;
};


class LASClassifyPaperTower:public LASClassifyTower
{
public:
	/**
	* tower plane
	* @param dataset :lidar dataset(in & out)
	* @param range : classify color info (in)
	* @param rectTowers: rect towers
	* @param abc : params
	* @return
	*/
	long LASTowerPlaneFit(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers, double *abc);

	/*
	*classify tower roughly
	*/
	long LASTowerRoughPlane(ILASDataset* dataset, vector<Rect2D> rectTowers, double* param, float heightThreshod = 10);
	
	/**
	* @ article
	*/
	long LASTowerRectExport(ILASDataset* dataset, vector<Rect2D> rectTowers, float cubeDis);

	/*
	* @ article
	* double *pca output first and second princpal component
	*/
	long LASRegionPCAXY(const char* pathRegion, double *pca);

	/*
	* @ article
	*/
	long LASRegionPCA(const char* pathDir);
};

#endif //LASGUI_LASSIMPLECLASSIFY_H

