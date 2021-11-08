#include "generator.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>


// Helper function to get the full name for an insertion point.
std::string GetFullInsertionPoint(
    const std::string& point_name,
    const google::protobuf::Descriptor* message = nullptr) {
  return message == nullptr ? point_name
                            : point_name + ":" + message->full_name();
}


// Helper function to return a pointer to an inserter stream
google::protobuf::io::ZeroCopyOutputStream* GetInserter(
    const std::string& file_name, const std::string& insertion_point,
    google::protobuf::compiler::GeneratorContext* context,
    const google::protobuf::Descriptor* message = nullptr) {
  auto full_insertion_point = GetFullInsertionPoint(insertion_point, message);
  return context->OpenForInsert(file_name, full_insertion_point);
}


// Helper function to return a printer for an insertion point
std::shared_ptr<google::protobuf::io::Printer> GetPrinter(
    const std::string& file_name, const std::string& insertion_point,
    google::protobuf::compiler::GeneratorContext* context,
    const google::protobuf::Descriptor* message = nullptr) {
  auto* inserter = GetInserter(file_name, insertion_point, context, message);
  return std::make_shared<google::protobuf::io::Printer>(inserter, '$');
}


void InsertBasicStatement(
    const std::string& file_name, const std::string& insertion_point,
    google::protobuf::compiler::GeneratorContext* context,
    const google::protobuf::Descriptor* message = nullptr,
    const std::string explicit_text = "") {
  auto printer = GetPrinter(file_name, insertion_point, context, message);
  
  auto output = explicit_text;

  if (output.empty()) {
    auto text = "@@" + insertion_point;

    if (message != nullptr) {
      text += ":" + message->full_name();
    }

    output = "std::cout << \"" + text + "\\n\";\n";
  }

  printer->Indent();
  printer->Print(output.c_str());
}


bool Generator::GenerateFor(
    const google::protobuf::Descriptor* message,
    const google::protobuf::FileDescriptor* file,
    google::protobuf::compiler::GeneratorContext* context) const {
  // ----------------------------------------------------------------------
  // H insertion points
  // ----------------------------------------------------------------------
  // includes
  // class_definition:example.Foo)
  // class_scope:example.Foo
  // field_get:example.Foo.version
  // field_set:example.Foo.version
  // field_mutable:example.Foo.version
  // field_release:example.Foo.version
  // field_set_allocated:example.Foo.version
  // field_get:example.Foo.a
  // field_set:example.Foo.a
  // field_get:example.Foo.b
  // field_set:example.Foo.b
  // field_get:example.Foo.c
  // field_set:example.Foo.c
  // field_mutable:example.Foo.c
  // field_release:example.Foo.c
  // field_set_allocated:example.Foo.c
  // namespace_scope
  // global_scope

  // ----------------------------------------------------------------------
  // CC insertion points
  // ----------------------------------------------------------------------
  // includes
  // arena_constructor:example.Foo
  // copy_constructor:example.Foo
  // destructor:example.Foo
  // message_clear_start:example.Foo
  // serialize_to_array_start:example.Foo
  // serialize_to_array_end:example.Foo
  // message_byte_size_start:example.Foo
  // class_specific_merge_from_start:example.Foo
  // class_specific_copy_from_start:example.Foo
  // namespace_scope
  // global_scope

  using google::protobuf::compiler::StripProto;
  auto hh_filename = StripProto(file->name()) + ".pb.h";
  auto cc_filename = StripProto(file->name()) + ".pb.cc";

  // ---------------------------------------------------------------------
  // Insert print statements into each of the insertion points so that it's
  // clear when these insertion points are relevant.

  // HEADER INSERTIONS

  InsertBasicStatement(hh_filename, "includes", context, nullptr, "#include <iostream>");

  // IMPLEMENTATION INSERTIONS

  InsertBasicStatement(cc_filename, "includes", context, nullptr, "#include <iostream>");
  InsertBasicStatement(cc_filename, "arena_constructor", context, message);
  InsertBasicStatement(cc_filename, "copy_constructor", context, message);
  InsertBasicStatement(cc_filename, "destructor", context, message);
  InsertBasicStatement(cc_filename, "message_clear_start", context, message);
  InsertBasicStatement(cc_filename, "serialize_to_array_start", context, message);
  InsertBasicStatement(cc_filename, "serialize_to_array_end", context, message);
  InsertBasicStatement(cc_filename, "message_byte_size_start", context, message);
  InsertBasicStatement(cc_filename, "class_specific_merge_from_start", context, message);
  InsertBasicStatement(cc_filename, "class_specific_copy_from_start", context, message);
  InsertBasicStatement(cc_filename, "namespace_scope", context, nullptr,
                       "// THIS WAS INSERTED INTO THE NAMESPACE SCOPE\n");
  InsertBasicStatement(cc_filename, "global_scope", context, nullptr,
                       "// THIS WAS INSERTER INTO THE GLOBAL SCOPE\n");

  return true;
}


bool Generator::Generate(const google::protobuf::FileDescriptor* file,
                         const std::string& parameter,
                         google::protobuf::compiler::GeneratorContext* context,
                         std::string* error) const {
  // Generate for each message. Short circuit on any failures.
  for (int i = 0; i < file->message_type_count(); ++i) {
    if (!GenerateFor(file->message_type(i), file, context)) {
      return false;
    }
  }

  return true;
}
