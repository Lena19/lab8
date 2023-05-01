#include <exception>

struct QueueFullException : public std::exception {
	const char * what () const throw () {
		return "Queue is full!";
	}
};

struct QueueEmptyException : public std::exception {
	const char * what () const throw () {
		return "Queue is empty!";
	}
};

struct QueueExpiredException : public std::exception {
	const char * what () const throw () {
		return "All the messages have been expired!";
	}
};