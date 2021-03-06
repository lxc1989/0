﻿#pragma once
#include "../logger.hpp"
#include "../request_handler.hpp"
#include "../billing_data.hpp"
#include <mysql.h>

class LoginHandler :public RequestHandler
{
public:
	LoginHandler(AccountModel& m, bool autoRegOpen) :RequestHandler(m), _autoRegOpen(autoRegOpen) {
		this->payloadType = 0xa2;
#ifdef OPEN_SERVER_DEBUG
		Logger::write(std::string("LoginHandler construct ,autoRegOpen: ") + (autoRegOpen ? "true" : "false"));
#endif //OPEN_SERVER_DEBUG
	}
	~LoginHandler();
	void processRequest(BillingData& requestData, BillingData& responseData);
private:
	//是否开启自动注册
	bool _autoRegOpen;
};

LoginHandler::~LoginHandler()
{
#ifdef OPEN_SERVER_DEBUG
	Logger::write("LoginHandler destrcut");
#endif //OPEN_SERVER_DEBUG
}
void LoginHandler::processRequest(BillingData& requestData, BillingData& responseData) {
	responseData.setPayloadType(requestData.getPayloadType());
	responseData.setId(requestData.getId());
	auto payloadData = requestData.getPayloadData();
	size_t offset = 0, i;
	//获取登录用户名
	unsigned char usernameLength = (unsigned char)payloadData[offset];
	std::string username;
	username.resize(usernameLength);
	username.clear();
	for (i = 0; i < usernameLength; i++) {
		offset++;
		username.append(1, payloadData[offset]);
	}
	//获取登录密码
	offset++;
	unsigned char passLength = (unsigned char)payloadData[offset];
	std::string password;
	password.resize(passLength);
	password.clear();
	for (i = 0; i < passLength; i++) {
		offset++;
		password.append(1, payloadData[offset]);
	}
	//登录IP
	offset++;
	unsigned char ipLength = (unsigned char)payloadData[offset];
	std::string loginIp;
	loginIp.resize(ipLength);
	loginIp.clear();
	for (i = 0; i < ipLength; i++) {
		offset++;
		loginIp.append(1, payloadData[offset]);
	}
	unsigned char loginResult = this->accountModel.getLoginResult(username, password);
	//如果未开启自动注册,登录一个不存在的账号时,返回密码错误
	if ((!this->_autoRegOpen) && (loginResult==9)) {
		loginResult = 3;
	}
	const char* loginResultStr[] = {
		"-",//0 无效值
		"login success",//1 登录成功
		"account not exists",//2 用户不存在
		"password not correct" ,//3 密码错误
		"account is allready online",//4 角色在线
		"account is allready online on other server",//5 有角色在线已在其它服务器登录
		"connect failed",//6 连接失败,请稍候再试
		"account baned",//7 账号被停权
		"point is not available",//8 点数不够?(收费游戏使用?)
		"regisiter"//9 账号注册弹窗
	};
	Logger::write(std::string("user [") + username + "] try to login from " + loginIp + " : " + loginResultStr[loginResult]);
	//
	responseData.appendChar(usernameLength);
	responseData.appendText(username);
	responseData.appendChar(loginResult);
}