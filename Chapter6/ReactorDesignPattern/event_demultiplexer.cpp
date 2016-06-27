/*************************************************************************
	> File Name: event_demultiplexer.cpp
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Sat 19 Sep 2015 08:55:41 PM PDT
 ************************************************************************/

#include "event_demultiplexer.h"
#include <errno.h>
#include <assert.h>
#include <vector>
using namespace std;

namespace reactor {
SelectDemultiplexer::SelectDemultiplexer() {
	this->clear();
}

int SelectDemultiplexer::WaitEvents(map<handle_t, EventHandler *> * handlers, int timeout) {
	m_timeout.tv_sec = timeout/1000;
	m_timeout.tv_usec = timeout%1000*1000;
	int max_fd = handlers->rbegin()->first;
	int ret = select(maxfd+1, &m_read_set, &m_write_set, &m_except_set, &m_timeout);
	if (ret <= 0) {
		return ret;
	}
	map<handle_t*, EventHandler *>::iterator iter = handlers->begin();
	while (iter != handlers->end()) {
		if (FD_ISSET(iter->first, &m_except_set)) {
			iter->second->HandleError();
			FD_CLR(iter->first, &m_read_set);
			FD_CLR(iter->first, &m_write_set);
		} else {
			if (FD_ISSET(iter->first, &m_read_set)) {
				iter->second->HandlerRead();
				FD_CLR(iter->first, &m_read_set);
			}
			if (FD_ISSET(iter->first, &m_write_set)) {
				iter->second->HandlerWrite();
				FD_CLR(iter->first, &m_write_set);
			}
		}
		FD_CLR(iter->first, &m_except_set);
		++iter;
	}
	return ret;
}

int SelectDemultiplexer::RequestEvent(handle_t handle, event_t evt) {
	if (evt & kReadEvent) {
		FD_SET(handle, &m_read_set);
	}
	if (evt & kWriteEvent) {
		FD_SET(handle, &m_write_set);
	}
	FD_SET(handle, &m_except_set);
	return 0;
}

int SelectDemultiplexer::UnrequestEvent(handle_t handle) {
	FD_CLR(handle, &m_read_set);
	FD_CLR(handle, &m_write_set);
	FD_CLR(handle, &m_exception_set);
}

void SelectDemultiplexer::Clear() {
	FD_ZERO(&m_read_set);
	FD_ZERO(&m_write_set);
	FD_ZERO(&m_except_set);
}

EpollDemultiplexer::EpollDemultiplexer() {
	m_epoll_fd = epoll_create(FD_SETSIZE);
	assert(m_epoll_fd != -1);
	m_fd_num = 0;
}

EpollDemultiplexer::~EpollDemultiplxer() {
	close(m_epoll_fd);
}

int EpollDemultiplexer::WaitEvents(map<handler_t, EventHandler *> * handlers, int timeout) {
	vector<epoll_event> ep_evts(m_fd_num);
	int num = epoll_wait(m_epoll_fd, &ep_evts[0], ep_evts.size(), timeout);
	if (num > 0) {
		for (int i = 0; i < num; ++i) {
			handle_t handle = ep_evts[i].data.fd;
			assert(handlers->find(handle) != handlers->end());
			if ((ep_evts[i].events & EPOLLERR) || (ep_evts[i].events & EPOLLLHUP)) {
				(*handlers)[handle]->HandleError();
			} else {
				if (ep_evts[i].events & EPOLLIN) {
					(*handlers)[handle]->HandleRead();
				}
				if (ep_evts[i].events & EPOLLOUT) {
					(*handlers)[handle]->HandleWrite();
				}
			}
		}
	}
	return num;
}

int EpollDemultiplexer::RequestEvent(handle_t handle, event_t evt) {
	epoll_event ep_evt;
	ep_evt.data.fd = handle;
	ep_evt.events = 0;

	if (evt & kReadEvt) {
		ep_evt.events |= EPOLLIN;
	}

	if (evt & kWriteEvt) {
		ep_evt.events |= EPOLLOUT;
	}

	ep_evt.events |= EPOLLONESHOT;
	// first use mod if return enoent stands for no such event then add it
	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, handle, &ep_evt) != 0) {
		if (errno == ENOENT) {
			if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, handle, &ep_evt) != 0) {
				return -errno;
			}
			++m_fd_num;
		}
	}
	return 0;
}

int EpollDemultiplexer::UnrequestEvent(handle_t handle) {
	epoll_event ep_evt;
	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, handle, &ep_evt) != 0) {
		return -errno;
	}
	--m_fd_num;
	return 0;
}
