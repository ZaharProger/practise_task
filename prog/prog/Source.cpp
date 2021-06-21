#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include <Windows.h>

//Звено списка == точка доступа
struct Host
{
	float x{ 0.0 }, y{ 0.0 };
	char is_active{ '0' };
	float signal_power{ 0.0 };
	sf::CircleShape active_distance{ 0.f };
	Host* next{ nullptr };
};

//Основа однонаправленного списка
struct List
{
	short N{ 0 };
	Host* begin{ nullptr };
};

//Проверка введенных длины и ширины холла по ОДЗ (length & width > 0)
void size_check(short&, short&, const short&);

//Проверка введенного радиуса покрытия по ОДЗ (radius > 0)
void radius_check(short&, const short&);

//Проверка введенной команды по ОДЗ (всего 2 команды под номерами 1 и 2)
void decide_check(short&, const short&, const short&);

//Расстановка точек доступа случайным образом
void set_wifi_random(List&, const short&, const short&);

//Расстановка точек доступа вручную
void enter_wifi(List&, const short&, const short&);

//Проверка координат точки, полученных случайным образом (точка должна лежать в плоскости холла)
void random_coords_check(float&, float&, const short&, const short&);

//Проверка координат точки, полученных от пользователя (точка должна лежать в плоскости холла)
void entered_coords_check(float&, float&, const short&, const short&);

//Формирование списка точек доступа
void add_to_list(List&, Host*);

//Поиск активных точек доступа в заданной точке холла (такие точки отделяются от остальных путём изменения значения переменной is_active)
void find_active_wifi(List&, const float&, const float&, const short&);

//Вычисление силы сигнала для отобранных точек
void calculate_signal_power(List&, const float&, const float&, const short&);

//Сортировка списка по убыванию силы сигнала для удобного вывода в консоль
void sort(List&);

//Вывод в консоль активных точек доступа с полной информацией о них
void print_active_wifi(List&);

//Отрисовка карты холла
void show_map(List&, const float&, const float&, const short&, const short&, const short&);

//Получение информации о точке доступа
void get_info(Host*);

int main()
{
	setlocale(0, "");
	srand(unsigned(time(NULL)));

	//Ввод длины и ширины холла
	short length, width;
	size_check(length, width, 0);

	//Ввод радиуса покрытия
	short radius;
	radius_check(radius, 0);

	//Выбор способа расстановки точек доступа
	std::cout << "Введите 1 для расстановки wifi точек вручную, 2 для случайной расстановки: ";
	short decide;
	decide_check(decide, 1, 2);

	//Расстановка точек доступа и формирование списка точек доступа
	List list;
	if (decide == 1)
		enter_wifi(list, length, width);
	else
		set_wifi_random(list, length, width);

	//Ввод координат произвольной точки, откуда будет вычисляться сила сигнала
	float x, y;
	entered_coords_check(x, y, length, width);

	//Поиск активных точек доступа с целью выделения их от остальных
	find_active_wifi(list, x, y, radius);

	//Вычисление силы сигнала для отобранных точек
	calculate_signal_power(list, x, y, radius);

	//Сортировка списка по убыванию силы сигнала
	sort(list);

	//Форматированный вывод информации об активных точках доступа
	print_active_wifi(list);

	//Запрос на отрисовку карты холла для наглядного представления найденных точек
	std::cout << "Введите 1 для просмотра точек доступа на карте, 0 для выхода: ";
	decide_check(decide, 0, 1);
	if (decide == 1)
		show_map(list, x, y, length, width, radius);

	system("pause");
	return 0;
}

void size_check(short& n, short& m, const short& lim)
{
	do
	{
		std::cout << "Введите размеры холла(через пробел): ";
		std::cin >> n >> m;
		if (std::cin.fail())
			std::cin.clear();

		std::cin.ignore(32767, '\n');
		if (n <= 0 || m <= 0 || std::cin.fail())
			std::cout << "Длина и ширина холла должны быть больше 0!\n";

	} while (n <= 0 || m <= 0 || std::cin.fail());
}

void radius_check(short& r, const short& lim)
{
	do
	{
		std::cout << "Введите радиус покрытия: ";
		std::cin >> r;
		if (std::cin.fail())
			std::cin.clear();

		std::cin.ignore(32767, '\n');
		if (r <= 0 || std::cin.fail())
			std::cout << "Радиус покрытия должен быть больше 0!\n";

	} while (r <= 0 || std::cin.fail());
}

