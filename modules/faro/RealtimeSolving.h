#pragma once
#include "FaroHandle.h"
#include "TaskHandle.h"


class RealtimeSolving
{
public:
	RealtimeSolving();
	~RealtimeSolving();

	/**
	 * @brief ����FLS�ļ�����ȡ��������,Ȼ�����ɻҶ�ͼ�����ͼ
	 * @param task ��������
	 * @param imagePath �����ͼ��·��
	 * @return ����,���ɻҶ�ͼ�����ͼ�Ƿ�ɹ�
	 */
	bool writeFaroImage(TaskFaroPart &task,const QString& imagePath);
	//Acquisition software  Visual data software Visualization software Visual software
	void test();
	
	bool isNeedAutoSplitRing{ true };
private:

};