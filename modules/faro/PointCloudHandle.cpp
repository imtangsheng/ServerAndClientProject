#include "PointCloudHandle.h"
#include <fstream>
#include <algorithm>
//typedef unsigned char uchar;

void Image::save(const std::string& path)
{
    std::ofstream file(path, std::ios::binary);
    if (!file) return;

    // д��򵥵�ͼ��ͷ
    file.write("P5\n", 3);
    file << width << " " << height << "\n255\n";

    // д��ͼ������
    for (const auto& row : data) {
        file.write(reinterpret_cast<const char*>(row.data()), width);
    }
}

void Image::save(std::vector<std::vector<unsigned char>> color, int width, int height, std::string savepath)
{
    Image grayImage(width, height);
    //����ά���鸳ֵ��Mat����
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            grayImage.at<uchar>(i, j) = color[i][j];
        }
    }
    //����Ҷ�ͼ��
    grayImage.save(savepath);
}

PointCloudImage::PointCloudImage()
{
}

PointCloudImage::~PointCloudImage()
{
}




// ֱ��ͼ���⻯����
static void _HistogramEqualization(const std::vector<std::vector<PointCloudSection>>& tunnel, std::vector<int>& grayEqual)
{
    int row = tunnel.size();
    int col = tunnel[0].size();
    int pointsNum = row * col;

    // ��ʼ��ͳ������
    std::vector<int> gray(256, 0);
    std::vector<double> grayProb(256, 0.0);
    std::vector<double> grayDist(256, 0.0);
    grayEqual.resize(256, 0);

    // ͳ�ƻҶ�ֵ�ֲ�
    for (const auto& row : tunnel) {
        for (const auto& point : row) {
            gray[point.color]++;
        }
    }

    // ����Ҷȸ��ʷֲ�
    const double alpha = 0.05;
    for (int i = 0; i < 256; i++) {
        grayProb[i] = static_cast<double>(gray[i]) / pointsNum;
    }

    // �����ۻ��ֲ�
    grayDist[0] = grayProb[0];
    for (int i = 1; i < 256; i++) {
        grayDist[i] = grayDist[i - 1] + grayProb[i] * std::pow(i / 255.0, alpha);
    }

    // ������⻯��ĻҶ�ֵ
    for (int i = 0; i < 256; i++) {
        grayEqual[i] = static_cast<uchar>(255 * grayDist[i] + 0.5);
    }
}

// ������
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

    // �������� (���ø�˹-Լ����Ԫ��)
    Matrix inverse() {
        Matrix augmented(rows, 2 * cols);
        // �����������
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                augmented(i, j) = data[i][j];
                augmented(i, j + cols) = (i == j) ? 1.0 : 0.0;
            }
        }

        // ��˹-Լ����Ԫ
        for (int i = 0; i < rows; i++) {
            // Ѱ����Ԫ
            double maxEl = std::abs(augmented(i, i));
            int maxRow = i;
            for (int k = i + 1; k < rows; k++) {
                if (std::abs(augmented(k, i)) > maxEl) {
                    maxEl = std::abs(augmented(k, i));
                    maxRow = k;
                }
            }

            // ������
            if (maxRow != i) {
                for (int k = i; k < 2 * cols; k++) {
                    std::swap(augmented(maxRow, k), augmented(i, k));
                }
            }

            // �����Խ���Ԫ�ع�һ��
            double div = augmented(i, i);
            for (int j = i; j < 2 * cols; j++) {
                augmented(i, j) /= div;
            }

            // ��Ԫ
            for (int j = 0; j < rows; j++) {
                if (i != j) {
                    double mult = augmented(j, i);
                    for (int k = i; k < 2 * cols; k++) {
                        augmented(j, k) -= augmented(i, k) * mult;
                    }
                }
            }
        }

        // ��ȡ����󲿷�
        Matrix inverse(rows, cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                inverse(i, j) = augmented(i, j + cols);
            }
        }
        return inverse;
    }

    // ����˷�
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
/// <summary>
/// ��5�ζ���ʽ�����Բ����
/// </summary>
/// <param name="points">��ά��</param>
/// <param name="a">������</param>
/// <param name="b">�̰���</param>
/// <param name="x">Բ��</param>
/// <param name="y">Բ��</param>
/// <param name="angle">ƫת��</param>
static void _EllipseFitting(std::vector<PointCloudSection> points, double& a, double& b, double& x, double& y, double& angle)
{
    // ����5x5ϵ�������5x1��������
    Matrix conf(5, 5);
    Matrix cons(5, 1);

    if (points.empty())
    {
        return;
    }
    for (const auto& p : points)
    {
        double conf10 = p.x, conf01 = p.y, conf20 = p.x * p.x, conf02 = p.y * p.y, conf11 = p.x * p.y,
            conf30 = conf20 * p.x, conf21 = conf20 * p.y, conf12 = p.x * conf02, conf03 = conf02 * p.y,
            conf31 = conf30 * p.y, conf22 = conf20 * conf02, conf13 = p.x * conf03, conf04 = conf03 * p.y;
        // ���ϵ������
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
        // ��䳣������
        cons(0, 0) -= conf31;
        cons(1, 0) -= conf22;
        cons(2, 0) -= conf30;
        cons(3, 0) -= conf21;
        cons(4, 0) -= conf20;

    }
    //A * B = C -> B = Inv(A) * B
    Matrix fittingResult = conf.inverse() * cons;
    double A = fittingResult(0, 0), B = fittingResult(1, 0), C = fittingResult(2, 0), D = fittingResult(3, 0), E = fittingResult(4, 0);
    //��ʽ
    // �������ĵ�
    double denominator = A * A - 4 * B;
    x = (2 * B * C - A * D) / denominator;
    y = (2 * D - A * C) / denominator;
    // ���㳤�������ת��
    double temp = 2 * (A * C * D - B * C * C - D * D + 4 * B * E - A * A * E);
    double sqrtTerm = sqrt(A * A + (1 - B) * (1 - B));
    a = sqrt(temp / (denominator * (B - sqrtTerm + 1)));
    b = sqrt(temp / (denominator * (B + sqrtTerm + 1)));
    // ������ת�Ƕ�
    angle = atan(sqrt((a * a - b * b * B) / (a * a * B - b * b)));
    if (A > 0)
    {
        angle = -angle;
    }
}