void decide_check(short& decide, const short& n, const short& m)
{
	do
	{
		std::cin >> decide;
		if (std::cin.fail())
			std::cin.clear();

		std::cin.ignore(32767, '\n');
		if (decide < n || decide > m || std::cin.fail())
			std::cout << "Команды под таким номером не существует!\n";

	} while (decide < n || decide > m || std::cin.fail());
}

void random_coords_check(float& x, float& y, const short& n, const short& m)
{
	do
	{
		short sign_decide{ 1 + rand() % 2 };

		if (sign_decide == 1)
		{
			x = static_cast<float>(rand() % 1000);
			y = static_cast<float>(rand() % 1000);
		}
		else
		{
			x = static_cast<float>(-(rand() % 1000));
			y = static_cast<float>(-(rand() % 1000));
		}

	} while (x < -n / 2 || x > n / 2 || y < -m / 2 || y > m / 2);
}

void entered_coords_check(float& x, float& y, const short& n, const short& m)
{
	do
	{
		std::cout << "Введите координаты точки(через пробел): ";
		std::cin >> x >> y;
		if (std::cin.fail())
			std::cin.clear();
		std::cin.ignore(32767, '\n');
		if (x < -n / 2 || x > n / 2 || y < -m / 2 || y > m / 2 || std::cin.fail())
			std::cout << "Точка расположена за пределами холла!\n";

	} while (x < -n / 2 || x > n / 2 || y < -m / 2 || y > m / 2 || std::cin.fail());
}

void set_wifi_random(List& list, const short& n, const short& m)
{
	short detector{ 1 };

	while (detector != 0 || list.N < 3)
	{
		Host* host{ new Host };
		random_coords_check(host->x, host->y, n, m);
		add_to_list(list, host);

		detector = rand() % 10;
	}

	std::cout << "Успешно создано " << list.N << " точек доступа!\n";
}

void enter_wifi(List& list, const short& n, const short& m)
{
	short decide;
	do
	{
		std::cout << "Должно быть расставлено не менее 3 точек!\n";
		std::cout << "Введите 0 для окончания создания точек: ";
		decide_check(decide, -32767, 32767);
		if (decide != 0)
		{
			Host* host{ new Host };
			entered_coords_check(host->x, host->y, n, m);
			add_to_list(list, host);
			std::cout << "Точка доступа успешно создана!\n";
		}
	} while (decide != 0 || list.N < 3);

	std::cout << "Успешно создано " << list.N << " точек доступа!\n";
}

void add_to_list(List& list, Host* host)
{
	if (list.N == 0)
	{
		host->next = nullptr;
		list.begin = host;
	}
	else
	{
		host->next = nullptr;
		Host* some_host{ list.begin };
		while (some_host->next != nullptr)
			some_host = some_host->next;
		some_host->next = host;
	}

	++list.N;
}

void find_active_wifi(List& list, const float& x, const float& y, const short& r)
{
	std::cout << "Поиск активных точек доступа...\n";
	Host* host{ list.begin };
	short k{ 0 };
	while (host != nullptr)
	{
		float vector_x{ host->x - x };
		float vector_y{ host->y - y };
		float distance{ sqrt(pow(vector_x, 2) + pow(vector_y, 2)) };

		if (distance < r)
		{
			++k;
			host->is_active = '1';
		}
			
		host = host->next;
	}

	std::cout << "Найдено " << k << " активных точек доступа!\n";
}

void sort(List& list)
{
	for (Host* host_1 = list.begin; host_1 != nullptr; host_1 = host_1->next)
		for (Host* host_2 = list.begin; host_2 != nullptr; host_2 = host_2->next)
			if (host_1->signal_power > host_2->signal_power)
			{
				std::swap(host_1->x, host_2->x);
				std::swap(host_1->y, host_2->y);
				std::swap(host_1->is_active, host_2->is_active);
				std::swap(host_1->signal_power, host_2->signal_power);
			}
}

void calculate_signal_power(List& list, const float& x, const float& y, const short& r)
{
	std::cout << "Определение силы сигнала в заданной точке холла...\n";
	Host* host{ list.begin };
	while (host != nullptr)
	{
		float vector_x{ host->x - x };
		float vector_y{ host->y - y };
		float distance{ sqrt(pow(vector_x, 2) + pow(vector_y, 2)) };

		if (host->is_active == '1')
			host->signal_power = 100 - (distance * 100 / r);

		host = host->next;
	}
}

