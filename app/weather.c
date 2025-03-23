/*
 * MIT License
 *
 * Copyright (c) 2023 梅花嵌入式
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "weather.h"

/* 示例输入：
 * +HTTPCGET:261,{"results":[{"location":{"id":"WTEMH46Z5N09","name":"Hefei","country":"CN","path":"Hefei,Hefei,Anhui,China","timezone":"Asia/Shanghai","timezone_offset":"+08:00"},"now":{"text":"Cloudy","code":"4","temperature":"36"},"last_update":"2024-08-26T16:20:13+08:00"}]}
 *
 * OK
 */

bool weather_parse(const char *data, weather_t *weather)
{
    /* 匹配目标："text":"Cloudy"中的Cloudy字段 */
    /* 在data中查找"\"text\":\""，其中\"是转义字符，表示双引号 */
    char *p = strstr(data, "\"text\":\"");
    /* 如果未找到，则返回false */
    if (p == NULL)
    {
        return false;
    }
    /* 指针后移，此时p指向字符串Cloudy"的C */
    p += strlen("\"text\":\"");
    /* 查找下一个双引号 */
    char *q = strchr(p, '\"');
    if (q == NULL)
    {
        return false;
    }
    /* 获取Cloudy这个字符串的长度 */
    int len = q - p;
    /* 防止字符串内容超过weather->weather的长度 */
    if (len >= sizeof(weather->weather))
    {
        len = sizeof(weather->weather) - 1;
    }
    /* 直接拷贝天气内容字符串到weather->weather */
    strncpy(weather->weather, p, len);
    weather->weather[len] = '\0';

    /* 匹配目标："temperature":"36"中的36字段 */
    /* 具体过程与前面的获取天气内容类似 */
    p = strstr(data, "\"temperature\":\"");
    if (p == NULL)
    {
        return false;
    }
    p += strlen("\"temperature\":\"");
    q = strchr(p, '\"');
    if (q == NULL)
    {
        return false;
    }
    len = q - p;
    if (len >= sizeof(weather->temperature))
    {
        len = sizeof(weather->temperature) - 1;
    }
    strncpy(weather->temperature, p, len);
    weather->temperature[len] = '\0';

    return true;
}
