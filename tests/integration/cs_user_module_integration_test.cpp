/**
 * @file cs_user_module_integration_test.cpp
 * @brief cs_user 模块集成测试
 *
 * 测试由 protobuf 生成的 cs_user.mjs 模块的导入和使用
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class CsUserModuleIntegrationTest
 * @brief cs_user 模块集成测试类
 */
class CsUserModuleIntegrationTest : public IntegrationTestHelper {
};

// ==================== 枚举测试 ====================

TEST_F(CsUserModuleIntegrationTest, MessageIdEnumExists) {
    // 测试 MessageId 枚举是否正确导出
    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        typeof MessageId;
    )", Value("object"));
}

TEST_F(CsUserModuleIntegrationTest, MessageIdEnumValues) {
    // 测试 MessageId 枚举值是否正确
    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.INVALID;
    )", Value(0));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.REGISTER_REQUEST;
    )", Value(1001));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.REGISTER_RESPONSE;
    )", Value(1002));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.LOGIN_REQUEST;
    )", Value(1003));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.LOGIN_RESPONSE;
    )", Value(1004));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.ENTER_SERVER_REQUEST;
    )", Value(1005));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.ENTER_SERVER_RESPONSE;
    )", Value(1006));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.GET_SERVERS_REQUEST;
    )", Value(1007));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.GET_SERVERS_RESPONSE;
    )", Value(1008));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.GET_CREATED_PLAYERS_REQUEST;
    )", Value(1009));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        MessageId.GET_CREATED_PLAYERS_RESPONSE;
    )", Value(1010));
}

TEST_F(CsUserModuleIntegrationTest, MessageIdDescriptor) {
    // 测试 MessageId 的描述符
    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        const descriptor = MessageId.__descriptor;
        descriptor.name;
    )", Value("MessageId"));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        const descriptor = MessageId.__descriptor;
        descriptor.package;
    )", Value("pokeworld.user.cs"));

    AssertEq(R"(
        import { MessageId } from './fixtures/modules/cs_user.mjs';
        const descriptor = MessageId.__descriptor;
        descriptor.fullName;
    )", Value("pokeworld.user.cs.MessageId"));
}

// ==================== RegisterRequest 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, RegisterRequestClassExists) {
    // 测试 RegisterRequest 类是否正确导出
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        typeof RegisterRequest;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestInstantiation) {
    // 测试 RegisterRequest 实例化
    AssertTrue(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req instanceof RegisterRequest;
    )");
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestDescriptor) {
    // 测试 RegisterRequest 的描述符
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.name;
    )", Value("RegisterRequest"));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.package;
    )", Value("pokeworld.user.cs"));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.fullName;
    )", Value("pokeworld.user.cs.RegisterRequest"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestFields) {
    // 测试 RegisterRequest 字段
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.fields.length;
    )", Value(3));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.fields[0].name;
    )", Value("email"));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.fields[1].name;
    )", Value("userName"));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        RegisterRequest.__descriptor.fields[2].name;
    )", Value("password"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestWithEmail) {
    // 测试 RegisterRequest 设置 email
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req.withEmail('test@example.com');
        req.email;
    )", Value("test@example.com"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestWithUserName) {
    // 测试 RegisterRequest 设置 userName
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req.withUserName('testuser');
        req.userName;
    )", Value("testuser"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestWithPassword) {
    // 测试 RegisterRequest 设置 password
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req.withPassword('password123');
        req.password;
    )", Value("password123"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterRequestChaining) {
    // 测试 RegisterRequest 链式调用
    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req.withEmail('test@example.com')
           .withUserName('testuser')
           .withPassword('password123');
        req.email;
    )", Value("test@example.com"));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req.withEmail('test@example.com')
           .withUserName('testuser')
           .withPassword('password123');
        req.userName;
    )", Value("testuser"));

    AssertEq(R"(
        import { RegisterRequest } from './fixtures/modules/cs_user.mjs';
        const req = new RegisterRequest();
        req.withEmail('test@example.com')
           .withUserName('testuser')
           .withPassword('password123');
        req.password;
    )", Value("password123"));
}

// ==================== RegisterResponse 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, RegisterResponseClassExists) {
    // 测试 RegisterResponse 类是否正确导出
    AssertEq(R"(
        import { RegisterResponse } from './fixtures/modules/cs_user.mjs';
        typeof RegisterResponse;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, RegisterResponseWithSuccess) {
    // 测试 RegisterResponse 设置 success
    AssertTrue(R"(
        import { RegisterResponse } from './fixtures/modules/cs_user.mjs';
        const res = new RegisterResponse();
        res.withSuccess(true);
        res.success;
    )");
}

