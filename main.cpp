#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

// Структура для хранения настроек игры
struct Config {
    float speed;
    int max_score;
    int field_width;
    int field_height;
    int paddle_height;
};

// Функция для загрузки конфигурации из файла config.ini
Config loadConfig(const std::string& fileName) {
    Config config;
    std::ifstream file(fileName);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("speed") != std::string::npos) config.speed = std::stof(line.substr(line.find("=") + 1));
            if (line.find("max_score") != std::string::npos) config.max_score = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("field_width") != std::string::npos) config.field_width = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("field_height") != std::string::npos) config.field_height = std::stoi(line.substr(line.find("=") + 1));
            if (line.find("paddle_height") != std::string::npos) config.paddle_height = std::stoi(line.substr(line.find("=") + 1));
        }
        file.close();
    }
    return config;
}

// Функция для сохранения конфигурации
void saveConfig(const std::string& fileName, Config& config) {
    std::ofstream file(fileName);
    if (file.is_open()) {
        file << "speed = " << config.speed << std::endl;
        file << "max_score = " << config.max_score << std::endl;
        file << "field_width = " << config.field_width << std::endl;
        file << "field_height = " << config.field_height << std::endl;
        file << "paddle_height = " << config.paddle_height << std::endl;
        file.close();
    }
}

// Класс для управления ракеткой
class Paddle {
public:
    int x, y, height;
    Paddle(int x, int y, int height) : x(x), y(y), height(height) {}
    
    void moveUp() {
        if (y > 1) y--; // Ограничение сверху
    }

    void moveDown(int field_height) {
        if (y + height < field_height - 1) y++; // Ограничение снизу
    }

    void draw() {
        for (int i = 0; i < height; i++) {
            mvprintw(y + i, x, "|");
        }
    }
};

// Класс для управления мячом
class Ball {
public:
    int x, y;
    int dx, dy;
    Ball(int x, int y) : x(x), y(y), dx(1), dy(1) {}

    void move() {
        x += dx;
        y += dy;
    }

    void bounce(int field_height) {
        if (y <= 1 || y >= field_height - 2) {
            dy = -dy; // Отскок от верхней и нижней стенки
        }
    }

    void draw() {
        mvprintw(y, x, "O");
    }

    bool checkPaddleCollision(Paddle& paddle) {
        return (x == paddle.x && y >= paddle.y && y < paddle.y + paddle.height);
    }

    bool outOfBounds(int field_width) {
        return x <= 0 || x >= field_width - 1;
    }

    void reset(int startX, int startY) {
        x = startX;
        y = startY;
        dx = -dx; // Меняем направление после гола
    }
};

// Функция для сохранения результатов в файл scores.txt
void saveScore(const std::string& playerName, int score) {
    std::ofstream file("scores.txt", std::ios::app);
    if (file.is_open()) {
        file << playerName << " - " << score << std::endl;
        file.close();
    }
}

void settingsMenu(Config& config) {
    int choice = 0;
    const char* options[] = {
        "Change speed",
        "Change max score",
        "Change field width",
        "Change field height",
        "Change paddle height",
        "Save and exit"
    };
    int num_options = sizeof(options) / sizeof(options[0]);
    
    while (true) {
        erase();
        mvprintw(1, 1, "Game settings:");
        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                attron(A_REVERSE);
                mvprintw(3 + i, 5, "%s", options[i]);  // Используем формат "%s"
                attroff(A_REVERSE);
            } else {
                mvprintw(3 + i, 5, "%s", options[i]);  // Используем формат "%s"
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
                switch (choice) {
                    case 0: // Change speed
                        mvprintw(10, 5, "Enter new speed (e.g., 1.0): ");
                        echo();
                        scanw("%f", &config.speed);
                        noecho();
                        break;
                    case 1: // Change max score
                        mvprintw(10, 5, "Enter max score: ");
                        echo();
                        scanw("%d", &config.max_score);
                        noecho();
                        break;
                    case 2: // Change field width
                        mvprintw(10, 5, "Enter field width: ");
                        echo();
                        scanw("%d", &config.field_width);
                        noecho();
                        break;
                    case 3: // Change field height
                        mvprintw(10, 5, "Enter field height: ");
                        echo();
                        scanw("%d", &config.field_height);
                        noecho();
                        break;
                    case 4: // Change paddle height
                        mvprintw(10, 5, "Enter paddle height: ");
                        echo();
                        scanw("%d", &config.paddle_height);
                        noecho();
                        break;
                    case 5: // Save and exit
                        saveConfig("config.ini", config);
                        return;
                }
        }
    }
}

