
// 全局变量定义
QSharedPointer<FileInfoDetails> gProjectFileInfo;//当前正在执行的项目信息(主要客户端使用)
FileInfoDetails* gTaskFileInfo = nullptr;//当前正在执行的任务

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
