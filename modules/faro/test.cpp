// Test.cpp
#include <QCoreApplication>
#include <qdatetime.h>
#include "RealtimeSolving.h"

#include <iostream>

// ����
struct PointXYZCT {
    float x, y, z; // ����
    float c;       // ��ɫ��ǿ��
    double t;      // ʱ���

    // ���캯��
    PointXYZCT(float x = 0, float y = 0, float z = 0, float c = 0, double t = 0)
        : x(x), y(y), z(z), c(c), t(t) {}

    // ��ӡ����
    void print() const {
        std::cout << "PointXYZCT: (" << x << ", " << y << ", " << z
            << "), C: " << c << ", T: " << t << std::endl;
    }
};

// ������
struct PointCloudSection : public PointXYZCT {
    // �����������Ӷ���ĳ�Ա�򷽷�
    PointCloudSection(float x = 0, float y = 0, float z = 0, float c = 0, double t = 0)
        : PointXYZCT(x, y, z, c, t) {}

    // ������Ķ��ⷽ��
    void printSection() const {
        std::cout << "PointCloudSection: (" << x << ", " << y << ", " << z
            << "), C: " << c << ", T: " << t << std::endl;
    }
};

// �������� PointXYZCT ���͵Ĳ���
void processPoint(const PointXYZCT& point) {
    std::cout << "Processing point: ";
    point.print();
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    RealtimeSolving solviong;
    solviong.test();

    return a.exec();
}
