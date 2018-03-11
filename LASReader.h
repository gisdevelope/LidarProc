#pragma once
//
// Created by wuwei on 17-12-26.
//

#ifndef LASLIB_LASREADER_H
#define LASLIB_LASREADER_H

#include <stdio.h>
#include "LASHeader.h"
#include "LASPoint.h"

class ILASDataset;

/*??дlidar???????*/
class LASReader
{
public:
	LASReader() { m_isDatasetOpen = false; m_lasFile = nullptr; }
	~LASReader() {
		m_isDatasetOpen = false;
		if (m_lasFile != nullptr)
			fclose(m_lasFile);
	}

	//1.??las???
	virtual long LidarReader_Open(const char* pathLidar, ILASDataset* dataset) = 0;
	//2.???las???
	virtual long LidarReader_Read(bool inMemory, int skip, ILASDataset* dataset) = 0;
	//3.??las???д??
	virtual long LidarReader_Write(const char* pathLidar, ILASDataset* dataset) = 0;
	//4.?????????
	//virtual void LidarReader_SetProgress(GDALProcessBase* progress) { m_Progress = progress; }

protected:
	//?????las????????
	FILE *m_lasFile;
	bool m_isDatasetOpen;
	//GDALProcessBase		*m_Progress;
};

class LidarMemReader : public  LASReader
{
public:
	//1.??las???
	long LidarReader_Open(const char* pathLidar, ILASDataset* dataset);
	//2.???las???
	long LidarReader_Read(bool inMemory, int skip, ILASDataset* dataset);
	//3.??las???д??
	long LidarReader_Write(const char* pathLidar, ILASDataset* dataset);
	//4.??????????????????
	long LidarReader_Export(const char* pathLidar, ILASDataset* dataset, int classType);
};

class LidarPatchReader : public LidarMemReader
{
public:

	/*split las data into patches
	 * const char* pathLas
	 * const char* pathSplitDir
	 * int splitSize
	 * */
	long LidarReader_SplitPatch(const char* pathLas,const char* pathSplitDir,int splitSize/*MB*/);

	/**
	* Read las header
	* @param lf
	* @param lasHeader
	* @return
	*/
	long LidarReader_ReadHeader(FILE* lf, LASHeader &lasHeader);

	/**
	* дLAS?
	* @param lf
	* @param lasHeader
	* @return
	*/
	long LidarReader_WriteHeader(FILE* lf, LASHeader lasHeader);

	/**
	* ??????Patch
	* @param lf
	* @param lasHeader
	* @param points
	* @param number
	* @return patch???Χ
	*/
	Rect2D LidarReader_ReadPatch(FILE* lf, LASHeader lasHeader, LASPoint* points, int &number);

	/**
	* д???Patch
	* @param lf
	* @param lasHeader
	* @param points
	* @param number
	* @return patch???Χ
	*/
	long LidarReader_WritePatch(FILE* lf, LASHeader lasHeader, LASPoint* points, int number);
};
#endif //LASLIB_LASREADER_H000
