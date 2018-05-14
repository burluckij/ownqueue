//
// Author:
//		Burlutsky Stas
//
//		burluckij@gmail.com
//

#include <mutex>
#include <memory>
#include <queue>
#include <windows.h>

namespace ownqueue
{
	//
	// Object which will be in a queue as one of all queue elements.
	//
	class Entry{
	public:
		Entry(){}
		Entry(std::string _name):m_name(_name){}

		std::string getName() const{
			return m_name;
		}

	private:
		std::string m_name;
	};

	class ExceptionQueue{
		public:
			ExceptionQueue(std::string _msg): m_message(_msg){}
			std::string message() const {
				return m_message;
			}

		private:
			std::string m_message;
	};

	class WorkQueue
	{
	public:

		WorkQueue(unsigned long _queueLimit = 10000);
		~WorkQueue();
		
		// Adds new element to the queue.
		bool add(const Entry& _object);

		// Removes and takes an element from the queue.
		bool take(Entry& _object);

		bool shutdown();

		bool isEnabled() const ;

		unsigned long length() const;

	private:
		mutable std::mutex m_lock;
		std::queue<Entry> m_internalQueue;
		unsigned long m_queueLimit;
		HANDLE m_hEventShutdown;
		HANDLE m_hEventNotEmpty;

		//
		// Forget about copy constructors.
		//

		WorkQueue(const WorkQueue& _other);
		WorkQueue& operator=(const WorkQueue&);
		WorkQueue(WorkQueue&& _other);
		WorkQueue& operator=(WorkQueue&&);
	};
}