void print_active_wifi(List& list)
{
	sort(list);

	std::cout << "--------------------------------------------\n";
	std::cout << "\t   АКТИВНЫЕ ТОЧКИ ДОСТУПА\n";
	std::cout << "--------------------------------------------\n";
	std::cout << "Координата X\t";
	std::cout << "Координата Y\t";
	std::cout << "Сила сигнала\n";
	std::cout << "--------------------------------------------\n";

	Host* host{ list.begin };
	while (host != nullptr)
	{
		if (host->is_active == '1')
		{
			std::cout << std::setw(7) << std::setprecision(1) << std::fixed << host->x;
			std::cout << std::setw(17) << std::setprecision(1) << std::fixed << host->y;
			std::cout << std::setw(15) << std::setprecision(2) << host->signal_power << "%\n";
			std::cout << "--------------------------------------------\n";
		}

		host = host->next;
	}
}

void show_map(List& list, const float& x, const float& y, const short& n, const short& m, const short& r)
{
	std::cout << "Запущена отрисовка карты холла...\n";
	Sleep(1000);
	std::cout << "Отрисовка успешно завершена!\n";
	std::cout << "----------------------------------------------\n";
	std::cout << "Обозначения:\n";
	std::cout << "Зеленые окружности - активные точки доступа;\n";
	std::cout << "Красные окружности - неактивные точки доступа;\n";
	std::cout << "Синяя окружность - заданная точка холла.\n";
	std::cout << "----------------------------------------------\n";
	std::cout << "Для просмотра информации о точке нажмите по ней левой клавишей мыши!\n";
	
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(n, m), "MAP", sf::Style::Close, settings);

	sf::RectangleShape center_line_x(sf::Vector2f(n, 3.f));
	center_line_x.setFillColor(sf::Color::Black);
	center_line_x.setOrigin(0.f, center_line_x.getSize().y / 2);
	center_line_x.setPosition(0.f, static_cast<float>(m / 2));

	sf::RectangleShape center_line_y(sf::Vector2f(3.f, m));
	center_line_y.setFillColor(sf::Color::Black);
	center_line_y.setOrigin(center_line_y.getSize().x / 2, 0.f);
	center_line_y.setPosition(static_cast<float>(n / 2), 0.f);

	sf::CircleShape point(10.f);
	point.setFillColor(sf::Color::Blue);
	point.setOrigin(10, 10);
	point.setPosition(static_cast<float>(n / 2 + x), static_cast<float>(m / 2 - y));

	Host* host{ list.begin };
	while (host != nullptr)
	{
		host->active_distance.setRadius(r);
		host->active_distance.setPointCount(100);
		host->active_distance.setOrigin(r, r);
		host->active_distance.setPosition(static_cast<float>(n / 2 + host->x), static_cast<float>(m / 2 - host->y));

		if (host->is_active == '1')
			host->active_distance.setFillColor(sf::Color(0, 255, 0, 80));
		else
			host->active_distance.setFillColor(sf::Color(255, 0, 0, 80));

		host = host->next;
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed)
				window.close();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			window.close();

		char detector = '0';
		window.clear(sf::Color(127, 199, 255));
		window.draw(center_line_x);
		window.draw(center_line_y);
		host = list.begin;
		while (host != nullptr)
		{
			int window_x{ sf::Mouse::getPosition(window).x };
			int window_y{ sf::Mouse::getPosition(window).y };

			if (host->active_distance.getGlobalBounds().contains(static_cast<float>(window_x), static_cast<float>(window_y)))
			{
				host->active_distance.setFillColor(sf::Color::Yellow);
				detector = '1';
			}
			else
			{
				if (host->is_active == '1')
					host->active_distance.setFillColor(sf::Color(0, 255, 0, 80));
				else
					host->active_distance.setFillColor(sf::Color(255, 0, 0, 80));

				detector = '0';
			}
				
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && detector == '1')
			{
				get_info(host);
				Sleep(1000);
			}
				

			window.draw(host->active_distance);
			host = host->next;
		}
		window.draw(point);
		window.display();
	}
}

void get_info(Host* host)
{
	std::cout << "--------------------------------------------\n";
	std::cout << "\t   ИНФОРМАЦИЯ О ТОЧКЕ ДОСТУПА\n";
	std::cout << "--------------------------------------------\n";
	std::cout << "Координата X\t";
	std::cout << "Координата Y\t";
	std::cout << "Сила сигнала\n";
	std::cout << "--------------------------------------------\n";

	std::cout << std::setw(7) << std::setprecision(1) << std::fixed << host->x;
	std::cout << std::setw(17) << std::setprecision(1) << std::fixed << host->y;
	std::cout << std::setw(15) << std::setprecision(2) << host->signal_power << "%\n";
	std::cout << "--------------------------------------------\n";
}

	
	