/*** 
 * @Author: Matt.SHI
 * @Date: 2022-12-10 14:46:29
 * @LastEditTime: 2023-01-04 19:35:10
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/gaussian_blur_demo.cpp
 * @Copyright © 2022 Essilor. All rights reserved.
 */


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef __ENABLE_GLUT__
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif //__ENABLE_GLUT__

#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tools/stb_image_write.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>

#include <features/framebuffer/FrameBuffer.h>

#include <iostream>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>

//#define _USING_CAMERA

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1440;
const int TEXT_WIDTH = 8;
const int TEXT_HEIGHT = 13;
#ifdef __ENABLE_GLUT__
void *font = GLUT_BITMAP_8_BY_13;
#endif // __ENABLE_GLUT__

int g_save_frame = 1;        //'s'
int g_draw_frame = 1;        //'d'
int g_using_framebuffer = 0; //'f'
int g_using_opencv = 0;
int g_using_camera = 0;

float g_running_kernelStep = 0.005;
bool g_running_params_changed = false;

#ifdef _USING_CAMERA
cv::VideoCapture g_cam;
#endif //
unsigned char g_save_base_path[] = "./frames/";

void saveFrameBuffer2PNG(const char *buf,
                         unsigned int w, unsigned int h, unsigned int c,
                         const char *filename)
{
    stbi_write_png(filename, w, h, c, buf, w * c);
}

void generateFilePath(char *filename, double index, const unsigned char *basePath)
{
    sprintf(filename, "%s_%.4f.png", basePath, index);
}

#ifdef _USING_CAMERA
void initCam()
{
    try
    {
        g_cam.open(0);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "error" << '\n';
    }
}


void readFrame(cv::Mat &frame)
{
    g_cam.read(frame);
}

#endif //

