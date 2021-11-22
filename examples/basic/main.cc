#include <protos/api/foo.pb.h>
#include <google/protobuf/message.h>

#include <iostream>
#include <string>

/**
 * This is a simple app that prints the destrcutor and initial values of
 * C++ classes generated via protoc (including any plugins in this project).
 * The purpose of this app is to show a proof-of-concept that the default
 * values for generated fields can be set within the generated C++ code
 * (as opposed to needing to be set manually from the "client" app).
 */

void PrintMessageInfo(const google::protobuf::Message& message) {
  auto name = message.GetTypeName();

  std::cout << "===========================================================\n"
            << name << " descriptor \n"
            << "===========================================================\n"
            << message.GetDescriptor()->DebugString() << "\n";

  std::cout << "===========================================================\n"
            << name << " values \n"
            << "===========================================================\n"
            << message.DebugString() << "\n";
}


int main(int argc, char** argv) {
  myproj::api::Foo foo;
  myproj::api::Bar bar;

  PrintMessageInfo(foo);
  PrintMessageInfo(bar);

  return 0;
}
