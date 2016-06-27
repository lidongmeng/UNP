/*************************************************************************
	> File Name: reactor_server.cpp
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Sat 19 Sep 2015 09:53:48 PM PDT
 ************************************************************************/

#include "reactor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

reactor::Reactor g_reactor;

const size_t kBufferSize = 1024;
char g_read_buffer[kBufferSize];
char g_write_buffer[kBufferSize];

class RequestHandler : public reactor::EventHandler {
	public:
		RequestHandler(reactor::handle_t handle) :
			EventHandler(),
			m_handle(handle){}

		virtual reactor::handle_t GetHandle() const {
			return m_handle;
		}
		
		virtual void HandleRead() {
			memset(g_read_buffer, 0, sizeof(g_read_buffer));
			int len = recv(m_handle, g_read_buffer, kBufferSize, 0);
			if (len > 0) {
				if (strncasecmp("time", g_read_buffer, 4) == 0) {
					g_reactor.RegisterHandler(this, reactor::kWriteEvent);
				} else if (strncasecmp("exit", g_read_buffer, 4) == 0) {
					close(m_handle);
					g_reactor.RemoveHandler(this);
					delete this;
				} else {
					fprintf(stderr, "Invalid request: %s", g_read_buffer);
					close(m_handle);
					g_reactor.RemoveHandler(this);
					delete this;
				}
			} else {
				printf("recv error\n");
			}
		}

		virtual void HandleWrite() {
			memset(g_write_buffer, 0, sizeof(g_write_buffer));
			int len = sprinf(g_write_buffer, "current time : %d\r\n", (int)time(NULL));
			len = send(m_handle, g_write_buffer, 0);
			if (len > 0) {
				fprintf(stderr, "send response to clinet fd=%d\n", (int)m_handle);
				g_reactor.RegisterHanlder(this, reactor::kReadEvent);
			} else {
				printf("send error\n");
			}
		}

		virtual void HandleError() {
			fprintf(stderr, "client %d closed\n", m_handle);
			close(m_handle);
			g_reactor.RemoveHandler(this);
			delete this;
		}
	private:
		reactor::handle_t m_handle;
};


class TimeServer::public reactor::EventHandler {
	private:
		string  m_ip;
		unsigned short m_port;
		reactor::handle_t m_handle;
	public:
		TimeSever(const char *ip, unsigned short port) :
			EventHandler(), m_ip(ip), m_port(port){
		}

		virtual reactor::handle_t Gethandle() const {
			return m_handle;
		}

		virtual void HandleRead() {
			struct sockaddr addr;
			socklen_t addlen = sizeof(sockaddr);
			reactor::handle_t handle = accept(m_handle, &addr, &addrlen);

			RequestHandler * handler = new RequestHandler(handle);
			if (g_reactor.RegisterHandler(handler, reactor::kReadEvent) != 0) {
				fprintf(stderr, "error: register handler failed\n");
				delete handler;
			}
		}

		bool Start() {
			m_handle = socket(AF_INET, SOCK_STREAM, 0);

			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(m_port);
			add.sin_addr.s_addr = inet_addr(m_ip.c_str());

			if (bind(m_handle, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
				printf("Error bind");
				return false;
			}

			if (listen(m_handle, 1024) < 0) {
				printf("listen error");
				return false;
			}

			return true;
		}
};


int main(int argc, char ** argv) {
	if (argc < 3) {
		fprintf(stderr, "usage : %s ip port\n", argv[0]);
		return 0;
	}

	TimeServer server(argv[1], atoi(argv[2]));

	if (!server.Start()) {
		fprintf(stderr, "start server error\n");
		return 0;
	}

	while (1) {
		g_reactor.RegisterHandler(&server, reactor::kReadEvent);
		g_reactor.HandleEvent(100);
	}

	return 0;
}
