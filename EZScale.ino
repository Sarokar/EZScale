#include <Q2HX711.h>

const byte hx711_data_pin = 3;
const byte hx711_clock_pin = 4;
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin); // prep hx711

unsigned char i;

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);  
} task;

const unsigned short tasksNum = 1;
task tasks[tasksNum];

float y1 = 0.099; // calibrated mass to be added
long x1 = 0L;
long x0 = 0L;
long avg_size = 10.0; // amount of averages for each mass measurement
float servingSize = 1.0;
float weight = 50.0;
float calories = 100.0;
float carbs = 3.0;
float fat = 2.0;
float protein = 4.0;


enum Scale_States{Scale_init, Scale_Weigh};
int TickFct_Scale(int state) {

  switch(state) {
    case Scale_init:
    
    break;

    case Scale_Weigh:
      long reading = 0;
      for (int jj=0;jj<int(avg_size);jj++){
        reading+=hx711.read();
      }
      reading/=long(avg_size);
      // calculating mass based on calibration and linear fit
      float ratio_1 = (float) (reading-x0);
      float ratio_2 = (float) (x1-x0);
      float ratio = ratio_1/ratio_2;
      float mass = y1*ratio;
      mass *= 1000;
      unsigned int servingSize = 1;
      weight = 50;
      calories = 100;
      carbs = 3;
      fat = 2;
      protein = 4;
      
      Serial.print("Raw: ");
      Serial.print(reading);
      Serial.print(", ");
      Serial.println(mass);
      Serial.println(weight);
      servingSize = (long)mass/weight;
      Serial.print("Serving Size: ");
      Serial.println(servingSize);
      calories *= servingSize;
      Serial.print("Calories: ");
      Serial.println(calories);
      carbs *= servingSize;
       Serial.print("carbs: ");
      Serial.println(carbs);
      fat *= servingSize;
       Serial.print("fat: ");
      Serial.println(fat);
      protein *= servingSize;
       Serial.print("Protein: ");
      Serial.println(protein);
    break;
    
    default:
    break;
  }

  switch(state) {
    case Scale_init:
      //delay(1000); // allow load cell and hx711 to settle
      // tare procedure
      for (int ii=0;ii<int(avg_size);ii++){
        delay(10);
        x0+=hx711.read();
      }
      x0/=long(avg_size);
      Serial.println("Add Calibrated Mass");
      // calibration procedure (mass should be added equal to y1)
      int ii = 1;
      while(true){
        if (hx711.read()<x0+10000){
        } else {
          ii++;
          delay(2000);
          for (int jj=0;jj<int(avg_size);jj++){
            x1+=hx711.read();
          }
          x1/=long(avg_size);
          break;
        }
      }
      Serial.println("Calibration Complete");  
      state = Scale_Weigh;
    break;

    default:
    break;
  }


  return state;
}

void setup() {
  Serial.begin(9600);
  i = 0;
  tasks[i].state = Scale_init;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_Scale;

}

void loop() {
  for (i = 0; i < tasksNum; ++i) {
    if ( (millis() - tasks[i].elapsedTime) >= tasks[i].period) {
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = millis(); // Last time this task was ran
    }
  }
  delay(200);

}
