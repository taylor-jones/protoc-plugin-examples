#ifndef MYAPP_SRC_JSON_READER
#define MYAPP_SRC_JSON_READER
#ifndef JSON_READER_H
#define JSON_READER_H

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QVariant>

class JsonReader : public QObject {
  Q_OBJECT

 public:
  JsonReader(QObject* parent = nullptr) : QObject(parent) {}
  Q_INVOKABLE

  QVariant ReadJsonFile(const QString& file_path) {
    QFile file(file_path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "Failed to open file" << file.fileName();
      return {};
    }
    QByteArray file_content(file.readAll());
    file.close();

    QJsonParseError status;
    QJsonDocument json_doc = QJsonDocument::fromJson(file_content, &status);
    if (status.error == QJsonParseError::NoError && !json_doc.isEmpty()) {
      if (json_doc.isArray()) return json_doc.array().toVariantList();
      if (json_doc.isObject()) return json_doc.object().toVariantMap();
    }

    return file_content;
  }
};

#endif  // JSON_READER_H


#endif // MYAPP_SRC_JSON_READER
