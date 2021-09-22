#ifndef UPLOAD_H
#define UPLOAD_H

//#include <string>
#include <iostream>
//#include <fstream>
//#include <algorithm>
#include <vector>
#include <string>
#include <ctime>
#include <QWidget>
#include <QtNetwork/qhttpmultipart.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QThread>
#include <QFile>
#include <QMessageBox>

class _upload : public QWidget
{
Q_OBJECT
public:
  void upload(std::string File_name, std::string File_dir, std::string Url1);

protected slots:
  void progress(qint64,qint64);
  void error(QNetworkReply::NetworkError);
  void end();

protected:
  QNetworkReply *Reply=nullptr;
};

#endif
