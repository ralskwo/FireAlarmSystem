// 0.2초마다 주변 밝기를 체크하여 전압값으로 보여주는 프로그램

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <errno.h>

#include <wiringPi.h>

#include <wiringPiSPI.h>

#include <unistd.h>

#include <softTone.h>



#define FLAME 8  //GPIO 8번핀을 불꽃감지센서 포트로 설정

#define SPI_CHANNEL 0

#define SPI_SPEED 100000

#define LED1 4

#define SPEAKER 1



int read_mcp3202_adc(unsigned char adcChannel)  //MCP3202가 보내는 데이터를 출력하는 함수

{

    unsigned char buff[3];

    int adcValue = 0;



    //MCP3202에게 아날로그 값을 읽어들여 마스터에게 보내라는 명령을 보내기위하여

    //1byte씩 버퍼에 명령을 넣는 작업을 수행한다.

    buff[0] = 0x01;

    buff[1] = 0xa0 & ((adcChannel & 0x01) << 6);

    buff[2] = 0x00;



    digitalWrite(FLAME, 0); //CE0핀의 출력을 끔으로서 slave에게 데이터를 보낼 준비를 한다.



    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

    digitalWrite(FLAME, 1); //CS핀에서 HIGH 출력한다.

    buff[1] = 0x0F & buff[1];  //들어온 데이터만 필터링 하여 버퍼에 다시 저장한다.

    adcValue = (buff[1] << 8) | buff[2];  //버퍼 안에 들어온 데이터를 합쳐 adcValue에 저장한다.





    return adcValue;

}



int main(void) {

    int adcChannel = 0;

    float adcVol = 0;

    int scale = 1791;

    char cmd[256];

    char cmd2[256];



    if (wiringPiSetup() == -1) {   //wiringPi초기화

        fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));

        return 1;



    }

    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {  //SPI를 읽어오기 위한 초기화 과정

        fprintf(stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));

        return 1;

    }





    softToneCreate(SPEAKER);

    pinMode(LED1, OUTPUT);

    pinMode(FLAME, OUTPUT);  //CS핀을 출력포트로 설정한다.

    digitalWrite(FLAME, 1);  //CS핀에서 HIGH 출력한다.

    system("clear");  //화면 지우기

    while (1) {

        adcVol = ((float)read_mcp3202_adc(adcChannel) / 4096.0f) * 3.3f; //읽어온 값을 전압값으로 변환하여 adcVol에 저장

        printf(" cds voltage = %f\n", adcVol);  //화면에 adcVol값 출력

        if ((float)adcVol > 3.15)

        {

            softToneWrite(SPEAKER, scale);    // notes[i]번째의 음계를 소리를

            digitalWrite(LED1, HIGH);



            sprintf(cmd, "raspivid -o video.h624 -fps 30 -t %d", 3000);

            system(cmd);



            sprintf(cmd2, "mutt -s warning!fire!! ralskwo@gmail.com < /dev/null");

            system(cmd2);



            while (1) //불이 계속 나있는 상태라면

            {

                system("clear");



                adcVol = ((float)read_mcp3202_adc(adcChannel) / 4096.0f) * 3.3f; //읽어온 값을 전압값으로 변환하여 adcVol에 저장

                printf(" cds voltage = %f\n", adcVol);  //화면에 adcVol값 출력



                softToneWrite(SPEAKER, scale);    // notes[i]번째의 음계를 소리를

                digitalWrite(LED1, HIGH);

                delay(500);



                softToneWrite(SPEAKER, 0);              // 소리를 멈추고

                digitalWrite(LED1, LOW);

                delay(500);                         // 500ms(0.5초) 대기            



                if ((float)adcVol < 3.15)

                {

                    break;

                }



                system("clear");

            }

        }



        else

        {

            softToneWrite(SPEAKER, 0);              // 소리를 멈추고

            digitalWrite(LED1, LOW);

        }



        delay(200);

        system("clear"); // 화면 지우기

    }

    return 0;

}