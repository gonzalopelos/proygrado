#include "mbed.h"
#include "FXOS8700Q.h"


//FXOS8700Q acc( A4, A5, FXOS8700CQ_SLAVE_ADDR0); // Proper Ports and I2C address for Freescale Multi Axis shield
//FXOS8700Q mag( A4, A5, FXOS8700CQ_SLAVE_ADDR0); // Proper Ports and I2C address for Freescale Multi Axis shield
FXOS8700Q_acc acc( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // Proper Ports and I2C Address for K64F Freedom board
FXOS8700Q_mag mag( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // Proper Ports and I2C Address for K64F Freedom board

Serial pc(USBTX, USBRX);

MotionSensorDataUnits mag_data;
MotionSensorDataUnits acc_data;

MotionSensorDataCounts mag_raw;
MotionSensorDataCounts acc_raw;


int main() {
float faX, faY, faZ;
float fmX, fmY, fmZ;
int16_t raX, raY, raZ;
int16_t rmX, rmY, rmZ;
acc.enable();
printf("\r\n\nFXOS8700Q Who Am I= %X\r\n", acc.whoAmI());
    while (true) {
        acc.getAxis(acc_data);
        mag.getAxis(mag_data);
        printf("FXOS8700Q ACC: X=%1.4f Y=%1.4f Z=%1.4f  ", acc_data.x, acc_data.y, acc_data.z);
        printf("    MAG: X=%4.1f Y=%4.1f Z=%4.1f\r\n", mag_data.x, mag_data.y, mag_data.z);
        acc.getX(&faX);
        acc.getY(&faY);
        acc.getZ(&faZ);
        mag.getX(&fmX);
        mag.getY(&fmY);
        mag.getZ(&fmZ);
        printf("FXOS8700Q ACC: X=%1.4f Y=%1.4f Z=%1.4f  ", faX, faY, faZ);
        printf("    MAG: X=%4.1f Y=%4.1f Z=%4.1f\r\n", fmX, fmY, fmZ);
        acc.getAxis(acc_raw);
        mag.getAxis(mag_raw);
        printf("FXOS8700Q ACC: X=%d Y=%d Z=%d  ", acc_raw.x, acc_raw.y, acc_raw.z);
        printf("    MAG: X=%d Y=%d Z=%d\r\n", mag_raw.x, mag_raw.y, mag_raw.z);
        acc.getX(&raX);
        acc.getY(&raY);
        acc.getZ(&raZ);
        mag.getX(&rmX);
        mag.getY(&rmY);
        mag.getZ(&rmZ);                
        printf("FXOS8700Q ACC: X=%d Y=%d Z=%d  ", raX, raY, raZ);
        printf("    MAG: X=%d Y=%d Z=%d\r\n\n", rmX, rmY, rmZ);    
        wait(1.0);
    }
}