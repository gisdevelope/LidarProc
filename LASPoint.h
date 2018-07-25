#pragma once
//
// Created by wuwei on 17-12-25.
//

#ifndef LASLIB_LASPOINT_H
#define LASLIB_LASPOINT_H
#include <string>
#include <functional>

#include "LASHeader.h"
#include "../LidarAlgorithm/Geometry.h"
#include "../LidarAlgorithm/GeometryFlann.h"
#include "../LidarBase/RTree.hpp"
#include<io.h>

using namespace std;
using namespace GeometryLas;
//struct LASPoint;
//typedef std::function<void(LASPoint*)> callback_operation_points_Ptr;

static unsigned char GetReturnNumber(unsigned char temp) { return (temp & 0x07) ; /*0x00000111*/ }
static unsigned char GetNumberOfReturns(unsigned char temp) { return (temp & 0x38) >> 3; /*00111000*/ }

//������
static bool GetScanDirectionFlag(unsigned char temp) { return ((temp & 0x02) >> 1) == 1 ? true : false; /*00000010*/ }
static bool GetEdgeOfFlightLine(unsigned char temp) { return temp & 0x01;	/*00000001*/ }

#ifndef _OUT_
#define _OUT_
#endif 

#ifndef _IN_
#define _IN_
#endif

#ifndef _INOUT_
#define _INOUT_
#endif

#ifndef COLORREF
#define COLORREF int
#endif

#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((unsigned char)(r)|((unsigned int)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(b))<<16)))
#endif

#ifndef ExRGB
#define ExRGB(color,extColor)							\
{														\
	extColor.Red = (color << 24) >> 24 & 0x000000ff;	\
	extColor.Green = (color << 16) >> 24 & 0x000000ff;	\
	extColor.Blue = (color << 8) >> 24 & 0x000000ff;	\
}
#endif

/*Las1.2��ɫ��չ*/
#pragma pack(1)
struct LASColorExt
{
	unsigned short Red;
	unsigned short Green;
	unsigned short Blue;
};
#pragma pack()

//las���ļ�
#pragma pack(1)/*�ֽڶ���*/
class LASPoint
{
public:
	/*
	* ��д
	*/
	void Write(FILE *fs, const LASHeader& info) const;
	void Read(FILE *fs, const LASHeader& info);

	/**
	* �ڴ��н����������ĵ�������
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


/*
	adaptor it's hard to adjust the 
*/
struct PointCloudBlockAdaptor
{
	std::vector<LASPoint> lasPoints;

	//reload operator [] to keep the code with same format
	inline void push_back(LASPoint pnt) { lasPoints.push_back(pnt); }

	//reload operator [] to keep the code with same format
	inline LASPoint &operator[](int i) { return lasPoints[i]; }

	//clear data
	inline void clear() { lasPoints.clear(); }

	// must return the number of data points
	//(for adatopr the function must be relized)
	inline size_t kdtree_get_point_count() const { return lasPoints.size(); }

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline double kdtree_get_pt(const size_t idx, int dim) const
	{
		if (dim == 0) return lasPoints[idx].m_vec3d.x;
		else if (dim == 1) return lasPoints[idx].m_vec3d.y;
		else return lasPoints[idx].m_vec3d.z;
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
};

typedef RTree<int, double, 2, double, 4>  LASBlockTree;
typedef PointCloudBlockAdaptor PCBlockAdaptor;
typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<double, PCBlockAdaptor>, PCBlockAdaptor, 3> kd_tree_block;

/*����ز��ź�*/
enum  eLASEcho
{
	eLidarEchoOnly = 0,
	eLidarEchoFirst = 1,
	eLidarEchoMidian = 2,
	eLidarEchoLast = 3
};
/*���Ƶ����*/
#pragma pack(1)
enum  eLASClassification
{
	elcCreated			 = 0,	// �����ģ�û�б������
	elcUnclassified		 = 1,	// �����ģ����޷�ʶ�����ĵ�
	elcGround			 = 2,	// �����
	elcLowVegetation	 = 3,	// ����ֲ��
	elcMediumVegetation  = 4,	// �еȸ߶ȵ�ֲ��
	elcHighVegetation	 = 5,	// �ߵ�ֲ��
	elcBuilding			 = 6,	// ������
	elcLowPoint			 = 7,	// ���ڵر�ĵ㣨������
	elcModelKeyPoint	 = 8,	// ���Ƶ�
	elcWater			 = 9,	// ˮ
	elcOverlapPoint		 = 12,	// �����ص���

