#include <fstream>
#include <algorithm>
#include "PointCloudImageGenerate.h"
typedef unsigned char uchar;
class Matrix;

int g_smooth_step = 5;                 //移动步长
double g_first_thinning = 360;     //第一次抽稀份数
double g_second_thinning = 1800;   //第二次抽稀份数
double g_pipe_wall_limit = 0.02;    //管壁点限值
double g_tunnel_face_value = 0.02;  //去除隧道表面的阈值
double g_depth_standard = 0.005;   //深度标准
double g_depth_variance_standard = 0.0005;   //深度方差标准
double g_pipe_wall_start_and_end_init_value = 0.02;   //管壁点前后长度初始化
double g_hole_depth_limit = 0.09;	//螺栓孔深度过滤阈值

//1. * *椭圆参数计算 * *：根据设计半径和扫描仪高度确定椭圆的参数。
void GetEllipse_(EllipseParams& ellipse, double dRadius, double dScanerHeight);
//2. * *坐标投影 * *：将点云的坐标投影到椭圆上，生成影像图的纵坐标。
void SolvePointWithFiltered_(std::vector<PointCloud>& points, FittingEllipse& fitSection, std::vector<PointCloud>& pointAfterSolution, double dDesignSecRadius);
//3. * *椭圆拟合 * *：使用5次多项式拟合椭圆参数。
void EllipseFitting_(const std::vector<PointCloud>& points, double& a, double& b, double& x, double& y, double& angle);
//4. * *点云抽稀 * *：根据角度对点云进行抽稀，减少数据量。
std::vector<PointCloud> ThinningPoints_(std::vector<PointCloud> &points, double x, double y, double thinning);
//5. * *稳定椭圆参数 * *：通过迭代剔除异常点，获得稳定的椭圆参数。
void StableFittingParameters_(std::vector<PointCloud> &points, double dDesignSecRadius, double& a, double& b, double& x, double& y, double& angle);
//6. * *管壁点提取 * *：根据拟合椭圆提取管壁上的点。
std::vector<PointCloud> ExtractingPipeWall_(std::vector<PointCloud> &points, double a, double b, double x, double y, double angle);
std::vector<PointCloud> ExtractingPipeWallExtend_(std::vector<PointCloud> &points, double a, double b, double x, double y, double angle);
//7. * *隧道分类 * *：对点云进行分类，区分管壁和其他部分
std::vector<double> TunnelClassify_(const std::vector<PointCloud> &points);
//8. * *移动平均平滑 * *：对深度数据进行平滑处理
std::vector<double> MovingSmooth_(std::vector<double> &depth);

void GetRealOrdinate_(const EllipseParams& ellipse, double x, double z, double& xe);
double GetDxyc_(double x, double y, double x0, double y0, double a, double b, double angle);

static bool _Cmp_(const PointCloud& a, const PointCloud& b)
{
	return a.x < b.x;
}


// 直方图均衡化处理
static void __HistogramEqualization(const std::vector<std::vector<PointCloud>>& tunnel, std::vector<int>& grayEqual)
{
	int row = tunnel.size();
	int col = tunnel[0].size();
	int pointsNum = row * col;

	// 初始化统计数组
	std::vector<int> gray(256, 0);
	std::vector<double> grayProb(256, 0.0);
	std::vector<double> grayDist(256, 0.0);
	grayEqual.resize(256, 0);

	// 统计灰度值分布
	for (const auto& row : tunnel) {
		for (const auto& point : row) {
			gray[point.color]++;
		}
	}

	// 计算灰度概率分布
	const double alpha = 0.05;
	for (size_t i = 0; i < 256; i++) {
		grayProb[i] = static_cast<double>(gray[i]) / pointsNum;
	}

	// 计算累积分布
	grayDist[0] = grayProb[0];
	for (size_t i = 1; i < 256; i++) {
		grayDist[i] = grayDist[i - 1] + grayProb[i] * std::pow(i / 255.0, alpha);
	}

	// 计算均衡化后的灰度值
	for (size_t i = 0; i < 256; i++) {
		grayEqual[i] = static_cast<uchar>(255 * grayDist[i] + 0.5);
	}
}

