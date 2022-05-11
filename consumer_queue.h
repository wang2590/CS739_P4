#ifndef CONSUMER_QUEUE_H
#define CONSUMER_QUEUE_H
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
using Clock = std::chrono::system_clock;
using Duration = std::chrono::duration<long int, std::ratio<1, 1000000000>>;
using TimePoint = std::chrono::time_point<Clock, Duration>;


template <typename T>
class consumer_queue {
 public:
  consumer_queue() = default;

  int do_fill(T data) {
    std::unique_lock<std::mutex> ul(lock);
    this->buffer.push(data);
    ul.unlock();
    return 0;
  };

  int do_get(TimePoint time_out, T& res) {
    std::unique_lock<std::mutex> ul(lock);
    while (this->buffer.empty()) {
      if (cv.wait_until(ul, time_out) == std::cv_status::timeout) {
        return -1;
      }
    }
    res = this->buffer.front();
    this->buffer.pop();
    ul.unlock();
    return 0;
  };

 private:
  std::queue<T> buffer;
  std::mutex lock;
  std::condition_variable cv;
};

#endif