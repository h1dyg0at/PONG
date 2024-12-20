#include <ncurses.h>    // Библиотека для работы с графическим интерфейсом в терминале.
#include <iostream>     // Стандартная библиотека C++ для работы с вводом и выводом.
#include <fstream>      // Для работы с файлами (чтение и запись).
#include <string>       // Для работы со строками.
#include <cstring> 

// Структура для хранения настроек игры
struct Config {
    float speed;         // Скорость игры.
    int max_score;       // Максимальный счёт, при котором игра завершается.
    int field_width;     // Ширина игрового поля.
    int field_height;    // Высота игрового поля.
    int paddle_height;   // Высота ракетки игрока.
    std::string name_Player1;
    std::string name_Player2;
};

void initColors() {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Default text color
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Highlighted text color
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Border color
}

void centeredPrint(int y, const char *text, int width) {
    int x = (width - strlen(text)) / 2;
    mvprintw(y, x, "%s", text);
}

// Функция для загрузки конфигурации из файла config.ini
Config loadConfig(const std::string& fileName) {
    Config config;
    std::ifstream file(fileName);   // Открытие файла для чтения
    if (file.is_open()) {           // Проверка успешного открытия файла
        std::string line;
        while (std::getline(file, line)) {  // Чтение каждой строки из файла
            // Поиск параметров и запись их значений в соответствующие поля структуры
            if (line.find("speed") != std::string::npos) config.speed = std::stof(line.substr(line.find("=") + 1));
            if (line.find("max_score") != std::string::npos) config.max_score = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("field_width") != std::string::npos) config.field_width = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("field_height") != std::string::npos) config.field_height = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("paddle_height") != std::string::npos) config.paddle_height = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("name_Player1") != std::string::npos) config.name_Player1 = line.substr(line.find("=") + 1);
            if (line.find("name_Player2") != std::string::npos) config.name_Player2 = line.substr(line.find("=") + 1);

        }
        file.close();  // Закрытие файла
    }
    return config;  // Возвращение структуры с загруженными параметрами
}

// Функция для сохранения конфигурации
void saveConfig(const std::string& fileName, Config& config) {
    std::ofstream file(fileName);   // Открытие файла для записи
    if (file.is_open()) {           // Проверка успешного открытия файла
        // Запись каждого параметра из структуры `config` в файл
        file << "speed = " << config.speed << std::endl;
        file << "max_score = " << config.max_score << std::endl;
        file << "field_width = " << config.field_width << std::endl;
        file << "field_height = " << config.field_height << std::endl;
        file << "paddle_height = " << config.paddle_height << std::endl;
        file << "name_Player1 = " << config.name_Player1 << std::endl;
        file << "name_Player2 = " << config.name_Player2 << std::endl;
        file.close();  // Закрытие файла
    }
}

// Класс для управления ракеткой
// Класс для управления ракеткой
class Paddle {
private:
    int x, y, height;  // Позиция и высота ракетки

public:
    Paddle(int x, int y, int height) : x(x), y(y), height(height) {}

    // Методы доступа к полям
    int getX() const { return x; }
    int getY() const { return y; }
    int getHeight() const { return height; }

    // Методы установки значений полей
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    void setHeight(int newHeight) { height = newHeight; }

    // Методы изменения значений полей
    void addX(int dx) { x += dx; }
    void addY(int dy) { y += dy; }

    void moveUp() {
        if (y > 1) y--;  // Перемещает ракетку вверх, если не достигнута граница
    }

    void moveDown(int field_height) {
        if (y + height < field_height - 1) y++;  // Перемещает ракетку вниз, если не достигнута нижняя граница
    }

    void draw() const {
        for (int i = 0; i < height; i++) {
            mvprintw(y + i, x, "|");  // Отрисовка ракетки вертикальной чертой на позиции `x`, `y`
        }
    }
};

// Класс для управления мячом
class Ball {
private:
    int x, y;     // Позиция мяча
    int dx, dy;   // Направление мяча

public:
    Ball(int x, int y) : x(x), y(y), dx(1), dy(1) {}

