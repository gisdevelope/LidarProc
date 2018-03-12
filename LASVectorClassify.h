#pragma once
#ifndef _LAS_VECTORCLASSIFY_H_
#define _LAS_VECTORCLASSIFY_H_

#include <map>
#include "ogrsf_frmts.h"
#include "LASSimpleClassify.h"
#include "LASPoint.h"


class LASVectorClassify : public LASSimpleClassify
{
public:
	/**
	* Rect2D transform GDAL OGREnvelope
	* @param rect
	* @return
	*/
	OGREnvelope LASRectToEnvelope(Rect2D rect);

	/**
	* OGREnvelope transform Rect2D
	* @param m_Envelope
	* @return
	*/
	Rect2D LASEnvelopToRect(OGREnvelope *m_Envelope);

	/**
	* name tpye mapping
	* vector<string> nameKey
	* vector<eLASClassification>typeKey
	*/
	bool  LASSetMaps(vector<string> nameKey, vector<eLASClassification>typeKey);

	/**
	* Las classify by shp data
	* @param pathLas
	* @param pShpData
	* @param pathOutLas
	* @return
	*/
	virtual void LASClassifyByVector(const char* pathLas, map<string,string> pShpPath, const char* pathOutLas)=0;


	bool TestGeos();

protected:
	/*
	* trans polygon to points
	* Point2Ds points
	 * todo:
	*/
	virtual void VectorPointsToPolygon(Point2Ds points, OGRPolygon* poPolygon);

	virtual void VectorPolygonToPoints(OGRPolygon* poPolygon, Point2Ds &points);

	std::map<string, int> name_type_maps;
};


class LASShpClassify : public LASVectorClassify
{
public:

	/**
	* Las classify by shp data
	* @param pathLas
	* @param pShpPath
	* @param pathOutLas
	* @return
	*/
	void LASClassifyByVector(const char* pathLas, map<string,string> pShpPath, const char* pathOutLas);

};

#endif
