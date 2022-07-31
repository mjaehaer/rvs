#include <drogon/drogon.h>
#include <cstdlib>
using namespace drogon;

typedef std::function<void(const HttpResponsePtr &)> Callback;

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
        [](const std::exception &err) {
		LOG_ERROR << "ERROR REDIS" << err.what();
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
//    drogon::app().registerHandler("/save?KEY={KEY}", &save, {drogon::Post});
//    drogon::app().registerHandler("/del?KEY={KEY}"
    		 .createRedisClient("127.0.0.1", 6379)
    		 .run();
	
    return 0;
}
