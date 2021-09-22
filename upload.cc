#include "upload.h"

using namespace std;

void _upload::upload(string File_name,string File_dir,string Url1)
{
  QHttpMultiPart *Multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  string Result="form-data; name=\"filetoupload\"; filename=\""+File_name+"\"";
  string File_name1=File_dir+File_name;

  QHttpPart File_part;
  File_part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
  File_part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString::fromStdString(Result)));
  QFile *File = new QFile(QString::fromStdString(File_name1));
  File->open(QIODevice::ReadOnly);
  File_part.setBodyDevice(File);
  File->setParent(Multipart);

  Multipart->append(File_part);

  QUrl Url(QString::fromStdString(Url1));
  QNetworkRequest Request(Url);

  QNetworkAccessManager Manager;
  QNetworkReply *Reply = Manager.post(Request, Multipart);
  Multipart->setParent(Reply);

//  qDebug() << "Error occured: ";

//  connect(Reply, SIGNAL(uploadProgress(qint64,qint64)),this,SLOT(progress(qint64,qint64)));
//  connect(Reply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(error(QNetworkReply::NetworkError)));
//  connect(Reply,SIGNAL(finished()),this, SLOT(end()));

  QThread::sleep(2);
  if (Reply->error() !=0)
      {
          QMessageBox::information(nullptr,"Connection Error",Reply->errorString());
      }
      else
      {
          QString message_d = QString("Upload Completed");
          QMessageBox::information(nullptr, "Upload Completed", message_d);
      }

}


void _upload::end()
{
    QString data = QString(Reply->readAll());
    qDebug() << data;
    qDebug() << "Upload finished";

//    uploadInProgress = false;
    if ( Reply->error() > 0 )
    {
        cout << "error" << endl;
        qDebug() << "Error occured: " << Reply->error() << " : " << Reply->errorString();
    }
    else
    {
      cout << "Okay" << endl;
        qDebug() << "Upload success";
    }
    delete Reply;
}

void _upload::progress(qint64 a, qint64 b)
{
    cout << " SOME PROGRESS!";
    cout << a  << "/" << b;
}

void _upload::error(QNetworkReply::NetworkError err)
{
    cout << " SOME ERROR!";
    cout << err;
}
