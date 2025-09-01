#pragma once
#include <QQueue>
// 配置参数
static constexpr double MAX_SPEED_DEVIATION_RATIO = 0.3;  // 最大速度偏差比例(30%)
//static constexpr qint64 MIN_TIME_INTERVAL = 100000;           // 最小时间间隔(us)
static constexpr double MIN_SPEED_THRESHOLD = 0.001;     // 最小速度阈值，避免除零
static constexpr double MAX_SPEED_RATIO = 3.0;           // 左右轮最大速度比值
static constexpr int HISTORY_BUFFER_SIZE = 50;           // 历史数据缓冲区大小
static constexpr int MIN_HISTORY_SIZE = 5;               // 计算平均速度的最小历史数据量
static constexpr double OUTLIER_THRESHOLD = 2.0;         // 异常值检测阈值（标准差倍数）

struct struMileage {
    qint64 pulse{-1};  // 脉冲数
    qint64 time{-1};   // 时间戳
    bool isValid() const {
        return pulse >= 0 && time > 0;
    }
};

// 计算速度（脉冲/毫秒）
inline static double CalculateSpeed(const struMileage& current, const struMileage& previous) {
    qint64 time_diff = current.time - previous.time;
    qint64 pulse_diff = current.pulse - previous.pulse;
    if (time_diff <= 0 || pulse_diff < 0) {
        return 0.0;
    }
    //不会出现负数
    return static_cast<double>(pulse_diff) / time_diff;
}

// 获取历史平均速度
inline static double GetAverageSpeed(const QQueue<double>& speed) {
    if (speed.isEmpty()) {
        qFatal("获取历史平均速度,结果数据是空,不能为空");
        return 0.0;//MIN_SPEED_THRESHOLD
    }
    double sum = 0.0;
    for (double s : speed) {
        sum += s;
    }
    return sum / speed.size();
}
// 获取历史平均速度和标准差
inline static std::pair<double, double> GetSpeedStats(const QQueue<double>& speed) {
    if (speed.size() < MIN_HISTORY_SIZE) {
        return { 0.0, 0.0 };
    }

    double sum = 0.0;
    for (double s : speed) {
        sum += s;
    }
    double mean = sum / speed.size();

    // 计算标准差
    double variance = 0.0;
    for (double s : speed) {
        variance += (s - mean) * (s - mean);
    }
    double stddev = std::sqrt(variance / speed.size());

    return { mean, stddev };
}

class MileageUnitSpeed {
public:
    // 历史数据缓冲区
    QQueue<double> speed_history;   // 左轮速度历史
    struMileage last_mileage;//最后收到的里程值
    // 更新历史数据
    void updateHistory(const struMileage& current) {
        if (!last_mileage.isValid()) {
            last_mileage = current;
            return;
        }
        double speed = CalculateSpeed(current, last_mileage);
        speed_history.enqueue(speed);
        if (speed_history.size() >= HISTORY_BUFFER_SIZE) {
            speed_history.dequeue();
        }
        last_mileage = current;
    }
    double getAverageSpeed() const {
        return GetSpeedStats(speed_history).first;
    }
    // 检测异常值
    bool isOutlier(double current_speed) const {
        auto [mean, stddev] = GetSpeedStats(speed_history);
        if (stddev < MIN_SPEED_THRESHOLD) {
            return false; // 速度变化很小，不认为是异常
        }

        return std::abs(current_speed - mean) > OUTLIER_THRESHOLD * stddev;
    }

    MileageUnitSpeed() = default;
    ~MileageUnitSpeed() = default;
    // 重置校正器
    void reset() {
        last_mileage = struMileage();
        speed_history.clear();
    }

};

class MileageCorrector {
private:
    // 历史数据缓冲区
    MileageUnitSpeed left_wheel;//left_wheel; 左轮数据
    MileageUnitSpeed right_wheel;
    struMileage current_mileage;
    bool is_need_initialized = true;

