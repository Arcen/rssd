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
		r.arg("articles");
		r.arg("-5");
		r.arg("-1");
		std::list<std::string> result;
		r.exec_array(result);
		std::string rss;
		rss =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">"
"  <channel>"
"    <atom:link rel=\"self\" type=\"application/rss+xml\" href=\"http://news.nicovideo.jp/topiclist?rss=2.0\"/>"
"    <title>トピックス一覧-ニコニコニュース</title>"
"    <link>http://news.nicovideo.jp/topiclist</link>"
"    <description>ニコニコニュースで取り上げている最新ニュースを提供しています。</description>"
"    <pubDate>Mon, 01 Jul 2013 14:39:44 +0900</pubDate>"
"    <lastBuildDate>Mon, 01 Jul 2013 14:39:44 +0900</lastBuildDate>"
"    <generator>ニコニコニュース</generator>"
"    <language>ja</language>"
"    <copyright>(c) niwango, inc. All rights reserved.</copyright>"
"    <docs>http://blogs.law.harvard.edu/tech/rss</docs>";

		for (std::list<std::string>::reverse_iterator it = result.rbegin(), end = result.rend(); it != end; ++it)
		{
			rss += *it;
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


