/**
 * 日志记录
 */

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE // 设置日志级别为TRACE,必须定义才能输出文件名和行号
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>

#define LOG_DEBUG SPDLOG_DEBUG 
#define LOG_INFO SPDLOG_INFO
#define LOG_WARN SPDLOG_WARN
#define LOG_ERROR SPDLOG_ERROR
// using namespace spdlog;
/**
这个格式字符串的各个部分含义如下：

%Y-%m-%d %H:%M:%S.%e: 时间戳（年-月-日 时:分:秒.毫秒）
%^%l%$: 日志级别（带颜色）
%s: 源文件名
%#: 行号
%!: 函数名
%v: 实际的日志消息

 [%n]  表示记录器名称 默认格式，其中包括记录器名称
 */
void setup_logging()
{
    try 
    {
        // 创建一个轮换文件日志记录器 mt多线程并发 multi thread  st single thread
        auto rotating_logger = spdlog::rotating_logger_mt(
            "file_logger",                   // 记录器名称
            "logs/logfile.txt",              // 日志文件名
            1024 * 1024 * 5,                 // 单个日志文件最大大小（这里设置为5MB）
            3                                // 保留的日志文件数量
        );
        
        // 设置日志级别
        rotating_logger->set_level(spdlog::level::info);

        // 设置日志格式
        rotating_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] %v");

        // 可选：添加控制台输出（彩色）
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] %v");
        
        // 创建一个包含文件和控制台输出的多重日志记录器
        auto multi_logger = std::make_shared<spdlog::logger>("multi_logger", 
            spdlog::sinks_init_list{rotating_logger->sinks()[0], console_sink});
        
        // 设置为默认记录器
        spdlog::set_default_logger(multi_logger);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}