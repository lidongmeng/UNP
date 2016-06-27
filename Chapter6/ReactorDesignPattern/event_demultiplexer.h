/*************************************************************************
	> File Name: event_demultiplexer.h
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Sat 19 Sep 2015 08:22:43 PM PDT
 ************************************************************************/

#include <set>
#include <map>
#include "reactor.h"
using namespace std;

namespace reactor {
	class EventDemultiplexer {
	public:
		virtual ~EventDemultiplexer() {}
		// wait for the events
		virtual int WaitEvents(map<handle_t, EventHandler*> * handlers, int timeout = 0) = 0;
		// add event to the demultiplexer
		virtual int RequestEvent(handle_t, event_t event) = 0;
		// delete event to the demultiplexer
		virtual int UnrequestEvent(handle_t handle) = 0;
	};

	class SelectDemultiplexer : public EventDemultiplexer {
	private:
		Clear();
	private:
		fd_set m_read_set;
		fd_set m_write_set;
		fd_set m_except_set;
		timeval m_timeout;
	public:
		SelectDemultiplexer();
		virtual int WaitEvents(map<handle_t, EventHandler *> * handlers, int timeout = 0);
		virtual int RequestEvent(handle_t handle, event_t evt);
		virtual int UnrequestEvent(handle_t handle, evevnt_t evt);
	};

	class EpollDemultiplexer : class EventDemultiplexer {
	private:
		int m_epoll_fd;
		int m_fd_num;
	public:
		EpollDemultiplexer();
		~EpollDemultiplexer();
		virtual int WaitEvent(map<handle_t, EventHandler *> * handlers, int timeout = 0);
		virtual int RequestEvent(handle_t handle, event_t evt);
		virtual int UnrequestEvent(handle_t handle) = 0;
	};
}
