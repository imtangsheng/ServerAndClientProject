#pragma once
#include "iTask.h"

//std::vector 本身不是线程安全的,使用读取txt的值
bool get_mileage_from_file(const std::string& taskPath, std::vector<Mileage>& mileage);
//倾角计
bool get_clinometer_from_file(const std::string& taskPath, std::vector<Clinometer>& vec);

bool PointCloudExpand(bool hasClinometer, TaskFaroPart &part);

template <class T>
static std::pair<int, int> binarySearch(const std::vector<T>& dataMap, const T& target);

bool expandPointcloud(std::vector<std::vector<PointCloud> >& pointcloudRes, const std::map<double, double>& mileageWithTime,
	Speed flag, double &start_mileage,double &end_mileage);

bool expandPointcloud(std::vector<std::vector<PointCloud> >& pointcloudRes, const std::map<double, double>& mileageWithTime, const std::map<double, double>& clinometerWithTime,
	Speed flag, bool Reverse, double& start_mileage, double& end_mileage);

bool WritePointCloudImage(TaskFaroPart& part,
	const QString& grayImagePath, const QString& depthImagePath,
	int merge_type = 0
);