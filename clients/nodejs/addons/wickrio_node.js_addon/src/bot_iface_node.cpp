#include <nan.h>
#include <v8.h>
#include "bot_iface.h"

using namespace v8;
using namespace std;
using namespace Nan;


void display(const v8::FunctionCallbackInfo<v8::Value> & args) {
        Isolate* isolate = args.GetIsolate();

        std::string client = std::string("name");
        BotIface botIface(client);
        //v8::Handle<v8::Int> a = v8::Handle<v8::int>::Cast(args[0]);
        //v8::Handle<v8::int> b = v8::Handle<v8::int>::Cast(args[1]);
        v8::String::Utf8Value param1(args[0]->ToString());
        std::string str = std::string(*param1);
        botIface.display(str);

        //Local<Number> retval = v8::Number::New(isolate, result);
        //args.GetReturnValue().Set(retval);
}

void cmdStringGetStatistics(const v8::FunctionCallbackInfo<v8::Value> & args){
        string command, response;
        std::string client = std::string("name");
        BotIface botIface(client);
        botIface.cmdStringGetStatistics(command);

        if (botIface.send(command, response) != BotIface::SUCCESS) {
                response = botIface.getLastErrorString();
                std::cout << "Send failed: " << response;
        }
        else {
                if (response.length() > 0) {
                        cout << response << endl;
                }
        }
        return;
}

void cmdStringClearStatistics(const v8::FunctionCallbackInfo<v8::Value> & args){
        string command, response;
        std::string client = std::string("name");
        BotIface botIface(client);
        botIface.cmdStringClearStatistics(command);

        if (botIface.send(command, response) != BotIface::SUCCESS) {
                response = botIface.getLastErrorString();
                std::cout << "Send failed: " << response;
        }
        else {
                if (response.length() > 0) {
                        cout << response << endl;
                }
        }
        return;
}


void cmdStringGetRooms(const v8::FunctionCallbackInfo<v8::Value> & args){
        string command, response;
        std::string client = std::string("name");
        BotIface botIface(client);
        botIface.cmdStringGetRooms(command);

        if (botIface.send(command, response) != BotIface::SUCCESS) {
                response = botIface.getLastErrorString();
                std::cout << "Send failed: " << response;
        }
        else {
                if (response.length() > 0) {
                        cout << response << endl;
                }
        }
        return;
}

void cmdStringAddRoom(const v8::FunctionCallbackInfo<v8::Value> & args) {
        Isolate* isolate = args.GetIsolate();
        std::string client = std::string("name");
        BotIface botIface(client);

        string command, response;

        vector <string> members;
        Local<Array> arr = Local<Array>::Cast(args[1]);
        for(int i=0; i<arr->Length(); i++) {
                Local<Value> item = arr->Get(i);
                v8::String::Utf8Value param2(item->ToString());
                std::string str = std::string(*param2);
                members.push_back(str);
        }
        vector <string> moderators;
        Local<Array> arr1 = Local<Array>::Cast(args[2]);
        for(int j=0; j<arr1->Length(); j++) {
                Local<Value> item = arr1->Get(j);
                v8::String::Utf8Value param3(item->ToString());
                std::string str = std::string(*param3);
                moderators.push_back(str);
        }
        v8::String::Utf8Value param4(args[3]->ToString());
        std::string title = std::string(*param4);
        v8::String::Utf8Value param5(args[4]->ToString());
        std::string description = std::string(*param5);
        v8::String::Utf8Value param6(args[5]->ToString());
        std::string ttl = std::string(*param6);
        v8::String::Utf8Value param7(args[6]->ToString());
        std::string bor = std::string(*param7);

        botIface.cmdStringAddRoom(command, members, moderators, title, description, ttl, bor);

        if (botIface.send(command, response) != BotIface::SUCCESS) {
                response = botIface.getLastErrorString();
                std::cout << "Send failed: " << response;
        }
        else {
                if (response.length() > 0) {
                        cout << response << endl;
                }
        }
        return;
}



void init(Handle <Object> exports, Handle<Object> module) {
        //2nd param: what we call from Javascript
        //3rd param: the name of the actual function
        NODE_SET_METHOD(exports, "display", display);
        NODE_SET_METHOD(exports, "cmdGetStatistics", cmdStringGetStatistics);
        NODE_SET_METHOD(exports, "cmdClearStatistics", cmdStringClearStatistics);
        NODE_SET_METHOD(exports, "cmdGetRooms", cmdStringGetRooms);
        //NODE_SET_METHOD(exports, "cmdAddRoom", cmdStringAddRoom);
        // NODE_SET_METHOD(exports, "cmdStringModifyRoom", cmdStringModifyRoom);
        // NODE_SET_METHOD(exports, "cmdStringDeleteRoom", cmd_string_delete_room);
        // NODE_SET_METHOD(exports, "cmdStringLeaveRoom", cmd_string_leave_room);
        // NODE_SET_METHOD(exports, "cmdStringGetRooms", cmd_string_get_rooms);
}


// associates the module name with initialization logic
NODE_MODULE(wickrio_addon, init)
