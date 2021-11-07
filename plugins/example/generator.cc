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


template <typename T>
std::string Generator::convert_scoped(const T* message) const {
  if (message->containing_type()) {
    return convert_scoped(message->containing_type()) + "_" + message->name();
  } else {
    std::string s = "";

    for (char c : message->full_name()) {
      if (c == '.') {
        s += "::";
      } else {
        s += c;
      }
    }

    return s;
  }
}


template <typename T>
std::string Generator::convert_unscoped(const T* message) const {
  std::vector<std::string> names;

  while (message) {
    names.push_back(message->name());
    message = message->containing_type();
  }

  reverse(names.begin(), names.end());
  std::string s;

  for (auto c : names) {
    s += c + "_";
  }

  s.pop_back();
  return s;
}


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

  auto includes_printer = GetPrinter(cc_filename, "includes", context);
  includes_printer->Print("#include <iostream>");

  // ---------------------------------------------------------------------
  // Insert print statements into each of the insertion points so that it's
  // clear when these insertion points are relevant.

  auto msg_name = message->full_name();
  auto printer = GetPrinter(cc_filename, "arena_constructor", context, message);
  auto raw = "std::cout << \"@@arena_constructor:" + msg_name + "\\n\";\n";
  printer->Print(raw.c_str());

  // ------------------

  msg_name = message->full_name();
  printer = GetPrinter(cc_filename, "copy_constructor", context, message);
  raw = "std::cout << \"@@copy_constructor:" + msg_name + "\\n\";\n";
  printer->Print(raw.c_str());

  // p->Print("\nauto* version_desc = descriptor()->FindFieldByName(\"version\");\n");
  // p->Print("if (version_desc != nullptr) {\n");
  // p->Print("  auto version_opts = version_desc->options();\n");
  // p->Print("  auto version_default = version_opts.GetExtension(example::my_field_options).my_default_value();\n");
  // p->Print("  version_ = version_default;\n");
  // p->Print("}\n");


  // auto arena_constructor_inserter = context->OpenForInsert(
  //     cc_filename, "arena_constructor:" + message->full_name());

  // auto arena_constructor_inserter = context->OpenForInsert(
  //     cc_filename, GetFullInsertionPoint("arena_constructor", message));

  // auto arena_constructor_inserter =
  //     GetInserter(cc_filename, "arena_constructor", context, message);

  // google::protobuf::io::Printer printer(arena_constructor_inserter, '$');

  // --------------------

  // auto arena_constructor_printer =
  //     GetPrinter(cc_filename, "arena_constructor", context, message);
  // arena_constructor_printer->Print("// HELLO WORLD 3\n");

  // --------------------

  // auto class_scope_inserter = context->OpenForInsert(
  //     hh_filename, "class_scope:" + message->full_name());

  // google::protobuf::io::Printer printer(class_scope_inserter, '$');

  // printer.Print("const static std::string TYPE_NAME;\n");
  // printer.Print("struct initializable_type {\n");
  // printer.Indent();

  // for (int i = 0; i < message->field_count(); ++i) {
  //   auto field = message->field(i);
  //   auto etype = field->cpp_type();
  //   std::string type =
  //       "::PROTOBUF_NAMESPACE_ID::" + std::string(field->cpp_type_name());

  //   if (etype == google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
  //     type = convert_scoped(field->enum_type());
  //   } else if (etype == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
  //     type = convert_scoped(field->message_type());
  //   }

  //   printer.Print("$type$ $name$;\n", "type", type.c_str(), "name",
  //                 field->name().c_str());
  // }

  // printer.Outdent();
  // printer.Print(
  //     "};\n\n$constructor$(const initializable_type &t) : $constructor$()
  //     {\n", "constructor", convert_unscoped(message));
  // printer.Indent();

  // for (int i = 0; i < message->field_count(); ++i) {
  //   auto field = message->field(i);
  //   printer.Print("set_$lname$(t.$name$);\n", "lname",
  //                 field->lowercase_name().c_str(), "name",
  //                 field->name().c_str());
  // }

  // printer.Outdent();
  // printer.Print("}\n");

  // for (int i = 0; i < message->nested_type_count(); ++i) {
  //   GenerateFor(message->nested_type(i), file, context);
  // }

  // auto namespace_scope_inserter = context->OpenForInsert(
  //     google::protobuf::compiler::StripProto(file->name()) + ".pb.h",
  //     "namespace_scope");

  // google::protobuf::io::Printer printer2(namespace_scope_inserter, '$');

  // printer2.Print("inline const std::string $1$::TYPE_NAME = \"$2$\";\n", "1",
  //                convert_scoped(message), "2", message->full_name());

  return true;
}


bool Generator::Generate(const google::protobuf::FileDescriptor* file,
                         const std::string& parameter,
                         google::protobuf::compiler::GeneratorContext* context,
                         std::string* error) const {
   // -----------------------------------------------------------------------
   // Process context for debgugging purposes
   // -----------------------------------------------------------------------
  auto basename = google::protobuf::compiler::StripProto(file->name());

  // NOTE: must use stderr because of the way protobuf reads from stdout
  std::cerr << "file->name(): " << file->name() << "\n";
  std::cerr << "basename: " << basename << "\n";
  std::cerr << "parameter: " << parameter << "\n";

  // This is empty. This can be set by us if we encounter an issue. If set,
  // it'll cause the build to fail and will print the error.
  // NOTE: must dereference to set value, like: *error = "my error";
  std::cerr << "error: " << *error << "\n";

  // Check for input options? Print if any are found.
  std::vector<std::pair<std::string, std::string> > options;
  google::protobuf::compiler::ParseGeneratorParameter(parameter, &options);

  if (options.size()) {
    std::cerr << "\n------------- PARSED OPTIONS ----------------\n";
    for (auto& opt : options) {
      std::cerr << opt.first << ": " << opt.second << "\n";
    }
    std::cerr << "\n------------- END PARSED OPTIONS ----------------\n";
  }

  // NOTE: GeneratorContext is used to produce output from this plugin.
  // See google::protobuf::compiler::code_generator.h

   // -----------------------------------------------------------------------
   // Begin processing messages from the input file descriptor
   // -----------------------------------------------------------------------

  // Generate for each message. Short circuit on any failures.
  for (int i = 0; i < file->message_type_count(); ++i) {
    if (!GenerateFor(file->message_type(i), file, context)) {
      return false;
    }
  }

  return true;
}
