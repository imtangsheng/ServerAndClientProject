// 速度m/h /3.6 mm/s  / (每秒最大点数) / (最大每圈点数 / 分辨率)
//(MAX_POINT_PER_ROUND / resolution) 标定的每圈点数

constexpr auto MAX_POINT_PER_ROUND = 40960.00;	//最大每圈点数;
constexpr auto MAX_POINT_PER_SEC = 976000.00;	//最大每秒点数;

constexpr double MAX_POINT_PER_SEC_ROUND = MAX_POINT_PER_SEC / MAX_POINT_PER_ROUND;

// 测量速率MeasurementRate对应的每秒点数
static double GetPointPerSec(int measurementRate = 8) {
	double pointsPerSecond;
	switch (measurementRate) {
	case 1:pointsPerSecond = 122000; break;
	case 2:pointsPerSecond = 244000; break;
	case 4:pointsPerSecond = 488000; break;
	case 8:pointsPerSecond = 976000; break;
	default:pointsPerSecond = 976000; // Default to highest rate
	}
	return  pointsPerSecond;
}

inline double GetAccuracy(uint16_t speed,uint8_t resolution, int measurementRate = 8) //点云精度mm,根据速度和参数公式计数
{
    return (speed / 3.6) / (GetPointPerSec(measurementRate)/(MAX_POINT_PER_ROUND / resolution));
}

//每秒扫描圈数 = 每秒点数 / 每圈点数，如下表所示：
//测量速率	分辨率	每秒点数	每圈点数	每秒圈数
//8	1	976000	40960	23.828125
//8	2	976000	20480	47.65625
//8	4	976000	10240	95.3125
//
//根据每秒扫描圈数和扫描精度可计算出小车最大允许行驶速度 =
//扫描精度(mm) * 每秒扫描圈数 * 3600(m / h)
inline int GetMaxSpeed(double accuracy, uint8_t resolution, int measurementRate = 8) {
	return accuracy * 3.6 * (GetPointPerSec(measurementRate) / (MAX_POINT_PER_ROUND / resolution));
}

constexpr double da = 0.000153398;// 2 * PI / MAX_POINT_PER_ROUND;//圈内角分辨率（弧度）
inline double GetMaxRadius(double accuracy, uint8_t resolution) {
	return accuracy / (da * resolution);
}

