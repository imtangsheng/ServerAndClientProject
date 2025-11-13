#pragma once

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QImage>
#include <QImageReader>
#include <QRgb>
#include <vector>
#include <algorithm>
#include <cmath>

class ImageSharpnessEvaluator
{
public:
    struct EvaluationResult {
        QString picName;
        double maxMinDiff;
        bool isValid;

        EvaluationResult() : maxMinDiff(0.0), isValid(false) {}
        EvaluationResult(const QString& name, double diff)
            : picName(name), maxMinDiff(diff), isValid(true) {
        }
    };

    /**
     * @brief 将彩色图像转换为灰度图像
     * @param colorImage 输入的彩色图像
     * @return 灰度图像
     *
     * 使用标准的灰度转换公式：Gray = 0.299*R + 0.587*G + 0.114*B
     * OpenCV: 默认使用BGR格式，所以实际公式是：
     * Gray = 0.299*B + 0.587*G + 0.114*R  // 注意B和R位置
     * 这个公式考虑了人眼对不同颜色的敏感度差异
     */
    static QImage convertToGrayscale(const QImage& colorImage) {
        if (colorImage.isNull()) {
            return QImage();
        }

        // 创建灰度图像
        QImage grayImage(colorImage.size(), QImage::Format_Grayscale8);

        int width = colorImage.width();
        int height = colorImage.height();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = colorImage.pixel(x, y);

                // 提取RGB分量
                int red = qRed(pixel);
                int green = qGreen(pixel);
                int blue = qBlue(pixel);

                // 使用加权平均计算灰度值
                // 对应OpenCV的BGR顺序

                //int gray = static_cast<int>(0.114 * red + 0.587 * green + 0.299 * blue);
                int gray = static_cast<int>(0.299 * red + 0.587 * green + 0.114 * blue);

                // 确保灰度值在有效范围内
                gray = qBound(0, gray, 255);

                grayImage.setPixel(x, y, qRgb(gray, gray, gray));
            }
        }

        return grayImage;
    }

    /**
     * @brief 计算图像的能量梯度函数值
     * @param img 输入的灰度图像
     * @return 图像清晰度评价值，数值越大表示图像越清晰
     *
     * 原理：能量梯度函数通过计算像素点与其邻域像素的差值平方和来评估图像清晰度
     * 清晰的图像在边缘处会有较大的灰度变化，模糊图像的灰度变化相对平缓
     *
     * 计算公式：E = Σ[(I(x+1,y) - I(x,y))² + (I(x,y+1) - I(x,y))²]
     * 其中I(x,y)表示位置(x,y)处的像素灰度值
     */
    static double calculateEnergyGradient(const QImage& img) {
        if (img.isNull() || img.width() < 2 || img.height() < 2) {
            qWarning() << "Invalid input image for energy calculation";
            return 0.0;
        }

        double energy = 0.0;
        int width = img.width();
        int height = img.height();

        // 遍历图像中的每个像素点（除了最后一行和最后一列）
        for (int y = 0; y < height - 1; ++y) {
            for (int x = 0; x < width - 1; ++x) {
                // 获取当前像素及其右方和下方邻居的灰度值
                QRgb currentPixel = img.pixel(x, y);
                QRgb rightPixel = img.pixel(x + 1, y);
                QRgb bottomPixel = img.pixel(x, y + 1);

                // 提取灰度值（对于灰度图像，R=G=B）
                int currentGray = qRed(currentPixel);
                int rightGray = qRed(rightPixel);
                int bottomGray = qRed(bottomPixel);

                // 计算水平和垂直方向的梯度
                int horizontalGradient = rightGray - currentGray;
                int verticalGradient = bottomGray - currentGray;

                // 累加梯度平方和
                energy += (horizontalGradient * horizontalGradient) +
                    (verticalGradient * verticalGradient);
            }
        }

        return energy;
    }

    /**
     * @brief 优化版本的能量梯度计算，使用QImage的scanLine进行快速像素访问
     * @param img 输入的灰度图像
     * @return 图像清晰度评价值
     */
    static double calculateEnergyGradientOptimized(const QImage& img) {
        if (img.isNull() || img.width() < 2 || img.height() < 2) {
            qWarning() << "Invalid input image for energy calculation";
            return 0.0;
        }

        // 确保图像格式为8位灰度
        QImage grayImg = img;
        if (img.format() != QImage::Format_Grayscale8) {
            grayImg = img.convertToFormat(QImage::Format_Grayscale8);
        }

        double energy = 0.0;
        int width = grayImg.width();
        int height = grayImg.height();

        // 使用scanLine进行快速像素访问
        for (int y = 0; y < height - 1; ++y) {
            const uchar* currentLine = grayImg.scanLine(y);
            const uchar* nextLine = grayImg.scanLine(y + 1);

            for (int x = 0; x < width - 1; ++x) {
                // 获取当前像素及其邻居的灰度值
                int currentGray = currentLine[x];
                int rightGray = currentLine[x + 1];
                int bottomGray = nextLine[x];

                // 计算梯度
                int horizontalGradient = rightGray - currentGray;
                int verticalGradient = bottomGray - currentGray;

                // 累加梯度平方和
                energy += (horizontalGradient * horizontalGradient) +
                    (verticalGradient * verticalGradient);
            }
        }

        return energy;
    }

    /**
     * @brief 评估目录中所有图像的清晰度并找出最清晰的图像
     * @param directoryPath 图像文件目录路径
     * @param useOptimized 是否使用优化版本的算法
     * @return 评估结果，包含最清晰图像的文件名和清晰度范围
     */
    static EvaluationResult evaluateDirectory(const QString& directoryPath, bool useOptimized = true) {
        QDir directory(directoryPath);
        if (!directory.exists()) {
            qWarning() << "Directory does not exist:" << directoryPath;
            return EvaluationResult();
        }

        // 获取Qt支持的所有图像格式
        QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
        QStringList imageFilters;
        for (const QByteArray& format : supportedFormats) {
            imageFilters << QString("*.%1").arg(QString::fromLatin1(format.toLower()));
        }

        QFileInfoList fileInfoList = directory.entryInfoList(imageFilters,
            QDir::Files | QDir::Readable);

        if (fileInfoList.isEmpty()) {
            qWarning() << "No image files found in directory:" << directoryPath;
            qDebug() << "Supported formats:" << supportedFormats;
            return EvaluationResult();
        }

        // 存储每个文件的能量值和文件名
        std::vector<std::pair<double, QString>> energyResults;

        // 处理每个图像文件
        for (const QFileInfo& fileInfo : fileInfoList) {
            QString filePath = fileInfo.absoluteFilePath();

            // 使用Qt读取图像
            QImageReader reader(filePath);
            if (!reader.canRead()) {
                qWarning() << "Cannot read image format:" << filePath;
                continue;
            }

            QImage img = reader.read();
            if (img.isNull()) {
                qWarning() << "Failed to load image:" << filePath;
                qWarning() << "Error:" << reader.errorString();
                continue;
            }

            // 转换为灰度图像
            QImage grayImg;
            if (img.format() == QImage::Format_Grayscale8) {
                grayImg = img;
            } else {
                grayImg = convertToGrayscale(img);
            }
            
            // 计算能量梯度
            double energyValue;
            if (useOptimized) {
                energyValue = calculateEnergyGradientOptimized(grayImg);
            } else {
                energyValue = calculateEnergyGradient(grayImg);
            }

            energyResults.emplace_back(energyValue, fileInfo.fileName());

            qDebug() << QString("File: %1, Size: %2x%3, Energy: %4")
                .arg(fileInfo.fileName())
                .arg(img.width())
                .arg(img.height())
                .arg(energyValue, 0, 'f', 2);
        }

        // 处理结果
        return processResults(energyResults);
    }

    /**
     * @brief 获取Qt支持的图像格式信息
     * @return 支持的图像格式列表
     */
    static QStringList getSupportedImageFormats() {
        QList<QByteArray> formats = QImageReader::supportedImageFormats();
        QStringList formatStrings;
        for (const QByteArray& format : formats) {
            formatStrings << QString::fromLatin1(format);
        }
        return formatStrings;
    }

