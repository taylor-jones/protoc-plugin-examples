#ifndef MYAPP_BASIC_OPTIONS_HELPERS
#define MYAPP_BASIC_OPTIONS_HELPERS

#include <google/protobuf/descriptor.h>
#include <string>


inline bool is_numeric_type(google::protobuf::FieldDescriptor::Type type) {
  switch (type) {
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE:    // fall through
    case google::protobuf::FieldDescriptor::TYPE_FLOAT:     // fall through
    case google::protobuf::FieldDescriptor::TYPE_INT64:     // fall through
    case google::protobuf::FieldDescriptor::TYPE_UINT64:    // fall through
    case google::protobuf::FieldDescriptor::TYPE_INT32:     // fall through
    case google::protobuf::FieldDescriptor::TYPE_FIXED64:   // fall through
    case google::protobuf::FieldDescriptor::TYPE_FIXED32:   // fall through
    case google::protobuf::FieldDescriptor::TYPE_UINT32:    // fall through
    case google::protobuf::FieldDescriptor::TYPE_SFIXED32:  // fall through
    case google::protobuf::FieldDescriptor::TYPE_SFIXED64:  // fall through
    case google::protobuf::FieldDescriptor::TYPE_SINT32:    // fall through
    case google::protobuf::FieldDescriptor::TYPE_SINT64:    // fall through
      return true;
    default:
      return false;
  }

  assert(0);
  return false;
}


inline bool is_string_type(google::protobuf::FieldDescriptor::Type type) {
  switch (type) {
    case google::protobuf::FieldDescriptor::TYPE_STRING:  // fall through
    case google::protobuf::FieldDescriptor::TYPE_BYTES:   // fall through
      return true;
    default:
      return false;
  }
  assert(0);
  return false;
}


inline bool is_non_numeric_scalar_type(
    google::protobuf::FieldDescriptor::Type type) {
  switch (type) {
    case google::protobuf::FieldDescriptor::TYPE_BOOL:  // fall through
    case google::protobuf::FieldDescriptor::TYPE_ENUM:  // fall through
      return true;
    default:
      return false;
  }
  assert(0);
  return false;
}

#endif  // MYAPP_BASIC_OPTIONS_HELPERS
