#include "generator.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/message.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/util/json_util.h>
#include <protos/options/options.pb.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

using google::protobuf::util::JsonParseOptions;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::MessageToJsonString;

std::string Int32ToString(int number) {
  if (number == std::numeric_limits<int32_t>::min()) {
    // This needs to be special-cased, see explanation here:
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52661
    return google::protobuf::StrCat(number + 1, " - 1");
  }
  return google::protobuf::StrCat(number);
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


std::string FieldValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field) {
  const auto reflection = message->GetReflection();

  switch (field->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      return Int32ToString(reflection->GetInt32(*message, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      return Int32ToString(reflection->GetBool(*message, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      return google::protobuf::StrCat(reflection->GetUInt32(*message, field)) + "u";
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      return Int64ToString(reflection->GetInt64(*message, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      return UInt64ToString(reflection->GetUInt64(*message, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
      return DoubleValueAsString(reflection->GetDouble(*message, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      return FloatValueAsString(reflection->GetFloat(*message, field));
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
      auto num_val = reflection->GetEnumValue(*message, field);
      return field->enum_type()->FindValueByNumber(num_val)->name();
    }
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
      auto str_value = reflection->GetString(*message, field);
      // auto c_escaped = google::protobuf::CEscape(str_value);
      return "\"" + EscapeTrigraphs(str_value) + "\"";
    }

    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        JsonPrintOptions print_opts;
        print_opts.add_whitespace = true;
        print_opts.always_print_primitive_fields = true;
        print_opts.always_print_enums_as_ints = false;
        print_opts.preserve_proto_field_names = true;
        std::string sub_message_json;
        const auto* sub_message = &reflection->GetMessage(*message, field);
        MessageToJsonString(*sub_message, &sub_message_json, print_opts);
        return sub_message_json;
    }

    // unsupported??????
    default:
      return "";
  }
}


bool Generator::GenerateJsonForMessage(
    const google::protobuf::Descriptor* message,
    const google::protobuf::FileDescriptor* file,
    google::protobuf::compiler::GeneratorContext* context) const {
  JsonPrintOptions print_opts;
  print_opts.add_whitespace = true;
  print_opts.always_print_primitive_fields = true;
  print_opts.always_print_enums_as_ints = false;
  print_opts.preserve_proto_field_names = true;

  const google::protobuf::MessageOptions* message_options = &message->options();
  const auto* widget_options_message = &message_options->GetExtension(myproj::widget_options);

  if (widget_options_message != nullptr && widget_options_message->IsInitialized()) {
    auto json_filename = google::protobuf::compiler::StripProto(message->name()) + ".json";
    auto inserter = context->Open("forms/" + json_filename);
    google::protobuf::io::Printer printer(inserter, '$');

    printer.Print("{\n");
    printer.Indent();

    const auto widget_options_descriptor = widget_options_message->descriptor();
    for (auto i = 0; i < widget_options_descriptor->field_count(); ++i) {
      auto* option_field = widget_options_descriptor->field(i);
      const auto option_field_name = option_field->name();
      const auto field_name = "\"" + option_field_name + "\": ";
      const auto field_value = FieldValueAsString(widget_options_message, option_field);
      printer.Print(field_name.c_str());
      printer.Print(field_value.c_str());
      printer.Print(",\n");
    }

    printer.Print("\"fields\": [\n");
    printer.Indent();

    int num_fields_printed = 0;

    // setup default values for each field (if found)
    for (auto i = 0; i < message->field_count(); ++i) {
      auto* fd = message->field(i);
      const google::protobuf::FieldOptions* field_opts = &fd->options();

      if (field_opts->HasExtension(myproj::control_options)) {
        const auto* control_options_message = &field_opts->GetExtension(myproj::control_options);

        if (control_options_message != nullptr && control_options_message->IsInitialized()) {
          if (num_fields_printed > 0) {
            printer.Print(",\n");
          }

          std::string opts_json;
          MessageToJsonString(*control_options_message, &opts_json, print_opts);

          if (i < message->field_count() - 1) {
            while (opts_json.back() == '\n') opts_json.pop_back();
          }

          printer.Print(opts_json.c_str());
          num_fields_printed++;
        }
      }
    }

    printer.Outdent();
    if (num_fields_printed == 0) printer.Print("\n");
    printer.Print("]\n");
    printer.Outdent();
    printer.Print("}\n");
  }

  return true;
}


bool Generator::Generate(const google::protobuf::FileDescriptor* file,
                         const std::string& parameter,
                         google::protobuf::compiler::GeneratorContext* context,
                         std::string* error) const {
  // Generate for each message. Short circuit on any failures.
  for (int i = 0; i < file->message_type_count(); ++i) {
    if (!GenerateJsonForMessage(file->message_type(i), file, context)) {
      auto* msg = file->message_type(i);
      std::cerr << "Generate failed on " << msg->full_name() << std::endl;
      return false;
    }
  }

  return true;
}
