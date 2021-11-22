#include "generator.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/common.h>

#include <protos/options/options.pb.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>

#include "helpers.h"


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


std::string formatted_value(const google::protobuf::FieldDescriptor* field, const std::string& value) {
  auto type = field->type();

  if (is_string_type(type)) {
    return "\"" + value + "\"";
  }

  if (is_numeric_type(type) || is_non_numeric_scalar_type(type)) {
    return value;
  }

  return {};
}


std::string Int32ToString(int number) {
  if (number == std::numeric_limits<int32_t>::min()) {
    // This needs to be special-cased, see explanation here:
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52661
    return google::protobuf::StrCat(number + 1, " - 1");
  } else {
    return google::protobuf::StrCat(number);
  }
}

static std::string Int64ToString(int64_t number) {
  if (number == std::numeric_limits<int64_t>::min()) {
    // This needs to be special-cased, see explanation here:
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52661
    return google::protobuf::StrCat("int64_t{", number + 1, "} - 1");
  }
  return google::protobuf::StrCat("int64_t{", number, "}");
}

static std::string UInt64ToString(uint64_t number) {
  return google::protobuf::StrCat("uint64_t{", number, "u}");
}

// Escape C++ trigraphs by escaping question marks to \?
std::string EscapeTrigraphs(const std::string& to_escape) {
  return google::protobuf::StringReplace(to_escape, "?", "\\?", true);
}

std::string DefaultValue(const google::protobuf::FieldDescriptor* field) {
  switch (field->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      return Int32ToString(field->default_value_int32());
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      return google::protobuf::StrCat(field->default_value_uint32()) + "u";
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      return Int64ToString(field->default_value_int64());
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      return UInt64ToString(field->default_value_uint64());
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
      double value = field->default_value_double();
      if (value == std::numeric_limits<double>::infinity()) {
        return "std::numeric_limits<double>::infinity()";
      } else if (value == -std::numeric_limits<double>::infinity()) {
        return "-std::numeric_limits<double>::infinity()";
      } else if (value != value) {
        return "std::numeric_limits<double>::quiet_NaN()";
      } else {
        return google::protobuf::SimpleDtoa(value);
      }
    }
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
      float value = field->default_value_float();
      if (value == std::numeric_limits<float>::infinity()) {
        return "std::numeric_limits<float>::infinity()";
      } else if (value == -std::numeric_limits<float>::infinity()) {
        return "-std::numeric_limits<float>::infinity()";
      } else if (value != value) {
        return "std::numeric_limits<float>::quiet_NaN()";
      } else {
        std::string float_value = google::protobuf::SimpleFtoa(value);
        // If floating point value contains a period (.) or an exponent
        // (either E or e), then append suffix 'f' to make it a float
        // literal.
        if (float_value.find_first_of(".eE") != std::string::npos) {
          float_value.push_back('f');
        }
        return float_value;
      }
    }
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      return field->default_value_bool() ? "true" : "false";
    // case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
    //   // Lazy:  Generate a static_cast because we don't have a helper function
    //   //   that constructs the full name of an enum value.
    //   return strings::Substitute(
    //       "static_cast< $0 >($1)", ClassName(field->enum_type(), true),
    //       Int32ToString(field->default_value_enum()->number()));
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
      return "\"" +
             EscapeTrigraphs(google::protobuf::CEscape(field->default_value_string())) +
             "\"";
    // case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
    //   return "*" + google::protobuf::FieldMessageTypeName(field, options) +
    //          "::internal_default_instance()";
  }
  // Can't actually get here; make compiler happy.  (We could add a default
  // case above but then we wouldn't get the nice compiler warning when a
  // new type is added.)
  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}



bool Generator::GenerateFor(
    const google::protobuf::Descriptor* message,
    const google::protobuf::FileDescriptor* file,
    google::protobuf::compiler::GeneratorContext* context) const {
  auto hh_filename = google::protobuf::compiler::StripProto(file->name()) + ".pb.h";
  auto cc_filename = google::protobuf::compiler::StripProto(file->name()) + ".pb.cc";

  auto arena_constructor_inserter =
      GetInserter(cc_filename, "arena_constructor", context, message);

  google::protobuf::io::Printer printer(arena_constructor_inserter, '$');

  for (auto i = 0; i < message->field_count(); ++i) {
    auto fd = message->field(i);
    const auto fopt = &fd->options();

    //
    // CFO
    //
    const auto* cfo = &fopt->GetExtension(myproj::CFO);

    if (cfo != nullptr && !cfo->default_value().empty()) {
      std::map<std::string, std::string> args{
        {"name", fd->name()},
        {"value", formatted_value(fd, cfo->default_value())},
      };

      printer.Print(args, "set_$name$($value$);\n");
    }

    //
    // FieldDefault
    //
    const auto* def = &fopt->GetExtension(myproj::field_default);

    if (def != nullptr && def->IsInitialized()) {
      std::string value;

      switch (def->kind_case()) {
        case google::protobuf::Value::kNumberValue:
          value = std::to_string(def->number_value());
          break;
        case google::protobuf::Value::kBoolValue:
          value = def->bool_value() ? "true" : "false";
          break;
        case google::protobuf::Value::kStringValue:
          value = "\"" + def->string_value() + "\"";
          break;
        // unsupported??????
        case google::protobuf::Value::kStructValue:
        case google::protobuf::Value::kListValue:
        case google::protobuf::Value::kNullValue:
        default:
          break;
      }

      if (!value.empty()) {
        std::map<std::string, std::string> args{
          {"name", fd->name()},
          {"value", value},
        };

        printer.Print(args, "set_$name$($value$);\n");
      }

    }
  }

  printer.Print("\n");

  return true;
}


bool Generator::Generate(const google::protobuf::FileDescriptor* file,
                         const std::string& parameter,
                         google::protobuf::compiler::GeneratorContext* context,
                         std::string* error) const {
  // Generate for each message. Short circuit on any failures.
  for (int i = 0; i < file->message_type_count(); ++i) {
    if (!GenerateFor(file->message_type(i), file, context)) {
      auto* msg = file->message_type(i);
      std::cerr << "Failed to generate for message " << msg->full_name()
                << ". Exiting...\n";
      return false;
    }
  }

  return true;
}
