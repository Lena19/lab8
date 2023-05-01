//message may have one of the 10 levels of importance
//0 - the most important
//9 - the least important
const int NUM_OF_PRIORITIES = 10;
const std::string messages[] = {
	"Error",
	"Message with the 1st level of importance",
	"Message with the 2nd level of importance",
	"Message with the 3rd level of importance",
	"Message with the 4th level of importance",
	"Message with the 5th level of importance",
	"Message with the 6th level of importance",
	"Message with the 7th level of importance",
	"Message with the 8th level of importance",
	"Log"
};

struct Message {
	std::string text;
	Message(std::string txt) : text(txt) {}
	size_t size() {
		return text.length();
	}
	friend std::ostream& operator<<(std::ostream& os, const Message& msg);
};

std::ostream& operator<<(std::ostream& os, const Message& msg) {
	os << msg.text;
	return os;
}