// 椭圆拟合处理
static void __FitEllipseForSection(const std::vector<PointCloud>& section,
	int index,std::vector<double>& minY,
	std::vector<MileageEllipse>& lme,double designSecRadius) 
{
	std::vector<PointCloud> tunnelDownsample;
	// 点云抽稀处理
	if (section.size() > 6000) {
		std::vector<PointCloud> sortedSection = section;
		std::sort(sortedSection.begin(), sortedSection.end(), _Cmp_);
		int sampleRate = section.size() / 3000;
		for (int i = 0; i < sortedSection.size(); i += sampleRate) {
			tunnelDownsample.push_back(sortedSection[i]);
		}
	}
	else {
		tunnelDownsample = section;
	}
	//***提取有效点进行椭圆拟合 注意这个对应值是不同的坐标轴***
	std::vector<PointCloud> validPoints;
	for (auto& point : tunnelDownsample) {
		point.mileage = point.y;point.y = point.z;
		if (std::abs(point.x) < 30 && std::abs(point.y) < 30 &&
			point.x != 0 && point.y != 0) {
			validPoints.push_back(point);
		}
	}

	// 进行椭圆拟合
	FittingEllipse ellipse;
	std::vector<PointCloud> filteredPoints;
	SolvePointWithFiltered_(validPoints, ellipse, filteredPoints, designSecRadius);

	// 更新里程信息
	auto minMileage = std::min_element(validPoints.begin(), validPoints.end(),
		[](const auto& a, const auto& b) { return a.mileage < b.mileage; });

	if (minMileage != validPoints.end()) {
		minY.push_back(minMileage->mileage);
	}

	double dm = (index == 0) ? 0 : (minY.back() - minY[minY.size() - 2]) / 30;
	if (index > 0) {
			lme.back().dm = dm;
	}
	lme.push_back({ minY.back(), dm, index, ellipse });
}

//螺栓孔深度过滤阈值 更新深度图
static void __UpdateDepthImage(std::vector<std::vector<unsigned char>>& depthImage,
	int row, int col, double depth) {
	if (depth < g_hole_depth_limit / 7) {
		depthImage[row][col] = 0; //0（黑色）：最近距离。
	}
	else if (depth < g_hole_depth_limit) {
		depthImage[row][col] = static_cast<unsigned char>(255 * depth / g_hole_depth_limit);
	}
	else {
		depthImage[row][col] = 255; //255（白色）：最远距离。
	}
}

inline static void __InterpolatePixel_(Image& grayImage,
	std::vector<std::vector<unsigned char>>& depthImage,
	int x, int y, int windowSize, int height, int width)
{
	int sumGray = 0, sumDepth = 0, count = 0;

	for (int dy = -windowSize; dy <= windowSize; dy++) {
		for (int dx = -windowSize; dx <= windowSize; dx++) {
			int newY = y + dy;
			int newX = x + dx;

			if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
				if (grayImage.at<uchar>(newY, newX) > 0) {
					sumGray += grayImage.at<uchar>(newY, newX);
					sumDepth += depthImage[newY][newX];
					count++;
				}
			}
		}
	}
	if (count == 0) count = 1;
	grayImage.at<uchar>(y, x) = static_cast<uchar>(sumGray / count);
	depthImage[y][x] = static_cast<unsigned char>(sumDepth / count);

}

static void __ProcessImages(Image& grayImage,
	std::vector<std::vector<unsigned char>>& depthImage,
	int height, int width) {
	// 填充空白像素
	const int windowSize = 3;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (grayImage.at<uchar>(y, x) == 0) {
				__InterpolatePixel_(grayImage, depthImage, x, y, windowSize, height, width);
			}
		}
	}
}

