
// ȫ�ֱ�������
FileInfoDetails* gProjectFileInfo = nullptr;//��ǰ����ִ�е���Ŀ��Ϣ(�ͻ���ʹ��)
FileInfoDetails* gTaskFileInfo = new FileInfoDetails();

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
