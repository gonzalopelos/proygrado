#include "MotorModule.h"

using namespace mbed;
using namespace modules;

#define VERSION "2.9"

Dm3 *dm3;

extern Mcc mcc;
extern "C" void mbed_reset();
//extern "C" Ticker ticker_pot;
extern "C" Ticker ticker_msg_rate;

#define MSG_RATE 1.0

#define NUMBER_CHASIS 2
#define MOTORS_PER_CHASIS 2 
#define NUMBER_MOTORS (NUMBER_CHASIS*MOTORS_PER_CHASIS)

float calibration[3][2] = {{0.492, -0.87}, {0.715, 0}, {0.949, 0.7}};
float d_p = 0.81/0.88; //width / length TODO

#define MODE_SETVEL 0
#define MODE_SETANGLE 1
#define MODE_RAW 2
int control_mode = MODE_SETVEL;

#define UNSENSORED_CONTROLLER_TYPE 0
#define PID_CONTROLLER_TYPE 1	
#define PP_CONTROLLER_TYPE 2	
int controller_type = UNSENSORED_CONTROLLER_TYPE;

uint8_t chasis_msg_dirty[] = {0, 0};
float vels_target[2][2] = {{0.0,0.0}, {0.0,0.0}}; //velocidad deseada
float vels_target_int[2][2] = {{0.0,0.0}, {0.0,0.0}}; //velocidad deseada intermedia que se va a escalonar para llegar a la deseada usando la aceleración máxima
#define MAX_ACC 0.9 // debe ser un float

#define NEUTRAL_STATE 0
#define ACC_STATE 1
#define DE_ACC_STATE 2
int acc_state[2][2] = {{NEUTRAL_STATE,NEUTRAL_STATE}, {NEUTRAL_STATE,NEUTRAL_STATE}}; // estado para cada rueda de forma de escalonar para llegar a la deseada usando la aceleración máxima
// pows contiene la ultima potencia que se pudo escribir sin error en I2C.
float pows[2][2] = {{0.0,0.0}, {0.0,0.0}}; //potencia actual de los motores
float last_expected_pows[2][2] = {{0.0,0.0}, {0.0,0.0}}; //potencias de salida de los controladores
float expected_pows[2][2] = {{0.0,0.0}, {0.0,0.0}}; //potencias de salida de los controladores
bool i2cerror = false;
bool new_target_vel = false;
int state_hall_in[NUMBER_CHASIS][NUMBER_MOTORS] = {{0,0}, {0,0}}; //para maquina de estado de halls que filtra los flancos ascendentes

#define HALL_ON 1
#define HALL_OFF 0

#define UMBRAL_OFF 0.001
#define UMBRAL_ON 0.999

#define UPDATE_FREQ_CONTROLLER_ON_MS POT_POLL/2
#define UPDATE_FREQ_CONTROLLER_OFF_MS 5*POT_POLL
#define UPDATE_FREQ_HALLS_MS 1
#define RATED_REPORT_MS 1000
#define TIMEOUT_VEL_MOTORS 1000

Mutex dirty_target_vel_mutex;

//AnalogIn halls[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{ENCODER0_IN, ENCODER1_IN}, {ENCODER2_IN, ENCODER3_IN}};

/* halls pooling 
AnalogIn halls0(ENCODER0_IN);
AnalogIn halls1(ENCODER1_IN);
AnalogIn halls2(ENCODER2_IN);
AnalogIn halls3(ENCODER3_IN);

AnalogIn halls[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{halls0, halls1}, {halls2, halls3}};
*/
DigitalIn hall_digital[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{(HALL0_IO), (HALL1_IO)}, {(HALL2_IO), (HALL3_IO)}};

DigitalIn halls0(HALL0_INT);
DigitalIn halls1(HALL1_INT);
DigitalIn halls2(HALL2_INT);
DigitalIn halls3(HALL3_INT);
DigitalIn halls4(HALL4_INT);
DigitalIn halls5(HALL5_INT);

DigitalIn halls[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{halls0, halls1}, {halls2, halls3}};

int hall_sensor_state[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{HALL_OFF,HALL_OFF}, {HALL_OFF,HALL_OFF}};
Timer time_hall[NUMBER_CHASIS][MOTORS_PER_CHASIS];
#define ALPHA 1.0
#define WIN_LEN 10
#define MAX_VEL 5.0 // metros por segundo
#define MIN_VEL 0.1 // metros por segundo

#define MAX_POW 100.0
#define MIN_POW 20.0

float vel_win[NUMBER_CHASIS][MOTORS_PER_CHASIS][WIN_LEN];
int motor_stop[NUMBER_CHASIS][MOTORS_PER_CHASIS]={{1, 1}, {1, 1}};

// http://mathworld.wolfram.com/LagrangeInterpolatingPolynomial.html
float cx1 = calibration[0][0];
float cy1 = calibration[0][1];
float cx2 = calibration[1][0];
float cy2 = calibration[1][1];
float cx3 = calibration[2][0];
float cy3 = calibration[2][1];

float q1 = cy1 / ((cx1-cx2) * (cx1-cx3)); //cy1 = nil
float q2 = cy2 / ((cx2-cx1) * (cx2-cx3)); //cy2 = nil
float q3 = cy3 / ((cx3-cx1) * (cx3-cx2)); //cy3 = nil

float pot_reading = 0;
float pot_angle = 0;
float control_angle, control_modulo = 0;
uint8_t reversed = 0;
//Medidas del robot en m
float P_MITAD_ANCHO = 0.42;
float L_LARGO_TREN = 0.43;
float RADIO_RUEDA = 0.16;

float lineal_X = (18.0*2*M_PI*RADIO_RUEDA)/360.0; // X para regla de 3 de velocidad Lineal
//float lineal_X = 666;

#define STRING_BUFF_SIZE 40
char stringbuff[STRING_BUFF_SIZE];

PIDModule pidModules[NUMBER_CHASIS][MOTORS_PER_CHASIS];

