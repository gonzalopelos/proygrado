# Proyecto RAS
Repositorio para versionado de componentes del proyecto RAS
## Configuracion del ambiente de desarrollo
Tenemos dos entornos de desarrollo, uno para desarrollar el firmware de la placa __FDRM-K64__ y otro para desarrollar una aplicacion cliente de consola.
### Entorno de desarrollo para el firmware
* IDE: [Kinetis® Design Studio Integrated Development Environment] (http://www.nxp.com/products/software-and-tools/run-time-software/kinetis-software-and-tools/ides-for-kinetis-mcus/kinetis-design-studio-integrated-development-environment-ide:KDS_IDE#).

### Entorno de desarrollo para la aplicacion cliente de consola
* IDE: [CLion] (https://www.jetbrains.com/clion/download/#section=linux-version).

Para instalarlo seguir los siguientes pasos:

1. Descargarlo desde el link anterior
2. Modificar la fecha de la pc estableciendola algunos anos en el futuro
3. Instalar el IDE
4. Ejecutar el IDE indicando que se desea utilizar la Trial Version
5. Cerrar el IDE.
6. Volver a establecer la fecha de la pc correctamente.
8. Listo, la trial durara hasta que la gente de JetBrains se avive o hasta que se cumpla la fecha establecida  30 dias.

## Deploy del Firmware
Una vez compilado el firmware base simplemente se debe conectar la placa y copiar el archivo __ras_fdrm.bin__ generado en la compilacion al directorio (usb storage) de la placa. Una vez que la placa lo detecta se instala automaticamente, solo resta presionar el boto Reset de dicha placa y se comenzara a ejecutar la rutina del firmware.
