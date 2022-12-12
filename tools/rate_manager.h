/*** 
 * @Author: Matt.SHI
 * @Date: 2022-12-11 18:47:16
 * @LastEditTime: 2022-12-11 18:49:38
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/tools/rate_manager.h
 * @Copyright © 2022 Essilor. All rights reserved.
 */

#ifndef _CIT_RATE_MANAGER_H_
#define _CIT_RATE_MANAGER_H_

namespace tools
{
    template<typename T>
    class RateManager
    {
        public:
            RateManager() : m_last_time(0), m_rate(0), m_count(100) 
            {
                _records = new T[m_count];
                memset(_records, 0, sizeof(T) * m_count);
            }
            ~RateManager() {}

        public:
            void PushARecord(T rate) { m_rate = rate; }
            void SetMaxCount(T count) { m_count = count; }

            T GetRate() { return m_rate; }
            T GetCount() { return m_count; }
        private:
            T m_rate;
            T _records;
            unsigned int m_count;
    };
}

#endif //_CIT_RATE_MANAGER_H_