// ==================== LoginRequest 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, LoginRequestClassExists) {
    // 测试 LoginRequest 类是否正确导出
    AssertEq(R"(
        import { LoginRequest } from './fixtures/modules/cs_user.mjs';
        typeof LoginRequest;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, LoginRequestFields) {
    // 测试 LoginRequest 字段
    AssertEq(R"(
        import { LoginRequest } from './fixtures/modules/cs_user.mjs';
        LoginRequest.__descriptor.fields.length;
    )", Value(2));

    AssertEq(R"(
        import { LoginRequest } from './fixtures/modules/cs_user.mjs';
        LoginRequest.__descriptor.fields[0].name;
    )", Value("email"));

    AssertEq(R"(
        import { LoginRequest } from './fixtures/modules/cs_user.mjs';
        LoginRequest.__descriptor.fields[1].name;
    )", Value("password"));
}

TEST_F(CsUserModuleIntegrationTest, LoginRequestWithEmailAndPassword) {
    // 测试 LoginRequest 设置 email 和 password
    AssertEq(R"(
        import { LoginRequest } from './fixtures/modules/cs_user.mjs';
        const req = new LoginRequest();
        req.withEmail('user@example.com').withPassword('secret');
        req.email;
    )", Value("user@example.com"));

    AssertEq(R"(
        import { LoginRequest } from './fixtures/modules/cs_user.mjs';
        const req = new LoginRequest();
        req.withEmail('user@example.com').withPassword('secret');
        req.password;
    )", Value("secret"));
}

// ==================== LoginResponse 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, LoginResponseClassExists) {
    // 测试 LoginResponse 类是否正确导出
    AssertEq(R"(
        import { LoginResponse } from './fixtures/modules/cs_user.mjs';
        typeof LoginResponse;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, LoginResponseWithSuccess) {
    // 测试 LoginResponse 设置 success
    AssertFalse(R"(
        import { LoginResponse } from './fixtures/modules/cs_user.mjs';
        const res = new LoginResponse();
        res.withSuccess(false);
        res.success;
    )");
}

// ==================== EnterServerRequest 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, EnterServerRequestClassExists) {
    // 测试 EnterServerRequest 类是否正确导出
    AssertEq(R"(
        import { EnterServerRequest } from './fixtures/modules/cs_user.mjs';
        typeof EnterServerRequest;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, EnterServerRequestWithServerId) {
    // 测试 EnterServerRequest 设置 serverId
    AssertEq(R"(
        import { EnterServerRequest } from './fixtures/modules/cs_user.mjs';
        const req = new EnterServerRequest();
        req.withServerId(100);
        req.serverId;
    )", Value(100));
}

// ==================== EnterServerResponse 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, EnterServerResponseClassExists) {
    // 测试 EnterServerResponse 类是否正确导出
    AssertEq(R"(
        import { EnterServerResponse } from './fixtures/modules/cs_user.mjs';
        typeof EnterServerResponse;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, EnterServerResponseWithSuccess) {
    // 测试 EnterServerResponse 设置 success
    AssertTrue(R"(
        import { EnterServerResponse } from './fixtures/modules/cs_user.mjs';
        const res = new EnterServerResponse();
        res.withSuccess(true);
        res.success;
    )");
}

// ==================== GetServersRequest 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, GetServersRequestClassExists) {
    // 测试 GetServersRequest 类是否正确导出
    AssertEq(R"(
        import { GetServersRequest } from './fixtures/modules/cs_user.mjs';
        typeof GetServersRequest;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, GetServersRequestNoFields) {
    // 测试 GetServersRequest 没有字段
    AssertEq(R"(
        import { GetServersRequest } from './fixtures/modules/cs_user.mjs';
        GetServersRequest.__descriptor.fields.length;
    )", Value(0));
}

// ==================== Server 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, ServerClassExists) {
    // 测试 Server 类是否正确导出
    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        typeof Server;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, ServerFields) {
    // 测试 Server 字段
    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        Server.__descriptor.fields.length;
    )", Value(3));

    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        Server.__descriptor.fields[0].name;
    )", Value("id"));

    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        Server.__descriptor.fields[1].name;
    )", Value("name"));

    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        Server.__descriptor.fields[2].name;
    )", Value("number"));
}

TEST_F(CsUserModuleIntegrationTest, ServerWithFields) {
    // 测试 Server 设置字段
    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        const server = new Server();
        server.withId(1).withName('Server1').withNumber(100);
        server.id;
    )", Value(1));

    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        const server = new Server();
        server.withId(1).withName('Server1').withNumber(100);
        server.name;
    )", Value("Server1"));

    AssertEq(R"(
        import { Server } from './fixtures/modules/cs_user.mjs';
        const server = new Server();
        server.withId(1).withName('Server1').withNumber(100);
        server.number;
    )", Value(100));
}

