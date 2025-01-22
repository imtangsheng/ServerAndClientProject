// Test.cpp
#include <QCoreApplication>
#include <qdatetime.h>
#include "RealtimeSolving.h"

#include <iostream>

// 基类
struct PointXYZCT {
    float x, y, z; // 坐标
    float c;       // 颜色或强度
    double t;      // 时间戳

    // 构造函数
    PointXYZCT(float x = 0, float y = 0, float z = 0, float c = 0, double t = 0)
        : x(x), y(y), z(z), c(c), t(t) {}

    // 打印函数
    void print() const {
        std::cout << "PointXYZCT: (" << x << ", " << y << ", " << z
            << "), C: " << c << ", T: " << t << std::endl;
    }
};

// 派生类
struct PointCloudSection : public PointXYZCT {
    // 派生类可以添加额外的成员或方法
    PointCloudSection(float x = 0, float y = 0, float z = 0, float c = 0, double t = 0)
        : PointXYZCT(x, y, z, c, t) {}

    // 派生类的额外方法
    void printSection() const {
        std::cout << "PointCloudSection: (" << x << ", " << y << ", " << z
            << "), C: " << c << ", T: " << t << std::endl;
    }
};

// 函数接受 PointXYZCT 类型的参数
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