float vels_lineal_actual[NUMBER_CHASIS][MOTORS_PER_CHASIS] =  {{0.0,0.0}, {0.0,0.0}};
int cursor_vel_win[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{0,0},{0,0}};

// los dirty se usan para minimizar el trafico en el serial. si se mando un reporte debido a una solicitud del host no se vuelve a enviar en un hilo con control de rate hasta el siguiente ciclo
bool dirty_pot[NUMBER_CHASIS] = {true, true};
bool dirty_target_vel[NUMBER_CHASIS] = {true, true};
bool dirty_power[NUMBER_CHASIS] = {true, true};

char msg_debug[100];

static void report_enable();
static void report_pid_parameters();
static void report_set_controller();
float time2vel(float time_elapsed);
void zero_vel_win(int chasis, int motor);


void report_debug(char * str, int len) {
	mcc.encoder.startFrame();
	mcc.encoder.push(MOTOR_PID);
	mcc.encoder.push(OPCODE_DEBUG);
	mcc.encoder.startList();
	//mcc.encoder.push(admin_str, sizeof(admin_str)-1);
	//mcc.encoder.push(MODULE_VERSION);
	mcc.encoder.push(str, len-1);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

float pot_angle_calibrator ( float x ) {
  float x_x1 = x-cx1;
  float x_x2 = x-cx2;
  float x_x3 = x-cx3;
  float p_x = x_x2 * x_x3 * q1 + x_x1 * x_x3 * q2 + x_x1 * x_x2 * q3;
  return p_x;
}

float median(int n, float x[]) {
    float temp;
    int i, j;
    // los siguientes dos loops ordenan en orden ascendente 
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(x[j] < x[i]) {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

/*
    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
        return((x[n/2] + x[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        return x[n/2];
    }

*/
    int cant_sumas = 0;
    float prom = 0;
    for(i=n/3; i<(2*n/3); i++) {
		prom = prom + x[i];
		cant_sumas ++;
	}
    
    

	return prom/(float)cant_sumas;
 }

void compute_actuals_vels(int chasis, int motor) {
	long time_elapsed = time_hall[chasis][motor].read_ms();
	float current_vel = time2vel(time_elapsed);	
	if (current_vel < MIN_VEL){
		//vel_win[chasis][motor][(cursor_vel_win[chasis][motor])] = 0;
        //cursor_vel_win[chasis][motor] = (cursor_vel_win[chasis][motor] + 1)%WIN_LEN;
        zero_vel_win(chasis, motor);
		vels_lineal_actual[chasis][motor] = 0;
		motor_stop[chasis][motor] = 1; 
		//motor_stop[chasis][iter_motor] = 1; 
	} else
		vels_lineal_actual[chasis][motor] = median(WIN_LEN, vel_win[chasis][motor]);	 
	
	//strncpy(msg_debug, "cav proc!", sizeof(msg_debug));
	//snprintf(msg_debug, sizeof(msg_debug), "%s", "cav proc!");
    //report_debug(msg_debug, strlen(msg_debug));
   }

unsigned int len;

void report_chasis_opcode(int opcode, int chasis, float info[NUMBER_CHASIS][MOTORS_PER_CHASIS]) {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID));
	mcc.encoder.push(opcode);
	mcc.encoder.startList();
	mcc.encoder.push(chasis + 1);
	len = snprintf(stringbuff, STRING_BUFF_SIZE, "%.3f",info[chasis][0]);
	mcc.encoder.push(stringbuff, len);
	len = snprintf(stringbuff, STRING_BUFF_SIZE, "%.3f",info[chasis][1]);
	mcc.encoder.push(stringbuff, len);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

void report_set_pid_parameters(float kp, float ki, float kd) {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID ));
	mcc.encoder.push(OPCODE_SET_PID_PARAMETERS);
	mcc.encoder.startList();
	//mcc.encoder.push(chasis + 1);
	unsigned int len;
	len = snprintf(stringbuff, STRING_BUFF_SIZE, "%.3f",kp);
	mcc.encoder.push(stringbuff, len);
	len = snprintf(stringbuff, STRING_BUFF_SIZE, "%.3f",ki);
	mcc.encoder.push(stringbuff, len);
	len = snprintf(stringbuff, STRING_BUFF_SIZE, "%.3f",kd);
	mcc.encoder.push(stringbuff, len);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

void report_power(int chasis) {
	//char i2cwfailure_str[] = "I2CWRITEFAILURE";
	//if (chasis_msg_dirty[chasis]==1 ) {
		chasis_msg_dirty[chasis]=0;
		if (i2cerror == true) {
			i2cerror = false;
			mcc.encoder.startFrame();
			mcc.encoder.push(MOTOR_PID);
			mcc.encoder.push(OPCODE_REPORT);
			mcc.encoder.startList();
			mcc.encoder.push(i2cwfailure_str, sizeof(i2cwfailure_str)-1);
			mcc.encoder.endList();
			mcc.encoder.endFrame();
		}
		report_chasis_opcode(OPCODE_SET_POWER_MOTOR, chasis, pows);
	//} // dirty chasis
}

void report_set_target_vel(int chasis) {
	report_chasis_opcode(OPCODE_SET_TARGET_VEL, chasis, vels_target);
}

float time2vel(float time_elapsed) {
	return (lineal_X/(float)time_elapsed)*1000;
}

void zero_vel_win(int chasis, int motor) {
	memset(vel_win[chasis][motor], 0, sizeof(vel_win[chasis][motor]));
	//for (int iter_win = 0; iter_win < WIN_LEN; iter_win ++)
	//	vel_win[chasis][motor][iter_win] = 0;
	//char * msg = "ZERO_VELS proc!"; 
	//report_debug(msg, strlen(msg));
	//snprintf(msg_debug, sizeof(msg_debug), "%s", "ZERO_VELS proc!");
	//report_debug(msg_debug, strlen(msg_debug));

}

