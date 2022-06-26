#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include "Wire.h"

int pin[4] = {3, 9, 10, 11};
int motor[4] = {0, 0, 0, 0};

int processed_data[3] = {0, 0, 0}; // throttle, roll, pitch
int throttle = 0;
int exp_angle[3] = {0, 0, 0}; // yaw, roll, pitch
int gyro_angle[3] = {0, 0, 0}; // yaw, roll, pitch

bool flag = false;
int idx = 0;
int cnt = 0;
String comdata = "";
MPU6050 mpu;

// MPU control/status vars
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, roll, pitch]   yaw/pitch/roll container and gravity vector

//PID
struct pid
{
    double kp = 2;
    double ki = 0;
    double kd = 0;
    double input;
    double output;
    double setpoint;
}roll_pid, pitch_pid;

enum axis_index{
    YAW,
    ROLL,
    PITCH
};

enum motor_index{
    LEFT_FRONT,
    RIGHT_BACK,
    RIGHT_FRONT,
    LEFT_BACK
};

int adjust(int res, int thr){
    if(res < 0 || thr == 0){
        return 0;
    }
    if(res > 255){
        return 255;
    }
    return res;
}

void setup()
{   
    
    Wire.begin();
    Wire.setClock(400000);
    
    //gyro
    mpu.initialize();
    
    mpu.dmpInitialize();

    mpu.setXGyroOffset(51);
    mpu.setYGyroOffset(8);
    mpu.setZGyroOffset(21);
    mpu.setXAccelOffset(1150);
    mpu.setYAccelOffset(-50);
    mpu.setZAccelOffset(1060);
    
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    
    mpu.setDMPEnabled(true); 
    
    
    //pid
    
    

    Serial.begin(9600);
    //Serial.setTimeout(10000);

    //pin mode
    pinMode(3, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
}

void loop()
{   
    idx = 0;
    cnt = 0;
    comdata = String("");
    flag = false;

    if(Serial.available()){
        int tmp;
        char c;
        while(true){
            while(true){
                tmp = char(Serial.read());
                if(tmp != -1){
                    break;
                }
            }
            c = char(tmp);
            if(c == '\n'){
                if(cnt == 2){
                    flag = true;
                }
                break;
            }else if(c == ','){
                cnt++;
            }
            comdata += c;
            delay(10);
        }
    }

    if(flag){

        // process comdata
        for(int i = 0; i <= 2; i++){
            processed_data[i] = 0;
        }

        for(int i = 0; i < comdata.length(); i++){
            if(comdata[i] == ','){
                idx++;
            }else{
                processed_data[idx] = processed_data[idx] * 10 + (comdata[i] - '0');
            }
        }

        throttle = processed_data[0];
        exp_angle[ROLL] = processed_data[1] - 90;
        exp_angle[PITCH] = processed_data[2] - 90;

        for(int i = 0; i <= 2; i++){
            processed_data[i] = 0;
        }

        
        // get angle
        mpu.dmpGetCurrentFIFOPacket(fifoBuffer);
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        
        for(int i = 0; i <= 2; i++){
            gyro_angle[i] = ypr[i] * 180 / M_PI;
        }

        
        // pid
        roll_pid.output = roll_pid.kp * (exp_angle[ROLL] - gyro_angle[ROLL] * -1);
        pitch_pid.output = pitch_pid.kp * (exp_angle[PITCH] - gyro_angle[PITCH]);
        

        // output motor
        
        motor[LEFT_FRONT] = throttle - roll_pid.output - pitch_pid.output;
        motor[LEFT_BACK] = throttle - roll_pid.output + pitch_pid.output;
        motor[RIGHT_FRONT] = throttle + roll_pid.output - pitch_pid.output;
        motor[RIGHT_BACK] = throttle + roll_pid.output + pitch_pid.output;
        
        /*
        motor[LEFT_FRONT] = throttle - exp_angle[ROLL] - exp_angle[PITCH];
        motor[LEFT_BACK] = throttle - exp_angle[ROLL] + exp_angle[PITCH];
        motor[RIGHT_FRONT] = throttle + exp_angle[ROLL] - exp_angle[PITCH];
        motor[RIGHT_BACK] = throttle + exp_angle[ROLL] + exp_angle[PITCH];
        */

        for(int i = 0; i <= 3; i++){
            motor[i] = adjust(motor[i], throttle);
        }

        for(int i = 0; i <= 3; i++){
            analogWrite(pin[i], motor[i]);
        }

        for(int i = 0; i <= 3; i++){
            Serial.println(motor[i]);
        }

        for(int i = 0; i <= 2; i++){
            Serial.println(gyro_angle[i]);
        }
    }
}

