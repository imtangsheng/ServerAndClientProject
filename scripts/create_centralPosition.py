
path = "centralPosition.txt" #文件名
central_position = 6 #起始位置,根据标准位置 -1
first_time = 436690672 #起始时间
time101 = 484500729 #id值为101 小车的起始时间
time_interval = (time101 - first_time) / 100.0 #时间间隔
#图片数量
pic_count = 3187

def create_central_position_file():
    with open(path, 'w', encoding='utf-8') as file:
        str_tab = "\t"
        title_central_position = "ID" + str_tab + \
            "CamID" + str_tab + \
            "CentralPosition" + str_tab + \
            "time" + str_tab + \
            "ActualPosition"
        file.write(title_central_position + "\n")
        
        pose = central_position
        trolley_time = first_time
        
        for i in range(1, pic_count + 1):
            for j in range(1, 4):
                msg = str(i) + str_tab + \
                    str(j) + str_tab + \
                    str(pose % 15 + 1) + str_tab + \
                    str(int(trolley_time - (j - 1) * 2)) + str_tab + \
                    str((pose + (j - 1) * 5) % 15 + 1)
                file.write(msg + "\n")
            pose += 1
            trolley_time = first_time + i * time_interval

create_central_position_file() 