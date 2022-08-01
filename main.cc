#include <drogon/drogon.h>
#include <cstdlib>
using namespace drogon;

typedef std::function<void(const HttpResponsePtr &)> Callback;

void del(const HttpRequestPtr &request, Callback &&callback, std::string &&key) {
    nosql::RedisClientPtr redisClient = app().getRedisClient();
    std::string command = "del " + key;
    redisClient->execCommandAsync(
	[callback](const drogon::nosql::RedisResult &r) {
		auto resp = HttpResponse::newHttpResponse();
		resp->setBody("Deleted");
		callback(resp);
	},
	[callback](const std::exception &err) {
                LOG_ERROR << "redis not work" << err.what();
        	auto resp = HttpResponse::newHttpResponse();
                resp->setBody("Error");
                resp->setStatusCode(k400BadRequest);
                callback(resp); 
	},
        command
);
}

void show(const HttpRequestPtr &request, Callback &&callback, std::string &&key) {
    nosql::RedisClientPtr redisClient = app().getRedisClient();
    std::string command = "get " + key;
    redisClient->execCommandAsync(
	[callback](const drogon::nosql::RedisResult &r) {
		if (r.type() == nosql::RedisResultType::kNil)
		{
			auto resp = HttpResponse::newHttpResponse();
			resp->setStatusCode(k404NotFound);
			callback(resp);
			return;		
		}
		std::string redisResponse = r.asString();
		Json::Value response;
		Json::CharReaderBuilder builder;
		Json::CharReader *reader = builder.newCharReader();
		Json::Value json;
		std::string errors;
		bool parsingSuccessful = 
			reader->parse(redisResponse.c_str(),
				redisResponse.c_str() + redisResponse.size(),
				&json, &errors);
		response["response"] = redisResponse;
		if (parsingSuccessful)
		{
			response["response"] = json;
		}
		auto resp = HttpResponse::newHttpJsonResponse(response);
		callback(resp);
		
	},	
	[](const std::exception &err) {
		LOG_ERROR << "Redis not work" << err.what();
	},
	command
);
}

void save(const HttpRequestPtr &request, Callback &&callback, std::string &&key) {
    nosql::RedisClientPtr redisClient = app().getRedisClient();
    auto jsonInput = request->getJsonObject();
    if (!jsonInput)
    {
        auto resp = HttpResponse::newHttpResponse();
	resp->setBody("json not found");
	resp->setStatusCode(k400BadRequest);
	callback(resp);
	return;
    }
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    const std::string output = Json::writeString(builder, *jsonInput);
    std::string command ="set " +  key + " " + "'"  + output + "'";
    redisClient->execCommandAsync(
        [callback](const drogon::nosql::RedisResult &r) {
	    auto resp = HttpResponse::newHttpResponse();
	    resp->setBody("Created");
	    resp->setStatusCode(k201Created);
	    callback(resp);
	},
        [callback](const std::exception &err) {
		LOG_ERROR << "ERROR REDIS" << err.what();
		auto resp = HttpResponse::newHttpResponse();
                resp->setBody("Error");
                resp->setStatusCode(k400BadRequest);
                callback(resp);	
},
        command
	
);
}

int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("127.0.0.1",80)
//		 .setLogPath("./")
//		 .setLogLevel(trantor::Logger::kWarn)
		 .registerHandler("/save?KEY={key}", &save, {drogon::Post})
		 .registerHandler("/show?KEY={key}", &show, {drogon::Get})
 		 .registerHandler("/del?KEY={key}", &del, {drogon::Delete})
    		 .createRedisClient("127.0.0.1", 6379)
    		 .run();
	
    return 0;
}