void report_get_vel_change(int chasis) {
	for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
	    	compute_actuals_vels(chasis, iter_motor);
			//vels_lineal_actual[chasis][iter_motor] =121;
	}
	report_chasis_opcode(OPCODE_GET_VEL_CHANGE, chasis, vels_lineal_actual);
}

void report_get_pot_change(int chasis) {
	float pot_info[NUMBER_CHASIS][MOTORS_PER_CHASIS] = {{control_angle, -1.0},{pot_angle, pot_reading}};
	report_chasis_opcode(OPCODE_GET_POT_CHANGE, chasis, pot_info);
}

int set_motors_power(int chasis, int motor, float power) {
	int i2cerror = dm3->motor_i2c(chasis*MOTORS_PER_CHASIS + motor, power);

	if(i2cerror == 0){
		pows[chasis][motor] = power;
	} else {
		expected_pows[chasis][motor] = 0;
		last_expected_pows[chasis][motor] = 0;
	}	
		// FIX: podría hacerse algo mas con las potencias cuando hay error?

	return i2cerror;
}

void compute_motors(unsigned int chasis, float fangle_local) {
	float fx = control_modulo * cos(fangle_local);
	float fy = control_modulo * sin(fangle_local);
	float out_l = fx + d_p*fy;
	float out_r = fx - d_p*fy;

	// originalmente seteaban potencias entre 0 - 100 ahora es una velocidad lineal.
	// Probar si no se estabilizan las velocidades
	//last_expected_pows[chasis][0] = last_expected_pows[chasis][0]/vels_target[chasis][0] * out_l;
	//last_expected_pows[chasis][1] = last_expected_pows[chasis][1]/vels_target[chasis][1] * out_r;
	/*if (out_l > vels_target[chasis][0])
		acc_state[chasis][0] = ACC_STATE;
	else if (out_l < vels_target[chasis][0])
		acc_state[chasis][0] = DE_ACC_STATE;
	if (out_r > vels_target[chasis][1])
		acc_state[chasis][1] = ACC_STATE;
	else if (out_r < vels_target[chasis][1])
		acc_state[chasis][1] = DE_ACC_STATE;
*/
	vels_target[chasis][0] = out_l;
	vels_target[chasis][1] = out_r;
}

//pancho ecuations
void compute_motors_angle(float control_angle, float pot_angle) {
	//asumiendo w max 150 rpm

	//control_angle y pot_angle deben estar en radianes
	//float control_angle_ajustado = control_angle/2.5;
	float w1ms = control_modulo;
	float w2ms = (P_MITAD_ANCHO) / (L_LARGO_TREN);
	float tanControl = tan(control_angle/2);
	float tanPot = tan(-pot_angle/2);

	vels_target[0][0] = w1ms * (1 + w2ms*tanControl); 		    //del izq R2 00   w1 *(1 + w2ms*tanControl)
	vels_target[0][1] = w1ms * (1 - w2ms*tanControl);     	    //del der R1 01
	vels_target[1][0] = w1ms * (1 + w2ms*tanPot); 				//atr izq R4 10
	vels_target[1][1] = w1ms * (1 - w2ms*tanPot);     			//atr der R3 11

	/*
	int ret_fl = dm3->motor_i2c(0, vels_target[0][0]);
	int ret_fr = dm3->motor_i2c(1, vels_target[0][1]);
	int ret_rl = dm3->motor_i2c(2, vels_target[1][0]);
	int ret_rr = dm3->motor_i2c(3, vels_target[1][1]);*/

	//int ret_rl = dm3->motor_i2c(2, 0);
	//int ret_rr = dm3->motor_i2c(2, 0);

	//i2cerror = (ret_fl!=0 || ret_fr!=0  || ret_rl!=0  || ret_rr!=0);
}
//DigitalOut led3(LED3);

// transforma de forma lineal la velocidad objetivo a potencia
void unsensored_controller() {
	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++)
		for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
			if (vels_target[iter_chasis][iter_motor] > MAX_VEL) vels_target[iter_chasis][iter_motor] = MAX_VEL;
			else if (vels_target[iter_chasis][iter_motor] < -MAX_VEL) vels_target[iter_chasis][iter_motor] = -MAX_VEL;

			expected_pows[iter_chasis][iter_motor] = vels_target[iter_chasis][iter_motor] / MAX_VEL * MAX_POW;
		}
}

void pid_controller() {
	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++)
		for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
			compute_actuals_vels(iter_chasis, iter_motor);
			// rampa de velocidad con aceleracion maxima
			if (vels_target_int[iter_chasis][iter_motor] >= vels_target[iter_chasis][iter_motor])
				vels_target_int[iter_chasis][iter_motor] = vels_target[iter_chasis][iter_motor];
			else 
				vels_target_int[iter_chasis][iter_motor] = MAX_ACC * UPDATE_FREQ_CONTROLLER_ON_MS/1000 + vels_target_int[iter_chasis][iter_motor];

			expected_pows[iter_chasis][iter_motor] = pidModules[iter_chasis][iter_motor].compute(vels_lineal_actual[iter_chasis][iter_motor], vels_target_int[iter_chasis][iter_motor]);
			/*
			if(iter_chasis == 0 && iter_motor == 0){
				//snprintf(msg_debug, sizeof(msg_debug), "pid ver-%s %f %f %f input: %f setPoint: %f", VERSION, pidModules[iter_chasis][iter_motor].getITerm(), pidModules[iter_chasis][iter_motor].getError(), pidModules[iter_chasis][iter_motor].getDInput(),pidModules[iter_chasis][iter_motor].getInputDeb(),pidModules[iter_chasis][iter_motor].getSetPointDeb());//expected_pows[iter_chasis][iter_motor], vels_lineal_actual[iter_chasis][iter_motor]);
    			snprintf(msg_debug, sizeof(msg_debug), "pid ver-%s vel_int: %f", VERSION, vels_target_int[iter_chasis][iter_motor]);
    			report_debug(msg_debug, strlen(msg_debug));
    		}
    		*/
		}
}

