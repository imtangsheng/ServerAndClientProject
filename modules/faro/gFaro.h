#pragma once
/**
 * @brief ���������ռ�,��ֹ�㷨ʹ���б�������
 */
#pragma execution_character_set("utf-8") 
#include <string>
#include <vector>
#include <QDebug>
#include <QDateTime>

#define FARO_LICENSE_KEY "J3CW4PNRTCTXFJ7T6KZUARUPL"

#include "Common_Structs.h"
using namespace CommonStructs;

 // ����չ�����ٶȣ�ö��ֵ����չ����ȶ���Ȧ
 // һȦΪ5000����


namespace Faro {

/**
 * @brief FLS�ļ���������ö��
*/
enum class FlsParseError {
	kInitError,
	kLicenseError,    ///< ���֤��֤ʧ��
	kFlsFormatError   ///< FLS�ļ���ʽ����
};



/**
* @brief �������ݽṹ��,�洢���������Ϣ
*/
//struct PointCloudXYZCT {
//	double x{ 0.0 };      ///< X����
//	double y{ 0.0 };      ///< Y����
//	double z{ 0.0 };      ///< Z����
//	int color{ 0 };   ///< ��ɫֵ
//	unsigned __int64 time{ 0 };   ///< ʱ���
//};



}//namespace Faro 