#include <iostream>
#include <array>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

#include "mpi.h"

#define DataType MPI_INT
#define ControlType MPI_UNSIGNED

using namespace std;

/*
* Id процесса с рангом 0
*/
constexpr int ROOT_RANK = 0;

/*
* Идентификаторы сообщений для MPI_Send и MPI_Recv
* Идентификатор от 0 - 1, сообщает процессу что он получил, команду или данные
*/

enum class Tags : int { // Перечисление
	CONTROL,
	DATA,
};

/*
* Управляющие команды/сигналы для дочерних процессов, что работают с FIFO
*/

enum class Command : unsigned {  // Перечисление
	PUSH, // Положить в буфер
	GET, // Получить из буфера
	POP, // Удалить из буфера
	STOP, // Остановить работу с буфером. Завершить работу цикла
};

using value_type = int;
//constexpr MPI_Datatype DataType = MPI_INT; // Ссылки на типы из библиотеки MPI
//constexpr MPI_Datatype ControlType = MPI_UNSIGNED; // Ссылки на типы из библиотеки MPI

/*
* Реализация FIFO по типу очереди
* Первый вошёл – первый вышел
*/

class MPIQueue {


	unsigned it_ = 0; // С какого места начинаем писать в очередь
	unsigned size_ = 0; // Размер уже записанного
	const unsigned capacity_; // Размер FIFO. Зависит от количества процессов или процессоров

public:

	/*
	*
	* Конструктор класса.
	* Получает от процесса 0, общее количество процессов
	*/
	MPIQueue(unsigned capacity) : capacity_(capacity) {}

	/*
	* Послать всем дочерним процессам сигнал записи и данные
	*/
	bool push(const value_type &val)
	{
		if (size_ == capacity_)
			return false;
		
		unsigned pos = (it_ + size_) % capacity_;

		Command cmd(Command::PUSH);
		MPI_Send(&cmd, 1, ControlType, pos + 1, int(Tags::CONTROL), MPI_COMM_WORLD);
		MPI_Send(&val, 1, DataType, pos + 1, int(Tags::DATA), MPI_COMM_WORLD);

		++size_;
		return true;
	}

	/*
	* Послать всем дочерним процессам сигнал на получение данных
	* Собёрем что у них есть
	*/
	bool get(value_type &val) const
	{
		if (size_ == 0)
			return false;

		Command cmd(Command::GET);
		MPI_Send(&cmd, 1, ControlType, it_ + 1, int(Tags::CONTROL), MPI_COMM_WORLD);
		MPI_Recv(&val, 1, DataType, it_ + 1, int(Tags::DATA), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		return true;
	}

	/*
	* Послать всем дочерним процессам сигнал на удаление данных из FIFO
	*/
	bool pop()
	{
		if (size_ == 0)
			return false;


		Command cmd(Command::POP);
		MPI_Send(&cmd, 1, ControlType, it_ + 1, int(Tags::CONTROL), MPI_COMM_WORLD);

		it_ = (it_ + 1) % capacity_;
		--size_;
		return true;
	}

};

/*
* Процесс с рангом 0
* Управляет всеми остальными процессами
*/

void master(int argc, char* argv[])
{
	// push to max, pop to 0, check data

	/*
	for (;;) {
		char cmd;
		std::cin >> cmd;
		switch (cmd) {
		case '+':
		case '-':
		case '=':
		case 'q':
			return;
		default:
			std::cout << "unexpected command\n";
		};
	}
	*/

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size); // Функция определения числа процессов в области связи MPI_Comm_size

	--size; // Минус процесс с рангом 0

	MPIQueue queue(size); // Обёртка FIFO

	value_type val = 0;

	/*
	* Тест
	* 
	* Неудачная попытка получить значение из пустого FIFO
	*/
	if (queue.pop() || queue.get(val)) {
		std::cerr << "get from empty queue";
		return;
	}


	while (queue.push(val++)) {} // Заполним буфер int

	for (unsigned i = 0; i < size; ++i) {

		/*
		* Тест 2
		*
		* Попытка получить значение из полного FIFO
		*/

		if (!queue.get(val) || !queue.pop()) {
			std::cerr << "get from full queue";
			return;
		}

		/*
		* Тест 3
		*
		* Полученный результат не соответствует ожиданию
		*/

		if (i != val) {
			std::cerr << "get wrong: " << val << "expected " << i;
			return;
		}
	}
}

/*
* Процессы с рангом > 0
*/
void slave(MPI_Status& status)
{
	value_type storage;

	bool full = false; // Флаг если очередь полна

	/*
	* Циклический буфер
	*/
	for (;;) {
		Command cmd;

		/*
		* Процесс ожидает сигнал от мастера на прерывание цикла
		*/
		if (MPI_Recv(&cmd, 1, ControlType, ROOT_RANK, int(Tags::CONTROL), MPI_COMM_WORLD, &status)) {
			return;
		}

		switch (cmd) {

			/*
			* Получить данные от мастера и поместить их в очередь
			*/
			case Command::PUSH:
				/* 
				* Тест 4
				* Что-то сделать если очередь заполнилась
				*/
				if (full) {
					// this is controlled in stack
				}
				full = true;
				if (MPI_Recv(&storage, 1, DataType, ROOT_RANK, int(Tags::DATA), MPI_COMM_WORLD, &status))
					return;
				std::cout << "push " << storage << std::endl;
				break;

			/*
			* Отправить данные мастеру
			*/
			case Command::GET:
				if (MPI_Send(&storage, 1, DataType, ROOT_RANK, int(Tags::DATA), MPI_COMM_WORLD)) {
					return;
				}
				std::cout << "get " << storage << std::endl;
				break;

			/*
			* Очистить очередь
			*/
			case Command::POP:
				/*
				* Тест 5
				* Что-то сделать если очередь пуста
				*/
				if(!full) {
					// this is controlled in stack
				}
				full = false;
				std::cout << "pop " << storage << std::endl;
				break;

			case Command::STOP:
				return;
		}
	}
}
int main(int argc, char* argv[]) {
	int rank;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Если это процесс с 0 рангом
	if (rank == 0) {
		master(argc, argv);

		Command cmd(Command::STOP);
		for(int i = 1; i < size; ++i)
			MPI_Send(&cmd, 1, ControlType, i, int(Tags::CONTROL), MPI_COMM_WORLD);

	} else {
		slave(status);
	}

	MPI_Finalize();

	return 0;
}