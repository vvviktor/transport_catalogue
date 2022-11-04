# transport_catalogue
**Transport Catalogue**

* Проект транспортного справочника с возможностью визуализации карты маршрутов в формате векторной графики SVG.
Реализован механизм построения кратчайшего по времени маршрута между остановками.
* Работа программы разделена на два процесса:
  * Посторение базы остановок и маршрутов с последующей сериализацией в файл этой базы вместе с базой 
  маршрутизации.
  * Десериализация и перестроение базы с предварительно рассчитанной маршрутизацией из файла. Обработка запросов к базе.

**********************************************************************************************************************

* Для сборки проекта требуется пакет Protobuf, доступный по ссылке https://github.com/google/protobuf Путь к папке с установленным 
пакетом protobuf укажите в файле `CMakeLists.txt`, раскомментировав соответствующую строку.

**********************************************************************************************************************

**Использование:**

* В консоли: `transport_catalogue make_base|process_requests <запрос (JSON)>`
* Синтаксис запроса на построение базы (JSON). Комментарии приведены для наглядности, в реальном вводе комментарии не допускаются:
```
{
      "serialization_settings": {          // Настройки сериализации
          "file": "transport_catalogue.db" // имя файла для сохранения базы
      },
      "routing_settings": {                // Настройки маршрутизации   
          "bus_wait_time": 2,              // время на ожидание посадки/пересадки (положительное целое значение) 
          "bus_velocity": 30               // скорость перемещения для всех маршрутов (положительное вещественное значение)
      },
      "render_settings": {                 // Настройки визуализации для вывода в формате SVG. Все размеры указываются в пикселях.
          "width": 1200,                   // Ширина.
          "height": 500,                   // Высота.
          "padding": 50,                   // Отступ.
          "stop_radius": 5,                // Радиус окружности для обозначения остановки. 
          "line_width": 14,                // Ширина линии маршрута.
          "bus_label_font_size": 20,       // Размер шрифта названия маршрута. 
          "bus_label_offset": [            // Величина горизонтального и вертикального отступа для названия маршрута.
              7,
              15
          ],
          "stop_label_font_size": 18,      // Размер шрифта названия остановки.
          "stop_label_offset": [           // Величина горизонтального и вертикального отступа для названия остановки.
              7,
              -3
          ],
          "underlayer_color": [            // Цвет контура текста и линий RGBa 
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,           // Толщина контура.        
          "color_palette": [               // Перечень цветов для линий маршрутов (применяется в порядке добавления маршрута в базу).
              "green",                     // Форматы, допустимые для графики SVG: RGB, RGBa, string.  
              [
                  255,
                  160,
                  0
              ],
              "red"
          ]
      },
      "base_requests": [                   // Запросы на добавлению объектов в базу.
          {                                // Запрос на добавление маршрута.
              "type": "Bus",               // Тип запроса "Bus"
              "name": "14",                // Название маршрута.
              "stops": [                   // Остановки маршрута (должны быть добавлены в базу в рамках запроса base_requests до или после
                  "Улица Лизы Чайкиной",   // маршрута.
                  "Электросети",
                  "Ривьерский мост",
                  "Гостиница Сочи",
                  "Кубанская улица",
                  "По требованию",
                  "Улица Докучаева",
                  "Улица Лизы Чайкиной"
              ],
              "is_roundtrip": true         // Характеристика колцевой - true: первая и последняя остановки совпадают, движение в одном направлении 
          },                               //                           false: маршрут туда и обратно. 
          {
              "type": "Bus",
              "name": "24",
              "stops": [
                  "Улица Докучаева",
                  "Параллельная улица",
                  "Электросети",
                  "Санаторий Родина"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Bus",
              "name": "114",
              "stops": [
                  "Морской вокзал",
                  "Ривьерский мост"
              ],
              "is_roundtrip": false
          },
          {                                  // Запрос на добавление остановки.
              "type": "Stop",                // Тип запроса "Stop".
              "name": "Улица Лизы Чайкиной", // Название остановки.
              "latitude": 43.590317,         // Географическая широта.
              "longitude": 39.746833,        // Географическая долгота. 
              "road_distances": {            // Расстояния до других остановок в базе (целые неотрицательные значения).
                  "Электросети": 4300,
                  "Улица Докучаева": 2000
              }
          },
          {
              "type": "Stop",
              "name": "Морской вокзал",
              "latitude": 43.581969,
              "longitude": 39.719848,
              "road_distances": {
                  "Ривьерский мост": 850
              }
          },
          {
              "type": "Stop",
              "name": "Электросети",
              "latitude": 43.598701,
              "longitude": 39.730623,
              "road_distances": {
                  "Санаторий Родина": 4500,
                  "Параллельная улица": 1200,
                  "Ривьерский мост": 1900
              }
          },
          {
              "type": "Stop",
              "name": "Ривьерский мост",
              "latitude": 43.587795,
              "longitude": 39.716901,
              "road_distances": {
                  "Морской вокзал": 850,
                  "Гостиница Сочи": 1740
              }
          },
          {
              "type": "Stop",
              "name": "Гостиница Сочи",
              "latitude": 43.578079,
              "longitude": 39.728068,
              "road_distances": {
                  "Кубанская улица": 320
              }
          },
          {
              "type": "Stop",
              "name": "Кубанская улица",
              "latitude": 43.578509,
              "longitude": 39.730959,
              "road_distances": {
                  "По требованию": 370
              }
          },
          {
              "type": "Stop",
              "name": "По требованию",
              "latitude": 43.579285,
              "longitude": 39.733742,
              "road_distances": {
                  "Улица Докучаева": 600
              }
          },
          {
              "type": "Stop",
              "name": "Улица Докучаева",
              "latitude": 43.585586,
              "longitude": 39.733879,
              "road_distances": {
                  "Параллельная улица": 1100
              }
          },
          {
              "type": "Stop",
              "name": "Параллельная улица",
              "latitude": 43.590041,
              "longitude": 39.732886,
              "road_distances": {}
          },
          {
              "type": "Stop",
              "name": "Санаторий Родина",
              "latitude": 43.601202,
              "longitude": 39.715498,
              "road_distances": {}
          }
      ]
  }
```
  * Синтаксис запроса к построенной базе (JSON). Комментарии приведены для наглядности, в реальном вводе комментарии не допускаются:
