#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <string>

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Application.h>
using namespace Poco::Net;
using namespace Poco::Util;

class RootHandler: public Poco::Net::HTTPRequestHandler
{ 
public: 
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) 
	{ 
		response.setContentType("text/xml");
		std::ostream& ostr = response.send();


		ostr << "<a>abc</a>";
		ostr << std::flush;
	} 
};

class MyRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory 
{
public: 
	MyRequestHandlerFactory() 
	{
	}
	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		return new RootHandler(); 
	}
};


bool update(const std::string & host, const std::string & path);

int main(int argc, char *argv[])
{

	update("news.nicovideo.jp", "/topiclist?rss=2.0");

	Poco::UInt16 port = 9999;
	HTTPServerParams::Ptr pParams(new HTTPServerParams); 
	pParams->setMaxQueued(100); 
	pParams->setMaxThreads(16); 
	ServerSocket svs(port); // set-up a server socket 
	HTTPRequestHandlerFactory::Ptr factory(new MyRequestHandlerFactory());
	HTTPServer srv(factory, svs, pParams); 
	// start the HTTPServer 
	srv.start(); 
	while (true)
	{
		sleep(1);
	}
	// Stop the HTTPServer 
	srv.stop();

	return 0;
}


