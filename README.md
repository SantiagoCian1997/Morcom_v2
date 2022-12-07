# Morcom_v2
Morcom_v2 es un firmware desarrollado para el control de máquinas pick and place, enfocado al movimiento de alta velocidad de los motores paso a paso respetando las curvas de aceleración, velocidad máxima y desaceleración. Pudiendo lograr frecuencias de hasta 110 kHz de frecuencia máxima en la salida de control de steps.

**_Morcom v2 is a firmware designed to control pick and place machines, focused on the high speed movement of stepper motors respecting the acceleration, maximum speed and deceleration curves. Being able to achieve frequencies of up to 110 kHz maximum frequency at the step control output._**

El proyecto se pensó para correr en un STM32F103 (72MHz) basado en un Cortex-M3, el cual es un uC muy barato y existen placas de desarrollo muy simples basados en el (Ej: STM32F103-bluepill board). De igual modo el proyecto, al estar desarrollado en C, se puede migrar a cualquier otro modelo de STM32 o cualquier otra arquitectura.

**_The project was made to run on a STM32F103 (72MHz) based on a CortexM3, which is a very cheap uC and there are very simple development boards based on it (eg: STM32F103-bluepill board). Similarly, the project, being developed in C, can be migrated to any other STM32 model or any other architecture._**