float sign(float x) {	
    return (x >= 0) - (x < 0);
}

// proportional power controller
void pp_controller() {
	float vel_error;
	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++)
		for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
			compute_actuals_vels(iter_chasis, iter_motor);
/*
			// rampa de velocidad con aceleracion maxima
			if (acc_state[iter_chasis][iter_motor]==ACC_STATE) {
				if (vels_target_int[iter_chasis][iter_motor] >= vels_target[iter_chasis][iter_motor]) {
					vels_target_int[iter_chasis][iter_motor] = vels_target[iter_chasis][iter_motor];
					acc_state [iter_chasis][iter_motor] = NEUTRAL_STATE;
				} else 
					vels_target_int[iter_chasis][iter_motor] = MAX_ACC * UPDATE_FREQ_CONTROLLER_ON_MS/1000 + vels_target_int[iter_chasis][iter_motor];
			} else if (acc_state[iter_chasis][iter_motor] == DE_ACC_STATE) {
				if (vels_target_int[iter_chasis][iter_motor] <= vels_target[iter_chasis][iter_motor]) {
					vels_target_int[iter_chasis][iter_motor] = vels_target[iter_chasis][iter_motor];
					acc_state [iter_chasis][iter_motor] = NEUTRAL_STATE;
				} else 
					vels_target_int[iter_chasis][iter_motor] = -MAX_ACC * UPDATE_FREQ_CONTROLLER_ON_MS/1000 + vels_target_int[iter_chasis][iter_motor];
			}
*/
			//vel_error = vels_target_int[iter_chasis][iter_motor] - vels_lineal_actual[iter_chasis][iter_motor];
			vel_error = vels_target[iter_chasis][iter_motor] - vels_lineal_actual[iter_chasis][iter_motor];
			//FIXME: determinar signo con los halls
			expected_pows[iter_chasis][iter_motor] = last_expected_pows[iter_chasis][iter_motor] + ALPHA * vel_error;
			if (vels_target[iter_chasis][iter_motor]==0 && vel_error==0)
				expected_pows[iter_chasis][iter_motor] = 0;
			else if (vels_target[iter_chasis][iter_motor]>=0 && expected_pows[iter_chasis][iter_motor]<=0) //capaz no va
				expected_pows[iter_chasis][iter_motor] = 0;
			else if(expected_pows[iter_chasis][iter_motor] > MAX_POW)
				expected_pows[iter_chasis][iter_motor] = MAX_POW;
			else if(expected_pows[iter_chasis][iter_motor] < -MAX_POW)
				expected_pows[iter_chasis][iter_motor] = -MAX_POW;
			
			//if(iter_chasis == 0 && iter_motor == 0){
				//snprintf(msg_debug, sizeof(msg_debug), "pid ver-%s %f %f %f input: %f setPoint: %f", VERSION, pidModules[iter_chasis][iter_motor].getITerm(), pidModules[iter_chasis][iter_motor].getError(), pidModules[iter_chasis][iter_motor].getDInput(),pidModules[iter_chasis][iter_motor].getInputDeb(),pidModules[iter_chasis][iter_motor].getSetPointDeb());//expected_pows[iter_chasis][iter_motor], vels_lineal_actual[iter_chasis][iter_motor]);
    			//snprintf(msg_debug, sizeof(msg_debug), "pid ver-%s state: %i", VERSION, acc_state [iter_chasis][iter_motor]);
    			//report_debug(msg_debug, strlen(msg_debug));
    		//}
    		last_expected_pows[iter_chasis][iter_motor] = expected_pows[iter_chasis][iter_motor];
		}
}

void flip_msg() {
	//chasis_msg_dirty[0]=1;
	chasis_msg_dirty[1]=1;
}

static int handle_set_target_vel(unsigned int  pid, unsigned int  opcode) {
	//mcc.send_message(pid, OPCODE_PING, data, data_length);
	if (mcc.incomming_params_count!=2) return -1;
	if (control_mode!=MODE_SETVEL && control_mode!=MODE_SETANGLE) return -2;

	new_target_vel = true;
	
	memcpy(stringbuff, mcc.incomming_params_s[0], mcc.incomming_params_n[0]);
	stringbuff[mcc.incomming_params_n[0]] = 0;
	control_modulo = atof(stringbuff);

	memcpy(stringbuff, mcc.incomming_params_s[1], mcc.incomming_params_n[1]);
	stringbuff[mcc.incomming_params_n[1]] = 0;
	control_angle = -atof(stringbuff);

	if (control_mode == MODE_SETVEL) {
		compute_motors(0, control_angle);
    	compute_motors(1, pot_angle-control_angle);
	} else if (control_mode == MODE_SETANGLE) {
		compute_motors_angle(control_angle, pot_angle);
	}
	/*
	if(controller_type == PID_CONTROLLER_TYPE){
		for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++) {
			for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++)	{
				pidModules[iter_chasis][iter_motor].new_set_vel();
			}
		}	
	}
	*/
	
	
	dirty_target_vel_mutex.lock();
	dirty_target_vel[0]=false;
	dirty_target_vel_mutex.unlock();
    report_set_target_vel(0); //reportar chasis 0
	return 1;
}

static int handle_motor_power_write(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=3) return -1;
	if (control_mode!=MODE_RAW) return -2;

	int chasis = mcc.incomming_params_n[0]-1;
	int motor = mcc.incomming_params_n[1]-1;

	memcpy(stringbuff, mcc.incomming_params_s[2], mcc.incomming_params_n[2]);
	stringbuff[mcc.incomming_params_n[2]] = 0;

	if (chasis<0 || chasis>1 || motor<0 || motor>1 ) return -2;

	if (dm3->enable()) {
		int ret = set_motors_power(chasis, motor, atof(stringbuff));
		i2cerror = (ret!=0) || i2cerror;
	}

    chasis_msg_dirty[chasis] = 1;
    dirty_power[chasis]=false;
    report_power(chasis);

	return 1;
}