bool GetPointCloudDepthAndGrayImage(
	std::vector<MileageEllipse>& lme, std::vector<std::vector<PointCloud>>& points,
	Image& gray_data, std::vector<std::vector<unsigned char>>& depth_data,
	double start_mileage, double end_mileage, int expand, double dDesignSecRadius, double dScannerHeight)
{
	EllipseParams m_ellipse;    //用于展开影像图的椭圆参数，用于后面求出纵坐标xe
	GetEllipse_(m_ellipse, dDesignSecRadius, dScannerHeight);   //无计算椭圆参数时根据输入参数得到椭圆参数
	//std::cout << expand << std::endl;
	//int splitMap = 0;   //每50圈一块的话一共能分几块
	double diff = end_mileage - start_mileage;   //应该是一个fls的实际间距里程
	// 计算图像尺寸
	size_t nImageHeight = (size_t)(2 * M_PI * dDesignSecRadius * expand) + 1;  //高
	size_t nImageWidth = (size_t)(diff * expand) + 1;  //宽
	// 创建图像矩阵
	//cv::Mat img(nImageHeight, nColCount, CV_8UC1);
	//cv::Mat gray_data(cv::Size(nColCount, nImageHeight), CV_8UC1);
	//std::vector<std::vector<unsigned char>> depth_data(nImageHeight + 1, std::vector<unsigned char>(nColCount + 1));
	gray_data.resize(nImageWidth, nImageHeight);
	depth_data.resize(nImageHeight + 1, std::vector<unsigned char>(nImageWidth + 1));

	// #进行直方图均衡化
	std::vector<int> gray_equal;
	__HistogramEqualization(points, gray_equal);
	
	// #处理每一圈的点云数据
	std::vector<double> minY;
	std::vector<std::vector<double>> depth;  //每一点的深度
	for (int i = 0; i < points.size(); i++) {
		// 每30圈进行一次椭圆拟合
		if (i % 30 == 0) {
			__FitEllipseForSection(points[i], i, minY, lme, dDesignSecRadius);
		}
		// 处理每个点
		for (auto& point : points[i]){
			// 计算展开坐标
			GetRealOrdinate_(m_ellipse, point.x, point.z, point.xe);
			int height = static_cast<int>(point.xe * expand);
			int width = static_cast<int>((point.y - start_mileage) * expand);
			//if (height < nImageHeight && width < nImageWidth)
			gray_data.at<uchar>(height, width) = gray_equal[point.color];// 更新灰度图 通过行索引为xe*n，列索引为nC-x1的位置来访问图像中的像素值
			point.depth = GetDxyc_(point.x, point.z,
				lme.back().ep.x, lme.back().ep.y, lme.back().ep.A, lme.back().ep.B, lme.back().ep.angle);
			if (point.depth == 0) {
				point.depth = std::sqrt(point.x * point.x + point.z * point.z) - dDesignSecRadius;
			}
			__UpdateDepthImage(depth_data, height, width, point.depth);//生成深度图部分
		}
	}

	// 图像后处理 补黑色像素点
	__ProcessImages(gray_data, depth_data, nImageHeight, nImageWidth);
	return true;
}

// 矩阵类
class Matrix {
private:
	std::vector<std::vector<double>> data;
	int rows, cols;
public:
	Matrix(int r, int c) : rows(r), cols(c) {
		data.resize(r, std::vector<double>(c, 0.0));
	}
	double& operator()(int i, int j) {
		return data[i][j];
	}
	// 矩阵求逆 (采用高斯-约旦消元法)
	Matrix inverse() {
		Matrix augmented(rows, 2 * cols);
		// 构建增广矩阵
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				augmented(i, j) = data[i][j];
				augmented(i, j + cols) = (i == j) ? 1.0 : 0.0;
			}
		}
		// 高斯-约旦消元
		for (int i = 0; i < rows; i++) {
			// 寻找主元
			double maxEl = std::abs(augmented(i, i));
			int maxRow = i;
			for (int k = i + 1; k < rows; k++) {
				if (std::abs(augmented(k, i)) > maxEl) {
					maxEl = std::abs(augmented(k, i));
					maxRow = k;
				}
			}
			// 交换行
			if (maxRow != i) {
				for (int k = i; k < 2 * cols; k++) {
					std::swap(augmented(maxRow, k), augmented(i, k));
				}
			}
			// 将主对角线元素归一化
			double div = augmented(i, i);
			for (int j = i; j < 2 * cols; j++) {
				augmented(i, j) /= div;
			}
			// 消元
			for (int j = 0; j < rows; j++) {
				if (i != j) {
					double mult = augmented(j, i);
					for (int k = i; k < 2 * cols; k++) {
						augmented(j, k) -= augmented(i, k) * mult;
					}
				}
			}
		}
		// 提取逆矩阵部分
		Matrix inverse(rows, cols);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				inverse(i, j) = augmented(i, j + cols);
			}
		}
		return inverse;
	}
	// 矩阵乘法
	Matrix operator*(const Matrix& other) {
		Matrix result(rows, other.cols);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < other.cols; j++) {
				double sum = 0;
				for (int k = 0; k < cols; k++) {
					sum += data[i][k] * other.data[k][j];
				}
				result(i, j) = sum;
			}
		}
		return result;
	}
};


