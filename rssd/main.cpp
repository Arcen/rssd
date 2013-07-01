#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <string>
#include "redis.h"

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Application.h>
#include <Poco/SAX/AttributesImpl.h>
#include <Poco/XML/XMLWriter.h>
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::XML;

class RootHandler: public Poco::Net::HTTPRequestHandler
{ 
public: 
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) 
	{ 
		response.setContentType("text/xml");
		std::ostream& ostr = response.send();
		redis r;
		r.arg("zrange");
		r.arg("history");
		r.arg("0");
		r.arg("-1");
		std::list<std::string> result;
		r.exec_array(result);

		std::string rss;
		rss =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<rss version=\"2.0\">"
"  <channel>"
"    <title>rss</title>"
"    <link>http://localhost/</link>"
"    <description>merge rss feeds</description>";

		for (std::list<std::string>::reverse_iterator it = result.rbegin(), end = result.rend(); it != end; ++it)
		{
			r.arg("hget");
			r.arg("items");
			r.arg(*it);
			rss += r.exec_string().second;
		}
		rss += "</channel></rss>";
		ostr << rss << std::flush;
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

int main(int argc, char *argv[])
{
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


