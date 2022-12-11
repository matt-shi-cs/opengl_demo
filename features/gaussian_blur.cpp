/*** 
 * @Author: Matt.SHI
 * @Date: 2022-12-10 14:46:29
 * @LastEditTime: 2022-12-11 17:08:21
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/gaussian_blur.cpp
 * @Copyright © 2022 Essilor. All rights reserved.
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int g_save_frame = 1;
int g_draw_frame = 1;
cv::VideoCapture g_cam;
unsigned char g_save_base_path[] = "./frames/";

void saveFrameBuffer2PNG(const char* buf, 
    unsigned int w, unsigned int h, unsigned int c, 
    const char* filename)
{
    stbi_write_png(filename, w, h, c, buf, w * c);
}

void generateFilePath(char* filename, double index,const unsigned char* basePath)
{
    sprintf(filename, "%s_%.4f.png", basePath,index);
}

void initCam()
{
    try
    {
        g_cam.open(0);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(...)
    {
        std::cerr << "error" << '\n';
    }
    
}

void readFrame(cv::Mat& frame)
{
    g_cam.read(frame);
}

unsigned int* create2DTexture(int count)
{
    unsigned int* pTextureIdxs = new unsigned int[count];
    glGenTextures(count, pTextureIdxs);
    for(int i = 0 ; i < count; i++)
    {
        glBindTexture(GL_TEXTURE_2D, pTextureIdxs[i]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT/*GL_REPEAT*/);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT/*GL_REPEAT*/);
        // set boarder color
        float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    }
    return pTextureIdxs;
}

void updateTexture2DMemData(unsigned int textureIdx, int width, int height, int channel, 
    unsigned int texturePixelFmt, unsigned int dataPixelFmt,unsigned char* data)
{
    glBindTexture(GL_TEXTURE_2D, textureIdx);
    glTexImage2D(GL_TEXTURE_2D, 0, texturePixelFmt, width, height, 0, dataPixelFmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

int main(int argv,const char* argc[])
{
    //init cam
    cv::Mat frameBufMat;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    //processing input params
    const char* input_data_path = nullptr;
    if(argv > 1)
    {
        input_data_path = argc[1];
        std::cout << "[INPUT] use input file, path: " << input_data_path<<std::endl;
    }
    else{
        std::cout << "[INPUT] use camera live stream: " << std::endl;
    }

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f  // top left 
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture 
    // -------------------------
    int texture2DCount = 1;
    unsigned int* textureIdxs = create2DTexture(texture2DCount);
    
    // load image, create texture and generate mipmaps
    int width = 0, height = 0, nrChannels = 0,reqComp = 3;
    if(nullptr != input_data_path)
    {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char *data = stbi_load(input_data_path/*FileSystem::getPath(input_data_path).c_str()*/, &width, &height, &nrChannels, reqComp);
        if (nullptr != data)
        {
            std::cout<<"[FILE]"<<input_data_path<<" width:"<<width<<" height:"<<height<<" nrChannels:"<<nrChannels<<std::endl;
            updateTexture2DMemData(textureIdxs[0], width, height, nrChannels, GL_RGB,GL_RGB,data);
            glViewport(0, 0, width, height);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
            return -1;
        }
    }
    else{
        initCam();
        readFrame(frameBufMat);
        width = frameBufMat.cols;
        height = frameBufMat.rows;
        std::cout << "[CAMERA] inited:  width = "<<width << " height = " << height << std::endl;
    }
    //texture end

    //using framebuffer
    FrameBuffer fbo;
    fbo.init(width, height);        // for single-sample FBO
    //================================================================================================     
    // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    double startTime = 0.0;
    double endTime = 0.0;
    int frameIndex = 0;
    bool bSave = false;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        //read cam
        if(!frameBufMat.empty())
        {
            readFrame(frameBufMat);
        }

        startTime = glfwGetTime();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // bind Texture
        glBindTexture(GL_TEXTURE_2D, textureIdxs[0]);

        if(!frameBufMat.empty())
        {
            updateTexture2DMemData(textureIdxs[0], frameBufMat.cols, frameBufMat.rows, 3,
                GL_RGB,GL_BGR, frameBufMat.data);
        }

        // render
        //fbo.bind();

        // render container
        ourShader.use();
        glBindVertexArray(VAO);

        //fbo.update();
        //fbo.unbind();

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        endTime = glfwGetTime();

        frameIndex++;
        if(endTime - startTime >= 0.001)
        {
            std::cout << "[Frame]The fps is " <<  frameIndex / (endTime - startTime) << std::endl;
            if(g_save_frame > 0)
            {
                char save_path[256] = {0};
                generateFilePath(save_path,endTime, g_save_base_path);
                const unsigned char* buffer = fbo.getColorBuffer();
                saveFrameBuffer2PNG((const char*)(buffer), width, height, 4,save_path);
            }
            frameIndex = 0;
        }
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        //std::cout << "the cost time is " << end - start << " ms" << std::endl;
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
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_save_frame = 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_draw_frame = 2;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