// ==================== GetServersResponse 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, GetServersResponseClassExists) {
    // 测试 GetServersResponse 类是否正确导出
    AssertEq(R"(
        import { GetServersResponse } from './fixtures/modules/cs_user.mjs';
        typeof GetServersResponse;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, GetServersResponseWithServers) {
    // 测试 GetServersResponse 设置 servers 数组
    AssertTrue(R"(
        import { GetServersResponse, Server } from './fixtures/modules/cs_user.mjs';
        const res = new GetServersResponse(111, 111);
        const server1 = new Server().withId(1).withName('Server1').withNumber(100);
        const server2 = new Server().withId(2).withName('Server2').withNumber(200);
        res.withSuccess(true).withServers([server1, server2]);
        res.success && res.servers.length === 2;
    )");
}

TEST_F(CsUserModuleIntegrationTest, GetServersResponseServerArrayAccess) {
    // 测试 GetServersResponse 访问 servers 数组元素
    AssertEq(R"(
        import { GetServersResponse, Server } from './fixtures/modules/cs_user.mjs';
        const res = new GetServersResponse();
        const server1 = new Server().withId(1).withName('Server1').withNumber(100);
        const server2 = new Server().withId(2).withName('Server2').withNumber(200);
        res.withServers([server1, server2]);
        res.servers[0].name;
    )", Value("Server1"));

    AssertEq(R"(
        import { GetServersResponse, Server } from './fixtures/modules/cs_user.mjs';
        const res = new GetServersResponse();
        const server1 = new Server().withId(1).withName('Server1').withNumber(100);
        const server2 = new Server().withId(2).withName('Server2').withNumber(200);
        res.withServers([server1, server2]);
        res.servers[1].number;
    )", Value(200));
}

// ==================== GetCreatedPlayersRequest 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, GetCreatedPlayersRequestClassExists) {
    // 测试 GetCreatedPlayersRequest 类是否正确导出
    AssertEq(R"(
        import { GetCreatedPlayersRequest } from './fixtures/modules/cs_user.mjs';
        typeof GetCreatedPlayersRequest;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, GetCreatedPlayersRequestNoFields) {
    // 测试 GetCreatedPlayersRequest 没有字段
    AssertEq(R"(
        import { GetCreatedPlayersRequest } from './fixtures/modules/cs_user.mjs';
        GetCreatedPlayersRequest.__descriptor.fields.length;
    )", Value(0));
}

// ==================== GetCreatedPlayersResponse 消息测试 ====================

TEST_F(CsUserModuleIntegrationTest, GetCreatedPlayersResponseClassExists) {
    // 测试 GetCreatedPlayersResponse 类是否正确导出
    AssertEq(R"(
        import { GetCreatedPlayersResponse } from './fixtures/modules/cs_user.mjs';
        typeof GetCreatedPlayersResponse;
    )", Value("function"));
}

TEST_F(CsUserModuleIntegrationTest, GetCreatedPlayersResponseWithEntityIds) {
    // 测试 GetCreatedPlayersResponse 设置 entityIds 数组
    AssertTrue(R"(
        import { GetCreatedPlayersResponse } from './fixtures/modules/cs_user.mjs';
        const res = new GetCreatedPlayersResponse();
        res.withSuccess(true).withEntityIds([1001, 1002, 1003]);
        res.success && res.entityIds.length === 3;
    )");
}

TEST_F(CsUserModuleIntegrationTest, GetCreatedPlayersResponseEntityIdArrayAccess) {
    // 测试 GetCreatedPlayersResponse 访问 entityIds 数组元素
    AssertEq(R"(
        import { GetCreatedPlayersResponse } from './fixtures/modules/cs_user.mjs';
        const res = new GetCreatedPlayersResponse();
        res.withEntityIds([1001, 1002, 1003]);
        res.entityIds[0];
    )", Value(1001));

    AssertEq(R"(
        import { GetCreatedPlayersResponse } from './fixtures/modules/cs_user.mjs';
        const res = new GetCreatedPlayersResponse();
        res.withEntityIds([1001, 1002, 1003]);
        res.entityIds[1];
    )", Value(1002));

    AssertEq(R"(
        import { GetCreatedPlayersResponse } from './fixtures/modules/cs_user.mjs';
        const res = new GetCreatedPlayersResponse();
        res.withEntityIds([1001, 1002, 1003]);
        res.entityIds[2];
    )", Value(1003));
}

// ==================== 综合场景测试 ====================

