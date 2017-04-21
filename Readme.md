# Rate my Haselnuss
Visually guess the taste of a hazelnut

## Disclaimer

This program has been created as part of my image processing practical course
at the FH Aachen. Therefore some parts of this are in german. If you're
interested in this and have trouble understanding it, do not hesitate to
contact me: I'll gladly translate it for you to english.

## License

This program is lincesed under MIT. See License.md for details.

## Idee
Dieses Programm versucht Haselnüsse visuell nach ihrem Geschmack zu
klassifizieren, insbesondere die Erkennung der schlecht schmeckenden soll
gelingen.


## Technische Vorraussetzungen
Dieses Programm benutzt CMake und kann wie jedes andere CMake-Projekt
kompiliert werden:
```
mkdir build
cd build
cmake ..
make
```

Grundsätzlich sollte dieses Programm platformunabhängig sein. Getestet ist
dieses jedoch nur unter Linux. Insbesondere die direkte Aufnahme der Fotos über
die Android Debug Bridge (ADB) funktioniert nur unter Linux.

Folgende Libraries werden benötigt:
* Qt5
* OpenCV 3.1
* fann 2.2 (Fast Artificial Neural Network Library,
  [http://leenissen.dk/fann/wp/]())
* JsonCPP [https://github.com/open-source-parsers/jsoncpp]()

## Aufnahmebedingungen
Aufnahmebedingungen für Eingabedaten:
* Heller, einfarber Hintergrund mit einem H-Wert (im HSV-Farbraum) von über 70
* Lockere Anordnung
* Senkrechte Aufnahme aus ca. 18 cm Entfernung.
* Senkrechte Beleuchtung (z.B. Blitz)
* Auflösung des Bildsensors: 4160x3120

Alle Zahlenwerte können geändert werden, wenn die Konfiguration entsprechend
angepasst wird. Die aktuelle Konfiguration ist allerdings nur unter diesen
Bedingungen getestet.  Abweichende Bedingungen können schlechtere Ergebnisse
liefern.

## Bedienung
Die Bedienung des Programmes sollte relativ selbsterklärend sein. Man läd ein
Bild in die Applikation und erhält eine Klassifikation der Nüsse visuell
angezeigt. Unter dem Konfigurationstab können die Erkennungs- und
Bewertungsschritte nachvollzogen und angepasst werden.
Einige Zahlenwerte können hier ebenfalls konfiguriert werden.

