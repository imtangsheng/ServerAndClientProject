#pragma once
/**
 * @brief ���������ռ�,��ֹ�㷨ʹ���б�������
 */
#pragma execution_character_set("utf-8") 
#include <string>
#include <vector>

#define FARO_LICENSE_KEY "J3CW4PNRTCTXFJ7T6KZUARUPL"

constexpr double M_PI = 3.14159265358979323846;

 /**
  * @brief FLS�ļ���������ö��
 */
enum class FlsParseError {
	kInitError,
	kLicenseError,    ///< ���֤��֤ʧ��
	kFlsFormatError   ///< FLS�ļ���ʽ����
};

// ����չ�����ٶȣ�ö��ֵ����չ����ȶ���Ȧ
// һȦΪ5000����
enum Speed {
	kFastest = 50,
	kFaster = 35,
	kFast = 25,
	kSlow = 15,
	kSlower = 5
};

//�����ṹ
//PointTunnel


struct PointFaro
{
	double x{ 0.0 };
	double y{0.0};
	double z{ 0.0 };
	unsigned short int color{ 0 };
	unsigned __int64 time{0};
};

struct PointCloudXYZCTXD : public PointFaro
{
	double xe = 0.0;
	double ds = 0.0;
};

struct PointCloudXYZCTXDF : public PointCloudXYZCTXD
{
	int flag = 0;
};

struct PointCloudXYZCTRGB : public PointFaro
{
	unsigned int R;
	unsigned int G;
	unsigned int B;
};

// ���ƽ�����Ϣ�ṹ
struct PointCloud : public PointFaro
{
	double xe{ 0.0 };          //չ���������
	double error{ 0.0 };		//��� 0
	double mileage{ 0.0 };     //���
	double angle{ 0.0 };		//�Ƕ� 0
	double area{ 0.0 };        //ɨ������� 0
	double depth{ 0.0 };       //ÿ������ 0
};

