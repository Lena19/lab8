#pragma once

#include <chrono>
#include <map>
#include "QueueExceptions.cpp"


//i "alleviated" the class according
//to the feedback (at least tried to).
//priority is no more a template parameter
//and can only be [convertible to] int.
//the basic container for the queue is changed
//to the multimap<int, std::pair<T, time_point when expired> >.
template<class T>
class MessageQueue {

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

private:
	std::multimap<int, std::pair<T, time_point>> _mqueue;
	size_t _maxsize;
	size_t _size;

	//true if managed to delete any non-zero amount of messages
	bool clearExpired();
	//true if message time_point bigger than tp_now
	bool isMessageValid(const std::pair<T, time_point>& message,
						const time_point& tp_now);
public:
	MessageQueue(size_t maxsize);
	void Add(const int& priority, const T& message, const time_point& tp_expired);
	T Get();

	//template<T>
	template<class> friend class QueueAnalyzer;
};

#include "MessageQueue.cpp"