double GetDxyc_(double x, double y, double x0, double y0, double a, double b, double angle)
{
	double dx = x - x0;
	double dy = y - y0;
	double A = std::atan2(y - y0, x - x0);
	A -= angle;
	double r = a * b / std::sqrt(std::pow(a * std::sin(A), 2) + std::pow(b * std::cos(A), 2));  //椭圆上一点到原点的距离
	return (std::sqrt(dx * dx + dy * dy) - r);
}


void GetEllipse_(EllipseParams& ellipse, double dRadius, double dScanerHeight)
{
	if (dRadius == 2.7)
	{
		ellipse.X0 = 0;
		ellipse.Y0 = dRadius - 0.915 - dScanerHeight;
		ellipse.A = dRadius + 0.001;
		ellipse.B = dRadius - 0.001;
		ellipse.Angle = 0;
	}
	else if (dRadius == 2.75)
	{
		ellipse.X0 = 0;
		ellipse.Y0 = dRadius - 0.850 - dScanerHeight;
		ellipse.A = dRadius + 0.001;
		ellipse.B = dRadius - 0.001;
		ellipse.Angle = 0;
	}
	else if (dRadius == 3.85)
	{
		ellipse.X0 = 0;
		ellipse.Y0 = dRadius - 1.250 - dScanerHeight;
		ellipse.A = dRadius + 0.001;
		ellipse.B = dRadius - 0.001;
		ellipse.Angle = 0;
	}
	else
	{
		ellipse.X0 = 0;
		ellipse.Y0 = dRadius - 0.850 - dScanerHeight;
		ellipse.A = dRadius + 0.001;
		ellipse.B = dRadius - 0.001;
		ellipse.Angle = 0;
	}
}

//输入椭圆参数以及扫描点的x、z坐标，将点云按投影到拟合圆轮廓上，输出影像图的纵坐标，横坐标为里程
void GetRealOrdinate_(const EllipseParams& ellipse, double x, double z, double& xe)
{
	double r = ellipse.A; //A,B都等于r
	double Xr, Yr;
	if (x == 0)
	{
		Xr = 0;
		Yr = ellipse.Y0 + std::sqrt(r * r - ellipse.X0 * ellipse.X0);
	}
	else
	{
		//联立Yr=ta*Xr和（Xr-ellipse.X0)^2+(Yr-ellipse.Y0)^2=r^2可求解Xr，Yr
		double ta = z / x;
		double yX_1 = ellipse.Y0 * ta + ellipse.X0;
		double yX_2 = 1 + ta * ta;
		if (x >= 0)
		{
			Xr = (yX_1 + std::sqrt(yX_1 * yX_1 + yX_2 * (r * r - ellipse.X0 * ellipse.X0 - ellipse.Y0 * ellipse.Y0))) / yX_2;
		}
		else
		{
			Xr = (yX_1 - std::sqrt(yX_1 * yX_1 + yX_2 * (r * r - ellipse.X0 * ellipse.X0 - ellipse.Y0 * ellipse.Y0))) / yX_2;
		}
		Yr = Xr * ta;
	}
	xe = (M_PI - std::atan2(Xr - ellipse.X0, Yr - ellipse.Y0)) * r;     //每点基于椭圆展开
}

