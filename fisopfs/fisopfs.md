# fisop-fs

## Estructuras en memoria

![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/224c85c5-4506-4d61-8ec6-0f14ca467af2)

Son 3 estructuras de tamaño estatico, tal que se soporta un nivel de recursion, y hasta 10 archivos de 8kB por directorio.
Tanto subdir como file tienen una variable "occupied" para justamente determinar si estan o no ocupadas.
El struct file contiene una variable "size", necesaria a la hora de appendear sobre archivo, (a pesar de que siempre ocupa FILE_LENGTH+NAME_LENGTH+8 estando ya sea serializado o en memoria)
La serializacion y deserializacion es muy simple a causa del tamaño estatico de las estructuras, solo se crea un archvo de dicho tamaño y se lee y escribe ahi.
(implementadas en serialize_filesystem y deserialize_filesystem)

## Encuentro de archivo o directorio segun path
Las funciones get_subdir_from_path y get_file_from_path parsean el path, ubicando la pocision de los slashes y verificando si dicho directorio o archivo existe, segun su nombre y segun si esta ocupado.

## Persistencia
La deserializacion ocurre en init(), y la serializacion en destroy() y flush()

## Atributos de archivo o directorio
Estos estan hardcodeados en las funciones set_attrib_dir y set_attrib_file. En las invocaciones de set_attrib_file, se modifica el valor de size dado que es necesario que sea el especifico del archivo, por ejemplo a la hora de hacer append al escribir.

