#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <protos/api/baz.pb.h>
#include <protos/api/foo.pb.h>

#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <memory>
#include <vector>
#include <functional>
#include <dirent.h>
#include <glob.h>

#include "json_reader.h"

using google::protobuf::util::JsonParseOptions;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::MessageToJsonString;

/**
 * This is a simple app that prints the descriptor and initial values of
 * C++ classes generated via protoc (including any plugins in this project).
 * The purpose of this app is to show a proof-of-concept that the default
 * values for generated fields can be set within the generated C++ code
 * (as opposed to needing to be set manually from the "client" app).
 */

void PrintMessageInfo(const google::protobuf::Message& message) {
  auto name = message.GetTypeName();
  std::cout << "===========================================================\n"
            << name << " descriptor \n"
            << "===========================================================\n"
            << message.GetDescriptor()->DebugString() << "\n";
  std::cout << "===========================================================\n"
            << name << " values \n"
            << "===========================================================\n"
            << message.DebugString() << "\n";
}


void ShowStructInfo(const google::protobuf::Descriptor* descriptor) {
  // auto* descriptor = settings.descriptor();
  for (auto i = 0; i < descriptor->field_count(); ++i) {
    const auto* field = descriptor->field(i);
    std::cout << "\n\n------- FIELD -------"
              << "\nnumber: " << field->number()
              << "\nname: " << field->name()
              << "\nfull_name: " << field->full_name()
              << "\njson_name: " << field->json_name()
              << "\ncamelcase_name: " << field->camelcase_name()
              << "\ncpp_type: " << field->cpp_type()
              << "\ncpp_type_name: " << field->cpp_type_name()
              << "\nlabel: " << field->label()
              << "\ntype_name: " << field->type_name()
              << "\n";
  }
}


std::string ReadJson(const std::string& file_path) {
  std::ifstream json_file(file_path, std::ifstream::binary);
  if (!json_file.is_open()) {
    std::cerr << "Failed to open file - " << file_path << "\n";
    return "";
  }
  return std::string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
}


void WriteJson(const std::string& file_path, const std::string& content) {
  std::ofstream out;
  out.open(file_path, std::ios::out | std::ios::binary);
  if (!out.is_open()) {
    std::cerr << "Failed to open file - " << file_path << "\n";
    return;
  }
  out << content;
  out.close();
}



int main(int argc, char* argv[]) {
  myproj::api::Foo foo;
  PrintMessageInfo(foo);

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("qml-form-builder");
  parser.addPositionalArgument("json-file", QCoreApplication::translate("main", "JSON file to load"));
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.size() == 0) {
    qCritical() << "Please provide an argument for a JSON input file.";
    return 1;
  }

  JsonReader json_reader;
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("JsonReader", &json_reader);
  engine.rootContext()->setContextProperty("jsonFile", args.at(0));
  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  if (engine.rootObjects().isEmpty()) return -1;
  return app.exec();
}
