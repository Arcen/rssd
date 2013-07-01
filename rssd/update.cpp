#include <stdio.h>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>

#include <Poco/SAX/SAXParser.h>
#include <Poco/SAX/ContentHandler.h>
#include <Poco/SAX/LexicalHandler.h>
#include <Poco/SAX/AttributesImpl.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/XML/XMLWriter.h>
#include <time.h>

#include "redis.h"
#include "picojson.h"

using namespace Poco;
using namespace Poco::XML;
using namespace Poco::Net;



class MyHandler: public ContentHandler, public LexicalHandler
{
public:
	std::string text;
	std::list<std::string> path;
	std::map<std::string,std::string> params;
	redis r;
	virtual ~MyHandler()
	{
	}
	virtual void setDocumentLocator(const Locator* loc){}
	virtual void startDocument(){}
	virtual void endDocument(){}
	virtual void startElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const Attributes& attrList)
	{
		path.push_back(localName);
	}
	time_t dateToUnixTime(const std::string & value)
	{
		//Mon, 01 Jul 2013 12:43:00 +0900
		std::string day = value.substr(5, 2);
		std::string month = value.substr(8, 3);
		std::string year = value.substr(12, 4);
		std::string hours = value.substr(17, 2);
		std::string minutes = value.substr(20, 2);
		std::string seconds = value.substr(23, 2);
		std::string zone_hour = value.substr(26, 3);
		std::string zone_minutes = value.substr(29, 2);
		struct tm t = { 0 };
		t.tm_year = atoi(year.c_str());
		static std::map<std::string,int> months;
		if (months.empty())
		{
			months["Jan"] = 1;
			months["Feb"] = 2;
			months["Mar"] = 3;
			months["Apr"] = 4;
			months["May"] = 5;
			months["Jun"] = 6;
			months["Jul"] = 7;
			months["Aug"] = 8;
			months["Sep"] = 9;
			months["Oct"] = 10;
			months["Nov"] = 10;
			months["Dec"] = 12;
		}
		t.tm_mon = months[month]-1;
		t.tm_mday = atoi(day.c_str());
		t.tm_hour = atoi(hours.c_str());
		t.tm_min = atoi(minutes.c_str());
		t.tm_sec = atoi(seconds.c_str());
		time_t tmt = mktime(&t);
		if (zone_hour.substr(0, 1) == "+")
		{
			tmt += atoi(zone_hour.c_str()) * 3600;
			tmt += atoi(zone_minutes.c_str()) * 60;
		} else if (zone_hour.substr(0, 1) == "-") {
			tmt -= atoi(zone_hour.c_str()) * 3600;
			tmt -= atoi(zone_minutes.c_str()) * 60;
		}
		return tmt;
	}
	virtual void endElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
	{
		while (!text.empty())
		{
			if (*text.begin() == ' ' || *text.begin() == '\r' || *text.begin() == '\t' || *text.begin() == '\n')
			{
				text.erase(text.begin(), text.begin() + 1);
			} else {
				break;
			}
		}
		while (!text.empty())
		{
			if (*text.rbegin() == ' ' || *text.rbegin() == '\r' || *text.rbegin() == '\t' || *text.rbegin() == '\n')
			{
				text.resize(text.size() - 1);
			} else {
				break;
			}
		}
		if (localName == "item")
		{
			if (!params["guid"].empty())
			{
				r.arg("ZADD");
				r.arg("articles");
				char buf[1024];
				sprintf(buf, "%d", dateToUnixTime(params["pubDate"]));
				r.arg(buf);//score

				std::ostringstream str;
				//XMLWriter writer(str, XMLWriter::WRITE_XML_DECLARATION | XMLWriter::PRETTY_PRINT);
				XMLWriter writer(str, 0);
				//writer.setNewLine("\n");
				writer.startDocument();
				//<rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">
				{
					//AttributesImpl attrs;
					//attrs.addAttribute("", "", "version", "", "2.0");
					//writer.startElement("", "", "rss", attrs);
				}
				{
					writer.startElement("", "", "item");
					{
						writer.startElement("", "", "title");
						writer.characters(params["title"]);
						writer.endElement("", "", "title");
					}
					{
						writer.startElement("", "", "link");
						writer.characters(params["link"]);
						writer.endElement("", "", "link");
					}
					{
						writer.startElement("", "", "description");
						writer.characters(params["description"]);
						writer.endElement("", "", "description");
					}
					writer.endElement("", "", "item");
				}
				writer.endDocument();
				r.arg(str.str());
				/*
				picojson::object o;
				o["title"] = (picojson::value)params["title"];
				o["link"] = (picojson::value)params["link"];
				o["description"] = (picojson::value)params["description"];
				picojson::value v(o);
				r.arg(v.serialize());*/
				r.exec_string();
			}
			params.clear();
		} else if (localName == "title" || localName == "link" || localName == "pubDate" || localName == "guid") {
			params[localName] = text;
		}
		path.pop_back();
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
	Poco::Net::HTTPClientSession s(host); 
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path); 
	s.sendRequest(request);
	Poco::Net::HTTPResponse response;
	std::istream & rs = s.receiveResponse(response);
	std::ostringstream oss;
	StreamCopier::copyStream(rs, oss);
	std::string str = oss.str();
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
