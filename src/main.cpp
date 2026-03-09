#include "ftxui/screen/screen.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "database.h"  // Подключаем заголовочный файл из include/
#include <memory>
#include <string>

using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    auto database = std::make_unique<Database>("data/students.db");

    // Переменные для форм
    std::string newSocialNick, newSocialNetwork, newPhone, newTimezone;
    std::string newTopic, newFrequency, newParent;
    int newDuration = 60;
    bool newMonday = false, newTuesday = false, newWednesday = false,
         newThursday = false, newFriday = false, newSaturday = false, newSunday = false;
    int selectedStudentId = -1;

    // Компоненты интерфейса
    auto inputSocialNick = Input(&newSocialNick, "Social Nick");
    auto inputSocialNetwork = Input(&newSocialNetwork, "Social Network");
    auto inputPhone = Input(&newPhone, "Phone");
    auto inputTimezone = Input(&newTimezone, "Timezone");

    auto inputTopic = Input(&newTopic, "Event Topic");
    auto inputDuration = Input(&newDuration, "Duration (minutes)");
    auto inputFrequency = Input(&newFrequency, "Frequency");
    auto inputParent = Input(&newParent, "Parent");

    auto buttonAddStudent = Button("Add Student", [&] {
        Student student{0, newSocialNick, newSocialNetwork, newPhone, newTimezone};
        if (database->insertStudent(student)) {
            newSocialNick.clear();
            newSocialNetwork.clear();
            newPhone.clear();
            newTimezone.clear();
        }
    });

    auto buttonAddEvent = Button("Add Event", [&] {
        if (selectedStudentId == -1) return;
        Event event{0, newTopic, newDuration, selectedStudentId, newFrequency,
                   newMonday, newTuesday, newWednesday, newThursday,
                   newFriday, newSaturday, newSunday, newParent};
        database->insertEvent(event);
        newTopic.clear();
        newDuration = 60;
        newFrequency.clear();
        newParent.clear();
        newMonday = newTuesday = newWednesday = newThursday =
        newFriday = newSaturday = newSunday = false;
    });

    auto renderer = Renderer([&] {
        auto students = database->getAllStudents();
        Elements studentList;
        studentList.push_back(text("Students:") | bold);
        for (const auto& student : students) {
            auto studentButton = Button(
                student.socialNick + " (" + student.socialNetwork + ")",
                [&] { selectedStudentId = student.id; }
            );
            if (selectedStudentId == student.id) {
                studentButton = studentButton | focusSelector | bgcolor(Color::BlueLight);
            }
            studentList.push_back(studentButton);
        }

        auto events = selectedStudentId != -1 ?
            database->getEventsByStudent(selectedStudentId) : std::vector<Event>();
        Elements eventList;
        eventList.push_back(text("Events for selected student:") | bold);
        for (const auto& event : events) {
            std::string days;
            if (event.monday) days += "Mon ";
            if (event.tuesday) days += "Tue ";
            if (event.wednesday) days += "Wed ";
            if (event.thursday) days += "Thu ";
            if (event.friday) days += "Fri ";
            if (event.saturday) days += "Sat ";
            if (event.sunday) days += "Sun ";

            eventList.push_back(
                text(event.topic + " (" + std::to_string(event.duration) + " min) " + days)
            );
        }

        return window(
            text("FTXUI SQLite App"),
            vbox({
                hbox({
                    vbox(studentList) | flex,
                    separator(),
                    vbox(eventList) | flex
                }),
                separator(),
                vbox({
                    text("Add New Student") | bold,
                    inputSocialNick,
            inputSocialNetwork,
            inputPhone,
            inputTimezone,
            buttonAddStudent
        }) | border,
        separator(),
        vbox({
            text("Add New Event") | bold,
            inputTopic,
            inputDuration,
            inputFrequency,
            Checkbox("Monday", &newMonday),
            Checkbox("Tuesday", &newTuesday),
            Checkbox("Wednesday", &newWednesday),
            Checkbox("Thursday", &newThursday),
            Checkbox("Friday", &newFriday),
            Checkbox("Saturday", &newSaturday),
            Checkbox("Sunday", &newSunday),
            inputParent,
            buttonAddEvent
        }) | border
    })
);
    });

    screen.Loop(renderer);
    return 0;
}
