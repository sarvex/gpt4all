#include "server.h"
#include "llm.h"
#include "download.h"
#include "chatlistmodel.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <iostream>

class MyServer: public Server { };
Q_GLOBAL_STATIC(MyServer, serverInstance)
Server *Server::globalInstance()
{
    return serverInstance();
}

static inline QString modelToName(const ModelInfo &info)
{
    QString modelName = info.filename;
    Q_ASSERT(modelName.startsWith("ggml-"));
    modelName = modelName.remove(0, 5);
    Q_ASSERT(modelName.endsWith(".bin"));
    modelName.chop(4);
    return modelName;
}

static inline QJsonObject modelToJson(const ModelInfo &info)
{
    QString modelName = modelToName(info);

    QJsonObject model;
    model.insert("id", modelName);
    model.insert("object", "model");
    model.insert("created", "who can keep track?");
    model.insert("owned_by", "humanity");
    model.insert("root", modelName);
    model.insert("parent", QJsonValue());

    QJsonArray permissions;
    QJsonObject permissionObj;
    permissionObj.insert("id", "foobarbaz");
    permissionObj.insert("object", "model_permission");
    permissionObj.insert("created", "does it really matter?");
    permissionObj.insert("allow_create_engine", false);
    permissionObj.insert("allow_sampling", false);
    permissionObj.insert("allow_logprobs", false);
    permissionObj.insert("allow_search_indices", false);
    permissionObj.insert("allow_view", true);
    permissionObj.insert("allow_fine_tuning", false);
    permissionObj.insert("organization", "*");
    permissionObj.insert("group", QJsonValue());
    permissionObj.insert("is_blocking", false);
    permissions.append(permissionObj);
    model.insert("permissions", permissions);
    return model;
}

Server::Server()
    : QObject(nullptr)
{
    m_server = new QHttpServer(this);
    //connect(m_server, &QTcpServer::newConnection, this, &Server::handleNewConnection);
    if (!m_server->listen(QHostAddress::LocalHost, 4891)) {
        qWarning() << "ERROR: Unable to start the server";
        return;
    }

    m_server->route("/v1/models", QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &request) {
            const QList<ModelInfo> modelList = Download::globalInstance()->modelList();
            QJsonObject root;
            root.insert("object", "list");
            QJsonArray data;
            for (const ModelInfo &info : modelList) {
                if (!info.installed)
                    continue;
                data.append(modelToJson(info));
            }
            root.insert("data", data);
            return QHttpServerResponse(root);
        }
    );

    m_server->route("/v1/models/<arg>", QHttpServerRequest::Method::Get,
        [](const QString &model, const QHttpServerRequest &request) {
            const QList<ModelInfo> modelList = Download::globalInstance()->modelList();
            QJsonObject object;
            for (const ModelInfo &info : modelList) {
                if (!info.installed)
                    continue;

                QString modelName = modelToName(info);
                if (model == modelName) {
                    object = modelToJson(info);
                    break;
                }
            }
            return QHttpServerResponse(object);
        }
    );
}
