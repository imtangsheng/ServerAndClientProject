// �ٶ�m/h /3.6 mm/s  / (ÿ��������) / (���ÿȦ���� / �ֱ���)
//(MAX_POINT_PER_ROUND / resolution) �궨��ÿȦ����

constexpr auto MAX_POINT_PER_ROUND = 40960.00;	//���ÿȦ����;
constexpr auto MAX_POINT_PER_SEC = 976000.00;	//���ÿ�����;

constexpr double MAX_POINT_PER_SEC_ROUND = MAX_POINT_PER_SEC / MAX_POINT_PER_ROUND;

// ��������MeasurementRate��Ӧ��ÿ�����
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

inline double GetAccuracy(uint16_t speed,uint8_t resolution, int measurementRate = 8) //���ƾ���mm,�����ٶȺͲ�����ʽ����
{
    return (speed / 3.6) / (GetPointPerSec(measurementRate)/(MAX_POINT_PER_ROUND / resolution));
}

//ÿ��ɨ��Ȧ�� = ÿ����� / ÿȦ���������±���ʾ��
//��������	�ֱ���	ÿ�����	ÿȦ����	ÿ��Ȧ��
//8	1	976000	40960	23.828125
//8	2	976000	20480	47.65625
//8	4	976000	10240	95.3125
//
//����ÿ��ɨ��Ȧ����ɨ�辫�ȿɼ����С�����������ʻ�ٶ� =
//ɨ�辫��(mm) * ÿ��ɨ��Ȧ�� * 3600(m / h)
inline int GetMaxSpeed(double accuracy, uint8_t resolution, int measurementRate = 8) {
	return accuracy * 3.6 * (GetPointPerSec(measurementRate) / (MAX_POINT_PER_ROUND / resolution));
}

constexpr double da = 0.000153398;// 2 * PI / MAX_POINT_PER_ROUND;//Ȧ�ڽǷֱ��ʣ����ȣ�
inline double GetMaxRadius(double accuracy, uint8_t resolution) {
	return accuracy / (da * resolution);
}