void showResults() {
    int ch;

    while (true) {
        erase();  // Очищаем экран

        std::ifstream file("scores.txt");
        if (file.is_open()) {
            std::string line;
            int i = 0;  // Начинаем с 0, чтобы корректно выводить строки на экране
            while (std::getline(file, line)) {
                mvprintw(1 + i, 5, "%s", line.c_str());  // Выводим каждую строку с отступом в 5 символов
                i++;
            }
            file.close();

            mvprintw(i + 2, 5, "Press 'Q' to return to menu.");  // Инструкция для возврата
            refresh();  // Обновляем экран для отображения
        } else {
            mvprintw(1, 5, "Error: Unable to open scores file.");
            mvprintw(3, 5, "Press 'Q' to return to menu.");
        }

        // Ожидание нажатия клавиши
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;  // Выходим из цикла и возвращаемся в главное меню
        }
    }
}


int showMenu() {
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

    while (true) {
        erase();
        mvprintw(1, 1, "PONG - Main Menu:");
        mvprintw(2, 1, "License GPL, see README.md");

        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                attron(A_REVERSE);
                mvprintw(4 + i, 5, "%s", menu[i]);  // Используем формат "%s"
                attroff(A_REVERSE);
            } else {
                mvprintw(4 + i, 5, "%s", menu[i]);  // Используем формат "%s"
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


// Основная функция игры
void gameLoop(Config& config, int gameMode) {
    int player1Score = 0, player2Score = 0;

    Paddle player1(1, config.field_height / 2 - config.paddle_height / 2, config.paddle_height);
    Paddle player2(config.field_width - 2, config.field_height / 2 - config.paddle_height / 2, config.paddle_height);
    Ball ball(config.field_width / 2, config.field_height / 2);

    bool isRunning = true;
    nodelay(stdscr, TRUE);  // Включаем неблокирующий режим для чтения клавиш
    timeout(100 / config.speed); // Скорость игры
    
    while (isRunning) {
        // Очищаем экран для перерисовки
        erase();
        
        // Отрисовка верхней и нижней границы поля
        for (int i = 0; i < config.field_width; i++) {
            mvprintw(0, i, "-");
            mvprintw(config.field_height - 1, i, "-");
        }

        // Отрисовка объектов: мяч, ракетки, счет
        ball.move();
        ball.bounce(config.field_height);
        player1.draw();
        player2.draw();
        ball.draw();

        mvprintw(1, config.field_width / 2 - 5, "Score: %d | %d", player1Score, player2Score);

        // Управление игроками
        int ch = getch();
        switch (ch) {
            case 'w': player1.moveUp(); break;
            case 's': player1.moveDown(config.field_height); break;
            case KEY_UP: player2.moveUp(); break;
            case KEY_DOWN: player2.moveDown(config.field_height); break;
            case 'q': isRunning = false; break; // Выход из игры
        }

        // Логика компьютера в режиме "Игрок против компьютера"
        if (gameMode == 2) {
            if (ball.y < player2.y) player2.moveUp();
            if (ball.y > player2.y + player2.height) player2.moveDown(config.field_height);
        }

        // Логика игры против стены
        if (gameMode == 3) {
            // Если мяч достигает правой границы (игрок пропустил)
            if (ball.x >= config.field_width - 1) {
                player1Score++;  // Увеличиваем счёт
                ball.reset(config.field_width / 2, config.field_height / 2);  // Перезапускаем мяч
                ball.dx = -ball.dx;  // Направляем мяч обратно к игроку
            }

            // Проверка столкновения с ракеткой
            if (ball.checkPaddleCollision(player1)) {
                ball.dx = -ball.dx;  // Меняем направление мяча
            }

            // Если мяч пересекает левую границу (игрок отбил успешно)
            if (ball.x <= 0) {
                ball.dx = -ball.dx;  // Меняем направление мяча
            }
        }

        // Проверка столкновения с ракетками
        if (ball.checkPaddleCollision(player1) || ball.checkPaddleCollision(player2)) {
            ball.dx = -ball.dx; // Меняем направление мяча
        }

        // Проверка на голы
        if (ball.outOfBounds(config.field_width) && gameMode != 3) {
            if (ball.x <= 0) player2Score++;
            if (ball.x >= config.field_width - 1) player1Score++;

            ball.reset(config.field_width / 2, config.field_height / 2); // Перезапуск мяча после гола
        }

        // Проверка на конец игры
        if (player1Score >= config.max_score || player2Score >= config.max_score) {
            isRunning = false;
        }

        refresh(); // Обновляем экран
    }

    // Сохранение результата в таблицу рекордов
    saveScore("Player1", player1Score);
    saveScore("Player2", player2Score);
}

int main() {
    // Инициализация ncurses
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    // Загрузка конфигурации
    Config config = loadConfig("config.ini");

    // Главное меню
    bool isRunning = true;
    while (isRunning) {
        int choice = showMenu();
        switch (choice) {
            case 0: // Играть: Игрок против игрока
                gameLoop(config, 1);
                break;
            case 1: // Играть: Игрок против компьютера
                gameLoop(config, 2);
                break;
            case 2: // Играть: Игрок против стены
                gameLoop(config, 3);
                break;
            case 3:
                showResults();
                break;
            case 4: // Настройки
                settingsMenu(config);
                break;
            case 5: // Выход
                isRunning = false;
                break;
        }
    }
    // Завершаем ncurses
    endwin();
    return 0;
}
