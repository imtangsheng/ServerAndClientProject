
// 全局变量定义
FileInfoDetails* gProjectFileInfo = nullptr;//当前正在执行的项目信息(客户端使用)
FileInfoDetails* gTaskFileInfo = nullptr;

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
