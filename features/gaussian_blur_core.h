/*** 
 * @Author: Matt.SHI
 * @Date: 2023-01-03 08:40:40
 * @LastEditTime: 2023-01-03 10:10:49
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/gaussian_blur_core.h
 * @Copyright © 2022 Essilor. All rights reserved.
 */

class Shader;
class FrameBuffer;

namespace ESSILOR
{
    class GassianBlurCore 
    {
        public:
            GassianBlurCore();
            virtual ~GassianBlurCore();

        public:
            int init(unsigned int w, unsigned int h,unsigned int channel);
            void unit();

            unsigned char*  doGaussianBlur(
                unsigned char *base_image_data,
                unsigned int base_image_width,
                unsigned int base_image_height,
                unsigned int base_image_channel,
                unsigned char *filter_zone_image_data,
                unsigned int filter_zone_image_width,
                unsigned int filter_zone_image_height,
                unsigned int filter_zone_image_channel);

        protected:
            void initGraphicEnv();
            int initOpenGL(unsigned int w, unsigned int h);
            void initShader(
                const char* vertexShaderFile = "../resources/features_res/gaussain_bulr/gauss_blur.vs",
                const char* fragmentShaderFile = "../resources/features_res/gaussain_bulr/gauss_blur.fs");
            void initFrameBuffer(unsigned int w, unsigned int h,unsigned int channel);

            void initTexture();

            unsigned int* createTexture2D(int textureCount = 1);
            unsigned int creatFilterZoneTexture2D();

            void updateTexture2DMemData(unsigned int textureIdx, unsigned int textureIDInGL,
                            int width, int height, int channel,
                            unsigned int texturePixelFmt, unsigned int dataPixelFmt, unsigned char *data);

        private:
            Shader *m_shader;
            FrameBuffer* m_frameBuffer;

            unsigned int m_VBO;
            unsigned int m_VAO;
            unsigned int m_EBO;

            unsigned int m_base_textureIdx;
            unsigned int m_filter_zone_textureIdx;

            unsigned char* m_result_buffer;
            unsigned int m_result_w;
            unsigned int m_result_h;
            unsigned int m_result_channel;    
    };
}