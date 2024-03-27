*WIP*
TODO: Translate to english.

# Overview

Intenciones:

- Simplicidad por encima de todo (en cuanto a evitar complejidad, no dificultad).
- Fácilmente 'hackeable':
	- Mínimas dependencias posibles y pequeñas y cómodamente integrables en el proyecto (e.g. header-only).
	- No encapsular ningún dato por defecto. Los símbolos internos claramente indicados o separados en otro header, pero disponibles en caso de ser de utilidad.
	- Reducir al mínimo la gestión de memoria, timers, logs, etc. en módulos que no se centren en ello con el fin de facilitar cualquier tipo de integración.
- Compilación rápida.
- Eficiencia y estabilidad.

Me la suda:
- Seguridad frente a ataques malintencionados o ingeniería inversa.
- Type safety (siempre que sea evidente el uso correcto o su representación en memoria).
- Modern ... lo que sea.
- Correctness (en cuanto al uso que suele hacerse de la palabra, que en mi experiencia es cuando no existe un motivo REAL que justifique un incremento de verbosidad o boilerplate realizado).
- Datos privados o encapsulados.
- Patrones de diseño.

# Encapsulation

Se preferirá siempre dejar cualquier símbolo expuesto. En caso de ser de uso interno será prefijado con '_' y acompañado de los comentarios que se crea necesario con fin de clarificar que no forma parte de la API pública.

Motivo: Ofrecer la máxima transparencia respecto al funcionamiento de la librería a cualquier usuario, confiando en que entenderá cuales son los símbolos públicos que debe utilizar sin dificultar el uso de *hacks* en caso de ser necesario. Uno de los grandes objetivos de la librería es que pueda ser útil incluso de formas no contempladas durante su creación.

# Source files

La librería se organiza en módulos. Cada uno de ellos debe de ser totalmente independiente (en medida de lo posible) y tener claramente definida su interfaz, ya sea en forma de datos, callbacks o cualquier tipo de stream.
Por regla general cada módulo tendrá su header público en la carpeta *include* con el prefijo de la librería, su translation unit en la carpeta *src* sin el prefijo y opcionalmente un header interno y más translation units en *src* también.

e.g.

nyas
 -- include
      -- nyas.h
      -- nyas_draw.h
	  -- nyas_math.h
	  -- nyas_scheduler.h
 -- src
      -- draw.h
	  -- draw.c
	  -- draw_gl3.c
	  -- math.c
	  -- scheduler.c

# Type declarations

En headers públicos cada [union], [enum] y [struct] serán declarados con sus respectivos [typedef] utilizando el mismo identificador.
Desgraciadamente el uso de 'c namespaces' es incompatible con c++.

e.g.
typedef struct *TYPE_TAG* {
	STRUCT MEMBERS...
} *TYPE_TAG*;

En el caso de los [enum], por lo general se declararán utilizando el prefijo y se añadirá un [typedef] extra para asignarle un tipo entero. Por lo general el primer valor será _DEFAULT y el último _COUNT.

e.g.
typedef int nyas_texture_format;
typedef enum nyas_texture_format_ {
	NYAS_TEXTURE_FORMAT_DEFAULT,
	NYAS_TEXTURE_FORMAT_RGBA_F16,
	NYAS_TEXTURE_FORMAT_R_8,
	NYAS_TEXTURE_FORMAT_RGB_8,
	...
	NYAS_TEXTURE_FORMAT_COUNT
} nyas_texture_format_;

