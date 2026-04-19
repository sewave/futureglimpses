# ATISBOS DEL FUTURO

[Pantalla de título]
Atisbos del futuro o Future glimpses es un pequeño juego de estrategia en tiempo real (o RTS) que homenajea a Warcraft / Age of empires, con un poco de cada uno.
Pretende ser un ejercicio para ver si era posible hacerlo funcional en un 486/66 con 16MB de RAM, especialmente para el concurso de [MS-DOS Club](https://msdos.club/).

## Historia



## Características

* Agrupación de mapas tipo campaña: podemos agrupar en carpetas varios mapas simulando una campaña.
* Mulidioma: Español / Inglés.
* Opciones: podemos ajustar el volumen, el idioma (español / inglés, solo desde el menú principal) o si cuando queremos ver las barras de vida de los enemigos.
* Minimapa: el clásico minimapa, podemos desplazarnos por el haciendo clic.
* Niebla de guerra: Una vez destapada queda el mapa visible.
* Grupos de unidades: podemos asignar grupos del 1 al 5 y tener un acceso rápido sin límite de unidades.
* Recursos: tenemos tres recursos, oro, madera y comida. Oro y comida se recolectan con los trabajadores, la comida se obtiene creando nuevas granjas y limita las unidades que podemos tener a la vez.
* Sistema de mensajes: para ver mensajes relacionados con los eventos que ocurren.
* Cola de unidades: puedes encolar entrenamientos en los edificios y cancelarlos.
* Pantalla de resultados: al terminar un mapa tendremos un resumen de datos sobre lo sucedido en la partida.

Y otras sorpresas que será más divertido descubrir jugando.

## Unidades

Obrero: la unidad básica, encargada de la recolección de recursos, la construcción y la reparación.
Soldado: la unidad básica de combate cuerpo a cuerpo, sencillo pero eficaz.
Arquero: la unidad básica de combate a distancia, permite atacar desde la lejanía.
Caballero: una mejora del soldado, mejor en todos los aspectos pero con un coste mayor.
Mago: la unidad de ataque a larga distancia, no solo ataca desde muy lejos si no que sus bolas de fuego dañan en área. Cuidado con dañar a tus tropas.

## Edificios

Alcaldía: el edificio principal, crea obreros y no puede ser construido.
Granja: permite tener más comida para generar más unidades.
Cuarteles: el edificio básico que permite entrenar soldados, arqueros y caballeros una vez tengan sus edificios.
Herrería: permite entrenar arqueros en los cuarteles.
Establos: permite entrenar caballeros en los cuarteles.
Torre: permite entrenar magos.

## Controles

La mayoría del juego puede controlarse con el ratón, algunos menús tienen atajos de teclado que pueden usarse también.
En el juego principal podemos usar el botón izquierdo para seleccionar una unidad o múltiples y el botón derecho para las acciones contextuales, dependiendo de que unidad elija que objetivo.
Si no queremos usar las acciones contextuales podemos usar los botones de comando de la unidad o sus atajos.

ALT: Mostrar barras de vida mientras se pulsa.
TAB: Mostrar cantidad restante del recurso sobre el que está el cursor mientras se pulsa.

Si pulsas SHIFT mientras estamos en modo construcción, podremos colocar varios edificios a la vez.

CTRL + 1-5: Crea un grupo rápido con la selección actual.
1-5: Seleccione el grupo rápido creado.

CTRL + Clic izquierdo: Añade una unidad a la selección actual.
SHIFT + Clic izquierdo: Elimina una unidad a la selección actual.

Si pulsas espacio teniendo alguna unidad seleccionada la cámara se posicionará centrando a la primera unidad de la selección.

Nota: No se pueden seleccionar múltiples edificios propios ni múltiples unidades/edificios enemigos.

## Mapas

Una de las mejores características del juego es la posibilidad de diseñar tus propios mapas, para ello hará falta el editor [Tiled](https://www.mapeditor.org/).
Y unas características concretas, en la carpeta /tools/map encontraréis un projecto de tiled con el que gestionarlo todo. Mi recomendación es copiar mapas y editarlos.

### Atributos de los mapas

Además de poder editar el terreno, tenemos la posibilidad de configurar algunas cosas para hacer los mapas únicos. A nivel de mapa, los encontraréis en "Mapa -> Atributos del mapa... -> Atributos personalizados"
Son los siguientes:
`AI_MODE`: El modo de funcionamiento de la IA, hay tres opciones.

* `IDLE`, la máquina no hace nada pero se defenderá.
* `PASSIVE`, la máquina recogerá recursos, creará su ejército y reconstruirá sus edificios, pero solo se defenderá.
* `AGGRESIVE`, lo mismo que `PASSIVE` pero además enviará oleadas de enemigos a por ti.

`ENABLE_BARRACKS`: Activa la construcción de cuarteles en el mapa.
`ENABLE_BLACKSMITH`: Activa la construcción de herrerías en el mapa.
`ENABLE_STABLES`: Activa la construcción de establos en el mapa.
`ENABLE_TOWER`: Activa la construcción de torres en el mapa.

`MSG_DESCRIPTION`: El mensaje de descripción del mapa, para el selector de niveles y la información del menú de pausa, la longitud del mismo depende del texto pero no debería llegar a los 256 caracteres.
`MSG_LOSE`: El mensaje que aparecerá al perder el mapa, de longitud similar al de descripción.
`MSG_TITLE`: El título del mapa, se verá en el selector y en la información del mapa en partida.
`MSG_WIN`: El mensaje que aparecerá al ganar el mapa, de longitud similar al de descripción.

`PEACE_TIME`: El tiempo en segundos que tardará la máquina en empezar a crear unidades y mandártelas, solo tiene sentido en modo `AGGRESSIVE`.

`RES_GOLD_COMPUTER`: El oro inicial con el que empieza la máquina.
`RES_GOLD_PLAYER`: El oro inicial con el que empieza el jugador.
`RES_WOOD_COMPUTER`: La madera inicial con la que empieza la máquina.
`RES_WOOD_PLAYER`: La madera inicial con la que empieza el jugador.

#### Unidades personalizadas

Podemos personalizar nuestras unidades con diferentes características, para ello usaremos sus propios atributos. Podemos verlos en "Atributos personalizados" al seleccionar una unidad, podemos crearlos pero es preferible copiarlos de los ejemplos y luego modificarlos.
`CUSTOM`: Actívalo para tener en cuenta los atributos personalizados de la unidad.
`MIN_DAMAGE`: Daño mínimo de la unidad.
`MAX_DAMAGE`: Daño máximo de la unidad.
`MAX_HEALTH`: Salud máxima de la unidad.
`MUST_SURVIVE`: Indica que esta unidad debe sobrevivir, si muere será una derrota instantánea.
`Nombre`: Esto no es una variable, podemos modificar el nombre del objeto de Tiled, hasta unos 10 caracteres.

## Créditos

Programación y diseño: Wave

Varios recursos fueron creados con IA, como las texturas, los edificios, la ilustración del título o los iconos del menú de comandos.

Créditos de recursos de terceros:
Este trabajo incluye contenido modificado de "[Bitrimus Font](https://ggbot.itch.io/bitrimus-font)" por [GGBotNet](https://www.ggbot.net), licenciado bajo [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

Este trabajo incluye contenido modificado de "[Fantasy Battle Pack](https://mattwalkden.itch.io/fantasy-battle-pack)" por [Matt Walkden](https://mattwalkden.itch.io).

Este trabajo incluye contenido modificado de "[Superpowers assets sound effects](https://opengameart.org/content/superpowers-assets-sound-effects)" - "medieval-fantasyy/5.wav (goldhit.wav)", "western-fps-2d/explosion-1.ogg (fbexplo.wav)", "medieval-fantasy/7.wav (ironhit.wav)", "prehistoric-platformer/hit-1.wav (work.wav)", "prehistoric-platformer/wood-2.wav (chop.wav)", "medieval-fantasyy/woosh-2.wav (arrowthr.wav)", 
"space-shooter/alert.wav (attack.wav)", "ninja-adventure/menu-1.ogg (notvalid.wav)", "prehistoric-platformer/hit-2.wav (crumble.wav)", "top-down-shooter/flame-thrower.wav (fblaunch.wav)", "western-fps-2d/arrow.ogg (arrowhit.wav)" y "western-fps-2d/scream-5.ogg (die.wav)" por [Sparklin Labs' Superpowers HTML5 game maker](http://superpowers-html5.com/), licenciado bajo [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

Este trabajo incluye contenido de "[DarkBasic Music Library](https://opengameart.org/content/darkbasic-music-library)" - "northern lights.mid (map1.mid)" y "~bog~ tune.mid (menus.mid)" por [DarkBasic](https://darkbasic.com/), licenciado bajo [CC-BY-4.0+](https://creativecommons.org/licenses/by/4.0/).

Este trabajo incluye contenido de "[Midi Pack 3 (35 so far)](https://opengameart.org/content/midi-pack-3-35-so-far)" - "9088malchakwilder8.mid (intro.mid)", "9095noobusfog.mid (defeat.mid)", "9099clavvictorytune.mid (victory.mid)", "9101pianochordmelody.mid (map2.mid)" y "9094telosvillagecentralsmarket.mid (map3.mid)" por [Tozan](https://opengameart.org/users/tozan), licenciado bajo [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

Se ha usado [Cool Text Graphics Generator](https://cooltext.com/) para generar el título del juego.

