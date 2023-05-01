#include "QueueAnalyzer.h"

template<typename T>
QueueAnalyzer<T>::QueueAnalyzer(const std::string& filepath) {
	_fpath = filepath;
	//file is no more being rewritten
	//every entry is appended to the end of the file
	tp_now = std::chrono::high_resolution_clock::now();
	_fout.open(_fpath, std::ios::app);
	_fout << "Current time: " << std::chrono::duration_cast<std::chrono::seconds>(tp_now.time_since_epoch()).count() << "\n";
	_fout << "New analyzer is created\n\n";
	_fout.close();
}

template<typename T>
typename QueueAnalyzer<T>::time_point QueueAnalyzer<T>::getCurrentTime() {
	return std::chrono::high_resolution_clock::now();
}

template<typename T>
size_t QueueAnalyzer<T>::getQueueSize() {
	return _pmQueue.size();
}

template<typename T>
std::map<int, double> QueueAnalyzer<T>::getPercentages() {
	std::map<int, double> perc;
	size_t total = _pmQueue.size();
	for(auto it = _pmQueue.begin(); it != _pmQueue.end(); it = _pmQueue.upper_bound(it->first)) {
		int key_count = _pmQueue.count(it->first);
		perc[it->first] = 100.0 * key_count / total; 
	}
	return perc;
}

template<typename T>
int QueueAnalyzer<T>::getQueueSizeBytes() {
	int queue_size = 0;
	for(auto it = _pmQueue.begin(); it != _pmQueue.end(); it++) {
		//from std::pair<T, time_point>
		T message = it->second.first;
		queue_size += message.size();
	}
	return queue_size;
}

template<typename T>
typename QueueAnalyzer<T>::time_point 
QueueAnalyzer<T>::getMaxTimeExpired() {
	//default time_point, much earlier than the current
	time_point last_expired = tp_now - std::chrono::hours(100500);

	//[priority, <T, timepoint>]
	auto find_max = [](std::pair<int, std::pair<T, time_point>> p1, 
						std::pair<int, std::pair<T, time_point>> p2){
							return p1.second.second > p2.second.second;	 
						};
	auto max_entry = std::max_element(_pmQueue.begin(), _pmQueue.end(), find_max);
	if(!_pmQueue.empty()) {
		last_expired = max_entry->second.second;
	}
	return last_expired;
}

template<typename T>
typename QueueAnalyzer<T>::time_point 
QueueAnalyzer<T>::getMinTimeExpired() {
	//default time_point, much later than the current
	time_point first_expired = tp_now + std::chrono::hours(100500);

	//[priority, <T, timepoint>]
	auto find_min = [](std::pair<int, std::pair<T, time_point>> p1, 
						std::pair<int, std::pair<T, time_point>> p2){
							return p1.second.second < p2.second.second;	 
						};
	auto min_entry = std::min_element(_pmQueue.begin(), _pmQueue.end(), find_min);
	if(!_pmQueue.empty()) {
		first_expired = min_entry->second.second;
	}
	return first_expired;
}							

template<typename T>
void QueueAnalyzer<T>::Analyze(const MessageQueue<T>& pmQueue) {
	_pmQueue = pmQueue._mqueue;
	tp_now = getCurrentTime();
	queue_size = getQueueSize();
	queue_size_bytes = getQueueSizeBytes();
	stats = getPercentages();
	first_expired = getMinTimeExpired();
	last_expired = getMaxTimeExpired();
	max_difference = 0;

	if(first_expired <= last_expired) {
		max_difference = std::chrono::duration<double>(last_expired - first_expired).count();
	}
	//gotta do it explicitly in order to 
	//not lock the thread for extra time
	//WriteToFile();
}

template<typename T>
void QueueAnalyzer<T>::WriteToFile() {
	_fout.open(_fpath, std::ios::app);
	_fout << "Current time: " << std::chrono::duration_cast<std::chrono::seconds>(tp_now.time_since_epoch()).count() << "\n";
	_fout << "Queue size: " << queue_size << " elements\n";
	_fout << "Stats:\n";
	_fout << "Priority\t%\n";
	for(auto it = stats.begin(); it != stats.end(); it++) {
		_fout << it->first << "\t\t\t" << it->second << "%\n";
	}
	_fout << "Total size of the queue: " << queue_size_bytes / 1024.0 << "Kb\n";
	_fout << "The biggest difference between time expired: " << max_difference << "s\n\n";
	_fout.close();
}
