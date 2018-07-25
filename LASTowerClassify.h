#pragma once

#include"LASSimpleClassify.h"

/**
classify tower points from las dataset
author:wuwei
version:1.0.0.0
*/
class LASClassifyTower :public LASSimpleClassify
{
public:

#ifdef _ARTICLE__USE_
	/*
	* get tower position
	* @param pathLocate :path
	*/
	long LASGetTowerLocation(const char* pathLocate);

	/*
	get tower location by pre define
	*/
	long LASGetTowerLocation();

	/*
	get tower localtion test
	*/
	long LASTowerRoughTest();

#endif
	/**
	* desc£ºtower range
	* @param dataset :lidar dataset(in & out)
	* @param range : classify color info (in)
	* @param rectTowers: rect towers
	* @return
	*/
	long LASTowerRect(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers);

	/**
	* tower plane
	* @param dataset :lidar dataset(in & out)
	* @param range : classify color info (in)
	* @param rectTowers: rect towers
	* @param abc : params
	* @return
	*/
	long LASTowerPlaneFit(ILASDataset* dataset, double range, vector<Rect2D> &rectTowers, double *abc);

	/**
	* classify tower roughly
	* @param dataset :lidar dataset(in & out)
	* @param rectTowers : range tower
	* @param heightThreshod: height thresthod
	* @return
	*/
	long LASTowerRough(ILASDataset* dataset, vector<Rect2D> rectTowers, float heightThreshod = 10);

	/*
	*classify tower roughly
	*/
	long LASTowerRoughPlane(ILASDataset* dataset, vector<Rect2D> rectTowers, double* param, float heightThreshod = 10);

	/**
	* classify tower refine
	* @param dataset :lidar dataset(in & out)
	* @param range : range tower
	* @param cubeDis : range cube
	* @param cubePoints: point thresthod
	* @return
	*/
	long LASTowerRefine(ILASDataset* dataset, double range, float cubeDis = 0.5, float cubePoints = 20);

	/**
	* classify tower refine for classified points
	* @param dataset :lidar dataset(in & out)
	* @param range : range tower
	* @param cubeDis : range cube
	* @param cubePoints: point thresthod
	* @return
	*/
	long LASTowerRefineClassified(ILASDataset* dataset, double range, float cubeDis = 0.5, float cubePoints = 20);

	/**
	* @ article
	*/
	long LASTowerRectExport(ILASDataset* dataset, vector<Rect2D> rectTowers, float cubeDis);

private:
	//tower position
	vector<Point2D> m_towerLocate;
};