Morcom_v2 fue pensado para usarlo en conjunto con [OpenPnP](https://openpnp.org/), utilizando líneas estándar de G code para su control, contando con rutinas de homing, parada de emergencia mediante finales de carrera y uso de coordenadas absolutas o relativas. 

**_Morcom v2 was designed to be used in conjunction with [OpenPnP](https://openpnp.org/), using standard G code lines for its control, with homing routines, emergency stop through limit switches and the use of absolute or relative coordinates._**

## Comandos G code / _G code commands_ 

| Command   | Description                                                | Example                     | Description                                                                                                                                                                                                                         |
| --------- | ---------------------------------------------------------- | --------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| G0        | Movement maximum speed                                     | G0 X100 Y200                | Move at full speed the motors X and Y to the positions of 100 unit and 200 unit.                                                                                                                                                    |
| G1        | Movement speed F#                                          | G1 X100 Y200 F1000          | Move at maximum speed of 1000unit/s.                                                                                                                                                                                                |
| G28       | Homing sequence                                            | G28                         | Start homing sequence.                                                                                                                                                                                                              |
| G90,M90   | Set absolute coordinates mode                              | G90 o M90                   | Change the coordinate system to absolute.                                                                                                                                                                                           |
| G91,M91   | Set relative coordinates mode                              | G91 o M91                   | Change the coordinate system to elative.                                                                                                                                                                                            |
| G92       | Set absolute axis position                                 | G92 X500 B300               | Set the absolute value of the current position of motors X and B at 500 unit and at 300 unit.                                                                                                                                       |
| M18       | Enable motors                                              |                             |                                                                                                                                                                                                                                     |
| M17       | Disable motors                                             |                             |                                                                                                                                                                                                                                     |
| M100      | Print coordinates                                          |                             | Print the location of the motors through the serial port.                                                                                                                                                                           |
| M999      | Request parameters and print help                          |                             | Prints the parameters of the motors and a command help.                                                                                                                                                                             |
| M998      | Save the data of the motors in the flash memory of the uC  |                             | Save the configuration of the motors in a space of the flash destined for this purpose.                                                                                                                                             |
| M1000     | Set maximum motor speed                                    | M1000 X100 Y100 Z50 B30 C40 | Set maximum motor speed X=100 unit/s, Y=100 unit/s, Z=50unit/s, B=30unit/s, C=40unit/s.                                                                                                                                             |
| M1001     | Set acceleration of motors                                 | M1001 X10 Z15               | Set motor acceleration X=10unit/s^2, Z=15unit/s^2.                                                                                                                                                                                  |
| M1002     | Set step per unit                                          | M1002 X35 Y35 C2.22         | Set motor resolution in step/unit of motors X=35 step/mm, Y=35 step/mm, C=2.22 step/degrees.                                                                                                                                        |
| M1003     | Set minimum speed of the deceleration curve (%V_max)       | M1003 X5 Y5 Z1              | This indicates in % the minimum speed value reached by the aeceleration curve (values between 6% and 1% are recommended).                                                                                                           |
| M1004     | Invert Dir pin logic                                       | M1004 X1 Y0 B1              | Invert the logic of the Dir pin of motors X and B, do not invert the logic of motor Y.                                                                                                                                              |
| M1010     | Activate axis homing sequence                              | M1010 X1 Y1 Z1 B0 C0        | Activate the homing sequence of motors X, Y and Z. Do not activate the homing sequence of motors B and C.                                                                                                                           |
| M1011     | Set home switch search direction                           | M1011 X1 Y1 Z0              | Find the home switch of the X and Y motors in the negative direction and the Z motor in the positive direction.                                                                                                                     |
| M1012     | Set home switch search speed (%V_max)                      | M1012 X10 Y10 Z5            | Find the motor home switch X at 10% of maximum motor speed, Y at 10% and Z at 5%.                                                                                                                                                   |
| M1013     | Set back once the switch is located (in unit)              | M1013 X40                   | Once the X motor home switch is found, move back 40mm to distance yourself from the switch.                                                                                                                                         |
| M1014     | Set location (in unit) after rollback is done              | M1014 X-60                  | Once the home switch of the X motor is found and the retreat is done, the absolute location of the motor is set to -60mm.                                                                                                           |
| M1015     | Set home search priority                                   | M1015 X0 Y5 Z3              | The search priority is used to indicate in which order the home switches of the axes will be searched. In this example, it will search for the X axis first, then the Z axis and finally the Y axis (lower value, higher priority). |
| X,Y,Z,B,C | Axis modifiers (coordinates or parameters)                 |                             |                                                                                                                                                                                                                                     |
| F         | Max movement speed modification                            |                             | Maximum movement speed G1, if the value of F is not modified, the previous one is used.                                                                                                                                             |


## Pin OUT
| Pin  | Description            |
| ---- | ---------------------- |
| PB00 | Step_X (OUTPUT)        |
| PB01 | Dir_X (OUTPUT)         |
| PB03 | Step_B (OUTPUT)        |
| PB04 | Dir_B (OUTPUT)         |
| PB10 | Step_Y (OUTPUT)        |
| PB11 | Dir_Y (OUTPUT)         |
| PB12 | Step_Z (OUTPUT)        |
| PB13 | Dir_Z (OUTPUT)         |
| PB14 | Step_C (OUTPUT)        |
| PB15 | Dir_C (OUTPUT)         |
| PB05 | Enable_motors (OUTPUT) |
| PA00 | Home_X (INPUT)         |
| PA01 | Home_Y (INPUT)         |
| PA02 | Home_Z (INPUT)         |
| PA03 | Home_B (INPUT)         |
| PA04 | Home_C (INPUT)         |
| PA05 | E_Stop (INPUT)         |

Todas las salidas de Step poseen un ancho de pulso de 3.2us aproximadamente. Las entradas de home y de parada de emergencia deben tener un nivel lógico de 0 en estado normal.

**_All Step outputs have a pulse width of approximately 3.2us. The home and emergency stop inputs must have a logic level of 0 in normal state._**

## Configuracion de motores / _motor configuration_
La configuración de los motores se realiza con los comando M1000 a M1004, los cuales permiten setear los valores de velocidad máxima, aceleración, resolución [ step/mm ] ó [ step/grados ] y dirección de movimiento. La lista completa de los parámetros se puede consultar con el comando M999 y toda modificación se puede guardar con el comando M998. 

**_The configuration of the motors is done with the commands M1000 to M1004, which allow setting the values of maximum speed, acceleration, resolution [ step/mm ] or [ step/degrees ] and movement direction. The complete list of parameters can be consulted with the M999 command and any modification can be saved with the M998 command._**

La frecuencia máxima que puede generar para el control de un motor es de 110kHz, la cual se ve reducida según la cantidad de motores a mover de manera simultánea.

**_The maximum frequency that can be generated to control a motor is 110kHz, which is reduced depending on the number of motors to move simultaneously._**

| Numero de motores moviendose en simultaneo | Frecuencia maxima de cada uno. [ Hz ]=[ step/s ] |
| ------------------------------------------ | ------------------------------------------------ |
| 1                                          | 110kHz                                           |
| 2                                          | 110kHz/2=55kHz                                   |
| 3                                          | 110kHz/3=36.66kHz                                |
| 4                                          | 110kHz/4=27.5kHz                                 |
| 5                                          | 110kHz/5=22kHz                                   |


Morcom_v2 detecta la cantidad de motores a mover de manera simultánea en cada trayectoria. Al utilizar OpenPnP en un modo común de funcionamiento como máximo mueve tres motores en simultáneo (X, Y y C). En este caso si alguno de los motores tiene una velocidad de movimiento superior a 36.66 k step/s se vera limitado a moverse a 36.66 k step/s. En las máquinas pick and place la interpolación de ejes no es necesaria, por lo que solo se implementa la interpolación de los ejes X e Y solo si la relación de desplazamiento es superior a 10 veces. Esto último solo se realiza con fines estéticos únicamente.

**_Morcom_v2 detects the number of motors to move simultaneously in each trajectory. When using OpenPnP in a common mode of operation, it moves at most three motors simultaneously (X, Y and C). In this case, if any of the motors has a movement speed greater than 36.66 k step/s, it will be limited to moving at 36.66 k step/s. In pick and place machines, interpolation of axes is not necessary, so interpolation of the X and Y axes is only implemented if the displacement ratio is greater than 10 times. The latter is only performed for aesthetic purposes only._**

## Compilacion, proyecto y grabado / _Build, project and burn_
Si usted sólo desea implementar el firmware Morcom_v2 en el microcontrolador STM32F103 puede simplemente grabar el .bin o .hex directamente, el cual se encuentra en \Debug\Morcom_v2.bin, utilizando STM_utility o STM_programmer. 

**_If you only want to implement the Morcom _ v2 firmware on the STM32F103 microcontroller you can simply write the .bin or .hex directly, which is located in \Debug\Morcom _ v2.bin, using STM _ utility or STM _ programmer._**

Si quiere utilizar el proyecto Morcom_v2 para modificarlo o para cargarlo desde STMCubeIDE puede simplemente importar el proyecto a su espacio de trabajo. También puede utilizar los archivos .h y .c para importarlos a cualquier IDE que desee y de ese modo correr el código en otras arquitecturas. Solo teniendo que cambiar la interacciones con el hardware tanto en el manejo del puerto serie como en las salidas generadas en run_m.c.

**_If you want to use the Morcom _ v2 project to modify it or to load it from STMCubeIDE you can simply import the project into your workspace. You can also use the .h and .c files to import them into any IDE you want to run the code on other architectures. Only having to change the interactions with the hardware both in the management of the serial port and in the outputs generated in run _ m.c._**

## Sugerencias de implementación / _Implementation Tips_
Para obtener los valores tan altos de frecuencias en Morcom_v2 se utilizaron funciones bloqueantes que solo limitan su periodo (1/f) con los tiempos de ejecución de cada ciclo, obteniendo de esta manera frecuencias muy superiores a las que se obtendría si se hiciera uso de interrupciones de tiempo. Esto acarrea la ventaja en frecuencia pero tiene como desventaja que el uC no podrá realizar tareas en “paralelo” mientras realiza el movimiento de los motores.

**_To obtain such high frequency values in Morcom_v2, blocking functions were used that only limit their period (1/f) with the execution times of each cycle, thus obtaining frequencies much higher than those that would be obtained if using time interruptions. This brings the advantage in frequency but has the disadvantage that the uC will not be able to carry out tasks in "parallel" while moving the motors._**

Por este motivo se recomienda activar los periféricos secundarios de la máquina (bomba de vacío, electroválvulas, luces de las cámaras, fedders, etc) con otro uC para poder comentarlos en cualquier momento, el cual puede ser utilizando el mismo puerto serie que Morcom_v2 o utilizando un segundo puerto serie. OpenPnP permite utilizar más de un drive para el manejo de la máquina, así que la segunda opción no presenta ningún inconveniente. De igual modo Morcom_v2 posee un espacio de código comentado donde el usuario puede agregar su procesamiento de las líneas series que este recibe, con el fin de poder comandar más periféricos si así este lo quisiera, con la limitación de que estos no serán comandado mientras los motores se están moviendo.

**_For this reason it is recommended to activate the secondary peripherals of the machine (vacuum pump, solenoid valves, camera lights, fedders, etc) with another uC to be able to comment on them at any time, which can be using the same serial port as Morcom _ v2 or using a second serial port. OpenPnP allows the use of more than one drive to manage the machine, so the second option does not present any inconvenience. In the same way, Morcom_v2 has a commented code space where the user can add his processing of the serial lines that he receives, in order to be able to command more peripherals if he wishes, with the limitation that these will not be commanded while the engines are moving._**