	elcDanger			 = 13,	//
	elcDangerLevel1		 = 14,	// 
	elcDangerLevel2		 = 15,
	elcDangerLevel3		 = 16,
	//...ext
	elcDangerEnd		 = 23,

	elcTowerRange	 	 = 24,		// 
	elcDriveWay			 = 25,      //��·

	elcFallingTree		 = 30,
	elcFallingTreeLevel1 = 31,
	elcFallingTreeLevel2 = 32,
	elcFallingTreeLevel3 = 33,
	elcFallingTreeEnd	 = 34,

	elcDeletedPoint		 = -1	// ��ɾ���ĵ�
};
#pragma pack()

static eLASClassification GetLidarClassification(unsigned char clsType)
{
	return (eLASClassification)clsType;
}

#pragma pack(1)/*�ֽڶ���*/
struct LASIndex
{
	int rectangle_idx;
	int point_idx_inRect;
};
#pragma pack()

/*
* �����ļ���
* version 1.2
* author: Frank.Wu
* �ֿ����������KD������
*/
class LASRectBlock {
public:
	LASRectBlock() { m_lasPoints.clear(); m_lasPoints_numbers = 0; m_block_tree = nullptr; }
	~LASRectBlock() {
		if (!m_lasPoints.lasPoints.empty())
			m_lasPoints.clear();
		if (m_block_tree != nullptr)
			delete m_block_tree;
		m_block_tree = nullptr;
		m_lasPoints_numbers = 0;
	}

	/*****************************************************************************
	* @brief : �����ڴ�,�ж��Ƿ񽫵��������ڴ��У�����ֻ������
	* @author : W.W.Frank
	* @date : 2015/11/29 20:02
	* @version : version 1.0
	* @inparam :
	* @outparam :
	*****************************************************************************/
	void LASRect_AllocateMemory(int lasPoints, bool inMemory, Rect2D rect);


	/*****************************************************************************
	* @brief : ��ÿ���齨��kd������
	* @author : W.W.Frank
	* @date : 2018.07.24
	* @version : version 1.1
	* @inparam :
	* @outparam :
	*****************************************************************************/
	void LASRectBuildTree();


public:
	Point3D			 m_rectCenter;
	Rect2D			 m_Rectangle;
	long long		 m_lasPoints_numbers;

	//construct the kd tree for each point
	kd_tree_block*	 m_block_tree;
	PCBlockAdaptor	 m_lasPoints;
};

/**
* �������ݼ�
*/
class ILASDataset {

public:
	ILASDataset();
	~ILASDataset();

	//����R���Ĺ���
	long LASDataset_BuildTree();
	//�����ڴ棬�Ƿ����ڴ��з��䣬����ֻ�Ƕ�ȡindex
	void LASDataset_AllocateMemory(int lasRects);
	//�����ݽ�������
	void LASDataset_Trim(bool inMemory);

	//��������û��̫���
	//bool LASDataset_Iterator(callback_operation_points_Ptr ptrFun);
	bool LASDataset_FixHeader();

	//�ҵ�ƥ��ľ��ε�id������id��ȡ���ĸ�������
	bool LASDataset_Search(int rectID, Rect2D  searchRect, vector<int> &rects);
	bool LASDataset_Search(int rectID, Point3D searchPnt, vector<int> &rects);

	//����˳�δ����ȡ��ά��
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
	LASIndex           *m_LASPointID; //ȫ�ֵ��ھֲ������еı��
};



/*
to meet the require of flann
using adaptor mode
*/
template<typename PCDrived>
struct PointCloudAdaptor
{
	//constructor
	PointCloudAdaptor(const PCDrived &obj_) : obj(obj_) { }

	const PCDrived &obj; //!using reference to save mem and spped up

						 //get datasource using inline to speed up
	inline const PCDrived& derived() const { return obj; }

	// must return the number of data points
	//(for adatopr the function must be relized)
	inline size_t kdtree_get_point_count() const { return derived().size(); }

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline double kdtree_get_pt(const size_t idx, int dim) const
	{
		if (dim == 0) return derived()[idx].x;
		else if (dim == 1) return derived()[idx].y;
		else return derived()[idx].z;
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
};


#endif //LASLIB_LASPOINT_H
