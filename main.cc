#include <drogon/drogon.h> 
#include <cstdlib> 
#include <ctime>
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
    std::string command = "zrange " + key + " 0 0 ";
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
		std::cout << redisResponse << std::endl;
		Json::Value response;
		Json::CharReaderBuilder builder;
		Json::CharReader *reader = builder.newCharReader();
		Json::Value json;
		std::string errors;
		bool parsingSuccessful = 
			reader->parse(redisResponse.c_str(),
				redisResponse.c_str() + redisResponse.size(),
				&json, &errors);
		response["value"] = redisResponse;
		if (parsingSuccessful)
		{
			response["value"] = json;
		}
	        auto resp = HttpResponse::newHttpJsonResponse(response);
	        callback(resp);
	},      
        [](const std::exception &err) {
                LOG_ERROR << "Redis not work" << err.what();
        },
	command);

//        auto resp = HttpResponse::newHttpJsonResponse(response);

//	callback(resp);
		
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
    std::time_t t = std::time(0);
    std::string command ="zadd " +  key + " 1 " + "'"  + output + "'"
	+ " 2 " + std::to_string(t);
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
		 .registerHandler("/save/{key}", &save, {drogon::Post})
		 .registerHandler("/show/{key}", &show, {drogon::Get})
 		 .registerHandler("/del/{key}", &del, {drogon::Delete})
    		 .createRedisClient("127.0.0.1", 6379)
    		 .run();
	
    return 0;
}