    MileageCorrector() = default;
    ~MileageCorrector() = default;
public:
    static MileageCorrector& instance() {
        static MileageCorrector self;
        return self;
    }
    void init() {// 重置校正器
        left_wheel.reset();
        right_wheel.reset();
        is_need_initialized = true;
    }
    struMileage Correct(qint64 left_time, qint64 left_pulse, qint64 right_time, qint64 right_pulse) {
        struMileage left, right;
        left.time = left_time; left.pulse = left_pulse;
        right.time = right_time; right.pulse = right_pulse;
        return Correct(left, right);
    }
    struMileage Correct(const struMileage& left, const struMileage& right) {
        //#1.首次数据处理,初始化判断
        if (is_need_initialized) {
            /*第一次取平均值,初始值需要取两个值才能计算单位脉冲时间的速度,然后完成初始化*/
            current_mileage.time = (left.time + right.time) / 2;
            current_mileage.pulse = (left.pulse + right.pulse) / 2;
            if (left_wheel.speed_history.isEmpty() || right_wheel.speed_history.isEmpty()) {
                left_wheel.last_mileage = left;
                right_wheel.last_mileage = right;
            } else {
                // 需要收集足够的历史数据才能进行校正
                if (left_wheel.speed_history.size() >= MIN_HISTORY_SIZE &&
                    right_wheel.speed_history.size() >= MIN_HISTORY_SIZE) {
                    is_need_initialized = true;
                }
                
                left_wheel.updateHistory(left);
                right_wheel.updateHistory(right);
            }
            return current_mileage;
        }

        // 计算当前速度
        double left_speed = CalculateSpeed(left, left_wheel.last_mileage);
        double right_speed = CalculateSpeed(right, right_wheel.last_mileage);
        // 获取历史平均速度
        double avg_left_speed = GetAverageSpeed(left_wheel.speed_history);
        double avg_right_speed = GetAverageSpeed(right_wheel.speed_history);
        // 计算速度偏差比
        double left_deviation_ratio = (left_speed - avg_left_speed) / avg_left_speed;
        double right_deviation_ratio = (right_speed - avg_right_speed) / avg_right_speed;

        // 计算脉冲和时间增量
        qint64 left_pulse_diff = left.pulse - left_wheel.last_mileage.pulse;
        qint64 right_pulse_diff = right.pulse - right_wheel.last_mileage.pulse;
        qint64 left_time_diff = left.time - left_wheel.last_mileage.time;
        qint64 right_time_diff = right.time - right_wheel.last_mileage.time;
        // 根据异常检测结果选择数据,增量计算
        qint64 pulse_increment;
        qint64 time_increment;
        /*出现的情况有:1.转弯,一边速度快,一边慢 2.轮子打滑,空转速度快 3.轮子卡住,速度慢 4.正常启动加速,后稳定匀速,同时快*/
        if (left_deviation_ratio > MAX_SPEED_DEVIATION_RATIO) { //左边过快,加速
            if (right_deviation_ratio > MAX_SPEED_DEVIATION_RATIO) { //右边也同时过快,或者加速
                // 两边都过快,取平均值
                //pulse_increment = (left_pulse_diff + right_pulse_diff) / 2;
                //time_increment = (left_time_diff + right_time_diff) / 2;
                // 选择偏离平均值较小的轮子
                if (left_deviation_ratio < right_deviation_ratio) {
                    pulse_increment = left_pulse_diff;
                    time_increment = left_time_diff;
                } else {
                    pulse_increment = right_pulse_diff;
                    time_increment = right_time_diff;
                }
            } else if (right_deviation_ratio < MAX_SPEED_DEVIATION_RATIO) { //转弯之类的不合理速度
                // 左边过快,右边慢，取平均值  其他:转弯时，使用较慢轮子的数据（内轮）
                pulse_increment = (left_pulse_diff + right_pulse_diff) / 2;
                time_increment = (left_time_diff + right_time_diff) / 2;
            } else {
                // 右边正常，左边异常
                pulse_increment = right_pulse_diff;
                time_increment = right_time_diff;
            }
        
        } else if (left_deviation_ratio < MAX_SPEED_DEVIATION_RATIO) { //左边慢,不转之类情况
            if (right_deviation_ratio < MAX_SPEED_DEVIATION_RATIO) {
                // 两边都过慢,取平均值
                pulse_increment = (left_pulse_diff + right_pulse_diff) / 2;
                time_increment = (left_time_diff + right_time_diff) / 2;
            } else if (right_deviation_ratio > MAX_SPEED_DEVIATION_RATIO) { //转弯之类的不合理速度
                // 右边快，取平均值
                pulse_increment = (left_pulse_diff + right_pulse_diff) / 2;
                time_increment = (left_time_diff + right_time_diff) / 2;
            } else {
                // 右边正常，左边异常
                pulse_increment = right_pulse_diff;
                time_increment = right_time_diff;
            }
        } else {
            if (right_deviation_ratio < MAX_SPEED_DEVIATION_RATIO) { //右边过快,或者加速
                // 左边正常
                pulse_increment = left_pulse_diff;
                time_increment = left_time_diff;
            } else if (right_deviation_ratio > MAX_SPEED_DEVIATION_RATIO) { //转弯之类的不合理速度
                // 左边正常
                pulse_increment = left_pulse_diff;
                time_increment = left_time_diff;
            } else {
                // 右边正常，左边正常 取平均值
                pulse_increment = (left_pulse_diff + right_pulse_diff) / 2;
                time_increment = (left_time_diff + right_time_diff) / 2;
            }
        }

        // 更新当前里程
        current_mileage.pulse += pulse_increment;
        current_mileage.time += time_increment;

        // 更新历史数据
        left_wheel.updateHistory(left);
        right_wheel.updateHistory(right);
        return current_mileage;
    }

};