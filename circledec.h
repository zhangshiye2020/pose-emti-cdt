#ifndef __CIRCLEDEC_H_
#define __CIRCLEDEC_H_

#define DEBUG
#define CIRCLE_NUM 9
#define BOARD_ROWS 3
#define BOARD_COLS 3

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <unordered_map>
#include <time.h>

typedef cv::Vec3f CircleType;    // float x,y,r

inline double getRoundness(double area, double arcLen) {
    return std::abs(1 - ((4 * CV_PI * area) / (arcLen * arcLen)));
}

/*
* 画直方图，统计用（调试）
*/
void drawHist(cv::Mat &src, cv::Mat &dst) {

}

void drawBoard(std::vector<std::vector<cv::Point>> contours) {   // 绘制板

}

/*
 * gamma矫正
 */
void gammaCorrection(cv::Mat &src, cv::Mat &dst, double gamma) {
    unsigned char lut[256];
    for (int i = 0; i < 256; i++) {
        lut[i] = cv::saturate_cast<uchar>(pow((float) i / 255, gamma) * 255.0f);
    }
    dst = src.clone();
    switch (src.type()) {
        case CV_8UC1: {
            for (auto it = dst.begin<uchar>(); it != dst.end<uchar>(); it++) {
                *it = lut[(*it)];
            }
            break;
        }
        case CV_8UC3: {
            for (auto it = dst.begin<cv::Vec3b>(); it != dst.end<cv::Vec3b>(); it++) {
                (*it)[0] = lut[(*it)[0]];
                (*it)[1] = lut[(*it)[1]];
                (*it)[2] = lut[(*it)[2]];
            }
            break;
        }
        default:
            break;
    }
}

/*
* 自动gamma矫正
*/
void autoGamma(cv::Mat &src, cv::Mat &dst) {
    const int channels = src.channels();
    const int type = src.type();
    assert(type == CV_8U || type == CV_8UC3);    // 只限8bit

    // 计算中位数
    cv::Scalar mean_scalar = cv::mean(src);
    double mean = mean_scalar[0];

    // 某篇论文的gamma value设置值
    double gamma_value = std::log10(0.5) / std::log10(mean / 255);
//    GammaCorrection(src, dst, gamma_value);

//    double gamma_value = 0.3;
    // 归一化，然后gamma变换
    cv::Mat norm, gamma;
    cv::normalize(src, norm, 1.0, 0.0, cv::NORM_MINMAX, CV_64F);
    //src.convertTo(norm,CV_64F,1/255.0);
    cv::pow(norm, gamma_value, gamma);

    cv::convertScaleAbs(gamma, dst, 255.0);

#ifdef DEBUG
    cv::imwrite("gamma.jpg", dst);
#endif

}

/*
* 预处理，包含一些图像增强方法，主要包括直方图均衡，二值化自适应，中值滤波
*/
void pretreatment(cv::Mat &src, cv::Mat &dst) {
    clock_t start = clock();
    cv::Mat gray, bin, equalize;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    // 直方图均衡算法
    cv::equalizeHist(gray, equalize);

    // gamma矫正
//    cv::Mat gamma;
//    autoGamma(gray, gamma);

    // 二值化: OTSU
    //cv::threshold(equalize, bin, 24, 255, cv::THRESH_OTSU);
    //cv::imwrite("otsu.jpg", bin);

    // 二值化，自适应，先用该算法试试
    cv::Mat adaptBin;
    cv::adaptiveThreshold(equalize, adaptBin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 191, 21);
    cv::bitwise_not(adaptBin, adaptBin);

    // 椒盐去噪(中值滤波)
    cv::Mat blur;
    cv::medianBlur(adaptBin, dst, 3);
    clock_t end = clock();
#ifdef DEBUG
    std::cout << "Time of pretreatment: " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
#endif
#ifdef DEBUG
    cv::imwrite("gray.jpg", gray);
    cv::imwrite("equalize.jpg", equalize);
    cv::imwrite("adaptiveBinary.jpg", adaptBin);
    cv::imwrite("blur.jpg", dst);
#endif
}

/*
 * 从轮廓中拟合圆，算法粗糙
 * contour:vector<Point> 圆的轮廓
 * c:CircleType-Vec3f    圆
 */
void fitCircle(std::vector<cv::Point> &contour, CircleType &c) {
    cv::RotatedRect rect = cv::fitEllipse(contour);
    cv::Point2f center = rect.center;
    float r = (rect.size.width + rect.size.height) / 4;
    c[0] = center.x;
    c[1] = center.y;
    c[2] = r;
}

