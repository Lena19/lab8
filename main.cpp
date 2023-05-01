#include <algorithm>
#include <ctime>
#include <future>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

#include "QueueAnalyzer.h"
#include "Message.cpp"


using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

//number of seconds threads are "alive"
const int alive_for = 300;

std::mutex mtx;
std::condition_variable cv;

size_t get_hash(const std::thread::id& id) {
	return std::hash<std::thread::id>{}(id);
}

//added volatile, when ran into a situation
//where the random numbers from threads 1-3
//were the same every time. 
//but i guess the problem is in the rand()
//itself, maybe time(0) is not a good seed for srand() :( 

//mt19937 might have solved this problem, idk,
//but i don't want to overcomplicate randomizer,
//so the way to adjust it, is just to use the threa id, because why not
volatile int get_priority() {
	return (get_hash(std::this_thread::get_id()) + rand()) % NUM_OF_PRIORITIES;
}

//1 to 7 seconds
volatile int get_sleep() {
	return (get_hash(std::this_thread::get_id()) + rand()) % 7 + 1;
}

//0 - add
//1 - delete
volatile int get_op_id() {
	return (get_hash(std::this_thread::get_id()) + rand()) % 2;
}

//generate both message and its priority
//message is relevant for [(priority + 1) * 2] seconds
//e.g. message with priority 3 will be relevant for 8 seconds
//message with priority 7 will be relevant for 16 seconds 
Message generate_message(int& priority, time_point& tp_expired) {
	time_point tp_now = std::chrono::high_resolution_clock::now();
	priority = get_priority();
	tp_expired = tp_now + std::chrono::seconds((priority + 1) * 2);
	return Message(messages[priority]);
}


void add_message(MessageQueue<Message>& mQueue) {	
	time_point tp_now = std::chrono::high_resolution_clock::now();
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(tp_now.time_since_epoch()).count() << '\n';
	try {
		int priority;
		time_point tp_expired;
		Message msg = generate_message(priority, tp_expired);
		mQueue.Add(priority, msg, tp_expired);
		std::cout << "Message with priority " << priority;
		std::cout << " added from thread " << std::this_thread::get_id() << "\n\n";
	}
	catch(QueueFullException& e) {
		std::cerr << "Thread " << std::this_thread::get_id() << " have a problem:\n";		
		std::cerr << e.what() << "\n\n";
		cv.notify_one();
	}
	catch(const std::exception& e) {		
		std::cerr << "Thread " << std::this_thread::get_id() << " have a problem:\n";
		std::cerr << e.what() << "\n\n";
	}
}


void delete_message(MessageQueue<Message>& mQueue) {
	time_point tp_now = std::chrono::high_resolution_clock::now();
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(tp_now.time_since_epoch()).count() << '\n';
	try {
		Message msg = mQueue.Get();
		std::cout << "Message deleted in thread " << std::this_thread::get_id() << "\n\n";
	}
	catch(const std::exception& e) {	
		std::cerr << "Thread " << std::this_thread::get_id() << " have a problem:\n";	
		std::cerr << e.what() << "\n\n";
	}
}

void analyze_queue(MessageQueue<Message>& mQueue, QueueAnalyzer<Message>& qAnalyzer) {
	time_point tp_now = std::chrono::high_resolution_clock::now();
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(tp_now.time_since_epoch()).count() << '\n';
	std::cout << "Analyzer is active in thread " << std::this_thread::get_id() << "\n\n";
	qAnalyzer.Analyze(mQueue);
}

//practically as i understand it's not a parallel program,
//not even pseudoparallel,
//because to ensure queue is processed by one thread at a time
//i should lock every single thread every time it starts working.
//what's the point? am i doing something wrong?
void random_thread(MessageQueue<Message>& mQueue) {

	//not the most elegant solution for keeping 
	//this alive for 5 minutes, but as i understand,
	//there's no actually a proper way to terminate
	//the thread after a certain amount of time.

	time_point start = std::chrono::high_resolution_clock::now();
	while(std::chrono::high_resolution_clock::now() - start < std::chrono::seconds(alive_for)) {
		std::unique_lock lock(mtx);

		if(get_op_id()) {
			delete_message(mQueue);
		}
		else {
			add_message(mQueue);
		}
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(get_sleep()));
		
	}
}

void delete_thread(MessageQueue<Message>& mQueue) {
	time_point start = std::chrono::high_resolution_clock::now();
	while(std::chrono::high_resolution_clock::now() - start < std::chrono::seconds(alive_for)) {
		std::unique_lock lock(mtx);

		delete_message(mQueue);

		lock.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int analyze_queue_thread(MessageQueue<Message>& mQueue, QueueAnalyzer<Message>& qAnalyzer) {
	static int calls = 0;
	time_point start = std::chrono::high_resolution_clock::now();
	while(std::chrono::high_resolution_clock::now() - start < std::chrono::seconds(alive_for)) {
		
		time_point tp_now = std::chrono::high_resolution_clock::now();
		int left = std::chrono::duration_cast<std::chrono::seconds>
					(start + std::chrono::seconds(alive_for) - tp_now).count();

		left = std::min(60, left + 1);

		std::unique_lock lock(mtx);
		//analyzer is called at the last second
		//regardless of whether the previous call
		//was 60 seconds ago or later
		cv.wait_for(lock, std::chrono::seconds(left));
		analyze_queue(mQueue, qAnalyzer);
		calls++;

		lock.unlock();
		qAnalyzer.WriteToFile();
		
	}
	return calls;
}

int main() {
	srand(time(0));
	MessageQueue<Message> mQueue(5);
	std::string file_path = "Log.txt";
	QueueAnalyzer<Message> qAnalyzer(file_path);

	auto t1 = std::async(std::launch::async, random_thread, std::ref(mQueue));
	auto t2 = std::async(std::launch::async, random_thread, std::ref(mQueue));
	auto t3 = std::async(std::launch::async, random_thread, std::ref(mQueue));
	auto t4 = std::async(std::launch::async, delete_thread, std::ref(mQueue));
	auto t5 = std::async(std::launch::async, analyze_queue_thread, 
						std::ref(mQueue), std::ref(qAnalyzer));
	t5.wait();
	std::cout << "Analyzer was called " << t5.get() << " times\n";
	return 0;
}