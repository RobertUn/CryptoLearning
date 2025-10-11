#include "Aes-128.cpp"

using namespace std;

AES_128 aes_128;

void DebugMode() {
    // Настройка кодировки
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");

    // 1. Исходные данные
    unsigned char plaintext[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    // Заполняем Key в column-major порядке!
    Key key = { {
        {0x00, 0x04, 0x08, 0x0c},  // столбец 0
        {0x01, 0x05, 0x09, 0x0d},  // столбец 1  
        {0x02, 0x06, 0x0a, 0x0e},  // столбец 2
        {0x03, 0x07, 0x0b, 0x0f}   // столбец 3
    } };

    cout << "Исходный текст: ";
    for (int i = 0; i < 16; i++) {
        cout << hex << setw(2) << setfill('0') << (int)plaintext[i] << " ";
    }
    cout << endl;

    cout << "Ключ: ";
    for (int i = 0; i < 16; i++) {
        cout << hex << setw(2) << setfill('0') << (int)((unsigned char*)&key)[i] << " ";
    }
    cout << endl;

    // 2. Расширяем ключ
    RoundsKeys roundKeys = KeyExpansion(key);

    cout << "Расширенные ключи:" << endl;
    for (int r = 0; r < 11; r++) {
        cout << "Round " << r << ": ";
        for (int i = 0; i < 16; i++) {
            cout << hex << setw(2) << setfill('0') << (int)roundKeys[r][i] << " ";
        }
        cout << endl;
    }

    // 3. Преобразуем plaintext в State
    State state;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = plaintext[i + 4 * j];
        }
    }

    cout << "Начальный State:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // 4. Начальный раунд
    state = AddRoundKey(state, key);
    cout << "После AddRoundKey (round 0):" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // 5. 9 основных раундов
    for (int round = 1; round <= 9; round++) {
        cout << "\n=== Round " << round << " ===" << endl;

        state = SubBytes(state);
        cout << "После SubBytes:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }

        state = ShiftRows(state);
        cout << "После ShiftRows:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }

        state = MixColumns(state);
        cout << "После MixColumns:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }
        // Преобразование из плоского массива в Key (column-major)
        Key roundKey;
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                // roundKeys[round] хранится в column-major: [col0_row0, col0_row1, col0_row2, col0_row3, col1_row0, ...]
                roundKey[row][col] = roundKeys[round][col * 4 + row];
            }
        }

        cout << "Ключ раунда " << round << ":" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)roundKey[i][j] << " ";
            }
            cout << endl;
        }

        state = AddRoundKey(state, roundKey);
        cout << "После AddRoundKey:" << endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
            }
            cout << endl;
        }
    }

    // 6. Финальный раунд
    cout << "\n=== Final Round ===" << endl;
    state = SubBytes(state);
    cout << "После SubBytes:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    state = ShiftRows(state);
    cout << "После ShiftRows:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // Преобразуем последний раундовый ключ
    Key finalRoundKey;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            finalRoundKey[row][col] = roundKeys[10][col * 4 + row]; // column-major
        }
    }

    cout << "Финальный ключ:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)finalRoundKey[i][j] << " ";
        }
        cout << endl;
    }

    state = AddRoundKey(state, finalRoundKey);
    cout << "После Final AddRoundKey:" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << setw(2) << setfill('0') << (int)state[i][j] << " ";
        }
        cout << endl;
    }

    // 7. Преобразуем State обратно в байты
    unsigned char ciphertext[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ciphertext[i + 4 * j] = state[i][j]; // column-major
        }
    }

    // 8. Выводим результат
    cout << "\nЗашифрованный текст: ";
    for (int i = 0; i < 16; i++) {
        cout << hex << setw(2) << setfill('0') << (int)ciphertext[i] << " ";
    }
    cout << endl;
}