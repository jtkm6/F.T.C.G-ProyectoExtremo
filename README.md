# Proyecto Extremo
## El presente proyecto cumple con las sigientes condiciones.
---
### Task #1
 - **Título**: Carga de volúmenes.
 - **Objetivo**: Leer volúmenes de 8-bits y 16-bits, estos deben etar en formato .raw o .dat.
 - **Puntaje**: 2 ptos.

 ---
### Task #11
- **Título**: Despliegue de varios volúmenes.
- **Objetivo**: Desplegar varios volúmenes a la vez en la pantalla.
- **Puntaje**: 1 ptos.

> Por defecto carga dos modelos uno de 256x256x256 8bits y otro de 512x512x512 16bits.

---
### Task #3
- **Título**: Despliegue de volúmenes usando Polígonos Alineados al Viewport
 - **Objetivos**:
    - Cargar los datos del volumen en una textura 3D.
    - Generar los polígonos paralelos al viewport.
    - Desplegar el volumen mediante el uso de los polígonos.
 - **Puntaje**: 5 ptos.

> Se despiega el volumen dentro de un cubo y se emplea raycasting en GPU para desplegarlo.

 ---
### Task #8
- **Título**: Aplicación de transformaciones afines al volumen.
 - **Objetivos**:
    - Aplicar transformaciones de traslación, rotación y escalamiento al volumen.
    - Aplicar acercamiento y alejamiento de la cámara al volumen.
 - **Puntaje**: 1 pto.

> Todo se maneja desde el teclado:
>	Flecha Derecha Selecciona el volumen desplegado a la derecha.
>	Flecha izquierda Selecciona el volumen desplegado a la izquierda.
>	Flecha Ariba & Flecha Abajo Desplazan el volumen en Y
>	W & S Desplazan el volumen en Z
>	A & D Desplazan el volumen en X
>	Mediante el mouse se puede rotar el volumen seleccionado

---
### Task #10
- **Título**: Despliegue de volúmenes aplicando algún método de iluminación.
 - **Objetivo**: Desplegar el volumen y aplicarle a éste algún método de iluminación.
 - **Puntaje**: 3 ptos.

> Mediante la tecla 'L' se enciende o apaga la iluminacion, y el puntero del raton se convierte en la fuente de luz. Solamente existe iluminacion difusa.

### Task #7
- **Título**: Creación de una interfaz para el control de la función de transferencia.
 - **Objetivos**:
    - Crear una interfaz mediante la cual partiendo de la identidad se pueda agregar, eliminar y mover puntos de control. Además de asignar la opacidad.
    - Crear un color piker para poder asignarle valores de color a cada uno de los puntos de control de la función.
 - **Puntaje**: 5 ptos.

> La ventana que permite modificar la funcion de transferencia es di implementacion propia tomando como referencia [ESTA](https://github.com/andresz1/transfer-function-glfw3) pero por uso de librerias esotericas y desacuerdos de sintaxis fue reimplementada.



- - - - 
### Autor
**Nombre:** Jorge Taoufik Khabazze Maspero  
**C.I.:** 23.692.079  
**E-Mail:** jtkm6jk@gmail.com