unsigned int *create2DTexture(int count)
{
    unsigned int *pTextureIdxs = new unsigned int[count];
    glGenTextures(count, pTextureIdxs);
    for (int i = 0; i < count; i++)
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

void updateTexture2DMemData(unsigned int textureIdx, unsigned int textureIDInGL,
                            int width, int height, int channel,
                            unsigned int texturePixelFmt, unsigned int dataPixelFmt, unsigned char *data)
{
    glActiveTexture(textureIDInGL); 
    glBindTexture(GL_TEXTURE_2D, textureIdx);
    glTexImage2D(GL_TEXTURE_2D, 0, texturePixelFmt, width, height, 0, dataPixelFmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

unsigned int creatFilterZoneTexture()
{
    unsigned int textureIdx;
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

void fillTextureByBuffer(unsigned char *data, unsigned int width, unsigned int height,
                                 unsigned int textureID, unsigned int textureIDInGL,
                                 unsigned int innerPixelFmt = GL_RGB, unsigned int inputPixelFmt = GL_RGB, unsigned int dataType = GL_UNSIGNED_BYTE)
{
    glActiveTexture(textureIDInGL); 
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, innerPixelFmt, width, height, 0, inputPixelFmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

unsigned char* fillTextureByFile(const char* filePath,
    int& width, int& height, int& nrChannels,
    unsigned int texureID, unsigned int textureIDInGL,
    unsigned int innerPixelFmt = GL_RGB, unsigned int inputPixelFmt = GL_RGB, unsigned int dataType = GL_UNSIGNED_BYTE)
{
    unsigned char *data = stbi_load(filePath, &width, &height, &nrChannels, nrChannels);
    if (data)
    {
       fillTextureByBuffer(data, width, height, 
       texureID, textureIDInGL, 
       innerPixelFmt, inputPixelFmt, dataType);
       std::cout << "Load texture:" <<filePath <<" done "<<"W:"<<width<<" H:"<<height<<" C:"<<nrChannels<<std::endl;
    }
    else
    {
        if(nullptr == filePath)
        {
            std::cout << "Failed to load texture:NULLptr" <<std::endl;
        }
        else {
            std::cout << "Failed to load texture:" <<filePath <<std::endl;
        }
    }
    return data;
}




///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
#ifdef __ENABLE_GLUT__
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);                         // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);   // set text color
    glRasterPos2i(x, y); // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
#endif //__ENABLE_GLUT__
}

void showInfo(int screenWidth, int screenHeight, double frameRate)
{
#ifdef __ENABLE_GLUT__
    // backup current model-view matrix
    glPushMatrix();   // save current modelview matrix
    glLoadIdentity(); // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);                 // switch to projection matrix
    glPushMatrix();                              // save current projection matrix
    glLoadIdentity();                            // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;

    drawString(ss.str().c_str(), 1, screenHeight - TEXT_HEIGHT, color, font);
    ss.str(""); // clear buffer

    ss << std::fixed << std::setprecision(3);
    ss << "Frame Rate: " << frameRate << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (3 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Press SPACE to toggle FBO." << std::ends;
    drawString(ss.str().c_str(), 1, 1, color, font);

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix(); // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW); // switch to modelview matrix
    glPopMatrix();              // restore to previous modelview matrix
#endif                          // __ENABLE_GLUT__
}

void updateBlurParams(float stepSize,const Shader& sdr)
{
    sdr.setFloat("kernelStep",stepSize);
}

int main(int argv, const char *argc[])
{
    // init cam
    cv::Mat liveFrameBufMat;
    unsigned char *frameBufForSaving = NULL;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // processing input params
    const char *input_data_path = nullptr;
    const char *input_zone_path = nullptr;
    if (argv > 2)
    {
        input_zone_path = argc[1];
        input_data_path = argc[2];
        g_using_camera = 0;
        std::cout << "[INPUT] use input file, path: " << input_zone_path << std::endl;
        std::cout << "[INPUT] use input file, path: " << input_data_path << std::endl;
    }
    else if(argv > 1)
    {
        input_zone_path = argc[1];
        g_using_camera = 1;
        std::cout << "[INPUT] use input file, path: " << input_zone_path << std::endl;
        std::cout << "[INPUT] use camera "<< std::endl;
    }
    else
    {
        g_using_camera = 1;
        std::cout << "[INPUT] use camera" << std::endl;
    }

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("../resources/features_res/gaussain_bulr/gauss_blur.vs",
                     "../resources/features_res/gaussain_bulr/gauss_blur.fs");
    ourShader.use();

    float vertices[] = {
        // positions          // colors           // texture coords
        1.0f, 1.0f, 0.0f,       1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f,      0.0f, 1.0f, 0.0f,   1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f,      1.0f, 1.0f, 0.0f,   0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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

    // load and create a texture
    // -------------------------
    int texture2DCount = 1;
    unsigned int *textureIdxs = create2DTexture(texture2DCount);
    std::cout<<"[TEXTURE] texture id: "<<textureIdxs[0]<<std::endl;
    //ourShader.setInt("imageTexture", textureIdxs[0]); 
    glUniform1i(glGetUniformLocation(ourShader.ID, "imageTexture"), 0);

    // load image, create texture and generate mipmaps
    int width = 0, height = 0, nrChannels = 0;
    if (nullptr != input_data_path)
    {
        unsigned char *data = fillTextureByFile(
            input_data_path, width, height, nrChannels,
            textureIdxs[0],GL_TEXTURE0,
            GL_RGBA,GL_RGB,GL_UNSIGNED_BYTE);
        liveFrameBufMat = cv::Mat(height, width, CV_8UC3, data);
        glViewport(0, 0, width, height);
        
       if (nullptr == data)
        {
            std::cout << "Failed to load texture" << std::endl;
        }
    }
    else
    {
#ifdef _USING_CAMERA
        initCam();
        readFrame(liveFrameBufMat);
        width = liveFrameBufMat.cols;
        height = liveFrameBufMat.rows;
        std::cout << "[CAMERA] inited:  width = " << width << " height = " << height << " fmt = "<<liveFrameBufMat.type() <<std::endl;
#endif //
    }
    // texture end

    //zone texture
    int nrChannels_zone = 3, width_zone = 0, height_zone = 0;
    unsigned int zoneTextureIdx = creatFilterZoneTexture();
    std::cout<<"[TEXTURE] filter zone id: "<<zoneTextureIdx<<std::endl;
    //ourShader.setInt("filterZones", zoneTextureIdx); 
    glUniform1i(glGetUniformLocation(ourShader.ID, "filterZones"), 1);
    unsigned char* g_zones_buffer =  fillTextureByFile(input_zone_path,
        width_zone, height_zone, nrChannels_zone,
        zoneTextureIdx,GL_TEXTURE1,
        GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE);

    // using framebuffer
    FrameBuffer fbo;
    fbo.init(width, height); // for single-sample FBO

    // init buffer 
    frameBufForSaving = new unsigned char[width * height * 4]; // RGBA

    //init shaderparam
    updateBlurParams(g_running_kernelStep,ourShader);

    //================================================================================================

    double startTime = 0.0;
    double endTime = 0.0;
    int frameIndex = 0;
    bool bSave = false;
    int opTextureIdx = 0;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        // read cam
#ifdef _USING_CAMERA
        if (g_using_camera)
        {
            readFrame(liveFrameBufMat);
        }
#endif //_USING_CAMERA
        if(g_running_params_changed)
        {
            updateBlurParams(g_running_kernelStep,ourShader);
            g_running_params_changed = false;
        }

        startTime = glfwGetTime();
        if(g_using_opencv)
        {
            //cv::GaussianBlur(liveFrameBufMat, liveFrameBufMat, cv::Size(17, 17), 0, 0);

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            updateTexture2DMemData(textureIdxs[0], GL_TEXTURE0,
                                    liveFrameBufMat.cols, liveFrameBufMat.rows, nrChannels,
                                    GL_RGBA, GL_BGR, liveFrameBufMat.data);
            glBindVertexArray(VAO);
            if (g_draw_frame > 0)
            {
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        else{
            
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            // bind Texture
            if (g_using_framebuffer)
            {
                opTextureIdx = fbo.getColorId();
            }
            else
            {
                opTextureIdx = textureIdxs[0];
            }

            if (!liveFrameBufMat.empty())
            {
                updateTexture2DMemData(opTextureIdx, GL_TEXTURE0,
                                    liveFrameBufMat.cols, liveFrameBufMat.rows, nrChannels,
                                    GL_RGBA, GL_BGR, liveFrameBufMat.data);
            }
            if(nullptr != g_zones_buffer)
            {
                updateTexture2DMemData(zoneTextureIdx, GL_TEXTURE1,
                                    width_zone, height_zone, nrChannels_zone,
                                    GL_RGBA, GL_BGR, g_zones_buffer);
            }

            ourShader.use();

            // render
            if (g_using_framebuffer)
            {
                fbo.bind();
            }
            glBindVertexArray(VAO);

            if (g_using_framebuffer)
            {
                fbo.update();
                fbo.unbind();
            }

            if (g_draw_frame > 0)
            {
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            } 
        }
        endTime = glfwGetTime();

        frameIndex++;
        if (endTime - startTime >= 0.0001)
        {
            double frameRate = frameIndex / (endTime - startTime);
            showInfo(width, height, frameRate);
            if(g_using_opencv)
            {
                printf("[Frame][OPENCV]The fps is %.2lf\r\n",frameRate);
            }
            else{
                printf("[Frame][SHADER]The fps is %.2lf\r\n",frameRate);
            }
            fflush(stdout);
            frameIndex = 0;
        }
        if (g_save_frame > 0)
        {
            char save_path[256] = {0};
            generateFilePath(save_path, endTime, g_save_base_path);
            if (g_using_framebuffer)
            {
                fbo.copyColorBuffer();
                const unsigned char *buffer = fbo.getColorBuffer();
                saveFrameBuffer2PNG((const char *)(buffer), width, height, 4, save_path);
            }
            else if (frameBufForSaving != nullptr)
            {
                glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frameBufForSaving);
                saveFrameBuffer2PNG((const char *)(frameBufForSaving), width, height, 3, save_path);
            }
            g_save_frame = 0;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        // -------------------------------------------------------------------------------
        // std::cout << "the cost time is " << end - start << " ms" << std::endl;
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    delete [] frameBufForSaving;
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_save_frame = 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_draw_frame = g_draw_frame ? 0 : 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        g_using_framebuffer = g_using_framebuffer ? 0 : 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        g_using_opencv = g_using_opencv ? 0 : 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    {
        g_running_kernelStep -= 0.0005;
        g_running_kernelStep = fmax(0.0001,g_running_kernelStep);
        g_running_params_changed = true;
    }
    else if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    {
        g_running_kernelStep += 0.0005;
        g_running_kernelStep = fmin(0.01,g_running_kernelStep);
        g_running_params_changed = true;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
