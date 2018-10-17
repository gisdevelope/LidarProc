#pragma once
//
// Created by wuwei on 17-12-25.
//

#ifndef LASLIB_LASHEADER_H
#define LASLIB_LASHEADER_H


#include <stdio.h>
#include <memory.h>
using namespace std;

#ifndef _MAX_LIMIT_
#define _MAX_LIMIT_ 99999999
#endif

#ifndef _MIN_LIMIT_
#define _MIN_LIMIT_ -99999999
#endif



// LASͷ�ṹ
#pragma pack(1)
class  LASHeader
{
public:

	// LASHeader���ļ��еĴ�Сdpw
	// sizeof(LASHeader)����Ľ�����ܱ������
	// ��Ϊ���������ܻ���һЩ�Ż�(��߽�����)
	//������ɫ��չ
	enum { HEADER_SIZE = 227 };

public:												// ���� ˵��		Ĭ��ֵ	�ֽڱ��
	char file_signature[4];							// *	�ļ���־	"LASF"	1-4
													//unsigned short file_source_id;				// *	�ļ����	0
													/*
													* This data field is reserved and must be zero filled by generating generating_software.
													* 1.0��las�ļ�Ϊ4�ֽڣ�1.1��Ϊ2�ֽڣ��������File_Source_ID����4���ֽ�
													*/
	unsigned int  reserved;							// -	����ʹ��	5-8		5-8
	unsigned int  project_ID_GUID_data_1;			// -	4�ֽ�		0		9-12
	unsigned short project_ID_GUID_data_2;			// -	2�ֽ�		0		13-14
	unsigned short project_ID_GUID_data_3;			// -	2�ֽ�		0		15-16
	unsigned char project_ID_GUID_data_4[8];		// -	8�ֽ�		0		17-24
	unsigned char version_major;					// *	���汾��			25
	unsigned char version_minor;					// *	���汾��			26
	unsigned char system_id[32];					// *	ϵͳ��ʶ			27-58
	char generating_software[32];					// *	�������			59-90
	unsigned short file_creation_day;				// -	����ʱ��			91-92
	unsigned short file_creation_year;				// -						93-94
	unsigned short header_size;						// *	Head��С			95-96
	unsigned int offset_to_point_data;				// *	���ݵ�ַ			97-100
	unsigned int number_of_variable_length_records; // *	�䳤��¼��Ŀ		101-104
	unsigned char point_data_format;				// *	�����ݸ�ʽGPS		105
	unsigned short point_data_record_length;		// *	�����ݳ���			106-107
	unsigned int number_of_point_records;			// *	�����Ŀ			108-111
	unsigned int number_of_points_by_return[5];	    // *	�ز�������			112-131

													/*
													* Xcoordinate = (Xrecord * x_scale_factor) + x_offset
													* Ycoordinate = (Yrecord * y_scale_factor) + y_offset
													* Zcoordinate = (Zrecord * z_scale_factor) + z_offset
													*/
	double x_scale_factor;							// *	����ϵ��			132-139
	double y_scale_factor;							// *	����ϵ��			140-147
	double z_scale_factor;							// *	����ϵ��			148-155

	double x_offset;								// *	���ƫ��			156-163
	double y_offset;								// *	���ƫ��			170-177
	double z_offset;								// *	���ƫ��			164-169

	double max_x;									// *	x���ֵ				178-195
	double min_x;									// *	x��Сֵ				186-193
	double max_y;									// *						194-201
	double min_y;									// *						202-209
	double max_z;									// *						210-217
	double min_z;									// *						218-225

public:
	/*
	���������
	*/
	LASHeader();
	LASHeader(const LASHeader& header);
	LASHeader& operator=(const LASHeader& header);

	/*
	���ݼ�ID
	*/
	inline unsigned short GetFile_Source_ID() { return (version_minor == 1 && version_minor == 1) ? reserved >> 16 : 0; }
	/*
	���ݼ�����λ
	*/
	inline unsigned int Getreserved() { return (version_minor == 1 && version_minor == 1) ? reserved &= 0x0000FFFF : reserved; }
	inline bool HasPoint()		   const { return point_data_record_length >= 12 ? true : false; }
	inline bool HasIntensity()	   const { return point_data_record_length >= 14 ? true : false; }
	inline bool HasReturnNumber() const { return point_data_record_length >= 15 ? true : false; }
	inline bool HasNumberofReturn()const { return point_data_record_length >= 15 ? true : false; }
	inline bool HasScanDirectionFlag() const { return point_data_record_length >= 15 ? true : false; }
	inline bool HasEdgeofFlightLine()const { return point_data_record_length >= 15 ? true : false; }
	inline bool HasScanAgnleRank()	const { return point_data_record_length >= 17 ? true : false; }
	inline bool HasFlightLine()const { return point_data_record_length >= 17 ? true : false; }

