/*** 
 * @Author: Matt.SHI
 * @Date: 2023-01-03 14:08:26
 * @LastEditTime: 2023-01-04 20:10:55
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/exports/gaussian_blur_lib_export.cpp
 * @Copyright © 2022 Essilor. All rights reserved.
 */

#include "gaussian_blur_lib_export.h"
#include <string.h>
#include <iostream>

ESSILOR::GassianBlurCore* g_ins = nullptr;

extern "C"
{
    
    long createIns(){
        //ESSILOR::GassianBlurCore* ins = new ESSILOR::GassianBlurCore();
        //return reinterpret_cast<long>(ins);
        if(nullptr == g_ins)
        {
            g_ins = new ESSILOR::GassianBlurCore();
        }
        return 0;
    }
    
    void destroyIns(long objIns)
    {
        //ESSILOR::GassianBlurCore* ins = reinterpret_cast<ESSILOR::GassianBlurCore*>(objIns);
        g_ins->unit();
        delete g_ins;
        g_ins = nullptr;
    }

    void initIns(long objIns, unsigned int w, unsigned int h,unsigned int channel,
        const char* vertexShaderFile, const char* fragmentShaderFile)
    {
        //ESSILOR::GassianBlurCore* ins = reinterpret_cast<ESSILOR::GassianBlurCore*>(objIns);
        if(nullptr != g_ins)
            g_ins->init(w, h, channel,vertexShaderFile,fragmentShaderFile);
    }

    void setKernelStep(long objIns, float kernelStep)
    {
        //ESSILOR::GassianBlurCore* ins = reinterpret_cast<ESSILOR::GassianBlurCore*>(objIns);
        if(nullptr != g_ins)
            g_ins->set_kernel_blur_step(kernelStep);
    }

    long doGaussianBlur(long objIns, 
        unsigned char* base_data, unsigned int base_w, unsigned int base_h, unsigned int base_channel,
        unsigned char* filter_data, unsigned int filter_w, unsigned int filter_h, unsigned int filter_channel,
        unsigned char* out_buffer)
    {
        //ESSILOR::GassianBlurCore* ins = reinterpret_cast<ESSILOR::GassianBlurCore*>(objIns);
        unsigned char* p_data = g_ins->doGaussianBlur(base_data, base_w, base_h, base_channel, 
            filter_data, filter_w, filter_h, filter_channel);
      
        unsigned long out_len = sizeof(unsigned char)*g_ins->getOutBufLen();
        memcpy(out_buffer,p_data,out_len);
        
        return out_len;
    }

    //export other functions
}