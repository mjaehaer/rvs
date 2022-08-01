#include <drogon/drogon.h>
#include <cstdlib>
using namespace drogon;

typedef std::function<void(const HttpResponsePtr &)> Callback;

void show(const HttpRequestPtr &request, Callback &&callback, std::string &&key) {
    nosql::RedisClientPtr redisClient = app().getRedisClient();
    std::string command = "get " + key;
    redisClient->execCommandAsync(
	[callback](const drogon::nosql::RedisResult &r) {
		std::cout << "OKE" << std::endl;
		std::string redisResponse = r.asString();
		std::cout << &redisResponse << std::endl;
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
		LOG_ERROR << "redis not work" << err.what();
	},
	command
);
    std::cout << key << std::endl;
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
//    drogon::app().registerHandler("/del?KEY={KEY}"
    		 .createRedisClient("127.0.0.1", 6379)
    		 .run();
	
    return 0;
}
