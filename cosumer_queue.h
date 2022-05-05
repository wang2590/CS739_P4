#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H
#include <queue>
#include <mutex>

template<typename T>
class consumer_queue {
    public:
        consumer_queue();
        
        bool consumer_ready() {
            return this->buffer.empty();
        };

        bool producer_ready() {
            return this->buffer.size() == this->max;
        }
        
        void do_fill(T data) {
            this->buffer.push(data);
        };
        
        T do_get() {
            T res = this->buffer.front();
            this->buffer.pop();
            return res;
        };

    private:
        std::queue<T> buffer;
        int max{4};
};


#endif