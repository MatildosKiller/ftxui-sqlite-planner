#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

struct Student {
    int id;
    std::string socialNick;
    std::string socialNetwork;
    std::string phone;
    std::string timezone;
};

struct UserEvent {
    int id;
    std::string topic;
    int duration;
    int studentId;
    std::string frequency;
    bool monday, tuesday, wednesday, thursday, friday, saturday, sunday;
    std::string parent;
};

class Database {
private:
    sqlite3* db;
public:
    Database(const std::string& dbPath);
    ~Database();
    
    bool createTables();
    bool insertStudent(const Student& student);
    bool insertEvent(const UserEvent& event);
    std::vector<Student> getAllStudents();
    std::vector<UserEvent> getEventsByStudent(int studentId);
   
};

#endif