	void Setreserved(unsigned int reservedi);
	void SetFile_Source_ID(unsigned short id);

	bool HasGPSTime()const;
	bool HasLASColorExt4() const;//�Ƿ�����ɫ ���ֽڶ���
	bool HasLASColorExt6() const;
	int  HasLASPointExt() const; //���ݸ�ʽ�Ƿ�����չ
	bool HasPointDataStartfile_signature() const { return version_minor == 0 && version_minor == 1; }

	//��д����ͷ�ļ�
	void ReadHeader(FILE *fs);
	void WriteHeader(FILE *fs) const;

	const static unsigned short Data_Record_Length_of_Format0; // 20������չ�����ṹʱ��0��ʽ���׼����
	const static unsigned short Data_Record_Length_of_Format1; // 28������չ�����ṹʱ��1��ʽ���׼����
	const static unsigned short Data_Record_Length_of_Format2; // 26��1.2����չ�����ṹʱ��1��ʽ���׼����
	const static unsigned short Data_Record_Length_of_Format3; // 34��1.2����չ�����ṹʱ��1��ʽ���׼����
	const static unsigned short HeaderSize_Def;				   // 227����׼ͷ�ļ�����

	const static char ErrorPointCnt[];			 // "number of point is more than LasFileHead.Number_of_point_records";
	const static char ErrorVarLenRecordCnt[];	 // "number of variable Length Record is more than LasFileHead.Number_of_variable_length_records";
	const static char ErrorOffsettoData[];		 // "number of bytes of Variable Length Record is more than LasFileHead.Offset_to_data";
};
#pragma pack()

// �䳤��¼ͷ��
#define min_las_record_after_header 54;
class  LASVariableRecord
{
public:											// ����	����	Ĭ��ֵ
	LASVariableRecord() {
		reserved = 0xAABB;
		memset(user_id, 0, sizeof(char) * 16);
		record_id = 0;
		record_length_after_header = min_las_record_after_header;
		memset(description, 0, sizeof(char));
		record_buffer = NULL;
	}
	~LASVariableRecord()
	{
		if (record_buffer != NULL)
			delete[]record_buffer;
	}
	unsigned short reserved;					// -	����	0
	char user_id[16];							// *			LASF_Specl/LASF_Projection
	unsigned short record_id;					// *	���
	unsigned short record_length_after_header;	// *	������¼��С
	char description[32];						// -	����˵��
	unsigned char* record_buffer;

	void Read(FILE* fs);
	void Write(FILE* fs) const;

	//Two bytes after the last variable length record, and before the point data
	const static unsigned short Min_Record_Length_After_Header;  // 54
	const static short Point_Data_Start_Signature;
};

// �ٷ�����ı䳤��¼�ṹ
// User ID: LASF_Projection
// Record ID: 34735
class  LASVariable_header_geo_keys
{
public:
	unsigned short key_directory_version;
	unsigned short key_revision;
	unsigned short minor_revision;
	unsigned short number_of_keys;

	LASVariable_header_geo_keys()
	{
		key_directory_version = 1;	// Always
		key_revision = 1;			// Always
		minor_revision = 0;			// Always
	}
};
class  LASVariable_header_key_entry
{
public:
	unsigned short key_id;
	unsigned short tiff_tag_location;
	unsigned short count;
	unsigned short value_offset;
};

// ��������
enum  LAS_CLASSIFICATION_TYPE
{
	CLASSIFICATION_NULL = 0,	// δ�������
	CLASSIFICATION_UNCLASSIFIED = 1,
	CLASSIFICATION_GROUND = 2,
	CLASSIFICATION_LOW_VEGETATION = 3,
	CLASSIFICATION_MEDIUM_VEGETATION = 4,
	CLASSIFICATION_HIGH_VEGETATION = 5,
	CLASSIFICATION_BUILDING = 6,
	CLASSIFICATION_LOW_POINT_NOISE = 7,
	CLASSIFICATION_MEDEL_KEYPOINT = 8,
	CLASSIFICATION_WATER = 9,
	CLASSIFICATION_OVERLAP_POINTS2 = 12,
	CLASSIFICATION_reserved						// ����ʹ��
};
#endif


