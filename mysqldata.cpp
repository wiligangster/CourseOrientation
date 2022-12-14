#ifndef MYSQLDATA_CPP
#define MYSQLDATA_CPP

#include <QMap>
#include <QString>
#include <QDebug>
#include <QtSql>
#include <QProcess>
#include <QJsonObject>
#include <QMessageBox>
#include <databasemanager.h>
#include "racemanager.h"

class MySQLData {


public:
    QSqlDatabase *m_db;   
    QJsonObject *m_data;

MySQLData()
{
    /*
    //Utilisation des informations de la base de données par défaut si on instance la classe sans paramètre
    QSqlDatabase m_dbCopy = QSqlDatabase::addDatabase("QMYSQL", "myMysql");

    m_dbCopy.setHostName("127.0.0.1");
    m_dbCopy.setDatabaseName("coursorient");
    m_dbCopy.setUserName("root");
    m_dbCopy.setPassword("walid13");

    if (!m_dbCopy.open())
    {
        qDebug() << "Erreur lors de la connexion du serveur : " << m_dbCopy.lastError();
    }

    m_db = new QSqlDatabase(m_dbCopy); // initialise le pointer avec un nouveau objet d'une copie de QSqlDatabase
    qDebug() << m_db->open();
    */
}

MySQLData(QString username, QString password, QString hostname, QString dbName)
{
    //QSqlDatabase m_dbCopy = QSqlDatabase::addDatabase("QMYSQL", "myMysql");

    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL", "myTestSQL"));//QSqlDatabase::addDatabase("QMYSQL", "myMysql");

    m_db->setUserName(username);
    m_db->setPassword(password);
    m_db->setHostName(hostname);
    m_db->setDatabaseName(dbName);


}

~MySQLData()
{
    //Destructeur
    m_db->close();
    delete m_db;
}


QSqlDatabase *getDatabase()
{
    return this->m_db;
}


int getConvertFormatSexe(QString sexe)
{
    if(sexe == "M") {
        return 1;
    }
    else if(sexe == "F")
    {
        return 2;
    }
    return 3;
}

bool importData()
{

    if (!m_db->open())
    {
        qDebug() << "Erreur lors de la connexion du serveur : " << m_db->lastError();
    }

    if(m_db->open())
    {
        qDebug() << "Le serveur communique avec la base de données.";
    } else {
        qWarning() << "Le serveur ne communique pas avec la base de données !";
    }

    //Constructeur

   QString connectionName;
   QSqlQuery query;
   query = m_db->exec();
   connectionName = "myNewDb";
   query.prepare(QString("SELECT email, password, firstname, lastname, year, gender FROM participants"));
   if (!query.exec())
   {
       return false;
   }
     DatabaseManager *m_db_saver = new DatabaseManager();
     if(m_db_saver->getDb().connectionNames().contains("myNewDb")){
         m_db_saver->getDb().removeDatabase(connectionName);
     }
     QSqlError err = m_db_saver->initDb(connectionName);

     while(query.next())
     {
             QString email = query.value(0).toString();
             QString password = query.value(1).toString();
             QString firstname = query.value(2).toString();
             QString lastname = query.value(3).toString();
             QString year = query.value(4).toString();
             int genderId = getConvertFormatSexe(query.value(5).toString());

                             // qDebug() << email << password << firstname << lastname << year << genderId;

             //qDebug() << m_db_saver->isParticipantExist(email);


             if(!m_db_saver->isParticipantExist(email))
             {
                 m_db_saver->addParticipant(lastname, firstname, email, password, year, genderId); //Ajouter un participant
             }
    }

     query.prepare(QString("SELECT * FROM races"));

     if (!query.exec())
     {
         return false;
     }

     while(query.next())
     {

         int race_id = query.value(0).toInt();
         int id_department = query.value(1).toInt();
         QString raceName = query.value(2).toString();
         QDateTime date = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
         QString location = query.value(4).toString();
         QString gps_longitude = query.value(5).toString();
         QString gps_latitude = query.value(6).toString();
         int difficulty = query.value(7).toInt();
         int type = query.value(8).toInt();
         int book = query.value(9).toInt();


         if(!m_db_saver->isRaceExist(raceName))
         {
             m_db_saver->addRace(race_id, id_department, raceName, date, location,
                                 gps_longitude, gps_latitude, difficulty, type, book);
         }
     }


     query.prepare(QString("SELECT p_race.participant_id, p_race.race_id FROM participants as p,"
                           " participants_races as p_race WHERE p_race.participant_id=p.id"));

     if (!query.exec())
     {
         return false;
     }

     while(query.next())
     {
         int participant_id = query.value(0).toInt();
         int race_id = query.value(1).toInt();


        // qDebug() << participant_id << race_id;

         //Application

         if(!m_db_saver->isParticipantRaceExist(participant_id))
         {
             qDebug() << "add 3";
             m_db_saver->addParticipantRace(participant_id, race_id);  //Ajouter les participants sur chaque course
         }
     }

     //ToDo : finish
    //m_db_saver->getDb().removeDatabase(connectionName);
    //m_db_saver->getDb().close();

    return true;
}


void exportSql(QString filePath)
{
    QProcess dumpProcess;
    QStringList args;
    //args << "-uroot" << "-pmysql" << "test";
    args << "-uroot" << "-pmysql" << filePath;
    dumpProcess.setStandardOutputFile(filePath);
    dumpProcess.start("mysqldump", args); //mysqldump requis !!!!!! dans l'environnement
    dumpProcess.waitForFinished(); //Attendre que le processus soit entirement finit.
}

bool exportData()
{

      QString connectionName;
      connectionName = "myExportNewDb";
      DatabaseManager *m_db_saver = new DatabaseManager();
      if(m_db_saver->getDb().connectionNames().contains(connectionName)){
          m_db_saver->getDb().removeDatabase(connectionName);
      }
      QSqlError err = m_db_saver->initDb(connectionName);

      QSqlQuery queryLocal = m_db_saver->getDb().exec();
      queryLocal.prepare("SELECT * FROM participant_races_data WHERE race_id=?");
      queryLocal.addBindValue(RaceManager::getInstance()->getRaceId());
      qDebug() << queryLocal.exec();

      while(queryLocal.next())
      {


          if (m_db->open())
          {

              qDebug() << RaceManager::getInstance()->getRaceId() << queryLocal.value(1) << queryLocal.value(2) << queryLocal.value(3) << queryLocal.value(4) << queryLocal.value(5);

              QSqlQuery query;
              query = m_db->exec();
              query.prepare(QString("INSERT INTO leaderboard_races (race_id, participant_id, beacons, start_time, end_time, points) "
                                    "values(?, ?, ?, ?, ?, ?)"));

              //id course actuelle
              query.addBindValue(RaceManager::getInstance()->getRaceId());

              //id participant
              query.addBindValue(queryLocal.value(1));

              //nombre de balises
              query.addBindValue(queryLocal.value(2));

              //start time
              query.addBindValue(queryLocal.value(4));

              //end time
              query.addBindValue(queryLocal.value(5));

              //points
              query.addBindValue(queryLocal.value(3));

              if (!query.exec())
              {
                  qDebug() << query.lastError();
                  return false;
              }

          } else {
              qDebug() << "MySQL NOT OPEN (MySQLData #249)";
              return false;
          }
      }


      return true;
}

QMap<QString, QString> SqlDataToMap()
{
    QSqlQuery q = m_db->exec();
    q.prepare("SHOW TABLES");
    q.exec();

    QVector<QString> tables;
    while(q.next())
    {
        tables.append(q.value(0).toString());
    }

    for (QString table : tables)
    {
        qDebug() << table;
    }

    QMap<QString, QString>parsed;
    parsed["test"] = "d";
    return parsed;
    }

//Date time save
/*
 * query.prepare("INSERT INTO table (table_date_time, ...) "
        " VALUES (:datetime, ...)");
QDateTime timestamp = QDateTime::currentDateTime();
timestamp.addDays(1);
// Qt::ISODate format...
query.bindValue(":datetime", timestamp);// 2017-09-05T11:50:39
// Custom format
query.bindValue(":datetime", timestamp.toString("yyyy-MM-dd hh:mm:ss"));  // same as above but without the T
*/
};

#endif
