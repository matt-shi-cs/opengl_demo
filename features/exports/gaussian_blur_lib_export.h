/*** 
 * @Author: Matt.SHI
 * @Date: 2023-01-03 13:29:47
 * @LastEditTime: 2023-01-09 10:59:19
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/exports/gaussian_blur_lib_export.h
 * @Copyright © 2022 Essilor. All rights reserved.
 */

#include "features/gaussian_blur_core.h"
#ifdef __APPLE__
#include <sys/shm.h>
#define EXPORT __attribute__((visibility("default")))
#else if __LINUX__
#include <sys/shm.h>
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT __declspec(dllexport)
#endif//__APPLE__

/*
char* g_shared_mem_addr = nullptr;
int g_segment_id = -1;
void init_mem()
{
    if(nullptr == g_shared_mem_addr)
    {
        struct shmid_ds shmbuffer;
        g_segment_id = shmget((key_t)1234567, 1, IPC_CREAT);
        g_shared_mem_addr = (char*)shmat(segment_id, NULL, 0);
    }
}

void unit_mem()
{
    if(g_segment_id > 0)
    {
        shmctl(g_segment_id, IPC_RMID, 0);
        g_segment_id = 0;
    }
    if(nullptr != g_shared_mem_addr)
    {
        g_shared_mem_addr = nullptr;
    }
}
*/


extern "C"
{
    EXPORT long createIns();
    
    EXPORT void destroyIns(long objIns);
    
    EXPORT void initIns(long objIns, unsigned int w, unsigned int h,unsigned int channel,
        const char* vertexShaderFile, const char* fragmentShaderFile);

    EXPORT void setPixelSize(long objIns, float pixelSizeX, float pixelSizeY);

    EXPORT long doGaussianBlur(long objIns, 
        unsigned char* data, unsigned int w, unsigned int h, unsigned int channel,
        unsigned char* filter_data, unsigned int filter_w, unsigned int filter_h, unsigned int filter_channel,
        unsigned char* out_buffer);

    //export other functions
}