```
  {
      "serialization_settings": {              // Настройки сериализации
          "file": "transport_catalogue.db"     // имя файла сохраненной базы (должен быть в рабочей папке программы)
      },
      "stat_requests": [                       // Запросы к базе.
          {
              "id": 218563507,                 // Идентификатор запроса.    
              "type": "Bus",                   // Тип запроса "Bus" - информация о маршруте.  
              "name": "14"                                    "Stop" - информация об остановке.
          },                                                  "Route" - запрос на построение маршрута.
          {                                                   "Map" - запрос на построение карты.
              "id": 508658276,
              "type": "Stop",
              "name": "Электросети"
          },
          {
              "id": 1964680131,
              "type": "Route",
              "from": "Электросети",
              "to": "Параллельная улица"
          },
          {
              "id": 1359372752,
              "type": "Map"
          }
      ]
  }
```
* Формат вывода для приведенного примера (JSON):
```
[
    {                                       // Ответ на запрос "Bus".
        "curvature": 1.60481,               // Характеристика кривизны маршрута. Отношение реальной указанной длины к сумме прямых между путевыми точками.
        "request_id": 218563507,            // Идентификатор запроса.
        "route_length": 11230,              // Общая длина маршрута.   
        "stop_count": 8,                    // Общее количество остановок маршрута.
        "unique_stop_count": 7              // Количество уникальных остановок маршрута.
    },
    {                                       // Ответ на запрос "Stop".
        "buses": [                          // Маршруты, проходящие через остановку.
            "14",
            "24"
        ],
        "request_id": 508658276             // Идентификатор запроса.
    },
    {                                       // Ответ на запрос "Route".
        "items": [                          // Составляющие маршрута.  
            {
                "stop_name": "Электросети", // Остановка.
                "time": 2,                  // Время ожидания на остановке. Фиксированное. Равно настройке bus_wait_time.
                "type": "Wait"              // Тип составляющей. Ожидание. 
            },
            {                               
                "bus": "24",                // Движение по указанному маршруту.
                "span_count": 1,            // Количество остановок без пересадки. 
                "time": 2.4,                // Время на перемещение по маршруту.
                "type": "Bus"               // Тип составляющей. Маршрут (автобус).
            }
        ],
        "request_id": 1964680131,           // Идентификатор запроса.
        "total_time": 4.4                   // Общее время всех составляющих маршрута.
    },
    {                                       // Вывод карты в формате SVG (Unescaped).
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"125.25,382.708 74.2702,281.925 125.25,382.708\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n  <polyline points=\"592.058,238.297 311.644,93.2643 74.2702,281.925 267.446,450 317.457,442.562 365.599,429.138 367.969,320.138 592.058,238.297\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n  <polyline points=\"367.969,320.138 350.791,243.072 311.644,93.2643 50,50 311.644,93.2643 350.791,243.072 367.969,320.138\" fill=\"none\" stroke=\"red\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n  <text x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >114</text>\n  <text x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\" >114</text>\n  <text x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >114</text>\n  <text x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\" >114</text>\n  <text x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >14</text>\n  <text x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgb(255,160,0)\" >14</text>\n  <text x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >24</text>\n  <text x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"red\" >24</text>\n  <text x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >24</text>\n  <text x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"red\" >24</text>\n  <circle cx=\"267.446\" cy=\"450\" r=\"5\" fill=\"white\" />\n  <circle cx=\"317.457\" cy=\"442.562\" r=\"5\" fill=\"white\" />\n  <circle cx=\"125.25\" cy=\"382.708\" r=\"5\" fill=\"white\" />\n  <circle cx=\"350.791\" cy=\"243.072\" r=\"5\" fill=\"white\" />\n  <circle cx=\"365.599\" cy=\"429.138\" r=\"5\" fill=\"white\" />\n  <circle cx=\"74.2702\" cy=\"281.925\" r=\"5\" fill=\"white\" />\n  <circle cx=\"50\" cy=\"50\" r=\"5\" fill=\"white\" />\n  <circle cx=\"367.969\" cy=\"320.138\" r=\"5\" fill=\"white\" />\n  <circle cx=\"592.058\" cy=\"238.297\" r=\"5\" fill=\"white\" />\n  <circle cx=\"311.644\" cy=\"93.2643\" r=\"5\" fill=\"white\" />\n  <text x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Гостиница Сочи</text>\n  <text x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Гостиница Сочи</text>\n  <text x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Кубанская улица</text>\n  <text x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Кубанская улица</text>\n  <text x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Морской вокзал</text>\n  <text x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Морской вокзал</text>\n  <text x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Параллельная улица</text>\n  <text x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Параллельная улица</text>\n  <text x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >По требованию</text>\n  <text x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >По требованию</text>\n  <text x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Ривьерский мост</text>\n  <text x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Ривьерский мост</text>\n  <text x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Санаторий Родина</text>\n  <text x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Санаторий Родина</text>\n  <text x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Улица Докучаева</text>\n  <text x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Улица Докучаева</text>\n  <text x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Улица Лизы Чайкиной</text>\n  <text x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Улица Лизы Чайкиной</text>\n  <text x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" >Электросети</text>\n  <text x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\" fill=\"black\" >Электросети</text>\n</svg>",
        "request_id": 1359372752            // Идентификатор запроса.
    }
]
```
