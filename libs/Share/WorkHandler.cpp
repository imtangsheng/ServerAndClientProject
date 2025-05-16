#include "WorkHandler.h"
#include "WorkHandler.h"
#include "WorkHandler.h"
// 全局变量定义
ProjectInfo* gProject = nullptr;
TaskInfo* gTask = nullptr;

WorkHandler::~WorkHandler(){
    if (gTaskState == TaskState_Running) {
        emit completed();
    }
}
