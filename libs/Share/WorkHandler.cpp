#include "WorkHandler.h"
#include "WorkHandler.h"
#include "WorkHandler.h"
// ȫ�ֱ�������
ProjectInfo* gProject = nullptr;
TaskInfo* gTask = nullptr;

WorkHandler::~WorkHandler(){
    if (gTaskState == TaskState_Running) {
        emit completed();
    }
}
