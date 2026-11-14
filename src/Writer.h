#ifndef WRITER_H
#define WRITER_H

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <mutex>
#include <thread>
#include <utility>
#include <stdexcept>

template <typename T>
class Writer {

public:
    explicit Writer(std::size_t queue_capacity)
        : queue_capacity_(queue_capacity)
    {
        if (queue_capacity_ == 0) throw std::invalid_argument("Writer queue capacity must be positive");
    }

    virtual ~Writer() {
        stop_noexcept();
    }

    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;

    void start() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (started_) return;

        started_ = true;
        stopping_ = false;

        writer_thread_ = std::thread(&Writer::run, this);
    }

    void enqueue(T data) {
        std::unique_lock<std::mutex> lock(mutex_);

        queue_not_full_.wait(lock, [this] {
            return queue_.size() < queue_capacity_ || writer_error_ || stopping_;
        });

        if (writer_error_) std::rethrow_exception(writer_error_);

        if (stopping_) throw std::runtime_error("Writer is stopping");

        queue_.push_back(std::move(data));

        lock.unlock();
        queue_not_empty_.notify_one();
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
        }

        queue_not_empty_.notify_all();
        queue_not_full_.notify_all();

        if (writer_thread_.joinable()) writer_thread_.join();

        if (writer_error_) std::rethrow_exception(writer_error_);
    }

protected:
    virtual void write(const T& data) = 0;

    void stop_noexcept() {
        try {
            stop();
        } catch (...) {
            // No podem llançar excepcions des del destructor.
        }
    }

private:
    void run() {
        try {
            while (true) {
                T data;

                {
                    std::unique_lock<std::mutex> lock(mutex_);

                    queue_not_empty_.wait(lock, [this] {
                        return !queue_.empty() || stopping_;
                    });

                    if (queue_.empty() && stopping_)
                        break;

                    data = std::move(queue_.front());
                    queue_.pop_front();
                }

                queue_not_full_.notify_one();

                write(data);
            }
        }
        catch (...) {
            {
                std::lock_guard<std::mutex> lock(mutex_);

                writer_error_ = std::current_exception();
                stopping_ = true;
                queue_.clear();
            }

            queue_not_full_.notify_all();
            queue_not_empty_.notify_all();
        }
    }

    const std::size_t queue_capacity_;

    std::deque<T> queue_;

    std::mutex mutex_;
    std::condition_variable queue_not_empty_;
    std::condition_variable queue_not_full_;

    std::thread writer_thread_;

    bool started_ = false;
    bool stopping_ = false;

    std::exception_ptr writer_error_;
};

#endif