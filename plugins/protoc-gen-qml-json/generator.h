#ifndef MYAPP_PROTOC_GEN_QML_JSON_GENERATOR
#define MYAPP_PROTOC_GEN_QML_JSON_GENERATOR

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <string>

class Generator : public google::protobuf::compiler::CodeGenerator {
 public:
  inline Generator() = default;
  inline ~Generator() = default;

  // Indicate that this code generator supports proto3 optional fields.
    // (Note: don't release your code generator with this flag set until you
    // have actually added and tested your proto3 support!)
  inline uint64_t GetSupportedFeatures() const override {
    return FEATURE_PROTO3_OPTIONAL;
  }

  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string& parameter,
                google::protobuf::compiler::GeneratorContext* context,
                std::string* error) const override;

 private:
  bool GenerateForMessage(
      const google::protobuf::Descriptor* message,
      const google::protobuf::FileDescriptor* file,
      google::protobuf::compiler::GeneratorContext* context) const;

  bool GenerateJsonForMessage(
      const google::protobuf::Descriptor* message,
      const google::protobuf::FileDescriptor* file,
      google::protobuf::compiler::GeneratorContext* context) const;
};

#endif // MYAPP_PROTOC_GEN_QML_JSON_GENERATOR