static int handle_get_vel_change(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;

	int chasis = mcc.incomming_params_n[0]-1;

	if (chasis<0 || chasis>NUMBER_CHASIS) return -2;

    report_get_vel_change(chasis);

	return 1;
}

static int handle_get_pot_change(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;

	int chasis = mcc.incomming_params_n[0]-1;

	if (chasis<0 || chasis>NUMBER_CHASIS) return -2;
	dirty_pot[chasis]=false;
    report_get_pot_change(chasis);

	return 1;
}

static void report_drivemode() {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID ));
	mcc.encoder.push(OPCODE_SET_DRIVE_MODE);
	mcc.encoder.startList();
	mcc.encoder.push(control_mode);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

static int handle_setdrivemode(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;

	control_mode = mcc.incomming_params_n[0];
	report_drivemode();
	//char * msg = "handle_setdrivemode proc!"; report_debug(msg, strlen(msg));

	return 1;
}

static void report_brake() {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID ));
	mcc.encoder.push(OPCODE_BRAKE);
	mcc.encoder.startList();
	mcc.encoder.push(dm3->brake());
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

static int handle_brake(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;

	int ret = dm3->brake(mcc.incomming_params_n[0]);
	report_brake();
	return ret==mcc.incomming_params_n[0];
}

static void report_enable() {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID ));
	mcc.encoder.push(OPCODE_ENABLE);
	mcc.encoder.startList();
	mcc.encoder.push(dm3->enable());
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

static int handle_enable(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;

	int enabled = dm3->enable(mcc.incomming_params_n[0]);
	report_enable();

	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++){
		for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {			
			int ret = set_motors_power(iter_chasis, iter_motor, 0.0);
			i2cerror = (ret!=0) || i2cerror;
			vels_target[iter_chasis][iter_motor] = 0;
		}
		dirty_power[iter_chasis]=false;
		report_power(iter_chasis);

		dirty_target_vel_mutex.lock();
		dirty_target_vel[iter_chasis]=false;
		dirty_target_vel_mutex.unlock();
		report_set_target_vel(iter_chasis);

	}

	return enabled==mcc.incomming_params_n[0];
}

void report_reversed() {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID ));
	mcc.encoder.push(OPCODE_REVERSE);
	mcc.encoder.startList();
	mcc.encoder.push(reversed);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

static int handle_reverse(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;

	uint8_t ret = dm3->enable(mcc.incomming_params_n[0]);

	if ( (ret!=0 && reversed==0) || (ret==0&&reversed==1)) {
		reversed = 1-reversed;
		int swap;
		swap = dm3->motor_i2c_address[0];
		dm3->motor_i2c_address[0] = dm3->motor_i2c_address[3];
		dm3->motor_i2c_address[3] = swap;
		swap = dm3->motor_i2c_address[1];
		dm3->motor_i2c_address[1] = dm3->motor_i2c_address[2];
		dm3->motor_i2c_address[2] = swap;
		dm3->motor_mult[0] = -dm3->motor_mult[0];
		dm3->motor_mult[1] = -dm3->motor_mult[1];
		dm3->motor_mult[2] = -dm3->motor_mult[2];
		dm3->motor_mult[3] = -dm3->motor_mult[3];

		report_reversed();
	}
	return 1;
}


static int handle_test(unsigned int  pid, unsigned int  opcode) {
	Thread::wait(2000);
	dm3->motor_i2c(0, 50);
	dm3->motor_i2c(1, 50);
	dm3->motor_i2c(2, 50);
	dm3->motor_i2c(3, 50);
    Thread::wait(1000);
	dm3->motor_i2c(0, -50);
	dm3->motor_i2c(1, -50);
	dm3->motor_i2c(2, -50);
	dm3->motor_i2c(3, -50);
    Thread::wait(1000);
	dm3->motor_i2c(0, 0);
	dm3->motor_i2c(1, 0);
	dm3->motor_i2c(2, 0);
	dm3->motor_i2c(3, 0);
	return 1;
}



static int handle_report(unsigned int  pid, unsigned int  opcode) {
	mcc.encoder.startFrame();
	mcc.encoder.push((long int)MOTOR_PID);
	mcc.encoder.push(OPCODE_REPORT);
	mcc.encoder.startList();
	//mcc.encoder.push(motor_str, sizeof(motor_str)-1);
	//mcc.encoder.push(MODULE_VERSION);
	mcc.encoder.push(ok_str, sizeof(ok_str)-1);
	mcc.encoder.endList();
	mcc.encoder.endFrame();

	report_brake();
	report_enable();
	report_reversed();
	report_drivemode();
	report_pid_parameters();
	report_set_controller();

	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++) {
		report_power(iter_chasis);
		report_set_target_vel(iter_chasis);
	}
	// FIX: New operarions

	
	/*if (dm3->horn()==1) {
		mcc.encoder.push(hornon_str, sizeof(hornon_str)-1);
	} else {
		mcc.encoder.push(hornoff_str, sizeof(hornoff_str)-1);
	}*/

	return 1;
}

static void tick() {
	if (dm3->enable()) {
		// Como funciona esto del tick??
	}
}

/* halls pooling
AnalogIn get_hall(int chasis, int motor) {
*/
DigitalIn get_hall(int chasis, int motor) {
	return halls[chasis][motor];
}


