#include "Client.hpp"

Client::Client(int fd) : fd(fd), _connected_at(time(NULL)) {}

Client::~Client() {
	if (fd >= 0) close(fd);
}

/*  hardcode*/
#include <map>
#include <string>
#include <vector>

struct HttpRequest {
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::vector<u_int8_t> body;
};

struct HttpResponse {
	int status_code;
	std::string status_msg;
	std::map<std::string, std::string> headers;
	std::vector<u_int8_t> body;

	HttpResponse() : status_code(200), status_msg("OK") {}
};

enum ParserState { MALFORMED = -2, INCOMPLETE = -1 };
HttpRequest makeFakeReq() {
	HttpRequest req;
	req.method = "GET";
	req.path = "/index.html";
	req.version = "HTTP/1.1";
	req.headers["Host"] = "localhost:8080";
	req.headers["Connection"] = "keep-alive";
	req.headers["Content-Length"] = "0";
	return req;
}
class HttpParser {
	ParserState _state;
	size_t _offset;

   public:
	int tryParse(std::vector<u_int8_t> &rbuf, HttpRequest &req) {
		req = makeFakeReq();
		return INCOMPLETE;
	}
};
// TODO: make it inside client.hpp
void queueResponse(const HttpResponse &res, std::vector<u_int8_t> &_wrbuf) {
	std::ostringstream head;

	head << "HTTP/1.1 " << res.status_code << " " << res.status_msg << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it =
			 res.headers.begin();
		 it != res.headers.end(); ++it)
		head << it->first << ": " << it->second << "\r\n";
	head << "Content-Length: " << res.body.size() << "\r\n";
	head << "\r\n";

	std::string headStr = head.str();
	_wrbuf.insert(_wrbuf.end(), headStr.begin(), headStr.end());
	_wrbuf.insert(_wrbuf.end(), res.body.begin(), res.body.end());
}

HttpResponse makeFakeRes() {
	HttpResponse res;
	res.status_code = 200;
	res.status_msg = "OK";
	res.headers["Content-Type"] = "text/plain";

	std::string body = "Hello from webserve\n";
	res.body.insert(res.body.end(), body.begin(), body.end());
	return res;
}
/*  hardcode*/

ClientStatus Client::onReadable() {
	u_int8_t buff[4096];
	char response[200];
	strncpy(response, "Response\n", 10);
	int n;
	int parseState;
	HttpRequest req;
	HttpParser parser;

	n = read(fd, buff, sizeof(buff));
	if (n == 0 || n == ERROR) return DISCONNECT;
	_rbuf.insert(_rbuf.end(), buff, buff + n);

	parseState = parser.tryParse(_rbuf, req);
	if (parseState == INCOMPLETE) return OK;
	queueResponse(makeFakeRes(), _wrbuf);

	_rbuf.erase(_rbuf.begin(), _rbuf.begin() + parseState);
	return WANT_WRITE;
}

ClientStatus Client::onWritable() {
	if (hasDataToWrite()) {
		int n = write(fd, _wrbuf.data(), _wrbuf.size());
		if (n <= 0) return DISCONNECT;
		_wrbuf.erase(_wrbuf.begin(), _wrbuf.begin() + n);
		if (hasDataToWrite()) return OK;
		return DONE_WRITE;
	}
	return OK;
}

bool Client::hasDataToWrite() const { return !_wrbuf.empty(); }
