#ifndef CONSUMER_QUEUE_H
#define CONSUMER_QUEUE_H
#include <mutex>
#include <queue>

template <typename T>
class consumer_queue {
 public:
  consumer_queue();

  int do_fill(T data) {
    std::unique_lock<std::mutex> ul(lock);
    while (buffer.size() == max) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    this->buffer.push(data);
    ul.unlock();
    return 0;
  };

  int do_get(auto start_time, T& res) {
    while (this->buffer.empty()) {
      auto end_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                start_time)
              .count() >= time_out) {
        std::cout << "Timeout: Read Failed!\n";
        return -1;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                              start_time)
            .count() >= time_out) {
      std::cout << "Timeout: Read Failed!\n";
      return -1;
    }

    std::unique_lock<std::mutex> ul(q.lock);
    res = this->buffer.front();
    this->buffer.pop();
    ul.unlock();
    return 0;
  };

 private:
  std::queue<T> buffer;
  int max{4};
  int time_out{10000};
  std::mutex lock;
};

#endif