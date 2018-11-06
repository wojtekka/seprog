seprog
======

Program pozwala korzystać z programatora Seprog firmy WG Electronics pod systemami Linux i Windows. Programator najwyraźniej nie jest już wspierany przez producenta, który dostarczał jedynie oprogramowanie dla systemu MS-DOS. Protokół komunikacji z programatorem został poznany metodą reverse engineering. Z powodu braku innych układów i przejściówek, program obsługuje tylko najpopularniejsze układy firmy Atmel: 89C51, 89C52, 89Cx051, 89S8252, 90S1200.

Niestety program nie jest zbyt rozbudowany. Obsługuje wyłącznie pliki binarne. Pozwala kasować zawartość układów, czytać i zapisywać. Pozwala też programować bity zabezpieczające. Nie jest programem interaktywnym -- wszystkie opcje (m.in. port programatora, rodzaj układu) podaje się w linii komend.

Program został przeniesiony do Win32 przez Krzysztofa Rusockiego <kszysiu@iceberg.elsat.net.pl>

Przykłady użycia:

* `seprog -c at89c51 -e -w program.bin`

  Wyczyści i zaprogramuje AT89C51 plikiem `program.bin`. Programator podłączony do portu wskazanego przez `/dev/seprog`.

* `seprog -p /dev/ttyS0 -c at89c2051 -r plik.bin`

  Czyta zawartość AT89C2051 i zapisuje do pliku `plik.bin`. Programator podłączony do portu `/dev/ttyS0`.

Historia zmian
--------------

- 0.3
  - zmiana sposobu podawania parametrów
- 0.2
  - kompiluje się za pomocą Cygwina i mingw32,
  - dodana obsługa AT29C010,
  - wyświetlanie postępu.
- 0.1
  - pierwsza wersja.

(C) Copyright 2003-2006 Wojtek Kaniewski <wojtekka@irc.pl>