void int_hall_rise(int chasis, int motor) {
	long time_elapsed = time_hall[chasis][motor].read_ms();
	int hall_value;
	float current_vel;
	if (state_hall_in[chasis][motor] == HALL_ON)
		return;
	state_hall_in[chasis][motor] = HALL_ON;
	// si se hace con el array no anda
	if ((chasis==0) && (motor==0)) {
		hall_value=halls0.read();
	} else if ((chasis==0) && (motor==1)) {
        hall_value=halls1.read();
    } else if ((chasis==1) && (motor==0)) {
        hall_value=halls2.read();
    } else if ((chasis==1) && (motor==1)) {
        hall_value=halls3.read();
    } else {  
		hall_value=0;
	} 
    if ((time_elapsed>0) && (hall_value == 1)){ // para no dividir entre cero y asegurarse que no fue ruido
    	time_hall[chasis][motor].reset();
        if (motor_stop[chasis][motor]) {
            motor_stop[chasis][motor] = 0;
        } else {
            current_vel = time2vel(time_elapsed);
            if ((current_vel > MIN_VEL) && (current_vel < MAX_VEL)) {
                vel_win[chasis][motor][(cursor_vel_win[chasis][motor])]=current_vel;
                cursor_vel_win[chasis][motor] = (cursor_vel_win[chasis][motor] + 1)%WIN_LEN;                
            } //else {
              //  char * msg = "vel err!"; report_debug(msg, strlen(msg)); //TODO
            //}
            //Para arreglar que sete a cero las velocidades

        }
    }
}

void int_hall_fall(int chasis, int motor) {
	if (state_hall_in[chasis][motor] == HALL_ON)
		state_hall_in[chasis][motor] = HALL_OFF;
}

#define LEN_SEC_HALL 6
// secuencia en regimen de los halls
int HALL_SEC[LEN_SEC_HALL]={5,4,6,2,3,1};


// maquina de estado alternativa que considera dos halls por rueda y tiene en cuenta que solo una vez se pasa por cero.
int hall_sec_state[MOTORS_PER_CHASIS] = {0,0};
int hall_value[MOTORS_PER_CHASIS];
int direction[MOTORS_PER_CHASIS] = {1,1};
int last_hall_value[MOTORS_PER_CHASIS] = {0,0};
void check_halls() {
	//int hall_value[MOTORS_PER_CHASIS];
	long time_elapsed;
	int chasis = 0; // por ahora para un solo chasis
	float current_vel;
    bool update;   
    for(int motor = 0; motor < MOTORS_PER_CHASIS; motor++){

	    if (motor == 0)
			hall_value[0] = (halls2.read()<<2) + (halls1.read()<<1) + halls0.read();
		else
			hall_value[1] = (halls5.read()<<2) + (halls4.read()<<1) + halls3.read();
		
		update = false;
		if(last_hall_value[motor] == hall_value[motor]){			
			switch(hall_sec_state[motor]){
				case 0:
					if (hall_value[motor]==HALL_SEC[1]){
						hall_sec_state[motor] = 1;					
					}
					else if (hall_value[motor]==HALL_SEC[5]){
						hall_sec_state[motor] = 11;
					}
					else {
						hall_sec_state[motor] = 0;
					}
					break;
				case 1:
					if (hall_value[motor]==HALL_SEC[2]){
						hall_sec_state[motor] = 2;
					}
					else if (hall_value[motor]==HALL_SEC[1]){
						hall_sec_state[motor] = 1;
					}
					else{
						hall_sec_state[motor] = 0;
					}
					break;	
				case 2:
					if (hall_value[motor]==HALL_SEC[3]){
						hall_sec_state[motor] = 3;
					}
					else if (hall_value[motor]==HALL_SEC[2]){
						hall_sec_state[motor] = 2;
					}
					else {
						hall_sec_state[motor] = 0;
					}
					break;	
				case 3:
					if (hall_value[motor]==HALL_SEC[4]){
						hall_sec_state[motor] = 4;
					}
					else if (hall_value[motor]==HALL_SEC[3]){
						hall_sec_state[motor] = 3;
					}
					else {
						hall_sec_state[motor] = 0;
					}	
					break;
				case 4:
					if (hall_value[motor]==HALL_SEC[5]){
						hall_sec_state[motor] = 5;
					}
					else if (hall_value[motor]==HALL_SEC[4]){
						hall_sec_state[motor] = 4;
					}
					else {
						hall_sec_state[motor] = 0;
					}	
					break;				
				case 5:
					if (hall_value[motor]==HALL_SEC[0]){				
						update = true;
						hall_sec_state[motor] = 0;
						direction[motor] = -1;
					}
					else if (hall_value[motor]==HALL_SEC[5]){
						hall_sec_state[motor] = 5;
					}
					else {
						hall_sec_state[motor] = 0;
					}
					break;	
				case 11:
					if (hall_value[motor]==HALL_SEC[4]){
						hall_sec_state[motor] = 12;
					}
					else if (hall_value[motor]==HALL_SEC[5]){
						hall_sec_state[motor] = 11;
					}
					else {
						hall_sec_state[motor] = 0;
					}	
					break;
				case 12:
					if (hall_value[motor]==HALL_SEC[3]){
						hall_sec_state[motor] = 13;
					}
					else if (hall_value[motor]==HALL_SEC[4]){
						hall_sec_state[motor] = 12;
					}
					else {
						hall_sec_state[motor] = 0;
					}	
					break;
				case 13:
					if (hall_value[motor]==HALL_SEC[2]){
						hall_sec_state[motor] = 14;
					}
					else if (hall_value[motor]==HALL_SEC[3]){
						hall_sec_state[motor] = 13;
					}
					else {
						hall_sec_state[motor] = 0;
					}
					break;
				case 14:
					if (hall_value[motor]==HALL_SEC[1]){
						hall_sec_state[motor] = 15;
					}
					else if (hall_value[motor]==HALL_SEC[2]){
						hall_sec_state[motor] = 14;
					}
					else {
						hall_sec_state[motor] = 0;
					}	
					break;
				case 15:			
					if (hall_value[motor]==HALL_SEC[0]){
						hall_sec_state[motor] = 0;				
						update = true;
						direction[motor] = 1;
					}
					else if (hall_value[motor]==HALL_SEC[1]){
						hall_sec_state[motor] = 15;
					}
					else {
						hall_sec_state[motor] = 0;
					}
					break;
			}
			if (update) {
				time_elapsed = time_hall[chasis][motor].read_ms();
				if (time_elapsed>0){ // para no dividir entre cero
				    time_hall[chasis][motor].reset();
				    if (motor_stop[chasis][motor]) {
				        motor_stop[chasis][motor] = 0;
				    } else {
				        current_vel = time2vel(time_elapsed);
				        if ((current_vel > MIN_VEL) && (current_vel < MAX_VEL)) {
				            vel_win[chasis][motor][(cursor_vel_win[chasis][motor])]=current_vel*direction[motor]*sign(dm3->motor_mult[chasis*MOTORS_PER_CHASIS+motor]);
				           // vel_win[chasis][motor][(cursor_vel_win[chasis][motor])]=current_vel*sign(pows[0][motor]); 
				            cursor_vel_win[chasis][motor] = (cursor_vel_win[chasis][motor] + 1)%WIN_LEN;
			            //	snprintf(msg_debug, sizeof(msg_debug), "hc(%s) new vel: %f . fin", VERSION, vel_win[chasis][motor][(cursor_vel_win[chasis][motor])]);
					//		report_debug(msg_debug, strlen(msg_debug));
				        }
				    }
				}
			}
		}
		last_hall_value[motor] = hall_value[motor];
/*
		if (motor == 1){
			snprintf(msg_debug, sizeof(msg_debug), "hc(%s) hall_v: %i - state: %i . fin", VERSION, hall_value[1],hall_sec_state[1]);
			report_debug(msg_debug, strlen(msg_debug));
		}*/
	 	
	}

}

