#include "MessageQueue.h"

template<class T> 
bool MessageQueue<T>::clearExpired() {
	bool is_message_deleted = false;
	
	auto mq_end = _mqueue.end();
	time_point tp_now = std::chrono::high_resolution_clock::now();

	//got long ass list of errors, 
	//smth like "use of deleted function".
	//no idea what the freak, similar lambdas
	//work perfectly fine in "QueueAnalyzer".
	//the issue should be with remove_if
	/*
	auto is_expired = [&tp_now](std::pair<int, std::pair<T, time_point>> p){
		return p.second.second < tp_now;
	};
	auto new_mq_end = remove_if(_mqueue.begin(), _mqueue.end(), is_expired);
	is_message_deleted = (mq_end != new_mq_end);
	*/

	//shamelessly taken from cppreference
	for (auto it = _mqueue.begin(); it != _mqueue.end();) {
        if (it->second.second < tp_now)
            it = _mqueue.erase(it);
        else
            ++it;
    }
	_size = _mqueue.size();

	return is_message_deleted;
	//might be easier with erase_if i guess, but it's only usable since c++20
}

template<class T>
MessageQueue<T>::MessageQueue(size_t maxsize) : 
				_maxsize(maxsize), _size(0) {}

template<class T>
void MessageQueue<T>::Add(const int& priority, const T& message, 
							const time_point& tp_expired) {
	
	if(_size == _maxsize) {
		if(!clearExpired()) {
			throw QueueFullException();
			
		}
	}
	
	_mqueue.insert({priority, {message, tp_expired}});
	_size++;
}


template<class T>
bool MessageQueue<T>::isMessageValid(const std::pair<T, time_point>& message, 
									 const time_point& tp_now) {
	return message.second > tp_now;
}

template<class T>
T MessageQueue<T>::Get() {
	
	time_point tp_now = std::chrono::high_resolution_clock::now();

	while(!_mqueue.empty()) {
		std::pair<T, time_point> message = _mqueue.begin()->second;
		_mqueue.erase(_mqueue.begin());
		_size--;
		if(isMessageValid(message, tp_now)) {
			return message.first;
		}
	}

	if(_mqueue.empty()) {
		throw QueueEmptyException();
	}
	else {
		throw QueueExpiredException();
	}
	
	//nope, still doesn't work
	//throw std::exception("Queue is empty or all the messages have been expired!");
}

