# Inteligentna Waga BIA z ESP32 i ThingSpeak

![IMG_20250918_005531](https://github.com/user-attachments/assets/50164375-0129-4fb8-bfb3-b8a00428a910)


---

## Opis Projektu

Jest to projekt inteligentnej wagi łazienkowej opartej na mikrokontrolerze ESP32. Urządzenie precyzyjnie mierzy masę ciała i wysyła zebrane dane do platformy chmurowej IoT **ThingSpeak**, co pozwala na zdalne monitorowanie i analizę wyników.

Projekt został zbudowany od podstaw w celu praktycznego zastosowania wiedzy z zakresu elektroniki, programowania i technologii Internetu Rzeczy. Jednym z głównych założeń było wykorzystanie tanich i łatwo dostępnych komponentów. Urządzenie posiada również zmontowany, autorski układ do analizy impedancji bioelektrycznej (BIA), który jest obecnie w fazie diagnostyki i rozwoju.

## Lista Funkcji

*   ✅ Precyzyjny pomiar masy ciała.
*   ✅ Wysyłanie danych pomiarowych do chmury ThingSpeak co 20 sekund.
*   ✅ Możliwość ręcznego tarowania wagi przez port szeregowy.
*   🔬 Zbudowany moduł do pomiaru BIA (obecnie w fazie diagnostycznej, zwraca wartość 0).
*   🌐 Pełna integracja z siecią Wi-Fi.

## Potrzebne Komponenty

**Elektronika:**
*   Mikrokontroler: **ESP32 DevKit V1**
*   Moduł wagi: **HX711**
*   Wzmacniacz operacyjny: **MCP602**
*   Rezystory: 10kΩ (2 szt.), 1kΩ (2 szt., 1%), 100kΩ (1 szt., 1%)
*   Kondensatory: 1µF (ceramiczny), 100nF (ceramiczny), 10µF (elektrolityczny)
*   Podstawka precyzyjna DIP-8
*   Płytka uniwersalna PCB
*   Złącza Goldpin

**Mechanika i Zasilanie:**
*   Czujniki tensometryczne: 4 sztuki, każda o nośności 50kg
*   Platforma wagi (np. deska, szkło hartowane)
*   Elektrody do pomiaru BIA (np. płytki ze stali nierdzewnej)
*   Moduł ładowarki Li-Ion **TP4056** i akumulator **18650** (dla wersji przenośnej)

## Schemat Połączeń

Poniższy schemat przedstawia logikę połączeń na płytce uniwersalnej.

```ascii
                +--------------------------------------+
                | Schemat Elektryczny: Waga BIA z AFE  |
                |              na ESP32                |
                +--------------------------------------+

   +3.3V ---[R1]10kΩ---+---[R2]10kΩ--- GND
                       |
                      === C3 10µF
                       |
               V_REF_HALF (~1.65V)

   +----------------------+      +-----------------+      +-------------------------------+
   |   ESP32 Dev Kit      |      |     HX711       |      |   MCP602 (U3)               |
   |                      |      |                 |      |                               |
   | D22 (DT) o------------------o DT              |      | VDD o----+3.3V                |
   | D21 (SCK) o-----------------o SCK             |      | VSS o----GND                  |
   | D25 (DAC) o--||---+                             OUT o-----> OPAMP_OUT ----o--> D34 (ADC)
   |                C1 1µF |                                         |                  |
   +----------------------+                                         [R5]100kΩ          |
                                                                     |                  |
                                            V_REF_HALF -------------[R4]1kΩ------------'

         [Tensometry] <---> HX711

         [Elektrody J2]
           Pin1 (DRIVE_A) <---- Wyjście z C1 1µF
           Pin2 (SENSE_A) ----> U3 In+ , oraz [R3]1kΩ -> GND
           Pin3 (DRIVE_B) -----------------> GND
           Pin4 (SENSE_B) -----------------> GND

