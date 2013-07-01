#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

#include <Poco/SAX/SAXParser.h>
#include <Poco/SAX/ContentHandler.h>
#include <Poco/SAX/LexicalHandler.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>

using namespace Poco;
using namespace Poco::XML;
using namespace Poco::Net;

class MyHandler: public ContentHandler, public LexicalHandler
{
public:
	std::string text;
	virtual ~MyHandler()
	{
	}
	virtual void setDocumentLocator(const Locator* loc){}
	virtual void startDocument(){}
	virtual void endDocument(){}
	virtual void startElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const Attributes& attrList)
	{
		printf("name %s\n", qname.c_str());
	}
	virtual void endElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
	{
		printf("end %s\n%s\n", qname.c_str(), text.c_str());
		text.clear();	
	}
	virtual void characters(const XMLChar ch[], int start, int length)
	{
		text += std::string(ch, start, length);
	}
	virtual void ignorableWhitespace(const XMLChar ch[], int start, int length){}
	virtual void processingInstruction(const XMLString& target, const XMLString& data){}
	virtual void startPrefixMapping(const XMLString& prefix, const XMLString& uri){}
	virtual void endPrefixMapping(const XMLString& prefix){}
	virtual void skippedEntity(const XMLString& name){}
	virtual void startDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId){}
	virtual void endDTD(){}
	virtual void startEntity(const XMLString& name){}
	virtual void endEntity(const XMLString& name){}
	virtual void startCDATA(){}
	virtual void endCDATA(){}
	virtual void comment(const XMLChar ch[], int start, int length){}
};

bool update(const std::string & host, const std::string & path)
{
	printf("str %s %s\n", host.c_str(), path.c_str());
	Poco::Net::HTTPClientSession s(host); 
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path); 
	s.sendRequest(request);
	Poco::Net::HTTPResponse response;
	std::istream & rs = s.receiveResponse(response);
	std::ostringstream oss;
	StreamCopier::copyStream(rs, oss);
	std::string str = oss.str();
	printf("str %s\n", str.c_str());
	MyHandler handler;
	SAXParser parser;
	parser.setFeature(XMLReader::FEATURE_NAMESPACES, true);
	parser.setFeature(XMLReader::FEATURE_NAMESPACE_PREFIXES, true);
	parser.setContentHandler(&handler);
	parser.setProperty(XMLReader::PROPERTY_LEXICAL_HANDLER, static_cast<LexicalHandler*>(&handler));
	try
	{
		parser.parseString(str);
	}
	catch (Poco::Exception& e) 
	{
		return false;
	}
	return true;
}
