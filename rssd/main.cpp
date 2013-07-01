#include <stdio.h>
#include <memory>
#include <string>
#if 0
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

class RootHandler: public Poco::Net::HTTPRequestHandler
{ 
public: 
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) 
	{ 
		Application& app = Application::instance();
		app.logger().information("Request from " + request.clientAddress().toString());
		response.setChunkedTransferEncoding(true); 
		response.setContentType("text/xml");
		std::ostream& ostr = response.send();
		ostr << "<html><head><title>HTTP Server powered by POCO C++ Libraries</title></head>"; 
		ostr << "<body>"; 
		ostr << "</body></html>"; 
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
		if (request.getURI() == "/") 
			return new RootHandler(); 
		else 
			return new DataHandler(); 
	}
};

#endif
bool update(const std::string & host, const std::string & path);

int main(int argc, char *argv[])
{
	/*
	Poco::UInt16 port = 9999;
	std::shared_ptr<HTTPServerParams> pParams(new HTTPServerParams); 
	pParams->setMaxQueued(100); 
	pParams->setMaxThreads(16); 
	ServerSocket svs(port); // set-up a server socket 
	HTTPServer srv(new MyRequestHandlerFactory(), svs, pParams); 
	// start the HTTPServer 
	srv.start(); 
	waitForTerminationRequest(); 
	// Stop the HTTPServer 
	srv.stop();
	*/

	update("news.nicovideo.jp", "/topiclist?rss=2.0");

	return 0;
}