TEST_F(CsUserModuleIntegrationTest, UserRegistrationScenario) {
    // 测试用户注册场景
    AssertEq(R"(
        import { RegisterRequest, RegisterResponse } from './fixtures/modules/cs_user.mjs';

        // 创建注册请求
        const req = new RegisterRequest();
        req.withEmail('newuser@example.com')
           .withUserName('newuser')
           .withPassword('securepassword');

        // 模拟服务器响应
        const res = new RegisterResponse();
        res.withSuccess(true);

        // 验证请求数据
        req.email === 'newuser@example.com' &&
        req.userName === 'newuser' &&
        req.password === 'securepassword' &&
        res.success === true ? 'registration_success' : 'registration_failed';
    )", Value("registration_success"));
}

TEST_F(CsUserModuleIntegrationTest, UserLoginScenario) {
    // 测试用户登录场景
    AssertEq(R"(
        import { LoginRequest, LoginResponse } from './fixtures/modules/cs_user.mjs';

        // 创建登录请求
        const req = new LoginRequest();
        req.withEmail('existinguser@example.com').withPassword('userpassword');

        // 模拟服务器响应
        const res = new LoginResponse();
        res.withSuccess(true);

        // 验证
        req.email === 'existinguser@example.com' &&
        req.password === 'userpassword' &&
        res.success === true ? 'login_success' : 'login_failed';
    )", Value("login_success"));
}

TEST_F(CsUserModuleIntegrationTest, ServerListScenario) {
    // 测试获取服务器列表场景
    AssertEq(R"(
        import { GetServersRequest, GetServersResponse, Server } from './fixtures/modules/cs_user.mjs';

        // 创建请求
        const req = new GetServersRequest();

        // 模拟服务器响应
        const res = new GetServersResponse();
        const server1 = new Server().withId(1).withName('PvP Server').withNumber(500);
        const server2 = new Server().withId(2).withName('PvE Server').withNumber(800);
        const server3 = new Server().withId(3).withName('Roleplay Server').withNumber(300);
        res.withSuccess(true).withServers([server1, server2, server3]);

        // 统计在线玩家总数
        const totalPlayers = res.servers.reduce((sum, server) => sum + server.number, 0);

        totalPlayers;
    )", Value(1600)); // 500 + 800 + 300
}

TEST_F(CsUserModuleIntegrationTest, GetCreatedPlayersScenario) {
    // 测试获取已创建角色列表场景
    AssertEq(R"(
        import { GetCreatedPlayersRequest, GetCreatedPlayersResponse } from './fixtures/modules/cs_user.mjs';

        // 创建请求
        const req = new GetCreatedPlayersRequest();

        // 模拟服务器响应
        const res = new GetCreatedPlayersResponse();
        res.withSuccess(true).withEntityIds([10001, 10002, 10005, 10008]);

        // 返回角色数量
        res.entityIds.length;
    )", Value(4));
}

TEST_F(CsUserModuleIntegrationTest, CompleteGameFlowScenario) {
    // 测试完整游戏流程场景
    AssertTrue(R"(
        import {
            MessageId,
            RegisterRequest,
            RegisterResponse,
            LoginRequest,
            LoginResponse,
            GetServersRequest,
            GetServersResponse,
            Server,
            EnterServerRequest,
            EnterServerResponse,
            GetCreatedPlayersRequest,
            GetCreatedPlayersResponse
        } from './fixtures/modules/cs_user.mjs';

        // 1. 注册新用户
        const registerReq = new RegisterRequest();
        registerReq.withEmail('player@example.com')
                   .withUserName('player1')
                   .withPassword('gamepass123');

        const registerRes = new RegisterResponse();
        registerRes.withSuccess(true);

        // 2. 登录
        const loginReq = new LoginRequest();
        loginReq.withEmail('player@example.com').withPassword('gamepass123');

        const loginRes = new LoginResponse();
        loginRes.withSuccess(true);

        // 3. 获取服务器列表
        const getServersReq = new GetServersRequest();

        const getServersRes = new GetServersResponse();
        const server = new Server().withId(5).withName('Adventure World').withNumber(1200);
        getServersRes.withSuccess(true).withServers([server]);

        // 4. 选择服务器
        const enterServerReq = new EnterServerRequest();
        enterServerReq.withServerId(5);

        const enterServerRes = new EnterServerResponse();
        enterServerRes.withSuccess(true);

        // 5. 获取已创建角色
        const getPlayersReq = new GetCreatedPlayersRequest();

        const getPlayersRes = new GetCreatedPlayersResponse();
        getPlayersRes.withSuccess(true).withEntityIds([50001, 50002]);

        // 验证所有步骤
        registerRes.success &&
        loginRes.success &&
        getServersRes.success &&
        getServersRes.servers.length === 1 &&
        getServersRes.servers[0].id === 5 &&
        enterServerRes.success &&
        getPlayersRes.success &&
        getPlayersRes.entityIds.length === 2;
    )");
}

} // namespace mjs::test
