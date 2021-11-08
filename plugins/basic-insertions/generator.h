#ifndef MYAPP_BASIC_INSERTIONS_GENERATOR
#define MYAPP_BASIC_INSERTIONS_GENERATOR

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <string>

class Generator : public google::protobuf::compiler::CodeGenerator {
 public:
  inline Generator() {}
  inline ~Generator() {}

  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string& parameter,
                google::protobuf::compiler::GeneratorContext* context,
                std::string* error) const override;

 private:
  bool GenerateFor(const google::protobuf::Descriptor* message,
                   const google::protobuf::FileDescriptor* file,
                   google::protobuf::compiler::GeneratorContext* context) const;
};

#endif // MYAPP_BASIC_INSERTIONS_GENERATOR
