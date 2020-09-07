/* -*-c++-*- SemiGlobalMatching - Copyright (C) 2020.
* Author	: Ethan Li <ethan.li.whu@gmail.com>
* https://github.com/ethan-li-coding/SemiGlobalMatching
*/
#include<SemiGlobalMatching.h>
#include<opencv2/opencv.hpp>

int main(int argv, char** argc)
{
  if(argv < 3)
   {
     std::cout<<"please input the left and right image path..."<<std::endl;
     return -1;
   }
  
   //read image
   std::string path_left = argc[1];
   std::string path_right = argc[2];
   
   cv::Mat img_left_c = cv::imread(path_left,cv::IMREAD_COLOR);
   cv::Mat img_left = cv::imread(path_left,cv::IMREAD_GRAYSCALE);
   cv::Mat img_right = cv::imread(path_right,cv::IMREAD_GRAYSCALE);
   
   cv::namedWindow("img_left_c",1);
   cv::namedWindow("img_left",1);
   cv::namedWindow("img_right",1);
   cv::imshow("img_left_c",img_left_c);
   cv::imshow("img_left",img_left);
   cv::imshow("img_right",img_right);
   cv::waitKey(50);

   if (img_left.data == nullptr || img_right.data == nullptr) {
        std::cout << "fail to read images" << std::endl;
        return -1;
   }
   if (img_left.rows != img_right.rows || img_left.cols != img_right.cols) {
        std::cout << "img_left.size not equals img_right" << std::endl;
        return -1;
   }

   const sint32 width = static_cast<uint32>(img_left.cols);
   const sint32 height = static_cast<uint32>(img_right.rows);

   //the graydata of the left and right image
   auto bytes_left = new uint8[width * height];
   auto bytes_right = new uint8[width * height];
   for (int i = 0; i < height; i++) {
       for (int j = 0; j < width; j++) {
            bytes_left[i * width + j] = img_left.at<uint8>(i, j);
            bytes_right[i * width + j] = img_right.at<uint8>(i, j);
       }
   }
    // SGM匹配参数设计
    SemiGlobalMatching::SGMOption sgm_option;
    // 聚合路径数
    sgm_option.num_paths = 8;
    // 候选视差范围
    sgm_option.min_disparity = argv < 4 ? 0 : atoi(argc[3]);
    sgm_option.max_disparity = argv < 5 ? 64 : atoi(argc[4]);
    // census窗口类型
    sgm_option.census_size = SemiGlobalMatching::Census5x5;
    // 一致性检查
    sgm_option.is_check_lr = true;
    sgm_option.lrcheck_thres = 1.0f;
    // 唯一性约束
    sgm_option.is_check_unique = true;
    sgm_option.uniqueness_ratio = 0.99;
    // 剔除小连通区
    sgm_option.is_remove_speckles = true;
    sgm_option.min_speckle_aera = 50;
    // 惩罚项P1、P2
    sgm_option.p1 = 10;
    sgm_option.p2_init = 150;
    // 视差图填充
    sgm_option.is_fill_holes = true;

    // 定义SGM匹配类实例
    SemiGlobalMatching sgm;
    
    // 初始化
    if (!sgm.Initialize(width, height, sgm_option)) {
        std::cout << "SGM初始化失败！" << std::endl;
        return -2;
    }
    
    // 匹配
    auto disparity = new float32[uint32(width * height)]();
    if (!sgm.Match(bytes_left, bytes_right, disparity)) {
        std::cout << "SGM匹配失败！" << std::endl;
        return -2;
    }

    // 显示视差图
    cv::Mat disp_mat = cv::Mat(height, width, CV_8UC1);
    float min_disp = width, max_disp = -width;
    for (sint32 i = 0; i < height; i++) {
        for (sint32 j = 0; j < width; j++) {
            const float32 disp = disparity[i * width + j];
            if (disp != Invalid_Float) {
                min_disp = std::min(min_disp, disp);
                max_disp = std::max(max_disp, disp);
            }
        }
    }
    for (sint32 i = 0; i < height; i++) {
        for (sint32 j = 0; j < width; j++) {
            const float32 disp = disparity[i * width + j];
            if (disp == Invalid_Float) {
                disp_mat.data[i * width + j] = 0;
            }
            else {
                disp_mat.data[i * width + j] = static_cast<uchar>((disp - min_disp) / (max_disp - min_disp) * 255);
            }
        }
    }
    
    cv::namedWindow("视差图",1);
    cv::namedWindow("视差图-伪彩",1);
    cv::imshow("视差图", disp_mat);
    cv::Mat disp_color;
    applyColorMap(disp_mat, disp_color, cv::COLORMAP_JET);
    cv::imshow("视差图-伪彩", disp_color);
   
    // 保存结果
    std::string disp_map_path = argc[2]; disp_map_path += ".d.bmp";
    std::string disp_color_map_path = argc[2]; disp_color_map_path += ".c.bmp";
    cv::imwrite(disp_map_path, disp_mat);
    cv::imwrite(disp_color_map_path, disp_color);

    cv::waitKey(0);

   //···············································································//
   // 释放内存
   delete[] disparity;
   disparity = nullptr;
   delete[] bytes_left;
   bytes_left = nullptr;
   delete[] bytes_right;
   bytes_right = nullptr;

   system("pause");
   
   return 1;
}
