#include "ftxui/screen/screen.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "database.h"
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>

using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    auto database = std::make_unique<Database>("data/students.db");

    // Переменные для форм
    std::string newSocialNick, newSocialNetwork, newPhone, newTimezone;
    std::string newTopic, newFrequency, newParent;
    std::string newDurationStr = "60";
    bool newMonday = false, newTuesday = false, newWednesday = false,
         newThursday = false, newFriday = false, newSaturday = false, newSunday = false;
    int selectedStudentId = -1;

    // Компоненты интерфейса
    auto inputSocialNick = Input(&newSocialNick, "Social Nick");
    auto inputSocialNetwork = Input(&newSocialNetwork, "Social Network");
    auto inputPhone = Input(&newPhone, "Phone");
    auto inputTimezone = Input(&newTimezone, "Timezone");

    auto inputTopic = Input(&newTopic, "Event Topic");
    auto inputDuration = Input(&newDurationStr, "Duration (minutes)");
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

        // Преобразование строки в число с валидацией
        int duration = 60;
        try {
            if (!newDurationStr.empty()) {
                duration = std::stoi(newDurationStr);
                if (duration < 1) duration = 1;
                else if (duration > 300) duration = 300;
            }
        } catch (const std::exception&) {
            duration = 60;
        }

        UserEvent event{0, newTopic, duration, selectedStudentId, newFrequency,
                   newMonday, newTuesday, newWednesday, newThursday,
                   newFriday, newSaturday, newSunday, newParent};
        database->insertEvent(event);
        newTopic.clear();
        newDurationStr = "60";
        newFrequency.clear();
        newParent.clear();
        newMonday = newTuesday = newWednesday = newThursday =
        newFriday = newSaturday = newSunday = false;
    });

   auto renderer = Renderer([&] {
  auto students = database->getAllStudents();

  // Список студентов - визуальные элементы (Elements)
  Elements studentList;
  studentList.push_back(text("Students:") | bold);
  for (const auto& student : students) {
    // Кнопка - компонент, чтобы получить Element, рендерим кнопку
    auto studentButton = Button(
      student.socialNick + " (" + student.socialNetwork + ")",
      [&] { selectedStudentId = student.id; });
    if (selectedStudentId == student.id) {
      studentButton = studentButton | bgcolor(Color::BlueLight) | color(Color::White) | bold;
    } else {
      studentButton = studentButton | bgcolor(Color::Black) | color(Color::GreenYellow);
    }
    studentList.push_back(studentButton->Render());
  }

  // Список событий - Elements
  std::vector<UserEvent> events;
  if (selectedStudentId != -1) {
    events = database->getEventsByStudent(selectedStudentId);
  }

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

    eventList.push_back(text(event.topic + " (" + std::to_string(event.duration) + " min) " + days));
  }

  // Для блоков с компонентами создадим vector<Component>
  std::vector<Component> addStudentComponents;
  addStudentComponents.push_back(Renderer([] { return text("Add New Student") | bold; }));
  addStudentComponents.push_back(inputSocialNick);
  addStudentComponents.push_back(inputSocialNetwork);
  addStudentComponents.push_back(inputPhone);
  addStudentComponents.push_back(inputTimezone);
  addStudentComponents.push_back(buttonAddStudent);
  auto addStudentBlock = vbox(addStudentComponents) | border;

  std::vector<Component> addEventComponents;
  addEventComponents.push_back(Renderer([] { return text("Add New Event") | bold; }));
  addEventComponents.push_back(inputTopic);
  addEventComponents.push_back(inputDuration);
  addEventComponents.push_back(inputFrequency);
  addEventComponents.push_back(Checkbox("Monday", &newMonday));
  addEventComponents.push_back(Checkbox("Tuesday", &newTuesday));
  addEventComponents.push_back(Checkbox("Wednesday", &newWednesday));
  addEventComponents.push_back(Checkbox("Thursday", &newThursday));
  addEventComponents.push_back(Checkbox("Friday", &newFriday));
  addEventComponents.push_back(Checkbox("Saturday", &newSaturday));
  addEventComponents.push_back(Checkbox("Sunday", &newSunday));
  addEventComponents.push_back(inputParent);
  addEventComponents.push_back(buttonAddEvent);
  auto addEventBlock = vbox(addEventComponents) | border;

  return window(
    text("FTXUI SQLite App"),
    vbox({
      hbox({
        vbox(studentList) | flex,
        separator(),
        vbox(eventList) | flex
      }),
      separator(),
      addStudentBlock,
      separator(),
      addEventBlock,
    }));
});


    screen.Loop(renderer);
    return 0;
}
