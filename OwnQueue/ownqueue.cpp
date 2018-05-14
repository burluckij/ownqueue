//
// Author:
//		Burlutsky Stas
//
//		burluckij@gmail.com
//

#include "ownqueue.h"

namespace ownqueue
{
	
	WorkQueue::WorkQueue(unsigned long _queueLimit /*= 10000*/) :
		m_queueLimit(_queueLimit)
	{
		//
		// At first we should create a not signaled, manual-reseted event for queue shutdown.
		//
		m_hEventShutdown = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (m_hEventShutdown)
		{
			//
			// Here we are creating an event for internal queue's state.
			// Object is in a signal state when our internal queue is not empty,
			// and has at least one object. 
			//
			m_hEventNotEmpty = CreateEventA(NULL, FALSE, FALSE, NULL);
			if (!m_hEventNotEmpty)
			{
				CloseHandle(m_hEventShutdown);

				throw ExceptionQueue("can't create queue internal state event");
			}
		}
		else
		{
			// We can't create an event object, it's critical for us.
			throw ExceptionQueue("can't create shutdown event");
		}
	}

	WorkQueue::~WorkQueue()
	{
		if (m_hEventShutdown){
			// I would recommend to call shutdown() method somewhere externally but not here.
			CloseHandle(m_hEventShutdown);
		}

		if (m_hEventNotEmpty){
			CloseHandle(m_hEventNotEmpty);
		}
	}

	bool WorkQueue::add(const Entry& _object)
	{
		m_lock.lock();

		//
		// Our internal queue is full.
		//
		if (m_internalQueue.size() >= m_queueLimit)
		{
			m_lock.unlock();
			return false;
		}

		//
		// Add to internal queue.
		//
		m_internalQueue.push(_object);

		//
		// Signal event about that.
		//
		SetEvent(m_hEventNotEmpty);

		m_lock.unlock();

		return true;
	}

	bool WorkQueue::take(Entry& _object)
	{
		m_lock.lock();

		//
		// Our internal queue is not empty.
		//
		if (!m_internalQueue.empty())
		{
			//
			// Takes the last one end removes it form the queue.
			//
			_object = m_internalQueue.front();
			m_internalQueue.pop();

			m_lock.unlock();
			return true;
		}

		//
		// Acquire resources.
		//
		HANDLE hEvents[2] = { m_hEventNotEmpty, m_hEventShutdown };

		m_lock.unlock();

		while (true)
		{
			//
			// Only one thread will woke up.
			//
			auto res = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

			if (res == WAIT_OBJECT_0)
			{
				//
				// There is a something new to take!
				//
				std::unique_lock<std::mutex> lock(m_lock);

				//
				// It's need to do a second check.
				//
				if (!m_internalQueue.empty())
				{
					_object = m_internalQueue.front();
					m_internalQueue.pop();
					return true;
				}
				else
				{
					//
					// Ok. There was a second thread which didn't wait in WaitForMultipleObjects(..)
					// Keep try again!
					//
				}
			}
			else if (res == (WAIT_OBJECT_0 + 1))
			{
				//
				// It is time to shutdown the queue, stop all attempts.
				//

				break;
			}
			else
			{
				//throw ExceptionQueue("an error occurred in WaitFor() function, may be handle event was destroyed, closed...");
				break;
			}
		}

		return false;
	}

	bool WorkQueue::shutdown()
	{
		std::unique_lock<std::mutex> lock(m_lock);

		if (m_hEventShutdown)
		{
			return SetEvent(m_hEventShutdown) == TRUE;
		}

		return false;
	}

	bool WorkQueue::isEnabled() const
	{
		bool enabled = true;
		std::unique_lock<std::mutex> lock(m_lock);

		if (m_hEventShutdown)
		{
			//
			// Verifies object state and returns true is shutdown event object is not signaled. 
			//
			enabled = (WaitForSingleObject(m_hEventShutdown, 0) != WAIT_OBJECT_0);
		}

		return enabled;
	}

	unsigned long WorkQueue::length() const
	{
		std::unique_lock<std::mutex> lock(m_lock);

		return m_internalQueue.size();
	}

}
