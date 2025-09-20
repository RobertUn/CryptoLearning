#include <iostream>
#include <string>
#include <cctype>

using namespace std;

char encryptChar(char c, int key) {
    if (!isalpha(c)) {
        return c;
    }
    char base = isupper(c) ? 'A' : 'a';
    return static_cast<char>((c - base + key) % 26 + base);
}

char decryptChar(char c, int key) {
    if (!isalpha(c)) {
        return c;
    }
    char base = isupper(c) ? 'A' : 'a';
    // Исправление для отрицательных чисел
    return static_cast<char>((c - base - key + 26) % 26 + base);
}

string encryptString(const string& text, int key) {
    string result;
    for (char c : text) {
        result += encryptChar(c, key);
    }
    return result;
}

string decryptString(const string& text, int key) {
    string result;
    for (char c : text) {
        result += decryptChar(c, key);
    }
    return result;
}

int main() {
    string inputText;
    string modeCrypt;
    int key;

    cout << "WARNING!!! THIS PROGRAM CAN WORK WITH ONLY ENGLISH TEXT" << endl << endl;
    cout << "Enter text: ";
    getline(cin, inputText);

    cout << "Enter key (integer number): ";
    cin >> key;

    cout << "Choose mode ('encrypt' or 'decrypt'): ";
    cin >> modeCrypt;

    string outputText;

    if (modeCrypt == "encrypt") {
        outputText = encryptString(inputText, key);
    }
    else if (modeCrypt == "decrypt") {
        outputText = decryptString(inputText, key);
    }
    else {
        cout << "Incorrect mode! Using encryption by default." << endl;
        outputText = encryptString(inputText, key);
    }

    cout << "Result: " << outputText << endl;
    return 0;
}

/* RU:
Логика режимов нуждается в доработке!
Данный псевдокод написан, чтобы понять логику алгоритма
Принцым работы алгоритма:

Алгоритм работает с помощью ASCII win-1251
Мы получаем символ
Потом мы определяем его регист. Елси нижний - "а", если верхний - "А"
Дальше мы отнимает из кода ASCII win-1251 изначального символа код ASCII win-1251 его регистра. Таким образом мы получаем его порядковый номер в алфавите - 1(это потом компенсируется)
Дальше мы либо прибовляем, либо отнимает ключ шифрования в зависимости от режима(шифрование - "+", дешифрование - "-")
Потом от получившегося числа нам нужет остаток деления на длинну алфавита, чтобы алгоритм был цикличным. На данном шаге мы получим порядковый номер символа алфавита, но его сдвиг от изначального символа будет не равен ключу, а будет равен ключу - 1(из-за причиты описанной выше)
В конце мы прибовляем код ASCII win-1251 регистра, чтобы получить код ASCII win-1251 символа со сдвигом равным изначальному ключу, на этом шаге мы компенсируем ту еденичку, которую потеряли в начале

Примечание:
При Дешифровке на моменте вычитания ключа нужно прибавить длинну алфавита. Это никак не повлияет на работу основного процесса, так как при получение остатка это 26 все равно уничтожится. Однако, это необходимо, потомучто первые символы алфавита при дешивровке могут дать отрицательное число и мы получим не верный результат(или вообще краш программы)
*/

/* ENG:
The mode logic needs refinement!
This pseudocode is written to understand the algorithm's logic.
The algorithm's operating principle:

The algorithm works using the ASCII win-1251 encoding.
We get a character.
Then we determine its case. If it's lowercase - use "a", if it's uppercase - use "A".
Next, we subtract the ASCII win-1251 code of its base case letter from the ASCII win-1251 code of the original character. This gives us its ordinal number in the alphabet - 1 (this is compensated for later).
Then, depending on the mode (encryption - "+", decryption - "-"), we either add or subtract the encryption key.
Then we need the modulus (remainder) of the division of the resulting number by the length of the alphabet to make the algorithm cyclical. At this step, we get the ordinal number of the alphabet character, but its shift from the original character will not be equal to the key, but will be equal to key - 1 (due to the reason described above).
Finally, we add the ASCII win-1251 code of the base case letter to get the ASCII win-1251 code of the shifted character, which is equal to the original key. At this step, we compensate for that unit (the 1) that was lost at the beginning.

Note:
During decryption, at the step of subtracting the key, it's necessary to add the length of the alphabet. This does not affect the main process, since when taking the modulus, this 26 will be eliminated anyway. However, this is necessary because the first characters of the alphabet during decryption can yield a negative number, and we would get an incorrect result (or even a program crash).
*/
