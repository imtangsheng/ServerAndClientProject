
// ȫ�ֱ�������
FileInfoDetails* gProjectFileInfo = nullptr;//��ǰ����ִ�е���Ŀ��Ϣ(�ͻ���ʹ��)
FileInfoDetails* gTaskFileInfo = nullptr;

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
