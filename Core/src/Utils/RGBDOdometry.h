/**
 * @file RGBDOdometry.h
 * @author guoqing (1337841346@qq.com)
 * @brief 使用 RGB-D 数据的视觉里程计对象
 * @version 0.1
 * @date 2020-01-10
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/*
 * This file is part of ElasticFusion.
 *
 * Copyright (C) 2015 Imperial College London
 * 
 * The use of the code within this file and all code within files that 
 * make up the software that is ElasticFusion is permitted for 
 * non-commercial purposes only.  The full terms and conditions that 
 * apply to the code within this file are detailed within the LICENSE.txt 
 * file and at <http://www.imperial.ac.uk/dyson-robotics-lab/downloads/elastic-fusion/elastic-fusion-license/> 
 * unless explicitly stated.  By downloading this file you agree to 
 * comply with these terms.
 *
 * If you wish to use any of this code for commercial purposes then 
 * please email researchcontracts.engineering@imperial.ac.uk.
 *
 */

#ifndef RGBDODOMETRY_H_
#define RGBDODOMETRY_H_

// 计时工具
#include "Stopwatch.h"
// GPU 纹理
#include "../GPUTexture.h"
// ? cuda 的一些小工具?
// TODO
#include "../Cuda/cudafuncs.cuh"
// 用于辅助计算旋转的小工具
#include "OdometryProvider.h"
// 保存了常用的显卡的最佳的 CUDA 核心资源分配模式
#include "GPUConfig.h"

// C++ STL
#include <vector>
// CUDA Vector
#include <vector_types.h>

/** @brief 使用 RGB-D 数据的视觉里程计对象 */
class RGBDOdometry
{
    public:
        // 使用 Eigen 的 128bit 字节对齐 (SSE指令集)
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        /**
         * @brief 构造函数
         * @param[in] width         图像宽度
         * @param[in] height        图像高度
         * @param[in] cx            cx
         * @param[in] cy            cy
         * @param[in] fx            fx
         * @param[in] fy            fy
         * @param[in] distThresh    最大距离阈值（默认0.1m）
         * @param[in] angleThresh   最大角度差阈值（默认为20°对应的正弦值）
         */
        RGBDOdometry(int width,                                     
                     int height,
                     float cx, float cy, float fx, float fy,
                     float distThresh = 0.10f,
                     float angleThresh = sin(20.f * 3.14159254f / 180.f));

        /** @brief 空析构函数 */
        virtual ~RGBDOdometry();

        /**
         * @brief // ? 开始执行 ICP 过程?
         * @param[in] filteredDepth     当前帧的双边滤波后的点云
         * @param[in] depthCutoff       深度截断值
         */
        void initICP(GPUTexture * filteredDepth, const float depthCutoff);

        void initICP(GPUTexture * predictedVertices, GPUTexture * predictedNormals, const float depthCutoff);

        /**
         * @brief ICP 迭代过程的初始化
         * @param[in&out] predictedVertices    输入的是当前帧的顶点, 输出的是 predicted 的模型的顶点信息
         * @param[in&out] predictedNormals     输入的是当前帧的法向, 输出的是 predicted 的模型的法向信息
         * @param[in]     depthCutoff          深度截断值
         * @param[in]     modelPose            模型的位姿 // ? 上次迭代的吧?
         */
        void initICPModel(GPUTexture * predictedVertices, GPUTexture * predictedNormals, const float depthCutoff, const Eigen::Matrix4f & modelPose);

        void initRGB(GPUTexture * rgb);

        void initRGBModel(GPUTexture * rgb);

        void initFirstRGB(GPUTexture * rgb);

        /**
         * @brief 执行 ICP 配准过程, 得相机位姿调整的增量
         * 
         * @param[in&out] trans         输入的时候是上一次迭代的相机平移, 输出是迭代后的相机平移
         * @param[in&out] rot           输入的时候是上一次迭代的相机旋转, 输出是迭代后的相机旋转
         * @param[in]     rgbOnly       // ? 只使用 RGB 项的误差
         * @param[in]     icpWeight     // ? 几何误差和 RGB 误差的相对大小权重?
         * @param[in]     pyramid       // ? 是否使用图像金字塔分层求解?
         * @param[in]     fastOdom      // ?
         * @param[in]     so3           // ? 是否使用已知旋转对 ICP 过程进行加速处理?
         */
        void getIncrementalTransformation(Eigen::Vector3f & trans,
                                          Eigen::Matrix<float, 3, 3, Eigen::RowMajor> & rot,
                                          const bool & rgbOnly,
                                          const float & icpWeight,
                                          const bool & pyramid,
                                          const bool & fastOdom,
                                          const bool & so3);

