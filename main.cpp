#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "circledec.h"
#include <time.h>

using namespace std;
/*
 * 聚类算法分类，但是效果不一定好
 */
void filterContoursByArea(cv::Mat contourAreas, int classes) {
    // 聚类算法
    cv::Mat labels;
    cv::kmeans(contourAreas, classes, labels,
               cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 10, 0), 5, cv::KMEANS_PP_CENTERS);

    for (int i = 0; i < labels.rows; i++) {
        cout << " " << labels.at<int>(i, 0);
    }
    cout << endl;
    for (int i = 0; i < contourAreas.rows; i++) {
        cout << " " << contourAreas.at<float>(i, 0);
    }
}

int main(int argc, char **argv) {
	clock_t start = clock();
	string folderpath = "/home/zhangshiye2020/图片/";
	string filenameNoExtension = "target";
	string extensionName = ".png";
	string filepath = folderpath + filenameNoExtension + extensionName;
    cv::Mat mat = cv::imread(filepath);
    cv::Mat dst, pre_src;
    pretreatment(mat, pre_src);
    vector<CircleType> circles;
    findCircleByContours(pre_src, circles);
    clock_t end = clock();
#ifdef DEBUG
    cout << "Time of total: " << double(end-start)/CLOCKS_PER_SEC << "s" << endl;
#endif
    return 0;
}
