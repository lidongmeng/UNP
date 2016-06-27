/*************************************************************************
	> File Name: reator.h
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Sat 19 Sep 2015 07:45:08 PM PDT
 ************************************************************************/

#include <unistd.h>
#include <stdint.h>
#include <sys/poll.h>

namespace reactor {
	typedef unsigned int event_t;
	enum {
		kReadEvent = 0x01;
		kWriteEvent = 0x02;
		kErrorEvent = 0x04;
		kEventMask = 0xff;
	};

	typedef int handle_t; // type of handle

	// event dealer
	class EventHanler {
	public:
		virtual handle_t GetHanle() const = 0;
		virtual void HandleRead() {}
		virtual void HandleWrite() {}
		virtual void HandleError() {}
	protected:
		EventHandler() {}
		virtual ~EventHandler() {}
	};

	class ReactorImplementation;

	// interface of reactor
	class Reactor {
	public:
		Reactor() {}
		~Reactor() {}
		int RegisterHandler(EventHandler * handler, event_t event);
		int RemoveHandler(EventHandler * handler);
		void HandleEvents(int timeout = 0);
	private:
		// private avoid copy
		Reactor(const Reactor &);
		Reactor & operator=(const Reactor &);

	private:
		ReactorImplementation * m_reactor_impl;
	};
} // namespace reactor