    int getX() const { return x; }
    int getY() const { return y; }
    int getDX() const { return dx; }
    int getDY() const { return dy; }

    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    void setDX(int newDX) { dx = newDX; }
    void setDY(int newDY) { dy = newDY; }

    void addX(int dxIncrement) { x += dxIncrement; }
    void addY(int dyIncrement) { y += dyIncrement; }

    void invertXDirection() {
        dx = -dx;
    }

    void invertYDirection() {
        dy = -dy;
    }

    void move() {
        x += dx;  // Изменение позиции мяча в направлении `dx`
        y += dy;  // Изменение позиции мяча в направлении `dy`
    }

    void bounce(int field_height) {
        if (y <= 1 || y >= field_height - 2) {
            dy = -dy;  // Меняет направление мяча при столкновении с верхней или нижней границей
        }
    }

    void draw() const {
        mvprintw(y, x, "O");  // Отрисовка мяча на экране символом `O`
    }

    bool checkPaddleCollision(const Paddle& paddle) const {
        return (x == paddle.getX() && y >= paddle.getY() && y < paddle.getY() + paddle.getHeight());
    }

    bool outOfBounds(int field_width) const {
        return x <= 0 || x >= field_width - 1;  // Проверка, находится ли мяч за границей игрового поля
    }

    void reset(int startX, int startY) {
        x = startX;  // Сбрасывает позицию мяча в центр
        y = startY;
        dx = -dx;  // Меняет направление мяча
    }
};



// Функция для сохранения результатов в файл scores.txt
void saveScore(const std::string& playerName_1, int score_1, const std::string& playerName_2, int score_2) {
    std::ofstream file("scores.txt", std::ios::app);  // Открываем файл в режиме добавления
    if (file.is_open()) {  // Проверка успешного открытия
        file << playerName_1 << " - " << score_1 << " " << playerName_2 << " - " << score_2 << std::endl;  // Запись имени игрока и счёта
        file.close();  // Закрытие файла
    }
}



// Меню настроек игры: позволяет пользователю изменять параметры игры.
void settingsMenu(Config& config) {
    int choice = 0; // Текущий выбранный параметр меню.
    // Названия опций настроек.
    const char* options[] = {
        "Change speed",
        "Change max score",
        "Change field width",
        "Change field height",
        "Change paddle height",
        "Change name Player 1",
        "Change name Player 2",
        "Save and exit"
    };
    int num_options = sizeof(options) / sizeof(options[0]); // Количество опций.

    while (true) {
        erase(); // Очистка экрана перед перерисовкой.
        mvprintw(1, 1, "Game settings:"); // Заголовок для меню настроек.
        
        // Отображение опций меню, подсвечивание текущей опции.
        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                attron(A_REVERSE); // Включить выделение для выбранной опции.
                mvprintw(3 + i, 5, "%s", options[i]);
                attroff(A_REVERSE); // Отключить выделение.
            } else {
                mvprintw(3 + i, 5, "%s", options[i]);
            }
        }

        int input = getch(); // Ожидание ввода от пользователя.
        char buffer[50];
        // Управление выбором в меню с помощью клавиш.
        switch (input) {
            case KEY_UP: // Переключение вверх.
                choice = (choice - 1 + num_options) % num_options;
                break;
            case KEY_DOWN: // Переключение вниз.
                choice = (choice + 1) % num_options;
                break;
            case 10: // Нажатие Enter для выбора опции.
                switch (choice) {
                    case 0: // Изменение скорости.
                        mvprintw(10, 5, "Enter new speed (e.g., 1.0): ");
                        echo(); // Режим отображения пользовательских символов on
                        scanw("%f", &config.speed);
                        noecho(); // off
                        break;
                    case 1: // Изменение максимального счета.
                        mvprintw(10, 5, "Enter max score: ");
                        echo();
                        scanw("%d", &config.max_score);
                        noecho();
                        break;
                    case 2: // Изменение ширины поля.
                        mvprintw(10, 5, "Enter field width: ");
                        echo();
                        scanw("%d", &config.field_width);
                        noecho();
                        break;
                    case 3: // Изменение высоты поля.
                        mvprintw(10, 5, "Enter field height: ");
                        echo();
                        scanw("%d", &config.field_height);
                        noecho();
                        break;
                    case 4: // Изменение высоты ракетки.
                        mvprintw(10, 5, "Enter paddle height: ");
                        echo();
                        scanw("%d", &config.paddle_height);
                        noecho();
                        break;
                    case 5: // Изменение игрока 1.
                        mvprintw(10, 5, "Enter name: ");
                        echo();
                        getstr(buffer);
                        config.name_Player1 = buffer;
                        noecho();
                        break;
                    case 6: // Изменение высоты ракетки.
                        mvprintw(10, 5, "Enter name: ");
                        echo();
                        getstr(buffer);
                        config.name_Player2 = buffer;
                        noecho();
                        break;
                    case 7: // Сохранение настроек и выход из меню.
                        saveConfig("config.ini", config);
                        return;
                }
        }
    }
}

