
// 全局变量定义
TaskInfo* gTask = nullptr;

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