/// <summary>
/// 椭圆拟合处理点并滤波
/// </summary>
/// <param name="points">二维点链表</param>
/// <param name="fitSection">存储信息的拟合椭圆</param>
/// <param name="pointAfterSolution">滤波后点链表</param>
void SolvePointWithFiltered_(std::vector<PointCloud>& points, FittingEllipse& fitSection, std::vector<PointCloud>& pointAfterSolution, double dDesignSecRadius)
{
	double firstThinning = 360;
	double secondThinning = 1800;
	double a, b, x, y, angle;
	//一次拟合
	EllipseFitting_(points, a, b, x, y, angle);
	//抽稀
	std::vector<PointCloud> pointAfterThinning = ThinningPoints_(points, x, y, firstThinning);
	//二次拟合
	EllipseFitting_(pointAfterThinning, a, b, x, y, angle);
	//找稳定的椭圆参数
	StableFittingParameters_(points, dDesignSecRadius, a, b, x, y, angle);
	//根据上面稳定的椭圆参数求管壁点
	std::vector<PointCloud> pipeWallPoint = ExtractingPipeWall_(points, a, b, x, y, angle);
	//根据上面稳定的椭圆参数滤波全部管壁点，上一步点数少
	pointAfterSolution = ExtractingPipeWallExtend_(points, a, b, x, y, angle);
	//抽稀
	std::vector<PointCloud> pointAfterDownsampling = ThinningPoints_(pipeWallPoint, x, y, secondThinning);
	//三次拟合
	EllipseFitting_(pointAfterDownsampling, a, b, x, y, angle);
	fitSection.A = a;
	fitSection.B = b;
	fitSection.x = x;
	fitSection.y = y;
	fitSection.angle = angle;
}

/// <summary>
/// 用5次多项式拟合椭圆参数
/// </summary>
/// <param name="points">二维点</param>
/// <param name="a">长半轴</param>
/// <param name="b">短半轴</param>
/// <param name="x">圆心</param>
/// <param name="y">圆心</param>
/// <param name="angle">偏转角</param>
void EllipseFitting_(const std::vector<PointCloud>& points, double& a, double& b, double& x, double& y, double& angle)
{
	//5次多项式用 5*5 得系数矩阵和 5*1 的常数矩阵
	//Eigen::Matrix<double, 5, 5> conf = Eigen::MatrixXd::Zero(5, 5);
	//Eigen::Matrix<double, 5, 1> cons = Eigen::MatrixXd::Zero(5, 1);
	Matrix conf(5, 5);
	Matrix cons(5, 1);
	if (points.empty())
	{
		return;
	}
	for (auto &p : points)
	{
		double conf10 = p.x, conf01 = p.y, conf20 = p.x * p.x, conf02 = p.y * p.y, conf11 = p.x * p.y,
			conf30 = conf20 * p.x, conf21 = conf20 * p.y, conf12 = p.x * conf02, conf03 = conf02 * p.y,
			conf31 = conf30 * p.y, conf22 = conf20 * conf02, conf13 = p.x * conf03, conf04 = conf03 * p.y;
		//
		conf(0, 0) += conf22;
		conf(0, 1) += conf13;
		conf(0, 2) += conf21;
		conf(0, 3) += conf12;
		conf(0, 4) += conf11;
		//
		conf(1, 0) += conf13;
		conf(1, 1) += conf04;
		conf(1, 2) += conf12;
		conf(1, 3) += conf03;
		conf(1, 4) += conf02;
		//
		conf(2, 0) += conf21;
		conf(2, 1) += conf12;
		conf(2, 2) += conf20;
		conf(2, 3) += conf11;
		conf(2, 4) += conf10;
		//
		conf(3, 0) += conf12;
		conf(3, 1) += conf03;
		conf(3, 2) += conf11;
		conf(3, 3) += conf02;
		conf(3, 4) += conf01;
		//
		conf(4, 0) += conf11;
		conf(4, 1) += conf02;
		conf(4, 2) += conf10;
		conf(4, 3) += conf01;
		conf(4, 4)++;
		//
		cons(0, 0) -= conf31;
		cons(1, 0) -= conf22;
		cons(2, 0) -= conf30;
		cons(3, 0) -= conf21;
		cons(4, 0) -= conf20;

	}
	//A * B = C -> B = Inv(A) * B
	//Eigen::Matrix<double, 5, 1> fittingResult = conf.inverse() * cons;
	Matrix fittingResult = conf.inverse() * cons;
	double A = fittingResult(0, 0), B = fittingResult(1, 0), C = fittingResult(2, 0), D = fittingResult(3, 0), E = fittingResult(4, 0);
	//公式
	x = (2 * B * C - A * D) / (A * A - 4 * B);
	y = (2 * D - A * C) / (A * A - 4 * B);
	double t = 2 * (A * C * D - B * C * C - D * D + 4 * B * E - A * A * E);
	double t1 = A * A - 4 * B;
	double t2 = sqrt(A * A + (1 - B) * (1 - B));
	a = sqrt(t / (t1 * (B - t2 + 1)));
	b = sqrt(t / (t1 * (B + t2 + 1)));
	angle = atan(sqrt((a * a - b * b * B) / (a * a * B - b * b)));
	if (A > 0)
	{
		angle = -angle;
	}
}

