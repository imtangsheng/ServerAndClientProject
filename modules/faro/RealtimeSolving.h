#pragma once
#include "FaroHandle.h"
#include "TaskHandle.h"


class RealtimeSolving
{
public:
	RealtimeSolving();
	~RealtimeSolving();

	/**
	 * @brief 解析FLS文件并提取点云数据,然后生成灰度图和深度图
	 * @param task 任务数据
	 * @param imagePath 输出的图像路径
	 * @return 解析,生成灰度图和深度图是否成功
	 */
	bool writeFaroImage(TaskFaroPart &task,const QString& imagePath);
	//Acquisition software  Visual data software Visualization software Visual software
	void test();
	
	bool isNeedAutoSplitRing{ true };
private:

};