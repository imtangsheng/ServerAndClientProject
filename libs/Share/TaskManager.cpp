
// ȫ�ֱ�������
TaskInfo* gTask = nullptr;

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
