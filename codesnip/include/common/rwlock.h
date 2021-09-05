#include <mutex>
#include <climits>
#include <condition_variable>

/**
 * @brief 读写锁，造轮子
 */
class RWLock
{
    static const uint32_t max_reader = UINT_MAX;
public:
    RWLock() : writing_(false), reader_count_(0) {}
    ~RWLock(){
        std::lock_guard<std::mutex> guard(mu_);
    }

    RWLock(const RWLock &) = delete;
    RWLock &operator=(const RWLock &) = delete;

    void WLock(){
        std::unique_lock<std::mutex> lock(mu_);
        // 首先是否有读等待
        while(writing_){
            reader_.wait(lock);
        }
        writing_ = true;
        while(reader_count_>0){
            writer_.wait(lock);
        }
    }

    void WULock(){
        std::lock_guard<std::mutex> guard(mu_);
        writing_ = false;
        reader_.notify_all();
    }

    void RLock(){
        std::unique_lock<std::mutex> lock(mu_);
        while(writing_||reader_count_>=max_reader){
            reader_.wait(lock);
        }
        reader_count_++;
    }

    void RUnlock(){
        std::lock_guard<std::mutex> guard(mu_);
        reader_count_--;
        if(writing_){
            if(reader_count_==0){
                writer_.notify_one();
            }
        } else {
            if(reader_count_==max_reader-1){
                reader_.notify_one();
            }
        }
    }

private:
    std::mutex mu_;
    std::condition_variable reader_;
    std::condition_variable writer_;
    bool writing_;
    uint32_t reader_count_;
};