/*
 * 过滤无效的边缘，返回轮廓下标
 * 主要通过轮廓的面积(> 50)、圆度、以及结构关系判断
 * contours:   边缘
 * hierarchy:  轮廓之间的树形关系
 * contoursIndex: 圆形轮廓下标
 */
void filterContours(std::vector<std::vector<cv::Point>> &contours, std::vector<cv::Vec4i> &hierarchy,
                    std::vector<int> &contoursIndex) {
    clock_t start = clock();
    std::unordered_map<int, std::vector<int>> map;   // 父子关系图
    double area, arcLen;
    for (int i = 0; i < contours.size(); i++) {
        area = cv::contourArea(contours[i]);
        arcLen = cv::arcLength(contours[i], true);

        if (area < 50) {
            continue;
        }
        double roundness = getRoundness(area, arcLen);
        if (roundness > 0.175) {
            continue;
        }
        int p = hierarchy[i][3];
//        map.at(p).push_back(i);
        map[p].push_back(i);

    }
    for (auto iter = map.begin(); iter != map.end(); iter++) {  // 依据父节点做二次赛选
        if (iter->first == -1) {    // 可以认为多余了
            continue;
        }
        // todo: 鲁棒性不强...
        if ((iter->second).size() == CIRCLE_NUM) {    // 误检率要求最低，同一个父节点要求有9个
//            contoursIndex.clear();
//            contoursIndex.insert(contoursIndex.begin(), iter->second.begin(), iter->second.end());
#ifdef DEBUG
//            std::cout << i << ": " << area << ", " << roundness << ", " << hierarchy[i][3] << std::endl;
            std::cout << "PIndex: " << iter->first << ", Num: " << CIRCLE_NUM << std::endl;
#endif
            contoursIndex.assign(iter->second.begin(), iter->second.end());
        }
    }
    clock_t end = clock();
#ifdef DEBUG
    std::cout << "Time of filter contour: " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
#endif
}

/*
* 通过轮廓找到圆
* src:		通过图像处理后的图片,二值化图片
* circles:	圆形
*/
void findCircleByContours(cv::Mat &src, std::vector<CircleType> &circles) {    // 通过轮廓找到圆

    clock_t start = clock();
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy; // 树形结构层次关系
    cv::findContours(src, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
    clock_t end = clock();
#ifdef DEBUG
    std::cout << "Time of find contours: " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    cv::Mat picFul(src.rows, src.cols, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::drawContours(picFul, contours, -1, cv::Scalar(255, 255, 255));
    cv::imwrite("contoursFull.jpg", picFul);
#endif

    std::vector<int> contoursIndex;
    filterContours(contours, hierarchy, contoursIndex); // 圆形轮廓下标

    // 轮廓
//    std::vector<std::vector<cv::Point>> circleContours;
    clock_t startOfFitCircle = clock();
    CircleType c;
    std::vector<cv::Point> circleContour;
    for (int i = 0; i < CIRCLE_NUM; i++) {
        int cIndex = contoursIndex[i];
        circleContour = contours[cIndex];
        fitCircle(circleContour, c);
        circles.push_back(c);
    }
    clock_t endOfFitCircle = clock();
#ifdef DEBUG
    std::cout << "Time of fit circle: " << double(endOfFitCircle - startOfFitCircle) / CLOCKS_PER_SEC << "s"
              << std::endl;
#endif

#ifdef DEBUG
    cv::Mat pic(src.rows, src.cols, CV_8UC3, cv::Scalar(0, 0, 0));
    for (int i = 0; i < contoursIndex.size(); i++) {
        int cIndex = contoursIndex[i];
        cv::drawContours(pic, contours, cIndex, cv::Scalar(255, 255, 255));
    }
    cv::imwrite("contours.jpg", pic);
#endif
}

/*
 * 已废弃，该连通域函数结果无法计算出其周长
 */
void findCircleByConnectedComponents(cv::Mat src, std::vector<CircleType> &circles) {
    //pretreatment(src, );
    cv::Mat labels, stats, centroids;
    int n_components = cv::connectedComponentsWithStats(src, labels, stats, centroids);    // 连通域
    for (int i = 0; i < n_components; i++) {
        int x = stats.at<int>(i, cv::CC_STAT_LEFT);
        int y = stats.at<int>(i, cv::CC_STAT_TOP);
        int height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
        int width = stats.at<int>(i, cv::CC_STAT_WIDTH);
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        //std::cout << i << ":" << area << std::endl;
        //cv::arcLength()
    }

}

#endif // __CIRCLEDEC_H_
