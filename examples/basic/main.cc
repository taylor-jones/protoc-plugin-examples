#include <protos/api/foo.pb.h>
#include <google/protobuf/message.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::cout << "\n ------------- Before initializing Foo ------------- \n";
  project::api::Foo foo;
  std::cout << "\n ------------- After initializing Foo ------------- \n";

  std::cout << "\n ------------- Before printing Foo ------------- \n";
  std::cout << foo.DebugString();
  std::cout << "\n ------------- After printing Foo ------------- \n";


  auto* foo_desc = foo.descriptor();
  auto foo_opts = foo_desc->options();

  auto* version_desc = foo_desc->FindFieldByName("version");
  auto version_opts = version_desc->options();
  auto version_def = version_opts.GetExtension(project::options::field_options).default_value();

  // for (auto i = 0; i < foo_desc->field_count(); ++i) {
  //   auto* field = foo_desc->field(i);
  //   auto field_base_opts = field->options();
  //   if (field_base_opts.HasExtension(project::options::field_options)) {
  //     auto extended_opts = field_base_opts.GetExtension(project::options::field_options);
  //     auto field_default = extended_opts.default_value();
  //     std::cout << field->full_name() << " has extended opts. Default: " << field_default << "\n";
  //   }
  // }

  std::cout << "--- foo_desc ---\n" << foo_desc->DebugString() << "\n";
  std::cout << "--- foo_opts ---\n" << foo_opts.DebugString() << "\n";

  std::cout << "--- version_desc ---\n" << version_desc->DebugString() << "\n";
  std::cout << "--- version_opts ---\n" << version_opts.DebugString() << "\n";
  std::cout << "--- version_def ---\n" << version_def << "\n";

  // foo.set_version(version_def);

  return 0;
}
