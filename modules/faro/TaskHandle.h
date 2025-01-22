#ifndef UTILS_H
#define UTILS_H


#include "gTask.h"




//std::vector 本身不是线程安全的,使用读取txt的值
bool get_mileage_from_file(const  std::string& taskPath, std::vector<Mileage>& mileage);
//倾角计
bool get_clinometer_from_file(const std::string& taskPath, std::vector<clinometer>& vec);

template <class T>
static std::pair<int, int> binarySearch(const std::vector<T>& dataMap, const T& target);

bool expandPointcloud(std::vector<std::vector<PointCloudXYZCT> >& pointcloudRes, const std::map<double, double>& mileageWithTime, Speed flag);

bool expandPointcloud(std::vector<std::vector<PointCloudXYZCT> >& pointcloudRes, const std::map<double, double>& mileageWithTime, const std::map<double, double>& clinometerWithTime, Speed flag, bool Reverse);



#endif // UTILS_H