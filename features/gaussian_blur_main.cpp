/*** 
 * @Author: Matt.SHI
 * @Date: 2023-01-03 09:33:35
 * @LastEditTime: 2023-01-04 19:26:24
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/gaussian_blur_main.cpp
 * @Copyright © 2022 Essilor. All rights reserved.
 */

#include "gaussian_blur_core.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <sys/time.h>

#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tools/stb_image_write.h>

#ifdef __APPLE__
#else
#include <termio.h>
#endif


ESSILOR::GassianBlurCore g_blur_core;
cv::VideoCapture g_cam;

constexpr int WIN_W = 1920;
constexpr int WIN_H = 1440;
constexpr int WIN_C = 3;

unsigned char g_save_base_path[] = "./frames/";
bool g_save_imgs = true;

int scanKeyboard()
{
    int input = 0;
    /*
    struct termios new_settings;
    struct termios stored_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    tcgetattr(0,&stored_settings);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
      
    input = getchar();
      
    tcsetattr(0,TCSANOW,&stored_settings);
    */
    return input;
}

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

unsigned char* loadFile(const char* filePath,
    int &width, int &height, int &nrChannels)
{
    return stbi_load(filePath, &width, &height, &nrChannels, nrChannels);
}

void generateFilePath(char *filename, double index, const unsigned char *basePath)
{
    sprintf(filename, "%s_%.4f.png", basePath, index);
}

void saveFrameBuffer2PNG(const char *buf,
                         unsigned int w, unsigned int h, unsigned int c,
                         const char *filename)
{
    stbi_write_png(filename, w, h, c, buf, w * c);
}

int main(int argv, const char *argc[])
{
    if(argv < 2)
    {
        std::cout << "please input the  filter-zone image path" << std::endl;
        return -1;
    }

    const char* filterZoneFile = argc[1];
    //init
    const char* vertexShaderFile = "../resources/features_res/gaussain_bulr/gauss_blur.vs";
    const char* fragmentShaderFile = "../resources/features_res/gaussain_bulr/gauss_blur.fs";
    g_blur_core.set_enable_gui(true);

    g_blur_core.init(WIN_W,WIN_H,WIN_C,vertexShaderFile,fragmentShaderFile);

    //init cam
    initCam();
    cv::Mat frameFromCam;

    //init filter zone
    int filterZoneW = 0, filterZoneH = 0, filterZoneC = 3;
    unsigned char* filterZoneData = loadFile(filterZoneFile,filterZoneW,filterZoneH,filterZoneC);

    int input = ' ';
    int frame_index = 0;
    while(input != 'x')
    {
        g_cam.read(frameFromCam);
        frame_index++;

        unsigned char* blurData = g_blur_core.doGaussianBlur(
            frameFromCam.data,frameFromCam.cols,frameFromCam.rows,frameFromCam.channels(),
            filterZoneData,filterZoneW,filterZoneH,filterZoneC);

        input = scanKeyboard();
        switch (input)
        {
        case 83 /*S*/:
            {
                g_save_imgs = true;
            }
            break;
        
        default:
            break;
        }

        if(frame_index%24 == 0)
        {
            char save_path[256] = {0};
            generateFilePath(save_path, time(nullptr), g_save_base_path);
            saveFrameBuffer2PNG((const char *)(blurData),WIN_W , WIN_H, WIN_C, save_path); 
        }
    }
}