/// <summary>
/// ��ά���ϡ(���Ƕ�)
/// </summary>
/// <param name="points">��ά��</param>
/// <param name="x">Բ��</param>
/// <param name="y">Բ��</param>
/// <param name="thinning">��ϡ����</param>
/// <returns>��ϡ��Ķ�ά��</returns>
static std::vector<PointCloudSection> _ThinningPoints(std::vector<PointCloudSection> points, double x, double y, double thinning)
{
    //�����ķ�λ�ǣ���󰴷�λ�Ǵ�С���У��Ա��ڰ�������ϡ
    for (int i = 0; i < points.size(); i++)
    {
        double dy = points[i].y - y, dx = points[i].x - x;
        points[i].angle = atan2(dy, dx);
    }
    sort(points.begin(), points.end(), [](const PointCloudSection& A, const PointCloudSection& B) {return A.angle < B.angle; });
    if (points.size() == 0) return points;
    double angleMin = points[0].angle;//��С�Ľ�
    double angleMax = points[points.size() - 1].angle;//���Ľ�
    double interval = 2 * M_PI / thinning;//��ϡ����
    int index = 0; //���
    std::vector<PointCloudSection> selectPoint;
    double piece = (angleMax - angleMin) / interval;
    //ԭ������һ����ϡ������Ȼ�����������ϡС���а����ķ�λ�ǣ���ɳ�ϡ
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
/// �ȶ���Բ����
/// </summary>
/// <param name="points">��ά��</param>
/// <param name="dDesignSecRadius">������ư뾶</param>
/// <param name="a">������</param>
/// <param name="b">�̰���</param>
/// <param name="x">Բ��</param>
/// <param name="y">Բ��</param>
/// <param name="angle">ƫת��</param>
static void _StableFittingParameters(std::vector<PointCloudSection> points, double dDesignSecRadius, double& a, double& b, double& x, double& y, double& angle)
{
    //����Բ����΢ƫ�ƣ��õ�һ��ѭ������������
    double xLast = x + 1;
    double yLast = y + 1;
    //�ж�����Բ�ĵľ���ƫ��ֵ�Ƿ�>0.001
    while (std::max(std::abs(xLast - x), std::abs(yLast - y)) > 0.001)
    {
        double errorAddition = 0;//ƽ�����
        //�������
        for (int i = 0; i < points.size(); i++)
        {
            points[i].error = abs(std::sqrt((points[i].x - x) * (points[i].x - x) + (points[i].y - y) * (points[i].y - y)) - dDesignSecRadius);
            errorAddition += points[i].error;
        }
        errorAddition = errorAddition / points.size();
        //��������2�����ĵ��޳�
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
        //����һ����Բ���
        _EllipseFitting(points, a, b, x, y, angle);
    }
}

/// <summary>
/// �ƶ�ƽ��
/// </summary>
/// <param name="depth">���</param>
/// <returns>���</returns>
static int gSmoothStep = 5;                 //�ƶ�����
static std::vector<double> _MovingSmooth(std::vector<double> depth)
{
    //�ƶ�ƽ������С��2ֱ���˳�
    if (gSmoothStep < 2) return depth;
    int n = depth.size();
    int span = std::min(n, gSmoothStep); //����
    int width = span - 1 + span % 2;  //��֤����Ϊ����
    if (width == 1) return depth; //����С��3���߲���С��3ʱ��������
    int height = (width - 1) / 2;
    double sumBegin = 0, sumBehind = 0;
    int length = width - 2;
    double* tmp = new double[n]();
    for (int i = 0; i < length; i++)//ǰ��벽������
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

static double gDepthStandard = 0.005;   //��ȱ�׼
static double gPipeWallStartAndEndInit = 0.02;   //�ܱڵ�ǰ�󳤶ȳ�ʼ��
static double gDepthVarianceStandard = 0.0005;   //��ȷ����׼
/// <summary>
/// ��ȡ�ܱڵ�
/// </summary>
/// <param name="points">��ά��</param>
/// <returns>�����</returns>
static std::vector<double> tunnelClassify(std::vector<PointCloudSection> points)
{
    std::vector<double> area;
    std::vector<double> error;
    std::vector<double> result;
    //�ֱ��¼��Ĳ���ɨ�������
    for (int i = 0; i < points.size(); i++)
    {
        area.push_back(points[i].area);
        error.push_back(points[i].error);
    }
    int num = points.size();
    //��¼ǰ�����������ֵ
    std::vector<double> depth;
    depth.push_back(0);
    for (int i = 1; i < num; i++)
    {
        depth.push_back(error[i] - error[i - 1]);
    }
    //�������
    std::vector<double> depthAfterSmooth = _MovingSmooth(depth);
    //������Ȳ�
    std::vector<double> depthDifference;
    //���ƽ��
    double depthAverage = 0;
    //��ȷ���
    double depthVariance = 0;
    for (int i = 0; i < depthAfterSmooth.size(); i++)
    {
        if (abs(depthAfterSmooth[i]) < gDepthStandard)
        {
            depthDifference.push_back(depthAfterSmooth[i]);
            depthAverage += depthAfterSmooth[i];
            depthVariance += (depthAfterSmooth[i] * depthAfterSmooth[i]);
        }
    }
    depthAverage /= depthDifference.size();
    depthVariance = depthVariance / depthDifference.size() - depthAverage * depthAverage;
    depthVariance = std::max(sqrt(depthVariance), gDepthVarianceStandard);
    std::vector<double> s;
    for (int i = 0; i < num; i++)
    {
        s.push_back(0);
    }
    int st = 0;
    double sd = 0, lt = gPipeWallStartAndEndInit, ht = gPipeWallStartAndEndInit;
    double ht1 = depthAverage + depthVariance, ht2 = depthAverage - depthVariance;
    for (int i = 1; i < num; i++)
    {
        double hd = depthAfterSmooth[i];
        if (abs(depth[i]) < abs(depthAfterSmooth[i]) || abs(depth[i]) > ht) hd = depth[i];
        //����ǰ��������Ȳ��ֵdepthAverage ������depthvariance���ܱڵ����Ȳ���ht1-ht2֮��
        if (hd < ht1 && hd > ht2)
        {
            st += 1;
            sd += area[i] - area[i - 1];
            //��������Ȳ���ht1~ht2֮�䣬ǰ�󳤶�sd>lt�����������һ����Χ�ĵ㼯�������ܱڵ�
            if (sd > lt && (abs(error[i]) < ht || abs(error[i] - s[i - 1]) < ht))
            {
                s[i] = (error[i] * 4 + s[i - 1] * 3) / 7;//�ܱڵ����s�ļ���
                for (int j = st - 1; j > 0; j--)
                {
                    s[i - j] = (error[i - j] * 3 + s[i - j - 1] * 4) / 7;//�ܱڵ����s�ļ���
                }
            }
            else
            {
                s[i] = s[i - 1];//�ǹܱڵ�sֵ����
            }
        }
        else
        {
            st = 0;
            sd = 0;
            s[i] = s[i - 1];
        }//�ǹܱڵ�sֵ����
    }
    return s;
}

static double gPipeWallLimit = 0.02;    //�ܱڵ���ֵ
static double gTunnelFaceValue = 0.02;  //ȥ������������ֵ
/// <summary>
/// ���������Բ����ÿ�㵽��Բ�ľ��룬��ȡ�ܱڵ�
/// </summary>
/// <param name="points">��ά��</param>
/// <param name="a">������</param>
/// <param name="b">�̰���</param>
/// <param name="x">Բ��</param>
/// <param name="y">Բ��</param>
/// <param name="angle">ƫת��</param>
/// <returns>���عܱ��ϵĵ�</returns>
static std::vector<PointCloudSection> _ExtractingPipeWall(std::vector<PointCloudSection> points, double a, double b, double x, double y, double angle)
{
    double averageR = (a + b) / 2;	//ȡ���̰����ƽ��ֵ
    double pointAngle;  //���ƫת��
    double length; //���Ӧ�ĳ���
    for (int i = 0; i < points.size(); i++)
    {
        pointAngle = atan2(points[i].y - y, points[i].x - x);
        //����Բ���̵õ��㵽��ԲԲ�ĵľ���
        length = a * b / sqrt(pow(a * sin(pointAngle - angle), 2) + pow(b * cos(pointAngle - angle), 2));
        points[i].error = sqrt(pow(points[i].y - y, 2) + pow(points[i].x - x, 2)) - length;
        points[i].area = (M_PI - pointAngle) * averageR;
        points[i].angle = pointAngle;
    }
    //����ɨ����������д�С��������
    sort(points.begin(), points.end(), [](const PointCloudSection& A, const PointCloudSection& B) {return A.area < B.area; });
    std::vector<double> temp = tunnelClassify(points);
    std::vector<PointCloudSection> selectPoint;
    for (int i = 0; i < points.size(); i++)
    {
        if (std::abs(points[i].error - temp[i]) < gPipeWallLimit && std::abs(points[i].error) < gTunnelFaceValue)
        {
            selectPoint.push_back(points[i]);
        }
    }
    return selectPoint;
}

/// <summary>
/// ���������Բ����ÿ�㵽��Բ�ľ��룬��ȡ�ܱڵ㣬���������ֵҪ���������˲�
/// </summary>
/// <param name="points">��ά��</param>
/// <param name="a">������</param>
/// <param name="b">�̰���</param>
/// <param name="x">Բ��</param>
/// <param name="y">Բ��</param>
/// <param name="angle">ƫת��</param>
/// <returns>���عܱ��ϵĵ�</returns>
static std::vector<PointCloudSection> _ExtractingPipeWallExtend(std::vector<PointCloudSection> points, double a, double b, double x, double y, double angle)
{
    double averageR = (a + b) / 2;	//ȡ���̰����ƽ��ֵ
    double pointAngle;  //���ƫת��
    double length; //���Ӧ�ĳ���
    for (int i = 0; i < points.size(); i++)
    {
        pointAngle = atan2(points[i].y - y, points[i].x - x);
        //����Բ���̵õ��㵽��ԲԲ�ĵľ���
        length = a * b / sqrt(pow(a * sin(pointAngle - angle), 2) + pow(b * cos(pointAngle - angle), 2));
        points[i].error = sqrt(pow(points[i].y - y, 2) + pow(points[i].x - x, 2)) - length;
        points[i].area = (M_PI - pointAngle) * averageR;
        points[i].angle = pointAngle;
    }
    //����ɨ����������д�С��������
    sort(points.begin(), points.end(), [](const PointCloudSection& A, const PointCloudSection& B) {return A.area < B.area; });
    std::vector<double> temp = tunnelClassify(points);
    std::vector<PointCloudSection> selectPoint;
    for (int i = 0; i < points.size(); i++)
    {

        if (abs(points[i].error - temp[i]) < gPipeWallLimit && abs(points[i].error) < gTunnelFaceValue * 2.5)
        {
            selectPoint.push_back(points[i]);
        }
    }
    return selectPoint;
}

/**
 * ���˵���
 * @param points ��������
 * @param fitSection �����Բ
 * @param pointAfterSolution ���˺�ĵ���
 * @param dDesignSecRadius ��ư뾶
 */
static void _SolvePointWithFiltered(std::vector<PointCloudSection>& points,
    FittingEllipse& fitSection, std::vector<PointCloudSection>& pointAfterSolution,
    double dDesignSecRadius) {
    double firstThinning = 360;
    double secondThinning = 1800;
    double a, b, x, y, angle;
    //һ�����
    _EllipseFitting(points, a, b, x, y, angle);
    //��ϡ
    std::vector<PointCloudSection> pointAfterThinning = _ThinningPoints(points, x, y, firstThinning);
    //�������
    _EllipseFitting(pointAfterThinning, a, b, x, y, angle);
    //���ȶ�����Բ����
    _StableFittingParameters(points, dDesignSecRadius, a, b, x, y, angle);
    //���������ȶ�����Բ������ܱڵ�
    std::vector<PointCloudSection> pipeWallPoint = _ExtractingPipeWall(points, a, b, x, y, angle);
    //���������ȶ�����Բ�����˲�ȫ���ܱڵ㣬��һ��������
    pointAfterSolution = _ExtractingPipeWallExtend(points, a, b, x, y, angle);
    //��ϡ
    std::vector<PointCloudSection> pointAfterDownsampling = _ThinningPoints(pipeWallPoint, x, y, secondThinning);
    //�������
    _EllipseFitting(pointAfterDownsampling, a, b, x, y, angle);
    fitSection.A = a;
    fitSection.B = b;
    fitSection.x = x;
    fitSection.y = y;
    fitSection.angle = angle;
}

inline static bool _CMP(const PointCloudXYZCT& a, const PointCloudXYZCT& b)
{
    return a.x < b.x;
}

// ��Բ��ϴ���
static void _FitEllipseForSection(const std::vector<PointCloudSection>& section,
    int index,
    std::vector<double>& minY,
    std::vector<MileageEllipse>& lme,
    double designSecRadius) {
    std::vector<PointCloudSection> tunnelDownsample;

    // ���Ƴ�ϡ����
    if (section.size() > 6000) {
        std::vector<PointCloudSection> sortedSection = section;
        std::sort(sortedSection.begin(), sortedSection.end(), _CMP);
        int sampleRate = section.size() / 3000;
        for (int i = 0; i < sortedSection.size(); i += sampleRate) {
            tunnelDownsample.push_back(sortedSection[i]);
        }
    }
    else {
        tunnelDownsample = section;
    }

    // ��ȡ��Ч�������Բ���
    std::vector<PointCloudSection> validPoints;
    for (const auto& point : tunnelDownsample) {
        if (std::abs(point.x) < 30 && std::abs(point.z) < 30 &&
            point.x != 0 && point.z != 0) {
            validPoints.push_back(point);
        }
    }

    // ������Բ���
    FittingEllipse ellipse;
    std::vector<PointCloudSection> filteredPoints;
    _SolvePointWithFiltered(validPoints, ellipse, filteredPoints, designSecRadius);

    // ���������Ϣ
    auto minMileage = std::min_element(validPoints.begin(), validPoints.end(),
        [](const auto& a, const auto& b) { return a.mileage < b.mileage; });

    if (minMileage != validPoints.end()) {
        minY.push_back(minMileage->mileage);
        double dm = (index == 0) ? 0 : (minY.back() - minY[minY.size() - 2]) / 30;
        if (index > 0) {
            lme[lme.size() - 1].dm = dm;
        }
        lme.push_back({ minY.back(), dm, index, ellipse });
    }
}

// ����
static double gHoleDepthLimit = 0.09;	//��˨����ȹ�����ֵ
static void _UpdateDepthImage(std::vector<std::vector<unsigned char>>& depthImage,
    int row, int col, double depth) {
    if (depth < gHoleDepthLimit / 7) {
        depthImage[row][col] = 0;
    }
    else if (depth < gHoleDepthLimit) {
        depthImage[row][col] = static_cast<unsigned char>(255 * depth / gHoleDepthLimit);
    }
    else {
        depthImage[row][col] = 255;
    }
}

static void _InterpolatePixel(Image& grayImage,
    std::vector<std::vector<unsigned char>>& depthImage,
    int x, int y, int windowSize, int height, int width) {
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

    if (count > 0) {
        grayImage.at<uchar>(y, x) = static_cast<uchar>(sumGray / count);
        depthImage[y][x] = static_cast<unsigned char>(sumDepth / count);
    }
}

static void _ProcessImages(Image& grayImage,
    std::vector<std::vector<unsigned char>>& depthImage,
    int height, int width) {
    // ���հ�����
    const int windowSize = 3;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grayImage.at<uchar>(y, x) == 0) {
                _InterpolatePixel(grayImage, depthImage, x, y, windowSize, height, width);
            }
        }
    }
}