/// <summary>
/// 二维点抽稀(按角度)
/// </summary>
/// <param name="points">二维点</param>
/// <param name="x">圆心</param>
/// <param name="y">圆心</param>
/// <param name="thinning">抽稀份数</param>
/// <returns>抽稀后的二维点</returns>
std::vector<PointCloud> ThinningPoints_(std::vector<PointCloud>& points, double x, double y, double thinning)
{
	//计算点的方位角，随后按方位角大小排列，以便于按份数抽稀
	for (int i = 0; i < points.size(); i++)
	{
		double dy = points[i].y - y, dx = points[i].x - x;
		points[i].angle = atan2(dy, dx);
	}
	sort(points.begin(), points.end(), [](const PointCloud& A, const PointCloud& B) {return A.angle < B.angle; });
	if (points.size() == 0) return points;
	double angleMin = points[0].angle;//最小的角
	double angleMax = points[points.size() - 1].angle;//最大的角
	double interval = 2 * M_PI / thinning;//抽稀份数
	int index = 0; //标记
	std::vector<PointCloud> selectPoint;
	double piece = (angleMax - angleMin) / interval;
	//原理：给定一个抽稀份数，然后求在这个抽稀小份中包含的方位角，达成抽稀
	for (int i = 0; i < piece; i++)
	{
		double startAngle = i * interval + angleMin;
		double endAndgle = startAngle + interval;
		for (int j = index; j < points.size() - 1; j++)
		{
			if (points[j].angle >= startAngle && points[j].angle <= endAndgle)
			{
				selectPoint.push_back(points[j]);
				index = j;
				break;
			}
		}
	}
	return selectPoint;
}

/// <summary>
/// 稳定椭圆参数
/// </summary>
/// <param name="points">二维点</param>
/// <param name="dDesignSecRadius">断面设计半径</param>
/// <param name="a">长半轴</param>
/// <param name="b">短半轴</param>
/// <param name="x">圆心</param>
/// <param name="y">圆心</param>
/// <param name="angle">偏转角</param>
void StableFittingParameters_(std::vector<PointCloud>& points, double dDesignSecRadius, double& a, double& b, double& x, double& y, double& angle)
{
	//先让圆心稍微偏移，让第一次循环可以跑起来
	double xLast = x + 1;
	double yLast = y + 1;
	//判断两次圆心的距离偏差值是否>0.001
	while (std::max(abs(xLast - x), std::abs(yLast - y)) > 0.001)
	{
		double errorAddition = 0;//平均误差
		//计算误差
		for (int i = 0; i < points.size(); i++)
		{
			points[i].error = abs(std::sqrt((points[i].x - x) * (points[i].x - x) + (points[i].y - y) * (points[i].y - y)) - dDesignSecRadius);
			errorAddition += points[i].error;
		}
		errorAddition = errorAddition / points.size();
		//将误差大于2倍误差的点剔除
		for (int i = 0; i < points.size(); i++)
		{
			if (points[i].error > 2 * errorAddition)
			{
				points.erase(points.begin() + i);
				i--;

			}
		}
		xLast = x;
		yLast = y;
		//进行一次椭圆拟合
		EllipseFitting_(points, a, b, x, y, angle);
	}
}

