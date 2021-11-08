#include <protos/foo.pb.h>
#include <protos/options.pb.h>
#include <google/protobuf/message.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::cout << "\n ------------- Before initializing Foo ------------- \n";
  example::Foo foo;
  std::cout << "\n ------------- After initializing Foo ------------- \n";

  std::cout << "\n ------------- Before printing Foo ------------- \n";
  std::cout << foo.DebugString();
  std::cout << "\n ------------- After printing Foo ------------- \n";


  auto* foo_desc = foo.descriptor();
  auto foo_opts = foo_desc->options();

  auto* version_desc = foo_desc->FindFieldByName("version");
  auto version_opts = version_desc->options();
  auto version_def = version_opts.GetExtension(example::field_options).default_value();

  std::cout << "--- foo_desc ---\n" << foo_desc->DebugString() << "\n";
  std::cout << "--- foo_opts ---\n" << foo_opts.DebugString() << "\n";

  std::cout << "--- version_desc ---\n" << version_desc->DebugString() << "\n";
  std::cout << "--- version_opts ---\n" << version_opts.DebugString() << "\n";
  std::cout << "--- version_def ---\n" << version_def << "\n";

  // foo.set_version(version_def);

  return 0;
}
