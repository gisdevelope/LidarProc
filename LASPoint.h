#pragma once
//
// Created by wuwei on 17-12-25.
//

#ifndef LASLIB_LASPOINT_H
#define LASLIB_LASPOINT_H
#include <string>
#include <functional>

#include "LASHeader.h"
#include "Geometry.h"
#include "RTree.hpp"

using namespace std;

static void getFiles(string cate_dir,vector<string> files)
{
#ifdef WIN32
    _finddata_t file;
    long lf;
    if ((lf=_findfirst(cate_dir.c_str(), &file)) == -1) {
        cout<<cate_dir<<" not found!!!"<<endl;
    } else {
        while(_findnext(lf, &file) == 0) {
            //cout<<file.name<<endl;
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            files.push_back(file.name);
        }
    }
    _findclose(lf);
#endif

#ifdef linux
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    if ((dir=opendir(cate_dir.c_str())) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8)    ///file
            //printf("d_name:%s/%s\n",basePath,ptr->d_name);
            files.push_back(ptr->d_name);
        else if(ptr->d_type == 10)    ///link file
            //printf("d_name:%s/%s\n",basePath,ptr->d_name);
            continue;
        else if(ptr->d_type == 4)    ///dir
        {
            files.push_back(ptr->d_name);
            /*
                memset(base,'\0',sizeof(base));
                strcpy(base,basePath);
                strcat(base,"/");
                strcat(base,ptr->d_nSame);
                readFileList(base);
            */
        }
    }
    closedir(dir);
#endif
    //???????С????????
    sort(files.begin(), files.end());
}

//struct LASPoint;
//typedef std::function<void(LASPoint*)> callback_operation_points_Ptr;

static unsigned char GetReturnNumber(unsigned char temp) { return (temp & 0x07) ; /*0x00000111*/ }
static unsigned char GetNumberOfReturns(unsigned char temp) { return (temp & 0x38) >> 3; /*00111000*/ }

//待测试
static bool GetScanDirectionFlag(unsigned char temp) { return ((temp & 0x02) >> 1) == 1 ? true : false; /*00000010*/ }
static bool GetEdgeOfFlightLine(unsigned char temp) { return temp & 0x01;	/*00000001*/ }

#ifndef COLORREF
#define COLORREF int
#endif

#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((unsigned char)(r)|((unsigned int)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(b))<<16)))
#endif

typedef RTree<int, double, 2, double, 4>  LASBlockTree;

/*激光回波信号*/
enum  eLASEcho
{
	eLidarEchoOnly = 0,
	eLidarEchoFirst = 1,
	eLidarEchoMidian = 2,
	eLidarEchoLast = 3
};
/*点云的类别*/
#pragma pack(1)
enum  eLASClassification
{
	elcCreated			 = 0,	// 创建的，没有被分类的
	elcUnclassified		 = 1,	// 无类别的，或无法识别类别的点
	elcGround			 = 2,	// 地面点
	elcLowVegetation	 = 3,	// 矮的植被
	elcMediumVegetation  = 4,	// 中等高度的植被
	elcHighVegetation	 = 5,	// 高的植被
	elcBuilding			 = 6,	// 建筑物
	elcLowPoint			 = 7,	// 低于地表的点（噪音）
	elcModelKeyPoint	 = 8,	// 控制点
	elcWater			 = 9,	// 水
	elcOverlapPoint		 = 12,	// 航带重叠点
	elcDanger			 = 13,	// 线路危险点
	elcTowerRange		 = 14,	// 杆塔范围
	elcDeletedPoint		 = -1	// 已删除的点
};
#pragma pack()

static eLASClassification GetLidarClassification(unsigned char clsType)
{
	return (eLASClassification)clsType;
}
/*Las1.2颜色扩展*/
#pragma pack(1)
struct LASColorExt
{
	unsigned short Red;
	unsigned short Green;
	unsigned short Blue;
};
#pragma pack()

//las点文件
#pragma pack(1)/*字节对齐*/
class LASPoint
{
public:
	/*
	* 读写
	*/
	void Write(FILE *fs, const LASHeader& info) const;
	void Read(FILE *fs, const LASHeader& info);

	/**
	* 内存中解析出单个的点云数据
	* @param data
	* @param info
	*/
	void ExtractFromBuffer(const unsigned char* data, const LASHeader& info);
	void ExportToBuffer(unsigned char* data, const LASHeader& info) const;

public:
	Point3D			m_vec3d;
	unsigned short  m_intensity;
	unsigned char   m_rnseByte;
	char			m_classify;
	char			m_scanAngle;
	unsigned char	m_userdata;
	unsigned short  m_flightID;
	double			m_gpsTime;
	LASColorExt		m_colorExt;
};
#pragma pack()

#pragma pack(1)/*字节对齐*/
struct LASIndex
{
	int rectangle_idx;
	int point_idx_inRect;
};
#pragma pack()

/*
* 点云文件块
*/
class LASRectBlock {
public:
	LASRectBlock() { m_lasPoints = NULL; m_lasPoints_numbers = 0; }
	~LASRectBlock() {
		if (m_lasPoints != NULL)
			delete[]m_lasPoints;
		m_lasPoints = NULL;
		m_lasPoints_numbers = 0;
	}

	/*****************************************************************************
	* @brief : 分配内存,判断是否将点分配进入内存中，或者只存索引
	* @author : W.W.Frank
	* @date : 2015/11/29 20:02
	* @version : version 1.0
	* @inparam :
	* @outparam :
	*****************************************************************************/
	void LASRect_AllocateMemory(int lasPoints, bool inMemory, Rect2D rect);

public:
	Point3D     m_rectCenter;
	Rect2D      m_Rectangle;
	LASPoint*   m_lasPoints;
	long long   m_lasPoints_numbers;
};

/**
* 点云数据集
*/
class ILASDataset {

public:
	ILASDataset();
	~ILASDataset();

	//构建R树的过程
	long LASDataset_BuildTree();
	//分配内存，是否在内存中分配，或者只是读取index
	void LASDataset_AllocateMemory(int lasRects);
	//对数据进行整理
	void LASDataset_Trim(bool inMemory);

	//遍历函数没想太清楚
	//bool LASDataset_Iterator(callback_operation_points_Ptr ptrFun);


	//找到匹配的矩形的id，根据id获取在哪个矩形中
	bool LASDataset_Search(int rectID, Rect2D  searchRect, vector<int> &rects);
	bool LASDataset_Search(int rectID, Point3D searchPnt, vector<int> &rects);

	//根据顺次次序获取三维点
	bool LASDataset_Search(int pointID, Point3D &searchPnt);

public:
	LASHeader			m_lasHeader;
	LASVariableRecord*	m_lasvarHeader;
	LASVariable_header_geo_keys  m_lasgeokeyHeader;
	LASVariable_header_key_entry m_lasgeoentryHeader;

	double              m_xrange[2], m_yrange[2], m_zrange[2];
	Point3D			    m_SetCenter;
	LASRectBlock       *m_lasRectangles;
	LASBlockTree		m_lasBlockTree;
	int					m_numRectangles;
	int                 m_totalReadLasNumber;
	LASIndex           *m_LASPointID; //全局点在局部矩形中的编号
};

#endif //LASLIB_LASPOINT_H