/// <summary>
/// 基于拟合椭圆计算每点到椭圆的距离，提取管壁点
/// </summary>
/// <param name="points">二维点</param>
/// <param name="a">长半轴</param>
/// <param name="b">短半轴</param>
/// <param name="x">圆心</param>
/// <param name="y">圆心</param>
/// <param name="angle">偏转角</param>
/// <returns>返回管壁上的点</returns>
std::vector<PointCloud> ExtractingPipeWall_(std::vector<PointCloud>& points, double a, double b, double x, double y, double angle)
{
	double averageR = (a + b) / 2;	//取长短半轴的平均值
	double pointAngle;  //点的偏转角
	double length; //点对应的长度
	for (int i = 0; i < points.size(); i++)
	{
		pointAngle = atan2(points[i].y - y, points[i].x - x);
		//由椭圆方程得到点到椭圆圆心的距离
		length = a * b / sqrt(pow(a * sin(pointAngle - angle), 2) + pow(b * cos(pointAngle - angle), 2));
		points[i].error = sqrt(pow(points[i].y - y, 2) + pow(points[i].x - x, 2)) - length;
		points[i].area = (M_PI - pointAngle) * averageR;
		points[i].angle = pointAngle;
	}
	//根据扫过的区域进行从小到大排列
	sort(points.begin(), points.end(), [](const PointCloud& A, const PointCloud& B) {return A.area < B.area; });
	std::vector<double> temp = TunnelClassify_(points);
	std::vector<PointCloud> selectPoint;
	for (int i = 0; i < points.size(); i++)
	{
		if (std::abs(points[i].error - temp[i]) < g_pipe_wall_limit && std::abs(points[i].error) < g_tunnel_face_value)
		{
			selectPoint.push_back(points[i]);
		}
	}
	return selectPoint;
}

/// <summary>
/// 基于拟合椭圆计算每点到椭圆的距离，提取管壁点，比上面的阈值要宽泛，用于滤波
/// </summary>
/// <param name="points">二维点</param>
/// <param name="a">长半轴</param>
/// <param name="b">短半轴</param>
/// <param name="x">圆心</param>
/// <param name="y">圆心</param>
/// <param name="angle">偏转角</param>
/// <returns>返回管壁上的点</returns>
std::vector<PointCloud> ExtractingPipeWallExtend_(std::vector<PointCloud>& points, double a, double b, double x, double y, double angle)
{
	double averageR = (a + b) / 2;	//取长短半轴的平均值
	double pointAngle;  //点的偏转角
	double length; //点对应的长度
	for (int i = 0; i < points.size(); i++)
	{
		pointAngle = atan2(points[i].y - y, points[i].x - x);
		//由椭圆方程得到点到椭圆圆心的距离
		length = a * b / sqrt(pow(a * sin(pointAngle - angle), 2) + pow(b * cos(pointAngle - angle), 2));
		points[i].error = sqrt(pow(points[i].y - y, 2) + pow(points[i].x - x, 2)) - length;
		points[i].area = (M_PI - pointAngle) * averageR;
		points[i].angle = pointAngle;
	}
	//根据扫过的区域进行从小到大排列
	sort(points.begin(), points.end(), [](const PointCloud& A, const PointCloud& B) {return A.area < B.area; });
	std::vector<double> temp = TunnelClassify_(points);
	std::vector<PointCloud> selectPoint;
	for (int i = 0; i < points.size(); i++)
	{

		if (abs(points[i].error - temp[i]) < g_pipe_wall_limit && abs(points[i].error) < g_tunnel_face_value * 2.5)
		{
			selectPoint.push_back(points[i]);
		}
	}
	return selectPoint;
}