// Функция отображения результатов, сохраненных в файле.
void showResults() {
    int ch;
    while (true) {
        erase(); // Очистка экрана перед перерисовкой.

        std::ifstream file("scores.txt");
        if (file.is_open()) {
            std::string line;
            int i = 0;
            while (std::getline(file, line)) {
                mvprintw(1 + i, 5, "%s", line.c_str()); // Показ каждой строки результата.
                i++;
            }
            file.close();

            mvprintw(i + 2, 5, "Press 'Q' to return to menu."); // Инструкция для возврата.
            refresh(); // Обновление экрана для отображения текста.
        } else {
            mvprintw(1, 5, "Error: Unable to open scores file.");
            mvprintw(3, 5, "Press 'Q' to return to menu.");
        }

        // Ожидание ввода для выхода из меню.
        ch = getch(); // Ввод с клавиатуры
        if (ch == 'q' || ch == 'Q') {
            break;
        }
    }
}

// Главное меню игры, позволяющее выбрать режим игры и настройки.
int showMenu() {
    initColors();
    int choice = 0;
    const char *menu[] = {
        "Play: Player vs Player",
        "Play: Player vs Computer",
        "Play: Player vs Wall",
        "Show Results",
        "Settings",
        "Exit"
    };
    int num_options = sizeof(menu) / sizeof(menu[0]);
    int max_width = COLS;

    while (true) {
        erase();
        attron(COLOR_PAIR(1));
        centeredPrint(1, "PONG - Main Menu", max_width);
        centeredPrint(2, "License GPL, see README.md", max_width);
        attroff(COLOR_PAIR(1));

        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                attron(A_REVERSE | COLOR_PAIR(2));
                centeredPrint(4 + i, menu[i], max_width);
                attroff(A_REVERSE | COLOR_PAIR(2));
            } else {
                attron(COLOR_PAIR(1));
                centeredPrint(4 + i, menu[i], max_width);
                attroff(COLOR_PAIR(1));
            }
        }

        int input = getch();
        switch (input) {
            case KEY_UP:
                choice = (choice - 1 + num_options) % num_options;
                break;
            case KEY_DOWN:
                choice = (choice + 1) % num_options;
                break;
            case 10: // Enter
                return choice;
        }
    }
}


