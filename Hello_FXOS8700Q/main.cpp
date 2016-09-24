/* FXOS8700Q Example Program
 * Copyright (c) 2014-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "FXOS8700Q.h"

Serial pc(USBTX, USBRX);
I2C i2c(PTE25, PTE24);
//FXOS8700Q fxos(i2c, FXOS8700CQ_SLAVE_ADDR1);
FXOS8700QAccelerometer acc(i2c, FXOS8700CQ_SLAVE_ADDR1);    // Configured for the FRDM-K64F with onboard sensors
FXOS8700QMagnetometer mag(i2c, FXOS8700CQ_SLAVE_ADDR1);
//FXOS8700QAccelerometer acc(i2c, FXOS8700CQ_SLAVE_ADDR0);    // Configured for use with the FRDM-MULTI shield
//FXOS8700QMagnetometer mag(i2c, FXOS8700CQ_SLAVE_ADDR0);


int main(void)
{
    motion_data_units_t acc_data, mag_data;
    motion_data_counts_t acc_raw, mag_raw;
    float faX, faY, faZ, fmX, fmY, fmZ, tmp_float;
    int16_t raX, raY, raZ, rmX, rmY, rmZ, tmp_int;

    acc.enable();
    mag.enable();
    printf("FXOS8700QAccelerometer Who Am I= %X\r\n", acc.whoAmI());
    printf("FXOS8700QMagnetometer Who Am I= %X\r\n", acc.whoAmI());
    while (true) {
        // counts based results
        acc.getAxis(acc_raw);
        mag.getAxis(mag_raw);
        printf("ACC: X=%06dd Y=%06dd Z=%06dd \t MAG: X=%06dd Y=%06dd Z=%06dd\r\n", acc_raw.x, acc_raw.y, acc_raw.z, mag_raw.x, mag_raw.y, mag_raw.z);
        acc.getX(raX);
        acc.getY(raY);
        acc.getZ(raZ);
        mag.getX(rmX);
        mag.getY(rmY);
        mag.getZ(rmZ);
        printf("ACC: X=%06dd Y=%06dd Z=%06dd \t MAG: X=%06dd Y=%06dd Z=%06dd\r\n", raX, raY, raZ, rmX, rmY, rmZ);
        printf("ACC: X=%06dd Y=%06dd Z=%06dd \t MAG: X=%06dd Y=%06dd Z=%06dd\r\n", acc.getX(tmp_int), acc.getY(tmp_int), acc.getZ(tmp_int), mag.getX(tmp_int), mag.getY(tmp_int), mag.getZ(tmp_int));
        // unit based results
        acc.getAxis(acc_data);
        mag.getAxis(mag_data);
        printf("ACC: X=%1.4ff Y=%1.4ff Z=%1.4ff \t MAG: X=%4.1ff Y=%4.1ff Z=%4.1ff\r\n", acc_data.x, acc_data.y, acc_data.z, mag_data.x, mag_data.y, mag_data.z);
        acc.getX(faX);
        acc.getY(faY);
        acc.getZ(faZ);
        mag.getX(fmX);
        mag.getY(fmY);
        mag.getZ(fmZ);
        printf("ACC: X=%1.4ff Y=%1.4ff Z=%1.4ff \t MAG: X=%4.1ff Y=%4.1ff Z=%4.1ff\r\n", faX, faY, faZ, fmX, fmY, fmZ);
        printf("ACC: X=%1.4ff Y=%1.4ff Z=%1.4ff \t MAG: X=%4.1ff Y=%4.1ff Z=%4.1ff\r\n", acc.getX(tmp_float), acc.getY(tmp_float), acc.getZ(tmp_float), mag.getX(tmp_float), mag.getY(tmp_float), mag.getZ(tmp_float));
        puts("");
        wait(5.0f);
    }
}