/// <summary>
/// 提取管壁点
/// </summary>
/// <param name="points">二维点</param>
/// <returns>隧道点</returns>
std::vector<double> TunnelClassify_(const std::vector<PointCloud>& points)
{
	std::vector<double> area;
	std::vector<double> error;
	std::vector<double> result;
	//分别记录点的差距和扫过的面积
	for (int i = 0; i < points.size(); i++)
	{
		area.push_back(points[i].area);
		error.push_back(points[i].error);
	}
	int num = points.size();
	//记录前后两误差的深度值
	std::vector<double> depth;
	depth.push_back(0);
	for (int i = 1; i < num; i++)
	{
		depth.push_back(error[i] - error[i - 1]);
	}
	//计算深度
	std::vector<double> depthAfterSmooth = MovingSmooth_(depth);
	//计算深度差
	std::vector<double> depthDifference;
	//深度平均
	double depthAverage = 0;
	//深度方差
	double depthVariance = 0;
	for (int i = 0; i < depthAfterSmooth.size(); i++)
	{
		if (abs(depthAfterSmooth[i]) < g_depth_standard)
		{
			depthDifference.push_back(depthAfterSmooth[i]);
			depthAverage += depthAfterSmooth[i];
			depthVariance += (depthAfterSmooth[i] * depthAfterSmooth[i]);
		}
	}
	depthAverage /= depthDifference.size();
	depthVariance = depthVariance / depthDifference.size() - depthAverage * depthAverage;
	depthVariance = std::max(sqrt(depthVariance), g_depth_variance_standard);
	std::vector<double> s;
	for (int i = 0; i < num; i++)
	{
		s.push_back(0);
	}
	int st = 0;
	double sd = 0, lt = g_pipe_wall_start_and_end_init_value, ht = g_pipe_wall_start_and_end_init_value;
	double ht1 = depthAverage + depthVariance, ht2 = depthAverage - depthVariance;
	for (size_t i = 1; i < num; i++)
	{
		double hd = depthAfterSmooth[i];
		if (abs(depth[i]) < abs(depthAfterSmooth[i]) || abs(depth[i]) > ht) hd = depth[i];
		//根据前面计算的深度差均值depthAverage ，方差depthvariance，管壁点的深度差大概ht1-ht2之间
		if (hd < ht1 && hd > ht2)
		{
			st += 1;
			sd += area[i] - area[i - 1];
			//对满足深度差再ht1~ht2之间，前后长度sd>lt，单点深度在一定范围的点集，看作管壁点
			if (sd > lt && (abs(error[i]) < ht || abs(error[i] - s[i - 1]) < ht))
			{
				s[i] = (error[i] * 4 + s[i - 1] * 3) / 7;//管壁点参与s的计算
				for (int j = st - 1; j > 0; j--)
				{
					s[i - j] = (error[i - j] * 3 + s[i - j - 1] * 4) / 7;//管壁点参与s的计算
				}
			}
			else
			{
				s[i] = s[i - 1];//非管壁点s值不变
			}
		}
		else
		{
			st = 0;
			sd = 0;
			s[i] = s[i - 1];
		}//非管壁点s值不变
	}
	return s;
}

/// <summary>
/// 移动平均
/// </summary>
/// <param name="depth">深度</param>
/// <returns>深度</returns>
std::vector<double> MovingSmooth_(std::vector<double>& depth)
{
	//移动平均步长小于2直接退出
	if (g_smooth_step < 2) return depth;
	int n = depth.size();
	int span = std::min(n, g_smooth_step); //步长
	int width = span - 1 + span % 2;  //保证步长为奇数
	if (width == 1) return depth; //点数小于3或者步长小于3时返回数组
	int height = (width - 1) / 2;
	double sumBegin = 0, sumBehind = 0;
	int length = width - 2;
	double* tmp = new double[n]();
	for (int i = 0; i < length; i++)//前后半步步长内
	{
		sumBegin += depth[i];
		sumBehind += depth[n - 1 - i];
		if (i % 2 == 0)
		{
			tmp[i / 2] = sumBegin / ((double)i + 1);
			tmp[n - 1 - i / 2] = sumBehind / ((double)i + 1);
		}
	}
	tmp[height] = (sumBegin + depth[width - 2] + depth[width - 1]) / width;
	length = n - height;
	for (int i = height + 1; i < length; i++)
	{
		tmp[i] = tmp[i - 1] + (depth[i + height] - depth[i - 1 - height]) / width;
	}
	for (int i = 0; i < n; i++)
	{
		depth[i] = tmp[i];
	}
	delete[](tmp);
	tmp = NULL;
	return depth;
}