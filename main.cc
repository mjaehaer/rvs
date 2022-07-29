#include <drogon/drogon.h>
#include <cstdlib>
using namespace drogon;

typedef std::function<void(const HttpResponsePtr &)> Callback;

void save(const HttpRequestPtr &request, Callback &&callback, const std::string &key) {
    std::cout << "start save" << std::endl;
    auto jsonInput = request->getJsonObject();
    if (!jsonInput)
    {
        auto resp = HttpResponse::newHttpResponse();
	resp->setBody("json not found");
	resp->setStatusCode(k400BadRequest);
	callback(resp);
	return;
    }
  //  std::string valueJson = (*jsonInput).asString();
    // Формируем JSON-объект
    Json::Value jsonBody;
    //jsonBody["message"] = "Hello,fff world";

    auto response = HttpResponse::newHttpJsonResponse(jsonBody);
    nosql::RedisClientPtr redisClient = app().getRedisClient();
    std::cout << "write in redis" << std::endl;
    redisClient->execCommandAsync(
        [callback](const drogon::nosql::RedisResult &r) {
	    auto resp = HttpResponse::newHttpResponse();
	    resp->setStatusCode(k201Created);
	    callback(resp);
	},
        [](const std::exception &err) {
            LOG_ERROR << "Redis error: " << err.what();
        },
        "set %s %b",
	key.c_str(),
	jsonInput
);
    std::cout << key << std::endl;
    callback(response);
}

int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("127.0.0.1",80)
//		 .setLogPath("./")
//		 .setLogLevel(trantor::Logger::kWarn)
		 .registerHandler("/save?KEY={KEY}", &save, {drogon::Post})
//    drogon::app().registerHandler("/save?KEY={KEY}", &save, {drogon::Post});
//    drogon::app().registerHandler("/del?KEY={KEY}"
    		 .createRedisClient("127.0.0.1", 6379)
    		 .run();
	
    return 0;
}
