#include "generator.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/strutil.h>

#include <protos/options/options.pb.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <map>


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


std::string DoubleValueAsString(double value) {
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


std::string FloatValueAsString(float value) {
  if (value == std::numeric_limits<float>::infinity()) {
    return "std::numeric_limits<float>::infinity()";
  } else if (value == -std::numeric_limits<float>::infinity()) {
    return "-std::numeric_limits<float>::infinity()";
  } else if (value != value) {
    return "std::numeric_limits<float>::quiet_NaN()";
  } else {
    auto str_value = google::protobuf::SimpleFtoa(value);
    // If floating point value contains a period (.) or an exponent (either E
    // or e), then append suffix 'f' to make it a float literal.
    if (str_value.find_first_of(".eE") != std::string::npos) {
      str_value.push_back('f');
    }
    return str_value;
  }
}


std::string FieldValueAsString(const google::protobuf::FieldDescriptor* field,
                          const google::protobuf::Value* value) {
  switch (value->kind_case()) {
    case google::protobuf::Value::kNumberValue: {
      switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
          return Int32ToString(value->number_value());
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
          return google::protobuf::StrCat(value->number_value()) + "u";
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
          return Int64ToString(value->number_value());
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
          return UInt64ToString(value->number_value());
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
          return DoubleValueAsString(value->number_value());
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
          return FloatValueAsString(value->number_value());
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
          return field->enum_type()
              ->FindValueByNumber(value->number_value())
              ->name();
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
          return std::to_string(value->number_value());
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
          return "";
      }

      case google::protobuf::Value::kBoolValue:
        return value->bool_value() ? "true" : "false";

      case google::protobuf::Value::kStringValue: {
        auto c_escaped = google::protobuf::CEscape(value->string_value());
        return "\"" + EscapeTrigraphs(c_escaped) + "\"";
      }

      // unsupported??????
      case google::protobuf::Value::kStructValue:
      case google::protobuf::Value::kListValue:
      case google::protobuf::Value::kNullValue:
      default:
        return "";
    }

    return "";
  }
}


  bool Generator::GenerateForMessage(
      const google::protobuf::Descriptor* message,
      const google::protobuf::FileDescriptor* file,
      google::protobuf::compiler::GeneratorContext* context) const {
    // setup a printer at the arena constructor insertion point.
    auto cc_filename =
        google::protobuf::compiler::StripProto(file->name()) + ".pb.cc";
    auto inserter = context->OpenForInsert(
        cc_filename, "arena_constructor:" + message->full_name());
    google::protobuf::io::Printer printer(inserter, '$');

    // setup default values for each field (if found)
    for (auto i = 0; i < message->field_count(); ++i) {
      auto fd = message->field(i);
      const auto field_opts = &fd->options();
      const auto* def = &field_opts->GetExtension(myproj::field_default);

      if (def != nullptr && def->IsInitialized()) {
        auto value_str_literal = FieldValueAsString(fd, def);

        if (!value_str_literal.empty()) {
          std::map<std::string, std::string> args{
              {"name", fd->name()},
              {"value", value_str_literal},
          };

          printer.Print(args, "set_$name$($value$);\n");
          printer.Print("\n");
        }
      }
    }

    return true;
  }


  bool Generator::Generate(
      const google::protobuf::FileDescriptor* file,
      const std::string& parameter,
      google::protobuf::compiler::GeneratorContext* context, std::string* error)
      const {
    // Generate for each message. Short circuit on any failures.
    for (int i = 0; i < file->message_type_count(); ++i) {
      if (!GenerateForMessage(file->message_type(i), file, context)) {
        auto* msg = file->message_type(i);
        std::cerr << "Failed to generate for message " << msg->full_name()
                  << ". Exiting...\n";
        return false;
      }
    }

    return true;
  }
