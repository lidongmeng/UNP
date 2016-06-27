/*************************************************************************
	> File Name: reactor.cpp
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Sat 19 Sep 2015 09:35:26 PM PDT
 ************************************************************************/

#include "reactor.h"
#include <assert.h>
#include "event_demultilexer.h"
using namespace std;

namespace reactor {
	class ReactorImplementation {
	public:
		ReactorImplementation();
		~ReactorImplementation();
		int RegisterHandler(EventHandler * handler, event_t evt);
		int RemoveHandler(EventHandler * handler);
		void HandleEvents(int timeout);
	private:
		EventDemultiplexer m_demultiplexer;
		map<handle_t, EventHandler*> m_handlers;
	};

	Reactor::Reactor() {
		m_reactor_impl = new ReactorImplementation();
	}

	int Reactor::RegisterHandler(EventHandler * handler, event_t evt) {
		return m_reactorImpl->RegisterHandler(handler, evt);
	}

	int Reactor::RemoveHandler(EventHandler * handler) {
		return m_reactorImpl->removerHandler(handler);
	}

	void Reactor::HandleEvents(int timeout) {
		m_reactorImpl->HandlerEvents(timeout);
	}

	ReactorImplementation::ReactorImplementation() {
		m_demultiplexer = new EpollDemultiplexer();
	}

	ReactorImplementation::~ReactorImplementation() {
		delete m_demultiplexer;
	}

	int ReactorImplementation::RegisterHandler(EventHandler * handler, event_t evt) {
		handle_t handle = handler->GetHandle();
		map<handle_t, EventHanlder*>::iterator iter = m_handlers.find(handler);
		if (iter == m_handlers.end()) {
			m_handlers[handler] = handler;
		}
		return m_demultiplexer->RequestEvent(handle, evt);
	}

	int ReactorImplementation::RemoveHandler(EventHandler * handler) {
		handle_t handle = handler->GetHandle();
		m_handlers.erase(handle);
		return m_demultiplexer->UnrequestEvent(handle);
	}

	void ReactorImplementation::HandleEvent(int timeout) {
		m_demultiplexer->WaitEvents(&m_handlers);
	}
}

