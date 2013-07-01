#ifndef INCLUDE_WRAPPER_REDIS_H
#define INCLUDE_WRAPPER_REDIS_H

#include <string>
#include <list>
#include <vector>
#include <memory>
#include <stdexcept>
#include <hiredis/hiredis.h>
class redis
{
	redisContext * ctx;
	std::vector<const char *> argv_ptr;
	std::vector<size_t> argv_len;
	std::list<std::string> argv;
public:
	redis()
		: ctx(redisConnect("127.0.0.1", 6379))
	{
		if (!ctx) {
			throw std::runtime_error("redis null");
		}
		if (ctx->err) {
			throw std::runtime_error("error redis");
		}
	}
	~redis()
	{
		redisFree(ctx);
	}
	void arg(const std::string & val)
	{
		argv.push_back(val);
		argv_len.push_back(val.size());
		argv_ptr.push_back(argv.back().c_str());
	}
	std::pair<bool, long long> exec_integer()
	{
		redisReply * reply = (redisReply*)redisCommandArgv(ctx, argv.size(), &argv_ptr[0], &argv_len[0]);
		argv.clear();
		argv_len.clear();
		argv_ptr.clear();
		std::pair<bool, long long> result;
		switch (reply->type)
		{
		case REDIS_REPLY_STATUS:
			{
				std::string status(reply->str, reply->str + reply->len);
				freeReplyObject(reply);
				return std::make_pair(true, 0);
			}
			break;
		case REDIS_REPLY_ERROR:
			{
				std::string status(reply->str, reply->str + reply->len);
				printf("error %s\n", status.c_str());
				freeReplyObject(reply);
				return std::make_pair(false, 0);
			}
			break;
		case REDIS_REPLY_INTEGER:
			{
				result = std::make_pair(true, reply->integer);
				freeReplyObject(reply);
				return result;
			}
			break;
		default:
			freeReplyObject(reply);
			return std::make_pair(false, 0);
		}
	}
	std::pair<bool, std::string> exec_string()
	{
		redisReply * reply = (redisReply*)redisCommandArgv(ctx, argv.size(), &argv_ptr[0], &argv_len[0]);
		argv.clear();
		argv_len.clear();
		argv_ptr.clear();
		std::pair<bool, std::string> result;
		switch (reply->type)
		{
		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_STRING:
			{
				std::string status(reply->str, reply->str + reply->len);
				freeReplyObject(reply);
				return std::make_pair(true, status);
			}
			break;
		case REDIS_REPLY_ERROR:
			{
				std::string status(reply->str, reply->str + reply->len);
				printf("error %s\n", status.c_str());
				freeReplyObject(reply);
				return std::make_pair(false, status);
			}
			break;
		default:
			freeReplyObject(reply);
			return std::make_pair(false, std::string());
		}
	}
	bool exec_array(std::list<std::string> & result)
	{
		redisReply * reply = (redisReply*)redisCommandArgv(ctx, argv.size(), &argv_ptr[0], &argv_len[0]);
		argv.clear();
		argv_len.clear();
		argv_ptr.clear();
		switch (reply->type)
		{
		case REDIS_REPLY_ARRAY:
			{
				for (size_t i = 0; i < reply->elements; ++i)
				{
					switch (reply->element[i]->type)
					{
					case REDIS_REPLY_STRING:
						std::string status(reply->element[i]->str, reply->element[i]->str + reply->element[i]->len);
						result.push_back(status);
						break;
					}
				}
				freeReplyObject(reply);
				return true;
			}
			break;
		default:
			freeReplyObject(reply);
			return false;
		}
	}
};

#endif