void gameLoop(Config& config, int gameMode) {
    int player1Score = 0, player2Score = 0;

    // Инициализация объектов Paddle и Ball с новыми аргументами, если они изменились
    Paddle player1(1, config.field_height / 2 - config.paddle_height / 2, config.paddle_height);
    Paddle player2(config.field_width - 2, config.field_height / 2 - config.paddle_height / 2, config.paddle_height);
    
    Ball ball(config.field_width / 2, config.field_height / 2);

    bool isRunning = true;
    nodelay(stdscr, TRUE);  // Включение неблокирующего режима.
    timeout(100 / config.speed); // Установка скорости игры.

    while (isRunning) {
        erase(); // Очистка экрана.

        // Рисование границ поля.
        for (int i = 0; i < config.field_width; i++) {
            mvprintw(0, i, "-");
            mvprintw(config.field_height - 1, i, "-");
        }
        if (gameMode == 3) {
            for (int i = 1; i < config.field_height - 1; i++) {
                mvprintw(i, 1, "|");
            }
        }

        // Обновление позиции и проверка столкновений мяча
        ball.move();
        ball.bounce(config.field_height);  // Удар о стены сверху и снизу
        player1.draw();
        player2.draw();
        ball.draw();

        // Отображение счёта
        mvprintw(1, config.field_width / 2 - 5, "Score: %d | %d", player1Score, player2Score);

        // Отображение имени игрока 1
        mvprintw(2, 0, "%s", config.name_Player1.c_str());

        // Логика для отображения имени игрока 2 или "Computer" в зависимости от режима
        if (gameMode == 1) {
            mvprintw(2, config.field_width - size(config.name_Player2)-4, "%s", config.name_Player2.c_str());
        } else if (gameMode == 2) {
            mvprintw(2, config.field_width - 12, "Computer");
        }

        int ch = getch();
        switch (ch) {
            case 'w': player1.moveUp(); break;
            case 's': player1.moveDown(config.field_height); break;
            case KEY_UP: player2.moveUp(); break;
            case KEY_DOWN: player2.moveDown(config.field_height); break;
            case 'q': isRunning = false; break;
        }

        // Логика движения компьютера в режиме игрока против компьютера
        if (gameMode == 2) {
            if (ball.getY() < player2.getY()) player2.moveUp();
            if (ball.getY() > player2.getY() + player2.getHeight()) player2.moveDown(config.field_height);
        }

        // Логика игры против стены
        if (gameMode == 3) {
            if (ball.getX() >= config.field_width - 1) {
                player1Score++;
                ball.reset(config.field_width / 2, config.field_height / 2);
                ball.invertXDirection();
            }
            if (ball.checkPaddleCollision(player1)) {
                ball.invertXDirection();
            }
            if (ball.getX() <= 2) {
                ball.invertXDirection();
            }
        }

        // Проверка на столкновение с ракетками в режиме двух игроков
        if (ball.checkPaddleCollision(player1) || ball.checkPaddleCollision(player2)) {
            ball.invertXDirection();
        }

        // Проверка на выход мяча за границу поля и добавление очков
        if (ball.outOfBounds(config.field_width) && gameMode != 3) {
            if (ball.getX() <= 0) player2Score++;
            if (ball.getX() >= config.field_width - 1) player1Score++;
            ball.reset(config.field_width / 2, config.field_height / 2);
        }

        // Проверка на завершение игры
        if (player1Score >= config.max_score || player2Score >= config.max_score) {
            isRunning = false;
        }

        refresh();  // Обновление экрана
    }
    
    // Сохранение счёта по завершении игры
    if (gameMode == 1) {
        saveScore(config.name_Player1, player1Score, config.name_Player2, player2Score);
    }
    if (gameMode == 2) {
        saveScore(config.name_Player1, player1Score, "Computer", player2Score);
    }
}


// Основная функция, инициализирующая ncurses и запускающая главное меню.
int main() {
    initscr(); // инициализация
    noecho(); // Символы минус
    curs_set(FALSE); // Курсор минус
    keypad(stdscr, TRUE); // Поддержка функциональных клавиш

    Config config = loadConfig("config.ini");

    bool isRunning = true;
    while (isRunning) {
        int choice = showMenu();
        switch (choice) {
            case 0:
                gameLoop(config, 1);
                break;
            case 1:
                gameLoop(config, 2);
                break;
            case 2:
                gameLoop(config, 3);
                break;
            case 3:
                showResults();
                break;
            case 4:
                settingsMenu(config);
                break;
            case 5:
                isRunning = false;
                break;
        }
    }
    endwin();
    return 0;
}