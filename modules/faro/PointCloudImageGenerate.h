#pragma once
#include "iFaro.h"

//namespace poiontcloud_image_generate
//{
extern int smoothStep;
extern double g_first_thinning;
extern double g_second_thinning;
extern double g_pipe_wall_limit;
extern double g_tunnel_face_value;
extern double g_depth_standard;
extern double g_depth_variance_standard;
extern double g_pipe_wall_start_and_end_init_value;
extern double g_hole_depth_limit;//��˨����ȹ�����ֵ
//} // namespace poiontcloud_image_generate

//�����Բ�Ĳ���
struct FittingEllipse
{
	double A;    //������
	double B;    //�̰���
	double x;    //�����Բ������x
	double y;    //�����Բ������y
	double angle;//ƫ�ƽǶ�
};

/// <summary>
/// ÿ50Ȧ��������Ʒֳ����ɿ飬ÿһ�����Ϣ
/// </summary>
struct MileageEllipse
{
	double mileage;  //��ʼ���
	double dm;       //Ȧ���
	int col;         //���������Բ��Ȧ��
	FittingEllipse ep;   //�����Բ�Ĳ���
};

//��Բ�����ṹ��
struct EllipseParams
{
	double A;
	double B;
	double X0;
	double Y0;
	double Angle;   //��Բ������x������ļн�
	EllipseParams() :A(0.0), B(0.0), X0(0.0), Y0(0.0), Angle(0.0) {}
	EllipseParams(double a, double b, double x0, double y0, double angle) :
		A(a), B(b), X0(x0), Y0(y0), Angle(angle) {}
};


class Image {
public:
	std::vector<std::vector<unsigned char>> data;
	int width{ 0 };
	int height{ 0 };
	Image() {}
	Image(int w, int h) : width(w), height(h) {
		data.resize(h, std::vector<unsigned char>(w));
	}
	// resize ֱ��ӳ�䵽 data
	void resize(int w, int h) {
		width = w;
		height = h;
		data.resize(h, std::vector<unsigned char>(w));
	}
	// ��� operator[] ����
	std::vector<unsigned char>& operator[](int index) {
		return data[index];
	}

	// ��� const operator[] ����
	const std::vector<unsigned char>& operator[](int index) const {
		return data[index];
	}

	unsigned char& at(int y, int x) {
		return data[y][x];
	}

	template<typename T>
	T& at(int y, int x) {
		return reinterpret_cast<T&>(data[y][x]);
	}
};

/**
 * @brief ����fls��Ӧ�����ͼ��Ҷ�ͼ
 * @param lme ��̼�������Ϣ
 * @param points �������
 * @param savephotopathGray ����ĻҶ�ͼ·��
 * @param savephotopathDepth ��������ͼ·��
 * @param start_mileage ��ʼ���
 * @param end_mileage ��ֹ���
 * @param expand չ���Ŀ��
 * @param dDesignSecRadius �����ư뾶
 * @param dScannerHeight ɨ���ǰ�װ�߶�
 * @return
*/
bool GetPointCloudDepthAndGrayImage(
	std::vector<MileageEllipse>& lme,	std::vector<std::vector<PointCloud> >& points,
	Image& gray_data, std::vector<std::vector<unsigned char>>& depth_data,
	double start_mileage,	double end_mileage,
	int expand,	double dDesignSecRadius,
	double dScannerHeight = 0.68
);