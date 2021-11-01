// 12150981 박중규

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define xsize 559
#define ysize 314
#define INPUTBMP "origin.bmp"
#define OUTPUTBMP "stego.bmp"
#define MAX_INPUT_SIZE 3000
#pragma warning(disable:4996)

char user[MAX_INPUT_SIZE] = { 0, };


void demicalToBinary(int input, int** binaryMatrix, int row) {
    int col = 7;
    int tmp = input;
    while (1)
    {
        binaryMatrix[row][col] = tmp % 2;
        tmp = tmp / 2;
        col--;

        if (tmp == 0)
            break;
    }
}

int main(int argc, char* argv[])
{

    int size;
    int count;
    FILE* fp;


    // 기능 1 : "origin.bmp"에 메세지 숨기기(옵션 "e")
    if (strcmp(argv[1], "e") == 0) {
        // "orign.bmp" 파일 열기
        fp = fopen(INPUTBMP, "rb");
        if (fp == NULL)
        {
            printf("파일오류");
            return 0;
        }

        // 파일 사이즈 저장
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);

        // input 이미지의 데이터를 저장할 배열
        char* inputImg;
        inputImg = (char*)malloc(size + 1);
        memset(inputImg, 0, size + 1);

        // "inputImg" 배열에 파일데이터 저장
        fseek(fp, 0, SEEK_SET);
        count = fread(inputImg, size, 1, fp);

        fclose(fp);


        // 메세지를 숨길 파일데이터를 저장할 배열
        char* outputImg;
        outputImg = (char*)malloc(size + 1);

        // "inputImg"배열에서 "outputImg"배열로 파일데이터 옮기기
        for (int i = 0; i < 46; i++) {
            outputImg[i] = inputImg[i];
        }


        ///////////////////////////////////////
        // 2차원 배열에 사용자가 입력한 메세지를 이진수로 저장하기
        gets_s(user, sizeof(user));

        int sizeOfUser = (int)strlen(user);

        int** binaryMatrix = (int**)malloc(sizeof(int*) * sizeOfUser);
        for (int i = 0; i < sizeOfUser; i++) {
            binaryMatrix[i] = (int*)malloc(sizeof(int) * 8);
        }
        for (int i = 0; i < sizeOfUser; i++) {
            for (int j = 0; j < 8;j++) {
                binaryMatrix[i][j] = 0;
            }
        }

        // 1개의 문자를 8개의 비트로 나누어 저장
        for (int i = 0; i < sizeOfUser; i++) {
            demicalToBinary((unsigned char)user[i], binaryMatrix, i);
        }
        ////////////////////////////////////////



        // 유저가 입려한 메세지의 사이즈 숨기기
        // 46~53번째 바이트에 255크기 간격으로 숨기기(최대크기 255*8 = 2040)
        if (sizeOfUser > 255 * 8) {
            printf("입력가능한 메세지의 최대 글자수는 2040자 입니다.");
            printf("\n");
            printf("입력한 메세지의 크기 : %d", sizeOfUser);
            return 0;
        }
        else if (255 * 7 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = 255;
            outputImg[51] = 255;
            outputImg[50] = 255;
            outputImg[49] = 255;
            outputImg[48] = 255;
            outputImg[47] = 255;
            outputImg[46] = sizeOfUser - 255 * 7;
        }
        else if (255 * 6 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = 255;
            outputImg[51] = 255;
            outputImg[50] = 255;
            outputImg[49] = 255;
            outputImg[48] = 255;
            outputImg[47] = sizeOfUser - 255 * 6;

            outputImg[46] = inputImg[46];
        }
        else if (255 * 5 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = 255;
            outputImg[51] = 255;
            outputImg[50] = 255;
            outputImg[49] = 255;
            outputImg[48] = sizeOfUser - 255 * 5;

            outputImg[47] = inputImg[47];
            outputImg[46] = inputImg[46];
        }
        else if (255 * 4 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = 255;
            outputImg[51] = 255;
            outputImg[50] = 255;
            outputImg[49] = sizeOfUser - 255 * 4;

            outputImg[48] = inputImg[48];
            outputImg[47] = inputImg[47];
            outputImg[46] = inputImg[46];
        }
        else if (255 * 3 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = 255;
            outputImg[51] = 255;
            outputImg[50] = sizeOfUser - 255 * 3;

            outputImg[49] = inputImg[49];
            outputImg[48] = inputImg[48];
            outputImg[47] = inputImg[47];
            outputImg[46] = inputImg[46];
        }
        else if (255 * 2 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = 255;
            outputImg[51] = sizeOfUser - 255 * 2;

            outputImg[50] = inputImg[50];
            outputImg[49] = inputImg[49];
            outputImg[48] = inputImg[48];
            outputImg[47] = inputImg[47];
            outputImg[46] = inputImg[46];
        }
        else if (255 < sizeOfUser) {
            outputImg[53] = 255;
            outputImg[52] = sizeOfUser - 255;

            outputImg[51] = inputImg[51];
            outputImg[50] = inputImg[50];
            outputImg[49] = inputImg[49];
            outputImg[48] = inputImg[48];
            outputImg[47] = inputImg[47];
            outputImg[46] = inputImg[46];
        }
        else if (sizeOfUser <= 255) {
            outputImg[53] = sizeOfUser;

            outputImg[52] = inputImg[52];
            outputImg[51] = inputImg[51];
            outputImg[50] = inputImg[50];
            outputImg[49] = inputImg[49];
            outputImg[48] = inputImg[48];
            outputImg[47] = inputImg[47];
            outputImg[46] = inputImg[46];
        }

        // 메세지 숨기기
        // 54번째 바이트부터 숨기기 시작(픽셀데이터의 시작이 54번째이기 때문)
        // 1바이트 당 1비트씩 숨기므로 8바이트는 문자 1개가 된다.
        int p = 54;
        for (int index = 0; index < sizeOfUser; index++) {
            for (int i = 0; i < 8;i++) {
                if (binaryMatrix[index][i] == inputImg[p] % 2) {
                    outputImg[p] = inputImg[p];
                }
                else {
                    if ((int)inputImg[p] == 0) {
                        outputImg[p] = inputImg[p] + 1;
                    }
                    else {
                        outputImg[p] = inputImg[p] - 1;
                    }
                }
                p++;
            }
        }

        // 메세지를 숨긴 이후 나머지 파일데이터 옮기기
        for (int i = p; i < size; i++) {
            outputImg[i] = inputImg[i];
        }

        // "stego.bmp" 파일 생성 후 bmp파일 작성
        fp = fopen(OUTPUTBMP, "wb");
        if (fp == NULL)
        {
            printf("파일오류");
            return 0;
        }

        fwrite(outputImg, size, 1, fp);

        fclose(fp);
    }

    // 기능 2 : "stego.bmp"에 있는 메세지 해독하기(옵션 "d")
    else if (strcmp(argv[1], "d") == 0) {
        // "stego.bmp" 파일 열기
        fp = fopen(OUTPUTBMP, "rb");
        if (fp == NULL)
        {
            printf("파일오류");
            return 0;
        }

        // 파일사이즈 저장
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);

        // 이미지 데이터를 담을 배열 생성
        char* inputImg;
        inputImg = (char*)malloc(size + 1);
        memset(inputImg, 0, size + 1);

        // 배열에 이미지 데이터 담기
        fseek(fp, 0, SEEK_SET);
        count = fread(inputImg, size, 1, fp);

        // 파일포인터를 46번째로 옮기기
        fseek(fp, 46, SEEK_SET);

        int sizeOfUser = 0;

        // 46~53번 째 바이트에 숨겨진 메세지 사이즈를 해독하여 저장
        int ch[8];
        for (int i = 0; i < 8; i++) {
            ch[i] = fgetc(fp);
            sizeOfUser += ch[i];
        }

        fclose(fp);

        // 이미지에 숨겨진 비트값들을 담을 2차원 배열 생성
        int** binaryMatrix = (int**)malloc(sizeof(int*) * sizeOfUser);
        for (int i = 0; i < sizeOfUser; i++) {
            binaryMatrix[i] = (int*)malloc(sizeof(int) * 8);
        }
        for (int i = 0; i < sizeOfUser; i++) {
            for (int j = 0; j < 8;j++) {
                binaryMatrix[i][j] = 0;
            }
        }

        // 54번째부터 메세지 사이즈만큼 비트 옮기기
        int p = 54;
        for (int index = 0; index < sizeOfUser; index++) {
            for (int i = 0; i < 8;i++) {
                binaryMatrix[index][i] = inputImg[p] % 2;
                p++;
            }
        }

        // 메세지를 담을 문자열 변수
        char* answer = (char*)malloc(sizeof(int*) * sizeOfUser + 1);
        for (int i = 0; i < sizeOfUser; i++) {
            answer[i] = '\0';
        }

        // 8비트 당 정수 1개로 바꾼후, 아스키코드에 따라 1개 문자로 변환
        // 변환된 문자들을 "answer"에 저장
        int tmp, e;
        for (int i = 0; i < sizeOfUser; i++) {
            tmp = 0;
            e = 0;
            for (int j = 7; j >= 0; j--) {
                tmp += binaryMatrix[i][j] * (1 << e);
                e++;
            }
            answer[i] = (char)tmp;
        }

        // 메세지 출력
        for (int i = 0; i < sizeOfUser; i++) {
            printf("%c", answer[i]);
        }

    }
    else {
        printf("잘못된 실행파일옵션입니다.\ne : 메세지 숨기기\nd : 메세지 해독하기");
        return 0;
    }

    return 0;
}