Ticker ticker;

//Inicia temporizadores
void MotorModule::init(){
	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++) 
		for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++)	{
			time_hall[iter_chasis][iter_motor].reset();
			time_hall[iter_chasis][iter_motor].start();
			pidModules[iter_chasis][iter_motor].setSampleTime(UPDATE_FREQ_CONTROLLER_ON_MS);
			//hall[i].fall(&hall_raised[i]);
			//.mode(PullUp);
		}

	// si se hace con el array no anda
	halls0.mode(PullUp); 
	halls1.mode(PullUp); 
	halls2.mode(PullUp);
	halls3.mode(PullUp); 
	halls4.mode(PullUp); 
	halls5.mode(PullUp); 

	ticker.attach(&check_halls, 0.0005); 
}

static int handle_set_pid_parameters(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=3) return -1;

	memcpy(stringbuff, mcc.incomming_params_s[0], mcc.incomming_params_n[0]);
	stringbuff[mcc.incomming_params_n[0]] = 0;
	float kp = atof(stringbuff);

	memcpy(stringbuff, mcc.incomming_params_s[1], mcc.incomming_params_n[1]);
	stringbuff[mcc.incomming_params_n[1]] = 0;
	float ki = atof(stringbuff);

	memcpy(stringbuff, mcc.incomming_params_s[2], mcc.incomming_params_n[2]);
	stringbuff[mcc.incomming_params_n[2]] = 0;
	float kd = atof(stringbuff);

	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++){
		for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
			pidModules[iter_chasis][iter_motor].setTunings(kp, ki, kd);	
		}
    }
	report_set_pid_parameters(kp, ki, kd);

	return 1;
}

static void report_pid_parameters(){
	double kp = pidModules[0][0].getKp();
	double ki = pidModules[0][0].getKi();
	double kd = pidModules[0][0].getKd();
	report_set_pid_parameters(kp, ki, kd);
}

static void report_set_controller() {
	mcc.encoder.startFrame();
	mcc.encoder.push((unsigned int) (MOTOR_PID ));
	mcc.encoder.push(OPCODE_SET_CONTROLLER);
	mcc.encoder.startList();
	mcc.encoder.push(controller_type);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

static int handle_set_controller(unsigned int  pid, unsigned int  opcode) {
	if (mcc.incomming_params_count!=1) return -1;
	controller_type = mcc.incomming_params_n[0];
	if (!((controller_type == UNSENSORED_CONTROLLER_TYPE) or (controller_type == PID_CONTROLLER_TYPE) or (controller_type == PP_CONTROLLER_TYPE))) return -2;

	report_set_controller();
	return 1;	
}


/* halls pooling ACTUALMENTE NO ESTA CORRIENDO ESTE HILO*/
void MotorModule::compute_speed_task(void const *argument) {
	int iter_chasis, iter_motor;
	long time_elapsed;

    while (true) {
	    	for (iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++)
	    		for (iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
		    		time_elapsed = time_hall[iter_chasis][iter_motor].read_ms();
		    		/*
		    		if(time_elapsed > KILLING_TIME_MS){ //Si paso un tiempo en ms sin recibir velocidad 
		    			vel_win[iter_chasis][iter_motor][(cursor_vel_win[iter_chasis][iter_motor])]=0.0;
		    			cursor_vel_win[iter_chasis][iter_motor] = (cursor_vel_win[iter_chasis][iter_motor] + 1)%WIN_LEN;
		    		}*/
		    		if ((hall_sensor_state[iter_chasis][iter_motor]==HALL_OFF) && (get_hall(iter_chasis, iter_motor).read() >= UMBRAL_ON)) {	
		    			time_hall[iter_chasis][iter_motor].reset();
		    			vels_lineal_actual[iter_chasis][iter_motor]  = (lineal_X/(float)time_elapsed)*1000;
		    			hall_sensor_state[iter_chasis][iter_motor] = HALL_ON;		    			
	   				    vel_win[iter_chasis][iter_motor][(cursor_vel_win[iter_chasis][iter_motor])]=vels_lineal_actual[iter_chasis][iter_motor];
	    				cursor_vel_win[iter_chasis][iter_motor] = (cursor_vel_win[iter_chasis][iter_motor] + 1)%WIN_LEN;
		    		} else if ((hall_sensor_state[iter_chasis][iter_motor] == HALL_ON) && (get_hall(iter_chasis, iter_motor).read() <= UMBRAL_OFF)) {
		    			hall_sensor_state[iter_chasis][iter_motor] = HALL_OFF;
		    		}
	    		}
    		Thread::wait(UPDATE_FREQ_HALLS_MS);
	}
}

void MotorModule::potpoll_task(void const *argument) {
//	led3 = false; //!led3;_SETANLGE
    while (true) {
//		led3 = true; //!led3;
		pot_reading = dm3->get_pot();
		pot_angle = pot_angle_calibrator(pot_reading);

		if (dm3->enable() && (control_mode != MODE_RAW)) { //si esta enabled y no en modo raw

			if (control_mode == MODE_SETVEL) {
				compute_motors(1, control_angle+pot_angle);
			} else if (control_mode == MODE_SETANGLE) {
				compute_motors_angle(control_angle, pot_angle);
			}

			if (controller_type == UNSENSORED_CONTROLLER_TYPE)
				unsensored_controller();
			else if (controller_type == PID_CONTROLLER_TYPE)
				pid_controller();
			else if (controller_type == PP_CONTROLLER_TYPE)
				pp_controller();

	    	for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++){
				for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
	    			int ret = set_motors_power(iter_chasis, iter_motor, expected_pows[iter_chasis][iter_motor]);
	    			i2cerror = (ret!=0) || i2cerror;
	    		}
	    	}
	    	Thread::wait(UPDATE_FREQ_CONTROLLER_ON_MS);
	    } else
			Thread::wait(UPDATE_FREQ_CONTROLLER_OFF_MS);

		//Thread::wait(POT_POLL);
	}

}

