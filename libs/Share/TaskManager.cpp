
// ȫ�ֱ�������
QSharedPointer<FileInfoDetails> gProjectFileInfo;//��ǰ����ִ�е���Ŀ��Ϣ(��Ҫ�ͻ���ʹ��)
FileInfoDetails* gTaskFileInfo = nullptr;//��ǰ����ִ�е�����

TaskManager::~TaskManager(){
    if (gTaskState == TaskState_Running) {
        emit finished();
    }
}
