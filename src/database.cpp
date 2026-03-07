#include "database.h"
#include <iostream>


Database::Database(const std::string& dbPath){
    if(sqlite3_open(dbPath.c_str(),&db)!=SQLITE_OK){
        std::cerr<<"cannot open database: " << sqlite3_errmsg(db)
    }else{
        createTables();
    }
}
Database::~Database(){
    sqlite3_close(db);
}

bool Database::createTables(){
    const char* createStudents = R"(
    CREATE TABLE IF NOT EXISTS Students(
    StudentId INTERGER PRIMARY KEY AUTOINCREMENT,
    SocialNick TEXT NOT NULL,
    SocialNetwork TEXT NOT NULL,
    Phone TEXT,
    Timezone TEXT
    );
    )";
     const char* createEvents = R"(
        CREATE TABLE IF NOT EXISTS Events (
            EventId INTEGER PRIMARY KEY AUTOINCREMENT,
            Topic TEXT NOT NULL,
            Duration INTEGER NOT NULL,
            StudentId INTEGER,
            Frequency TEXT,
            Monday BOOLEAN DEFAULT FALSE,
            Tuesday BOOLEAN DEFAULT FALSE,
            Wednesday BOOLEAN DEFAULT FALSE,
            Thursday BOOLEAN DEFAULT FALSE,
            Friday BOOLEAN DEFAULT FALSE,
            Saturday BOOLEAN DEFAULT FALSE,
            Sunday BOOLEAN DEFAULT FALSE,
            Parent TEXT,
            FOREIGN KEY (StudentId) REFERENCES Students(StudentId)
        );
    )";

char* errMsg = nullptr;
    if (sqlite3_exec(db, createStudents, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    if (sqlite3_exec(db, createEvents, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}
bool Database::insertStudent(const Student& student) {
    const char* sql = "INSERT INTO Students (SocialNick, SocialNetwork, Phone, Timezone) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, student.socialNick.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, student.socialNetwork.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, student.phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, student.timezone.c_str(), -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::vector<Student> Database::getAllStudents() {
    std::vector<Student> students;
    const char* sql = "SELECT StudentId, SocialNick, SocialNetwork, Phone, Timezone FROM Students;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Student student;
            student.id = sqlite3_column_int(stmt, 0);
            student.socialNick = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            student.socialNetwork = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            student.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            student.timezone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            students.push_back(student);
        }
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<Event> Database::getEventsByStudent(int studentId) {
    std::vector<Event> events;
    const char* sql = R"(
        SELECT EventId, Topic, Duration, Frequency,
               Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday, Parent
        FROM Events WHERE StudentId = ?;
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, studentId);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Event event;
            event.id = sqlite3_column_int(stmt, 0);
            event.topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            event.duration = sqlite3_column_int(stmt, 2);
            event.frequency = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            event.monday = sqlite3_column_int(stmt, 4) != 0;
            event.tuesday = sqlite3_column_int(stmt, 5) != 0;
            event.wednesday = sqlite3_column_int(stmt, 6) != 0;
            event.thursday = sqlite3_column_int(stmt, 7) != 0;
            event.friday = sqlite3_column_int(stmt, 8) != 0;
            event.saturday = sqlite3_column_int(stmt, 9) != 0;
            event.sunday = sqlite3_column_int(stmt, 10) != 0;
            event.parent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
            events.push_back(event);
        }
    }
    sqlite3_finalize(stmt);
    return events;
}

bool Database::insertEvent(const Event& event) {
    // Проверка существования студента
    const char* checkStudentSql = "SELECT 1 FROM Students WHERE StudentId = ?";
    sqlite3_stmt* checkStmt;

    if (sqlite3_prepare_v2(db, checkStudentSql, -1, &checkStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare check statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(checkStmt, 1, event.studentId);

    bool studentExists = (sqlite3_step(checkStmt) == SQLITE_ROW);
    sqlite3_finalize(checkStmt);

    if (!studentExists) {
        std::cerr << "Student with ID " << event.studentId << " does not exist" << std::endl;
        return false;
    }

    // Начало транзакции
    if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* sql = R"(
        INSERT INTO Events (
            Topic, Duration, StudentId, Frequency,
            Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday, Parent
        ) VALUES (
            ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?, ?, ?
        )
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    // Привязка параметров
    sqlite3_bind_text(stmt, 1, event.topic.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, event.duration);
    sqlite3_bind_int(stmt, 3, event.studentId);
    sqlite3_bind_text(stmt, 4, event.frequency.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, event.monday ? 1 : 0);
    sqlite3_bind_int(stmt, 6, event.tuesday ? 1 : 0);
    sqlite3_bind_int(stmt, 7, event.wednesday ? 1 : 0);
    sqlite3_bind_int(stmt, 8, event.thursday ? 1 : 0);
    sqlite3_bind_int(stmt, 9, event.friday ? 1 : 0);
    sqlite3_bind_int(stmt, 10, event.saturday ? 1 : 0);
    sqlite3_bind_int(stmt, 11, event.sunday ? 1 : 0);
    sqlite3_bind_text(stmt, 12, event.parent.c_str(), -1, SQLITE_TRANSIENT);

    // Выполнение запроса
    int result = sqlite3_step(stmt);
    bool success = (result == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to insert event: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
    } else {
        // Фиксация транзакции
        if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to commit transaction: " << sqlite3_errmsg(db) << std::endl;
            success = false;
        }
    }

    sqlite3_finalize(stmt);
    return success;
}
