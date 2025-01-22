#pragma once
#include "gFaro.h"
using namespace Faro;


// ƽ�ƺ�ȫ�ֲ����ṹ
struct ShiftAndGlobal
{
    double x, y, z, p;
};

// ��Բ��ϲ����ṹ
struct FittingEllipse
{
    double A;    //������
    double B;    //�̰���
    double x;    //�����Բ������x
    double y;    //�����Բ������y
    double angle;//ƫ�ƽǶ�
};

// ÿ50Ȧ��������Ʒֳ����ɿ飬ÿһ�����Ϣ
// �����Բ��Ϣ�ṹ
struct MileageEllipse
{
    double mileage;  //��ʼ���
    double dm;       //Ȧ���
    int col;         //���������Բ��Ȧ��
    FittingEllipse ep;   //�����Բ�Ĳ���
};

// ��Բ���������ṹ
struct EllipseParams {
    double A{ 0.0 };  // ������
    double B{ 0.0 };  // �̰��� 
    double X0{ 0.0 }; // ����x
    double Y0{ 0.0 }; // ����y
    double Angle{ 0.0 }; // ��Բ������x������ļн�
};

// ���ƽ�����Ϣ�ṹ
struct PointCloudSection : public PointCloudXYZCT
{
    double xe{ 0.0 };          //չ���������
    double error{ 0.0 };		// ���
    double mileage{ 0.0 };     //���
    double angle{ 0.0 };		// �Ƕ�
    double area{ 0.0 };        //ɨ�������
    double depth{ 0.0 };       //ÿ������
};

// ͼ������
struct Image {
    std::vector<std::vector<unsigned char>> data;
    int width;
    int height;
    unsigned char& at(int y, int x) {
        return data[y][x];
    }

    template<typename T>
    T& at(int y, int x) {
        return reinterpret_cast<T&>(data[y][x]);
    }

    Image(int w, int h) : width(w), height(h) {
        data.resize(h, std::vector<unsigned char>(w));
    }

    void save(const std::string& path);
    static void save(std::vector<std::vector<unsigned char>> color, int width, int height, std::string savepath);
};

/**
 * @brief Faro�������ݴ�����
 * ���ڽ����ʹ�������Faroɨ���ǵĵ�������
 */
class PointCloudImage
{
public:

    PointCloudImage();
    ~PointCloudImage();


    //Algorithm image ImagePointCloud
    /**
     * @brief ����fls��Ӧ�����ͼ��Ҷ�ͼ
     * @param lme ��̼�������Ϣ
     * @param tunnel �������
     * @param savephotopathGray ����ĻҶ�ͼ·��
     * @param savephotopathDepth ��������ͼ·��
     * @param start_mileage ��ʼ���
     * @param end_mileage ��ֹ���
     * @param expand չ���Ŀ��
     * @param dDesignSecRadius �����ư뾶
     * @param dScannerHeight ɨ���ǰ�װ�߶�
     * @return
    */
    bool generateImages(
        std::vector<MileageEllipse>& lme,
        std::vector<std::vector<PointCloudSection>>& tunnel,
        const std::string& savephotopathGray,
        const std::string& savephotopathDepth,
        double start_mileage,
        double end_mileage,
        int expand,
        double dDesignSecRadius,
        double dScannerHeight = 0.68
    );

private:
    EllipseParams m_ellipse;// ����չ��Ӱ��ͼ����Բ���������ں������������xe
};
