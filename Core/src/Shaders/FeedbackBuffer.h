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

#ifndef FEEDBACKBUFFER_H_
#define FEEDBACKBUFFER_H_

#include "Shaders.h"
#include "Uniform.h"
#include "Vertex.h"
#include "../Utils/Resolution.h"
#include "../Utils/Intrinsics.h"
#include <pangolin/gl/gl.h>
#include <pangolin/display/opengl_render_state.h>

#include "../Defines.h"

/** @brief //? 疑似和绘制点云有关的类型  从深度图像计算得到点云数据*/
class FeedbackBuffer
{
    public:
        /**
         * @brief 构造函数
         * @param[in] program 用于生成点云数据的着色器
         */
        FeedbackBuffer(std::shared_ptr<Shader> program);

        /** @brief 析构函数 */
        virtual ~FeedbackBuffer();

        std::shared_ptr<Shader> program;        ///? 从深度图计算点云的着色器

        void compute(pangolin::GlTexture * color,
                     pangolin::GlTexture * depth,
                     const int & time,
                     const float depthCutoff);

        /**
         * @brief // ? 渲染点云?
         * @param[in] mvp               虚拟观察相机所在的位姿
         * @param[in] pose              当前相机的位姿
         * @param[in] drawNormals       是否使用法向贴图
         * @param[in] drawColors        是否使用RGB纹理贴图
         * @return EFUSION_API render 
         */
        EFUSION_API void render(pangolin::OpenGlMatrix mvp, const Eigen::Matrix4f & pose, const bool drawNormals, const bool drawColors);

        EFUSION_API static const std::string RAW, FILTERED;

        GLuint vbo;         ///< 顶点缓冲对象的id
        GLuint fid;         ///? 帧缓冲的id?

    private:
        std::shared_ptr<Shader> drawProgram;
        GLuint uvo;
        GLuint countQuery;
        const int bufferSize;                   ///? 顶点的个数? 占用字节的个数?
        unsigned int count;                     ///?
};

#endif /* FEEDBACKBUFFER_H_ */
