//
// Author:
//		Burlutsky Stas
//
//		burluckij@gmail.com
//

#include <iostream>
#include <thread>
#include <string>
#include "ownqueue.h"

void takeNElements(ownqueue::WorkQueue& _queue, unsigned long _count)
{
	printf("\n%s (thread id is %d): Take %d elements from queue.\n", __FUNCTION__, GetCurrentThreadId(), _count);

	for (unsigned long i = 0; i < _count; ++i)
	{
		ownqueue::Entry entry;
		if (_queue.take(entry))
		{
			printf("\n%s (thread id is %d): successfully take element from queue: %s\n", __FUNCTION__, GetCurrentThreadId(), entry.getName().c_str());
		}
		else
		{
			bool closed = !_queue.isEnabled();
			printf("\n%s (thread id is %d): can't take element from queue. %s\n", __FUNCTION__, GetCurrentThreadId(),
				(closed ? "Queue is closed.": ""));

			if (closed)
			{
				break;
			}
		}
	}

	printf("\n%s (thread id is %d): Finished.\n", __FUNCTION__, GetCurrentThreadId());
}

int main()
{
	ownqueue::WorkQueue wrkqueue(5);

	wrkqueue.add(ownqueue::Entry("obj1"));
	wrkqueue.add(ownqueue::Entry("obj2"));
	wrkqueue.add(ownqueue::Entry("obj3"));
	wrkqueue.add(ownqueue::Entry("obj4"));

	std::thread(takeNElements, std::ref(wrkqueue), 5).detach();
	std::thread(takeNElements, std::ref(wrkqueue), 4).detach();

	Sleep(10);

	while (true)
	{
		std::string name;
		std::cout << "\nCurrent queue size is: " << wrkqueue.length();
		std::cout << "\nEnter word for new entry name: ";
		std::cin >> name;
		std::cout << std::endl;

		if (name == "exit")
		{
			break;
		}
		else if (name == "shutdown")
		{
			wrkqueue.shutdown();
		}
		else
		{
			auto added = wrkqueue.add(ownqueue::Entry(name));
			if (!added)
			{
				std::cout << "\nCan't add an entry to the queue\n";
			}
		}
	}

	std::cout << "\nPress any key to exit.\n";
	getchar();
}

