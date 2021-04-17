// 0.2�ʸ��� �ֺ� ��⸦ üũ�Ͽ� ���а����� �����ִ� ���α׷�

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <errno.h>

#include <wiringPi.h>

#include <wiringPiSPI.h>

#include <unistd.h>

#include <softTone.h>



#define FLAME 8  //GPIO 8������ �Ҳɰ������� ��Ʈ�� ����

#define SPI_CHANNEL 0

#define SPI_SPEED 100000

#define LED1 4

#define SPEAKER 1



int read_mcp3202_adc(unsigned char adcChannel)  //MCP3202�� ������ �����͸� ����ϴ� �Լ�

{

    unsigned char buff[3];

    int adcValue = 0;



    //MCP3202���� �Ƴ��α� ���� �о�鿩 �����Ϳ��� ������� ����� ���������Ͽ�

    //1byte�� ���ۿ� ����� �ִ� �۾��� �����Ѵ�.

    buff[0] = 0x01;

    buff[1] = 0xa0 & ((adcChannel & 0x01) << 6);

    buff[2] = 0x00;



    digitalWrite(FLAME, 0); //CE0���� ����� �����μ� slave���� �����͸� ���� �غ� �Ѵ�.



    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

    digitalWrite(FLAME, 1); //CS�ɿ��� HIGH ����Ѵ�.

    buff[1] = 0x0F & buff[1];  //���� �����͸� ���͸� �Ͽ� ���ۿ� �ٽ� �����Ѵ�.

    adcValue = (buff[1] << 8) | buff[2];  //���� �ȿ� ���� �����͸� ���� adcValue�� �����Ѵ�.





    return adcValue;

}



int main(void) {

    int adcChannel = 0;

    float adcVol = 0;

    int scale = 1791;

    char cmd[256];

    char cmd2[256];



    if (wiringPiSetup() == -1) {   //wiringPi�ʱ�ȭ

        fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));

        return 1;



    }

    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {  //SPI�� �о���� ���� �ʱ�ȭ ����

        fprintf(stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));

        return 1;

    }





    softToneCreate(SPEAKER);

    pinMode(LED1, OUTPUT);

    pinMode(FLAME, OUTPUT);  //CS���� �����Ʈ�� �����Ѵ�.

    digitalWrite(FLAME, 1);  //CS�ɿ��� HIGH ����Ѵ�.

    system("clear");  //ȭ�� �����

    while (1) {

        adcVol = ((float)read_mcp3202_adc(adcChannel) / 4096.0f) * 3.3f; //�о�� ���� ���а����� ��ȯ�Ͽ� adcVol�� ����

        printf(" cds voltage = %f\n", adcVol);  //ȭ�鿡 adcVol�� ���

        if ((float)adcVol > 3.15)

        {

            softToneWrite(SPEAKER, scale);    // notes[i]��°�� ���踦 �Ҹ���

            digitalWrite(LED1, HIGH);



            sprintf(cmd, "raspivid -o video.h624 -fps 30 -t %d", 3000);

            system(cmd);



            sprintf(cmd2, "mutt -s warning!fire!! ralskwo@gmail.com < /dev/null");

            system(cmd2);



            while (1) //���� ��� ���ִ� ���¶��

            {

                system("clear");



                adcVol = ((float)read_mcp3202_adc(adcChannel) / 4096.0f) * 3.3f; //�о�� ���� ���а����� ��ȯ�Ͽ� adcVol�� ����

                printf(" cds voltage = %f\n", adcVol);  //ȭ�鿡 adcVol�� ���



                softToneWrite(SPEAKER, scale);    // notes[i]��°�� ���踦 �Ҹ���

                digitalWrite(LED1, HIGH);

                delay(500);



                softToneWrite(SPEAKER, 0);              // �Ҹ��� ���߰�

                digitalWrite(LED1, LOW);

                delay(500);                         // 500ms(0.5��) ���            



                if ((float)adcVol < 3.15)

                {

                    break;

                }



                system("clear");

            }

        }



        else

        {

            softToneWrite(SPEAKER, 0);              // �Ҹ��� ���߰�

            digitalWrite(LED1, LOW);

        }



        delay(200);

        system("clear"); // ȭ�� �����

    }

    return 0;

}