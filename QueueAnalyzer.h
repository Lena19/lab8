#pragma once

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include "MessageQueue.h"

template<class T> 				
class QueueAnalyzer {

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
using prior_queue = std::multimap<int, std::pair<T, time_point>>;

private:
	
	//hmmmm, don't use raw pointers, right?
	//reference doesn't work, crying that it
	//is not initialized. and anyway if actual
	//_pmQueue somehow gets destroyed, i'll 
	//just end up with some garbage poined to.
	//copying is not an option, because i only
	//get an initial state of the queue, which
	//is most probably still empty, and the 
	//analysis just doesn't make sense this way

	//i think i'll just make a copy every time, 
	//by passing const reference directly
	//to the Analyze(). anyway i need some kind
	//of "snapshot". would be stupid to do an 
	//analysis on the dynamically changing queue.
	//like if it's big, i might end up with 
	//analyzing two different states of the same
	//queue. yeah, i know, i can just lock the
	//queue until analysis is finished, but it doesn't
	//sound reasonable, 'cause i see an analyzer as a
	//auxiliary function, it just doesn't make sense
	//to stop the queue.

	//however, is it still possible to get a situation,
	//where while copying the queue "snapshot" to the
	//field _pmQueue, this snapshot is changed? 
	//and the same stuff again - i have two versions
	//of one queue in one field. 
	
	//when you think you have two chairs, 
	//but it's actually the same one (╯° °）╯︵ ┻━┻

	//at the end i just decided to lock the thread every time.
	//0% parallel, >0% safe


	//[priority, std::pair<T message, time_point when expired>]
	//prior_queue* _pmQueue;
	prior_queue _pmQueue; 

	std::string _fpath;		
	std::ofstream _fout;

	time_point tp_now;

	//number of elements
	size_t queue_size;

	//total size in bytes
	int queue_size_bytes;
	std::map<int, double> stats;
	time_point first_expired, 
				last_expired;
	double max_difference;

	time_point getCurrentTime();
	size_t getQueueSize();
	std::map<int, double> getPercentages();
	int getQueueSizeBytes();
	time_point getMaxTimeExpired();
	time_point getMinTimeExpired();		
public:
	//QueueAnalyzer(const MessageQueue<T>& pmQueue, const std::string& filepath);
	QueueAnalyzer(const std::string& filepath);
	//format for a human to read, except for current time_point
	void Analyze(const MessageQueue<T>& pmQueue);
	void WriteToFile();
};

#include "QueueAnalyzer.cpp"