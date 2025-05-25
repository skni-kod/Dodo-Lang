## Składnia języka Dodo Lang

Jest inspirowana C++ i Rust, z pewnymi różnicami w zakresie definicji typów.

Nieważna jest kolejność definicji w kontekście globalnym.

### Typy danych

W przeciwieństwie do większości innych języków istnieją tylko 2 odróżnialne rodzaje typów danych: typy proste i złożone.

Jedyną różnicą między nimi jest dziedziczenie operatorów domyślnych z podstawowych rodzajów danych, a więc wartości całkowitych z i bez znaku, adresów i zmiennoprzecinkowych.
Poza tym ich obsługa ze strony języka jest identyczna.

Do każdej kompilacji domyślne dołączany jest plik z definicjami podstawowych typów prostych o ilościach bitów w nazwie:
- i8, i16, i32, i64 - całkowite ze znakiem,
- u8, u16, u32, u64 - całkowite bezwzględne
- f16, f32, f64 - zmiennoprzecinkowe

Dodatkowo istnieją typy znakowy (jednobajtowy char) i logiczny (bool).

Brak wartości oznacza się słowem void.

Definiowanie typów odbywa się następującą składnią:
```
primitive SIGNED_INTEGER/UNSIGNED_INTEGER/FLOATING_POINT <wielkość> type nazwa ;/{ ... }
```
lub
```
type nazwa { ... }
```
W typach bez słowa kluczowego primitive trzeba wpisać co najmniej jedną zmienną członkowską. We wszystkich można definiować metody i przeładowania operatorów.

### Funkcja główna

Punkt wejściowy działa analogicznie do języków z rodziny C/C++.

```
i32 Main() {
  return 0;
}
```
### Zmienne
Tworzenie zmiennych odbywa się za pomocą słowa kluczowego let. Domyślnie posiadają one stałą wartość, aby to zmienić trzeba dopisać mut. Po tym podaje się typ, modyfikatory typu, nazwę i wartość.
```
void fun() {
  let u16 var = 5;
  var += 10; // Błąd! Zmienna o stałej wartości
  let mut u16 var2 = 45;
  var /= 5; // Ok
}
```
### Dołączanie innych plików
Przy użyciu wyrażenia:
```
import "plik.dodo";
```
można dołączyć dowolny plik zawarty w katalogach podanych przy wywołaniu kompilatora.

### Wywołania systemowe
Można bezpośrednio używać wywołań systemowych systemu Linux o dedukowanych z argumentów typach argumentów za pomocą słowa syscall. Przykładowo w funkcji do drukowania wiadomości w konsoli:
```
void Print(let u64 length, let char* message) {
    // nr 1 - sys_write(unsigned int fd,	const char *buf,	size_t count)
    syscall(1, 0, message, length);
    return;
}
```
Wartość zwracana z syscall zawsze jest typu u64 i może zostać przekonwerowana na inne w zależności od potrzeby.

Projekt jest w stanie ciągłego rozwoju