        Eigen::MatrixXd getCovariance();

        float lastICPError;             ///< ICP 匹配过程最后的残差
        float lastICPCount;             ///< ICP 过程中的内点个数
        float lastRGBError;             ///< 颜色误差
        float lastRGBCount;             ///? 颜色的内点个数?
        float lastSO3Error;             ///?
        float lastSO3Count;             ///? 内点个数?

        Eigen::Matrix<double, 6, 6, Eigen::RowMajor> lastA;         ///< PL-ICP 并行化后得到的超定方程的最小二乘解方程对应的系数矩阵
        Eigen::Matrix<double, 6, 1> lastb;                          ///< PL-ICP 并行化后得到的超定方程的最小二乘解方程对应的非齐次项

    private:
        void populateRGBDData(GPUTexture * rgb,
                              DeviceArray2D<float> * destDepths,
                              DeviceArray2D<unsigned char> * destImages);

        std::vector<DeviceArray2D<unsigned short> > depth_tmp;      ///? 深度图像的 vector, 用于做什么?

        DeviceArray<float> vmaps_tmp;                               ///? 顶点图, 这个是做什么的, 又为什么没有使用图像金字塔的形式?
        DeviceArray<float> nmaps_tmp;                               ///? 法向图, 疑问同上

        std::vector<DeviceArray2D<float> > vmaps_g_prev_;           ///? 顶点图, 金字塔形式 保存什么的顶点图? 而且顶点图中的数据类型不应该是 float3 吗?
        std::vector<DeviceArray2D<float> > nmaps_g_prev_;           ///? 法向图? 

        std::vector<DeviceArray2D<float> > vmaps_curr_;             ///? 顶点图?
        std::vector<DeviceArray2D<float> > nmaps_curr_;             ///? 法向图?

        CameraModel intr;                                           ///< 相机模型对象, 记录了相机的内参

        DeviceArray<JtJJtrSE3> sumDataSE3;                          ///< 用于存储计算误差的累加和, 主要内容放在显存中
        DeviceArray<JtJJtrSE3> outDataSE3;                          ///< 最终累加和
        DeviceArray<int2> sumResidualRGB;                           ///? 

        DeviceArray<JtJJtrSO3> sumDataSO3;                          ///? 只考虑旋转的时候的误差累加和?
        DeviceArray<JtJJtrSO3> outDataSO3;                          ///? 对应上一个变量

        const int sobelSize;                                        ///?
        const float sobelScale;                                     ///? 1/(sobelSize^2)
        const float maxDepthDeltaRGB;                               ///?
        const float maxDepthRGB;                                    ///?

        std::vector<int2> pyrDims;                                  ///? 从下到上, 记录了图像金字塔中每一层图像的大小 (high, width)

        static const int NUM_PYRS = 3;                              ///< 使用的图像金字塔的层数

        DeviceArray2D<float> lastDepth[NUM_PYRS];                   ///? 上一帧的深度图像? input还是predicted?
        DeviceArray2D<unsigned char> lastImage[NUM_PYRS];           ///? 上一帧的灰度图像, 和上面的内容相对应(彩色图像全部都转换成为灰度图像了)

        DeviceArray2D<float> nextDepth[NUM_PYRS];                   ///? 什么深度图像
        DeviceArray2D<unsigned char> nextImage[NUM_PYRS];           ///? 什么彩色图像
        DeviceArray2D<short> nextdIdx[NUM_PYRS];                    ///? 
        DeviceArray2D<short> nextdIdy[NUM_PYRS];                    ///?

        DeviceArray2D<unsigned char> lastNextImage[NUM_PYRS];

        DeviceArray2D<DataTerm> corresImg[NUM_PYRS];                ///? 

        DeviceArray2D<float3> pointClouds[NUM_PYRS];                ///? 什么点云对应的"金字塔"?

        std::vector<int> iterations;                                ///? 和迭代\图像金字塔有关
        std::vector<float> minimumGradientMagnitudes;               ///? 和梯度有关?

        float distThres_;                                           ///< 成为匹配点的最大距离阈值
        float angleThres_;                                          ///< 成为匹配点的最大角度阈值的正弦值

        Eigen::Matrix<double, 6, 6> lastCov;

        const int width;                                            ///< 输入图像宽度
        const int height;                                           ///< 输入图像高度
        const float cx, cy, fx, fy;                                 ///< 相机内参
};

#endif /* RGBDODOMETRY_H_ */
