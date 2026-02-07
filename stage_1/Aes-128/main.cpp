#include "FileProcess.cpp"
#include "FileUtils.cpp"


int main() {
	// Настройка кодировки
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);
	setlocale(LC_ALL, "Russian");

	AESEncryptor aes128encrypt;
	Block key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	Block plaintext = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	aes128encrypt.setKey(key);
	aes128encrypt.encryptBlock(plaintext);

	AESDecryptor aes128decrypt;
	Block ciphertext = { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a };
	aes128decrypt.setKey(key);
	aes128decrypt.decryptBlock(ciphertext);

	string file_path = "1.txt";

	Test ts;
	ts.enc("1.txt", "2.txt", key);
	//ts.dec("2.txt", "1.txt", key);

	Backup bk("backups");
	bk.copyFile("1.txt");
	string a = bk.getLastCopiedPath();
	cout << a << endl;

	FileName fl;
	string d = fl.addSuffix("123.txt");
	cout << d << endl;

}