void MotorModule::security_stop_task(void const *argument) {
    while (true) {
		if(control_mode != MODE_RAW){
			if(!new_target_vel and dm3->enable()){
				for (int iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++){
					for (int iter_motor = 0; iter_motor < MOTORS_PER_CHASIS; iter_motor ++) {
		    			int ret = set_motors_power(iter_chasis, iter_motor, 0);
		    			i2cerror = (ret!=0) || i2cerror;
		    			if(!ret){
		    				dm3->enable(0);
		    				report_enable();
		    				vels_target[iter_chasis][iter_motor] = 0;
		    			}
		    		}
		    		dirty_target_vel_mutex.lock();
	    			dirty_target_vel[iter_chasis]=false;
	    			dirty_target_vel_mutex.unlock();
		    		report_set_target_vel(iter_chasis);
		    		report_get_vel_change(iter_chasis);
	    			dirty_power[iter_chasis]=false;
					report_power(iter_chasis);		    	
	    		}
			}else{
				new_target_vel = false;			
			}
		}
		Thread::wait(TIMEOUT_VEL_MOTORS);
    }

}

void MotorModule::rated_report_vel_task(void const *argument) {
	int iter_chasis;

    while (true) {
    	for (iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++) {
    		dirty_target_vel_mutex.lock();
    		if (dirty_target_vel[iter_chasis]) {
	    		report_set_target_vel(iter_chasis); //se comento porque ya reporta en handle_target_vel
	    		
    		} else
   				dirty_target_vel[iter_chasis]=true;
   			dirty_target_vel_mutex.unlock();		
   			report_get_vel_change(iter_chasis); //va afuera del if para que siempre reporte 
			
			Thread::wait(RATED_REPORT_MS/2);
    	}
	}

}

void MotorModule::rated_report_pow_task(void const *argument) {
	int iter_chasis;

    while (true) {
    	for (iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++) {
    		if (dirty_power[iter_chasis])
    			report_power(iter_chasis);
    		else
   				dirty_power[iter_chasis]=true;
			Thread::wait(RATED_REPORT_MS);
    	}
	}

}

void MotorModule::rated_report_pot_task(void const *argument) {
	int iter_chasis;

    while (true) {
    	for (iter_chasis = 0; iter_chasis < NUMBER_CHASIS; iter_chasis ++) {
    		if (dirty_pot[iter_chasis])
	    		report_get_pot_change(iter_chasis);
	    	else
   				dirty_pot[iter_chasis]=true;

			Thread::wait(RATED_REPORT_MS);
    	}
	}

}

MotorModule::MotorModule() {
	//ticker_pot.attach(&flip_pot, POT_RATE); // the address of the function to be attached (flip) and the interval (2 seconds)
	dm3 = Dm3::Instance();
	ticker_msg_rate.attach(&flip_msg, MSG_RATE);

	for (unsigned int i=0; i<MOTOR_OPCODES; ++i) {
		MotorModule::opcode_callbacks[i]=NULL;
	}
	MotorModule::opcode_callbacks[OPCODE_REPORT] = &handle_report;
	MotorModule::opcode_callbacks[OPCODE_ENABLE] = &handle_enable;
	MotorModule::opcode_callbacks[OPCODE_SET_TARGET_VEL] = &handle_set_target_vel;
	MotorModule::opcode_callbacks[OPCODE_BRAKE] = &handle_brake;
	MotorModule::opcode_callbacks[OPCODE_REVERSE] = &handle_reverse;
	MotorModule::opcode_callbacks[OPCODE_TEST] = &handle_test;
	MotorModule::opcode_callbacks[OPCODE_SET_DRIVE_MODE] = &handle_setdrivemode;
	MotorModule::opcode_callbacks[OPCODE_SET_POWER_MOTOR] = &handle_motor_power_write;
	MotorModule::opcode_callbacks[OPCODE_SET_CONTROLLER] = &handle_set_controller;
	MotorModule::opcode_callbacks[OPCODE_SET_PID_PARAMETERS] = &handle_set_pid_parameters;
	MotorModule::opcode_callbacks[OPCODE_GET_VEL_CHANGE] = &handle_get_vel_change;
	MotorModule::opcode_callbacks[OPCODE_GET_POT_CHANGE] = &handle_get_pot_change;

	MotorModule::pid = mcc.register_opcode_callbacks(MotorModule::opcode_callbacks, MOTOR_OPCODES);

	mcc.register_poll_callback(&tick);

	handle_report(MOTOR_PID, OPCODE_REPORT);
}
