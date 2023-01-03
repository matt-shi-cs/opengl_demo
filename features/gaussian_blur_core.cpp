/*** 
 * @Author: Matt.SHI
 * @Date: 2022-12-29 17:30:09
 * @LastEditTime: 2023-01-03 13:11:32
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/gaussian_blur_core.cpp
 * @Copyright © 2022 Essilor. All rights reserved.
 */
#include "gaussian_blur_core.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef __ENABLE_GLUT__
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif //__ENABLE_GLUT__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>

#include <features/framebuffer/FrameBuffer.h>

#include <iostream>
#include <algorithm>

namespace ESSILOR
{
   GassianBlurCore::GassianBlurCore():
   m_result_buffer(nullptr),
   m_frameBuffer(nullptr),
   m_shader(nullptr)
   {
    
   }

   GassianBlurCore::~GassianBlurCore()
   {

   }

    int GassianBlurCore::init(unsigned int w, unsigned int h,unsigned int channel)
    {
        initOpenGL(w,h);
        initGraphicEnv();
        initShader();
        initFrameBuffer(w, h, channel);
        initTexture();

        return 0;
    }

    unsigned char* GassianBlurCore::doGaussianBlur(
        unsigned char *base_image_data,
        unsigned int base_image_width,
        unsigned int base_image_height,
        unsigned int base_image_channel,

        unsigned char *filter_zone_image_data,
        unsigned int filter_zone_image_width,
        unsigned int filter_zone_image_height,
        unsigned int filter_zone_image_channel)
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //GLuint opTextureIdx = m_frameBuffer->getColorId();

        //update buffer
        updateTexture2DMemData(m_base_textureIdx, GL_TEXTURE0,
                                base_image_width, base_image_height, base_image_channel,
                                GL_RGB, GL_BGR, base_image_data);
            
        updateTexture2DMemData(m_filter_zone_textureIdx, GL_TEXTURE1,
                                filter_zone_image_width, filter_zone_image_height, filter_zone_image_channel,
                                GL_RGB, GL_BGR, filter_zone_image_data);

        //call shader
        m_shader->use();
        glBindVertexArray(m_VAO);

        //draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //copy texture to frame buffer
        glReadPixels(0, 0, m_result_w, m_result_h, GL_RGBA, GL_UNSIGNED_BYTE, m_result_buffer);
        return m_result_buffer;
            
    }

    void GassianBlurCore::unit()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        glfwTerminate();
    }


    int GassianBlurCore::initOpenGL(unsigned int w, unsigned int h)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
        GLFWwindow *window = glfwCreateWindow(w, h, "LearnOpenGL", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }
    }

    void GassianBlurCore::initGraphicEnv()
    {
        float vertices[] = {
            // positions          // colors           // texture coords
            1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // top right
            1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // bottom right
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // bottom left
            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f   // top left
        };
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    void GassianBlurCore::initShader(
        const char* vertexShaderFile,
        const char* fragmentShaderFile)
    {
        if(nullptr == m_shader)
        {
            m_shader = new Shader(vertexShaderFile,fragmentShaderFile);
            m_shader->use();
        }
    }

    void GassianBlurCore::initFrameBuffer(unsigned int w, unsigned int h,unsigned int channel)
    {
        if(nullptr == m_frameBuffer)
        {
            m_frameBuffer = new FrameBuffer();
            m_frameBuffer->init(w,h);
        }
        if(nullptr == m_result_buffer)
        {
            m_result_buffer = new unsigned char[w*h*channel];
            m_result_w = w;
            m_result_h = h;
            m_result_channel = channel;
        }
    }

    void GassianBlurCore::initTexture()
    {
        int texture2DCount = 1;
        unsigned int *textureIdxs = createTexture2D(texture2DCount);
        std::cout<<"[TEXTURE] texture id: "<<textureIdxs[0]<<std::endl;
        glUniform1i(glGetUniformLocation(m_shader->ID, "imageTexture"), 0);
        m_base_textureIdx = textureIdxs[0];

        unsigned int zoneTextureIdx = creatFilterZoneTexture2D();
        std::cout<<"[TEXTURE] filter zone id: "<<zoneTextureIdx<<std::endl;
        //ourShader.setInt("filterZones", zoneTextureIdx); 
        glUniform1i(glGetUniformLocation(m_shader->ID, "filterZones"), 1);
        m_filter_zone_textureIdx = zoneTextureIdx;
    }

    unsigned int* GassianBlurCore::createTexture2D(int textureCount){
        unsigned int *pTextureIdxs = new unsigned int[textureCount];
        glGenTextures(textureCount, pTextureIdxs);
        for (int i = 0; i < textureCount; i++)
        {
            glBindTexture(GL_TEXTURE_2D, pTextureIdxs[i]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT /*GL_REPEAT*/); // set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT /*GL_REPEAT*/);
            // set boarder color
            float borderColor[] = {1.0f, 1.0f, 0.0f, 1.0f};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        return pTextureIdxs;
    }

    unsigned int GassianBlurCore::creatFilterZoneTexture2D()
    {
        unsigned int textureIdx = 0;
        glGenTextures(1, &textureIdx);
        glBindTexture(GL_TEXTURE_2D, textureIdx); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT /*GL_REPEAT*/); // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT /*GL_REPEAT*/);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureIdx;
    }

    void GassianBlurCore::updateTexture2DMemData(unsigned int textureIdx, unsigned int textureIDInGL,
                            int width, int height, int channel,
                            unsigned int texturePixelFmt, unsigned int dataPixelFmt, unsigned char *data)
    {
        glActiveTexture(textureIDInGL); 
        glBindTexture(GL_TEXTURE_2D, textureIdx);
        glTexImage2D(GL_TEXTURE_2D, 0, texturePixelFmt, width, height, 0, dataPixelFmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

} // namespace ESSILOR