//�޼�����Բ����ʱ������������õ���Բ����
static void _GetEllipse(EllipseParams& ellipse, double dRadius, double dScanerHeight)
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

//������Բ�����Լ�ɨ����x��z���꣬�����ư�ͶӰ�����Բ�����ϣ����Ӱ��ͼ�������꣬������Ϊ���
static void _GetRealOrdinate(const EllipseParams& ellipse, double x, double z, double& xe)
{
    double r = ellipse.A; //A,B������r
    double Xr, Yr;
    if (x == 0)
    {
        Xr = 0;
        Yr = ellipse.Y0 + std::sqrt(r * r - ellipse.X0 * ellipse.X0);
    }
    else
    {
        //����Yr=ta*Xr�ͣ�Xr-ellipse.X0)^2+(Yr-ellipse.Y0)^2=r^2�����Xr��Yr
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
    xe = (M_PI - std::atan2(Xr - ellipse.X0, Yr - ellipse.Y0)) * r;     //ÿ�������Բչ��
}

//��Բ��һ�㵽ԭ��ľ���
static double _GetDxyc(double x, double y, double x0, double y0, double a, double b, double angle)
{
    double dx = x - x0;
    double dy = y - y0;
    double A = std::atan2(y - y0, x - x0);
    A -= angle;
    double r = a * b / std::sqrt(std::pow(a * std::sin(A), 2) + std::pow(b * std::cos(A), 2));  //��Բ��һ�㵽ԭ��ľ���
    return (std::sqrt(dx * dx + dy * dy) - r);
}

bool PointCloudImage::generateImages(
    std::vector<MileageEllipse>& lme,
    std::vector<std::vector<PointCloudSection>>& tunnel,
    const std::string& savephotopathGray,
    const std::string& savephotopathDepth,
    double start_mileage,
    double end_mileage,
    int expand,
    double dDesignSecRadius,
    double dScannerHeight)
{
    _GetEllipse(m_ellipse, dDesignSecRadius, dScannerHeight);

    int splitMap = 0;   //ÿ50Ȧһ��Ļ�һ���ּܷ���
    double diff = end_mileage - start_mileage;   //Ӧ����һ��fls��ʵ�ʼ�����
    // ����ͼ��ߴ�
    int imageHeight = (int)(2 * M_PI * dDesignSecRadius * expand) + 1;  //��
    int imageWidth = (int)(diff * expand) + 1;  //��
    // ����ͼ�����
    Image grayImage(imageWidth, imageHeight);
    std::vector<std::vector<unsigned char>> depthImage(imageHeight + 1, std::vector<unsigned char>(imageWidth + 1));
    // ����ֱ��ͼ���⻯
    std::vector<int> grayEqual;
    _HistogramEqualization(tunnel, grayEqual);

    // #2 ����ÿһȦ�ĵ�������
    std::vector<double> minY;
    for (int i = 0; i < tunnel.size(); i++) {
        // ÿ30Ȧ����һ����Բ���
        if (i % 30 == 0) {
            _FitEllipseForSection(tunnel[i], i, minY, lme, dDesignSecRadius);
        }

        // ����ÿ����
        for (const auto& point : tunnel[i])
        {
            // ����չ������
            double xe;
            _GetRealOrdinate(m_ellipse, point.x, point.z, xe);

            int row = static_cast<int>(xe * expand);
            int col = static_cast<int>((point.y - start_mileage) * expand);

            // ���»Ҷ�ͼ
            if (row < imageHeight && col < imageWidth) {
                grayImage.at<uchar>(row, col) = grayEqual[point.color];
                // �������ֵ
                double depth = _GetDxyc(point.x, point.z,
                    lme.back().ep.x, lme.back().ep.y,
                    lme.back().ep.A, lme.back().ep.B,
                    lme.back().ep.angle);

                // �������ͼ
                if (depth == 0) {
                    depth = std::sqrt(point.x * point.x + point.z * point.z) - dDesignSecRadius;
                }
                _UpdateDepthImage(depthImage, row, col, depth);
            }
        }
    }
    // ͼ�����
    _ProcessImages(grayImage, depthImage, imageHeight, imageWidth);

    grayImage.save(savephotopathGray);
    Image::save(depthImage, imageWidth, imageHeight, savephotopathDepth);   //�������ͼ�ļ�
    return true;
}


