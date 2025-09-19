#pragma once
using StateHandler = std::function<void()>;
class SHAREDLIB_EXPORT StateEvent: public QObject
{
    Q_OBJECT
public:
    using StateType = int;//typedef int StateType;
    enum State : StateType {
        Offline = 0,    // 离线
        Waiting,        // 等待  
        PreStart,       // 准备开始
        Starting,       // 开始
        Started,        // 已开始
        Running,        // 运行中
        Finished,       // 已完成
        Paused,         // 暂停
        Resumed,        // 恢复
        Cancelled,      // 取消
        Timeout,        // 超时
        Failed,         // 失败
        Stopped,        // 停止
        Aborted,        // 中止
        Error           // 错误
    };
    static StateEvent& TaskEvent() {
        static StateEvent event;
        return event;
    };
public:
    // 构造函数
    StateEvent() = default;
    StateEvent(double value) : state(static_cast<State>(value)) {}
    StateEvent(State state) : state(state){}
    QAtomicInteger<StateType> state{ Waiting };
    //State state = Unknown;
    void setState(State newState) {
        State oldState = static_cast<State>(state.loadAcquire());
        if (oldState != newState) {
            state.storeRelease(newState);
            QReadLocker locker(&lock);
            emit state_changed(newState);
            auto it = handlers.find(newState);
            if (it != handlers.end()) {
                StateHandler handler = it.value(); 
                locker.unlock();
                handler();
            }
        }
    }
    void addHandler(State newState, StateHandler handler) {
        QWriteLocker locker(&lock);
        handlers[newState] = handler;
    }

    // 状态转换为字符串
    QString toString() const {
        static const QMap<State, QString> stateNames = {
            {Offline, "离线"},
            {Waiting, "等待"},
            {PreStart, "准备开始"},
            {Starting, "开始"},
            {Started, "已开始"},
            {Running, "运行中"},
            {Finished, "已完成"},
            {Paused, "暂停"},
            {Resumed, "恢复"},
            {Cancelled, "取消"},
            {Timeout, "超时"},
            {Failed, "失败"},
            {Stopped, "停止"},
            {Aborted, "中止"},
            {Error, "错误"}
        };
        State currentState =  static_cast<State>(state.loadAcquire());
        return stateNames.value(currentState, "离线");
    }

    // 返回 double类型的状态值（状态枚举转换为 double）
    double toDouble() const {
        return static_cast<double>(state.loadAcquire());
    }
    // 操作符重载
        // 赋值操作符
    // 赋值操作符 返回引用,可以支持链式赋值
    StateEvent& operator=(double value) {
        setState(static_cast<State>(value));
        return *this;
    }
    void operator=(const State &newState) {
        setState(newState);
    }

    bool operator==(const StateEvent& other) const {
        return state.loadAcquire() == other.state.loadAcquire();
    }
    bool operator!=(const StateEvent& other) const {
        return !(*this == other);
    }

    operator State() const {
        return static_cast<State>(state.loadAcquire());
    }
    
protected:
    mutable QReadWriteLock lock; // 保护 handlers的互斥锁
    QMap<State, StateHandler> handlers;
signals:
    void state_changed(StateEvent::State newState);
};
