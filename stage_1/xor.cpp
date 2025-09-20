#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <fstream>

using namespace std;

vector<unsigned char> xorCrypt(const vector<unsigned char>& data, const string& key) {
	vector<unsigned char> result;
	result.reserve(data.size()); // Резервируем память для оптимизации

	if (key.empty()) { return data; };

	for (size_t  i = 0; i < data.size(); ++i) {
		unsigned char key_byte = key[i % key.size()];
		result.push_back(data[i] ^ key_byte);
	}

	return result;
}

vector<unsigned char> readFile(const string& filePath) {
	ifstream file(filePath, ios::binary);
	if (!file.is_open()) {
		cout << "Failed to open file: " << filePath << endl;
		return {};
	}

	file.seekg(0, ios::end);
	streamsize file_size = file.tellg();
	file.seekg(0, ios::beg);
	vector<unsigned char> file_data(file_size);

	if (!file.read(reinterpret_cast<char*>(file_data.data()), file_size)) {
		cout << "Failed to read file: " << filePath << endl;
		return {};
	}

	return file_data;
}

void writeFile(const string& filePatch, const vector<unsigned char>& data) {
	ofstream file(filePatch, ios::binary);
	file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

int main() {
	string intputFilePatch, outputFilePatch, key;

	/*std::cout << "Enter input file patch: ";
	std::cin >> intputFilePatch;
	std::cout << "Enter output file patch: ";
	std::cin >> outputFilePatch;
	std::cout << "Enter key: ";
	std::cin.ignore();
	std::getline(std::cin, key);*/
	intputFilePatch = "testFiles/test2.txt";
	outputFilePatch = "testFiles/test1.txt";
	key = "abc";

	try {
		auto file_data = readFile(intputFilePatch);
		auto result_data = xorCrypt(file_data, key);
		writeFile(outputFilePatch, result_data);

		cout << "Operation completed successfully!" << endl;
		cout << "Remember: to decrypt, run the program again on the encrypted file with the SAME key." << endl;
	} catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;
		return 1;
	}
	return 0;
}

/*RU:
Логика программы нуждается в доработке!
Данный псевдокод написан, чтобы понять логику алгоритма
Принцым работы алгоритма:

Алгоритм работает с помощью операции xor. Она проверяет выражение х-байт != у-байту. Если прадва - 1, если нет - 0
Сначал мы создаем веркто с unsigned символами, тк байты не могут быть отрицательными
Дальше мы резервируем память для этого вектора ради оптимизации
Потом, прозодимся по всему вектору и добавляем в конечный вектор результат xor'a элемента вектора и элемента ключа с номером объекта вектора, поделенного на длину ключа(это надо чтобы ключ был цикличным)

Если применить xor к двум символам, то компьютер возьмет бинарный вид их ASCII кодов и также побайтово сравнит их. Хor может быть применен к любым файлам, тк алгоритм работает с байтами.
*/