private:
    /**
     * @brief 处理评估结果，找出最清晰的图像
     * @param energyResults 所有图像的能量值和文件名对
     * @return 处理后的评估结果
     */
    static EvaluationResult processResults(std::vector<std::pair<double, QString>>& energyResults) {
        if (energyResults.empty()) {
            qWarning() << "No valid images processed";
            return EvaluationResult();
        }

        if (energyResults.size() == 1) {
            qDebug() << "Only one image processed:" << energyResults[0].second;
            return EvaluationResult(energyResults[0].second, 0.0);
        }

        // 按能量值排序（升序）
        std::sort(energyResults.begin(), energyResults.end());

        double minEnergy = energyResults.front().first;
        double maxEnergy = energyResults.back().first;
        QString bestImageName = energyResults.back().second;

        qDebug() << QString("Energy range: %1 - %2").arg(minEnergy, 0, 'f', 2).arg(maxEnergy, 0, 'f', 2);
        qDebug() << "Sharpest image:" << bestImageName;

        // 检查最高两个能量值是否相同（对焦失败的情况）
        if (energyResults.size() >= 2) {
            double secondMaxEnergy = energyResults[energyResults.size() - 2].first;
            const double tolerance = 1e-6; // 浮点数比较容差

            if (std::abs(maxEnergy - secondMaxEnergy) < tolerance) {
                qWarning() << "Focus failed: multiple images with same maximum energy value";
                qWarning() << QString("Max energy: %1, Second max: %2").arg(maxEnergy).arg(secondMaxEnergy);
                return EvaluationResult();
            }
        }

        return EvaluationResult(bestImageName, maxEnergy - minEnergy);
    }
};

// JSON输出辅助函数
QString resultToJson(const ImageSharpnessEvaluator::EvaluationResult& result) {
    QJsonObject jsonObj;

    if (!result.isValid) {
        jsonObj["result"] = -1;
    } else {
        jsonObj["pic_name"] = result.picName;
        jsonObj["Max-Min"] = result.maxMinDiff;
    }

    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact);
}

//#include <QCoreApplication>
//int main(int argc, char* argv[]) {
//    QCoreApplication app(argc, argv);
//
//    // 显示Qt支持的图像格式
//    QStringList supportedFormats = ImageSharpnessEvaluator::getSupportedImageFormats();
//    qDebug() << "Supported image formats:" << supportedFormats;
//
//    // 测试目录路径
//    QString directoryPath = "E:/Test/pictures";
//
//    // 如果有命令行参数，使用命令行参数作为目录路径
//    if (argc > 1) {
//        directoryPath = QString::fromLocal8Bit(argv[1]);
//    }
//
//    qDebug() << "Evaluating directory:" << directoryPath;
//
//    // 执行图像清晰度评估
//    ImageSharpnessEvaluator::EvaluationResult result =
//        ImageSharpnessEvaluator::evaluateDirectory(directoryPath, true);
//
//    // 输出JSON格式结果
//    QString jsonResult = resultToJson(result);
//    qDebug().noquote() << "Result:" << jsonResult;
//
//    return 0;
//}