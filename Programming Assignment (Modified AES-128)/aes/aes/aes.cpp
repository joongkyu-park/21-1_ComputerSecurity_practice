// 12150981 박중규

#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma warning(disable:4996)

#define KEY "key.bin"
#define PLAINTEXT "plain.bin"
#define PLAINTEXT2 "plain2.bin"
#define CIPHERTEXT "cipher.bin"
#define IP 0b111100111  // 사용되는 Irreducible polynomial (𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)

int pState[4][4]{ 0 };  // 평문 또는 비문을 입력받을 2차원 배열
int kState[4][4]{ 0 };  // Key expansion을 진행할 2차원 배열
int key[4][4]{ 0 }; // 키를 입력받을 2차원 배열
int cState[4][4]{ 0 };  // 암호화 또는 복호화를 라운드별로 진행할 2차원 배열
int size;
char result[4][4];  // 최종 암호화 또는 복호화의 결과가 담길 2차원 배월

// SubBytes에서 사용되는 행렬
int A[8][8] = { 
    {1,0,0,0,1,1,1,1},
    {1,1,0,0,0,1,1,1},
    {1,1,1,0,0,0,1,1},
    {1,1,1,1,0,0,0,1},
    {1,1,1,1,1,0,0,0},
    {0,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,1,0},
    {0,0,0,1,1,1,1,1} 
};

// SubBytes에서 사용되는 행렬의 역행렬
int A_inv[8][8] = {
    {0,0,1,0,0,1,0,1},
    {1,0,0,1,0,0,1,0},
    {0,1,0,0,1,0,0,1},
    {1,0,1,0,0,1,0,0},
    {0,1,0,1,0,0,1,0},
    {0,0,1,0,1,0,0,1},
    {1,0,0,1,0,1,0,0},
    {0,1,0,0,1,0,1,0}
};

// Mixcolumns에서 사용되는 행렬
int mix_table[4][4] = {
    {2,3,1,1},
    {1,2,3,1},
    {1,1,2,3},
    {3,1,1,2}
};

// Mixcolumns에서 사용되는 행렬의 역행렬
int mix_table_inv[4][4] = {
    {14, 11, 13, 9},
    {9, 14, 11, 13},
    {13, 9, 14, 11},
    {11, 13, 9, 14}
};

// SubBytes와 역연산 함수
void SB();
void SB_inv();

// ShiftRows와 역연산 함수
void SR();
void SR_inv();

// MixColumns와 역연산 함수
void MC();
void MC_inv();

// KeyExpansion 함수
void KE(int phase);

// AddRoundkey 함수
void AR();

// 유한체 관련 계산 함수들
int deg(int bp);
int bin_ext_euclid(int a, int b);
int bin_inv(int num);
bool carry(int a);
int bin_mul(int a, int b, int n);

int main(int argc, char* argv[])
{
    FILE* rfp = NULL;
    FILE* wfp = NULL;
    bool flag = false;

    // Encryption 모드
    if (strcmp(argv[1], "e") == 0) {

        int input;
        int i = 0;
        int j = 0;

        // "key.bin" 읽기
        if ((rfp = fopen(KEY, "rb")) == NULL) {
            fputs("파일 열기 에러", stderr);
            exit(1);
        }
        // key 저장
        while ((input = fgetc(rfp)) != EOF) {
            key[i][j] = input;
            j++;
            if (j == 4) {
                i++;
                j = 0;
            }
        }
        // "cipher.bin" 쓰기
        if ((wfp = fopen(CIPHERTEXT, "wb")) == NULL) {
            fputs("파일 쓰기 에러", stderr);
            exit(1);
        }

        // "plain.bin" 읽기
        if ((rfp = fopen(PLAINTEXT, "rb")) == NULL) {
            fputs("파일 열기 에러", stderr);
            exit(1);
        }

        while (1) {
            if (flag) // "plain.bin"을 16바이트 씩 읽고, 더 이상 읽을 내용이 없으면 종료
                break;

            // Key Expansion을 위한 키 복사
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    kState[i][j] = key[i][j];
                }
            }

            i = 0;
            j = 0;
            input = 0;
            
            // plain text 저장
            while ((input = fgetc(rfp)) != EOF) {
                pState[i][j] = input;
                j++;
                if (j == 4) {
                    i++;
                    j = 0;
                }
                if (i == 4)
                    break;
            }

            // "plain.bin"에 더 읽을 데이터가 있는지 확인
            if ((input = fgetc(rfp)) == EOF) {
                flag = true;
            }
            else {
                fseek(rfp, -1, SEEK_CUR);
            }

            // 암호화 Round 진행을 위하여 plain text 복사
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    cState[i][j] = pState[i][j];
                }
            }

            // Round 진행 전 최초 AddRoundkey
            AR();

            // 10번의 Round 수행
            int phase = 1;
            while (phase <= 10) {
                
                // SubBytes
                SB();

                // Shiftrows
                SR();

                // MixColumns (10 라운드 제외)
                if (phase != 10) {
                    MC();
                }

                // KeyExpansion
                KE(phase);

                // AddRoundkey
                AR();

                phase++;
            }

            // 최종적으로 암호화된 데이터 옮기기
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result[i][j] = cState[i][j];
                }
            }

            // "cipher.bin"에 암호화된 파일 작성
            fwrite(result, sizeof(char), sizeof(result), wfp);
        }
    }

    // Decryption 모드
    else if (strcmp(argv[1], "d") == 0) {
        int input;
        int i = 0;
        int j = 0;

        // "key.bin" 읽기
        if ((rfp = fopen(KEY, "rb")) == NULL) {
            fputs("파일 열기 에러", stderr);
            exit(1);
        }

        // key 저장
        while ((input = fgetc(rfp)) != EOF) {
            key[i][j] = input;
            j++;
            if (j == 4) {
                i++;
                j = 0;
            }
        }

        // "plain2.bin" 쓰기
        if ((wfp = fopen(PLAINTEXT2, "wb")) == NULL) {
            fputs("파일 쓰기 에러", stderr);
            exit(1);
        }

        // "cipher.bin" 읽기
        if ((rfp = fopen(CIPHERTEXT, "rb")) == NULL) {
            fputs("파일 열기 에러", stderr);
            exit(1);
        }

        while (1) {
            if (flag)   // "cipher.bin"을 16바이트 씩 읽고, 더 이상 읽을 내용이 없으면 종료
                break;

            // Key Expansion을 위한 키 복사
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    kState[i][j] = key[i][j];
                }
            }

            i = 0;
            j = 0;
            input = 0;
            // cipher text 저장
            while ((input = fgetc(rfp)) != EOF) {
                pState[i][j] = input;
                j++;
                if (j == 4) {
                    i++;
                    j = 0;
                }
                if (i == 4)
                    break;
            }

            // "cipher.bin"에 더 읽을 데이터가 있는지 확인
            if ((input = fgetc(rfp)) == EOF) {
                flag = true;
            }
            else {
                fseek(rfp, -1, SEEK_CUR);
            }

            // 복호화 Round 진행을 위하여 cipher text 복사
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    cState[i][j] = pState[i][j];
                }
            }

            // Round 진행 전 최초 AddRoundkey를 위한 KeyExpansion (w[40,43])
            int KE_phase = 1;
            while (KE_phase <= 10) {
                KE(KE_phase);
                KE_phase++;
            }
            KE_phase = 1;

            // Round 진행 전 최초 AddRoundkey
            AR();

            // 키값 초기화
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    kState[i][j] = key[i][j];
                }
            }

            // 10번의 Round 수행
            int phase = 1;
            while (phase <= 10) {
                // ShiftRows 역연산
                SR_inv();

                // SubBytes 역연산
                SB_inv();

                // KeyExpansion (라운드에 따라 w[36,39] -> ... -> w[0, 3])
                while (KE_phase <= 10-phase) {
                    KE(KE_phase);
                    KE_phase++;
                }
                KE_phase = 1;

                // AddRoundkey
                AR();

                // 키값 초기화
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        kState[i][j] = key[i][j];
                    }
                }

                // MixColumns 역연산 (10 라운드 제외)
                if (phase != 10) {
                    MC_inv();
                }

                phase++;
            }

            // 최종적으로 복호화된 데이터 옮기기
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result[i][j] = cState[i][j];
                }
            }

            // "plain2.bin"에 복호화된 파일 작성
            fwrite(result, sizeof(char), sizeof(result), wfp);
        }
    }

    else {
        printf("잘못된 실행파일옵션입니다.\ne : 암호화\nd : 복호화");
        return 0;
    }

    fclose(rfp);
    fclose(wfp);

    return 0;
}

// SubBytes 함수
void SB() {
    for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4;j++) {
            int ctmp_bit[8]{ 0 };
            int arr_for_xor[8] = { 1,1,0,0,0,1,1,0 };
            int result_bit[8]{ 0 };

            // 모듈러의 곱셈의 역원으로 바꾸기 (𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)
            int ctmp;
            if (cState[i][j] == 0) {
                ctmp = 0;
            }
            else if (cState[i][j] == 1) {
                ctmp = 1;
            }
            else {
                ctmp = bin_inv(cState[i][j]);
            }
            
            // 정수를 비트로 바꾸기
            for (int index = 7; index >= 0; --index) {
                int bit = ctmp >> index & 1;
                ctmp_bit[index] = bit;
            }
            
            // A행렬과 연산 후 열벡터와 XOR
            for (int k = 0; k < 8; k++) {
                int tmp = 0;
                for (int l = 0; l < 8; l++) {
                    if (l == 0) {
                        tmp = (A[k][l] * ctmp_bit[l]);
                    }
                    else {
                        tmp = tmp ^ (A[k][l] * ctmp_bit[l]);
                    }
                }
                result_bit[k] = tmp ^ arr_for_xor[k];
            }

            // 비트를 정수로 바꾸기
            int result = 0;
            for (int index = 0; index < 8; index++) {
                result += result_bit[index] * (int)pow(2, index);
            }

            // 결과값 저장
            cState[i][j] = result;
        }
    }
}

// SubBytes 역연산 함수
void SB_inv() {
    for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4;j++) {
            int ctmp_bit[8]{ 0 };
            int result_bit[8]{ 0 };
            int arr_for_xor[8] = { 1,0,1,0,0,0,0,0 };

            int ctmp = cState[i][j];

            // 정수를 비트로 바꾸기
            for (int index = 7; index >= 0; --index) {
                int bit = ctmp >> index & 1;
                ctmp_bit[index] = bit;
            }

            // A의 역행렬과 연산 후 열벡터와 연산
            for (int k = 0; k < 8; k++) {
                int tmp = 0;
                for (int l = 0; l < 8; l++) {
                    if (l == 0) {
                        tmp = (A_inv[k][l] * ctmp_bit[l]);
                    }
                    else {
                        tmp = tmp ^ (A_inv[k][l] * ctmp_bit[l]);
                    }
                }
                result_bit[k] = tmp ^ arr_for_xor[k];
            }

            // 비트를 정수로 바꾸기
            int result = 0;
            for (int index = 0; index < 8; index++) {
                result += result_bit[index] * (int)pow(2, index);
            }

            // 계산한 값을 모듈러 곱셈의 역원으로 바꾸기 (𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)
            if (result == 0) {
                result = 0;
            }
            else if (result == 1) {
                result = 1;
            }
            else {
                result = bin_inv(result);
            }

            // 결과값 저장
            cState[i][j] = result;

        }
    }

}

// ShiftRows 함수
void SR() {
    int copy[8][8] = { 0 };

    // 데이터 복사
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            copy[i][j] = cState[i][j];
        }
    }

    // ShiftRow 과정
    cState[0][1] = copy[1][1];
    cState[0][2] = copy[2][2];
    cState[0][3] = copy[3][3];

    cState[1][1] = copy[2][1];
    cState[1][2] = copy[3][2];
    cState[1][3] = copy[0][3];

    cState[2][1] = copy[3][1];
    cState[2][2] = copy[0][2];
    cState[2][3] = copy[1][3];

    cState[3][1] = copy[0][1];
    cState[3][2] = copy[1][2];
    cState[3][3] = copy[2][3];
}

// ShiftRows 역연산 함수
void SR_inv() {
    int copy[8][8] = { 0 };

    // 데이터 복사
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            copy[i][j] = cState[i][j];
        }
    }

    // ShiftRow 과정
    cState[1][1] = copy[0][1];
    cState[2][2] = copy[0][2];
    cState[3][3] = copy[0][3];

    cState[2][1] = copy[1][1];
    cState[3][2] = copy[1][2];
    cState[0][3] = copy[1][3];

    cState[3][1] = copy[2][1];
    cState[0][2] = copy[2][2];
    cState[1][3] = copy[2][3];

    cState[0][1] = copy[3][1];
    cState[1][2] = copy[3][2];
    cState[2][3] = copy[3][3];
}

// MixColumns 함수
void MC() {
    int copy[8][8] = { 0 };

    // 데이터 복사
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            copy[i][j] = cState[i][j];
        }
    }
    
    // mixcolumns table과 행렬연산 후 값 저장 (𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int result = 0;
            for (int k = 0; k < 4; k++) {
                result = result ^ (bin_mul(mix_table[j][k], copy[i][k], IP));
            }
            cState[i][j] = result;
        }
    }
}

// MixColumns 역연산 함수
void MC_inv() {
    int copy[8][8] = { 0 };

    // 데이터 복사
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            copy[i][j] = cState[i][j];
        }
    }

    // mixcolumns table의 역행렬과 행렬연산 후 값 저장 (𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int result = 0;
            for (int k = 0; k < 4; k++) {
                result = result ^ (bin_mul(mix_table_inv[j][k], copy[i][k], IP));
            }
            cState[i][j] = result;
        }
    }
}

// KeyExpansion
void KE(int phase) {
    // 마지막 word의 순서를 바꾸어서 저장
    int B[4] = { kState[3][1],kState[3][2],kState[3][3],kState[3][0] };

    // 각 바이트를 S-box에 통과시키기
    for (int i = 0; i < 4; i++) {
        int ktmp;
        if (B[i] == 0) {
            ktmp = 0;
        }
        else if (B[i] == 1) {
            ktmp = 1;
        }
        else {
            ktmp = bin_inv(B[i]);
        }

        int ktmp_bit[8]{ 0 };
        int arr_for_xor[8] = { 1,1,0,0,0,1,1,0 };
        int result_bit[8]{ 0 };

        for (int index = 7; index >= 0; --index) {
            int bit = ktmp >> index & 1;
            ktmp_bit[index] = bit;
        }
                                
        for (int k = 0; k < 8; k++) {
            int tmp = 0;
            for (int l = 0; l < 8; l++) {
                if (l == 0) {
                    tmp = (A[k][l] * ktmp_bit[l]);
                }
                else {
                    tmp = tmp ^ (A[k][l] * ktmp_bit[l]);
                }            
            }
            result_bit[k] = tmp ^ arr_for_xor[k];
        }

        int result = 0;
        for (int index = 0; index < 8; index++) {
            result += result_bit[index] * (int)pow(2, index);
        }

        B[i] = result;
    }

    // phase에 따라 RC값 구하기
    int RC[4] = { 0 };
    if (phase == 9) {   // 9번째 라운드키의 RC[0] (𝑥^8 mod 𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)
        RC[0] = 0b11100111;
    }
    else if (phase == 10) { // 10번째 라운드키의 RC[0] (𝑥^9 mod 𝑥^8 + 𝑥^7 + 𝑥^6 + 𝑥^5 + 𝑥^2 + 𝑥 + 1)
        RC[0] = 0b101001;
    }
    else {  // 1~8번재 라운드키의 RC[0]
        RC[0] = (int)pow(2, phase-1);
    }

    // RC와 word를 XOR
    for (int i = 0; i < 4; i++) {
        B[i] = B[i] ^ RC[i];
    }
 
    // 첫 번째 word와 위에서 계산한 word를 xor하여 다음 word의 첫 번째 원소를 구하기
    for (int i = 0; i < 4; i++) {
        kState[0][i] = B[i] ^ kState[0][i];
    }

    // 위에서 계산된 다음 word의 첫 번째 원소로부터 시작하여 이전 word와 XOR하여 다음 word들을 구하기
    for (int i = 1; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            kState[i][j] = kState[i][j] ^ kState[i-1][j];
        }
    }
}

// AddRoundKey
void AR() {
    for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4;j++) {
            // key와  XOR 수행
            cState[i][j] = cState[i][j] ^ kState[i][j];
        }
    }
}

// 유한체 관련 계산 함수들

// binary polynomial의 degree 구하기
int deg(int bp) {
    for (int i = 31; i >= 0;i--) {
        if ((bp & (1 << i)) != 0)
            return i;
    }
    return 0;
}
// binary polynomial상에서의 확장 유클리드
int bin_ext_euclid(int a, int b) {
    int u = a;
    int v = b;

    int g_1 = 1;
    int g_2 = 0;
    int h_1 = 0;
    int h_2 = 1;

    int j = -1;

    while (u != 0) {
        j = deg(u) - deg(v);
        if (j < 0) {
            int tmp = u;
            u = v;
            v = tmp;

            tmp = g_1;
            g_1 = g_2;
            g_2 = tmp;

            tmp = h_1;
            h_1 = h_2;
            h_2 = tmp;

            j = -j;
        }

        u = u ^ (v << j);
        g_1 = g_1 ^ (g_2 << j);
        h_1 = h_1 ^ (h_2 << j);
    }

    int d = v;
    int g = g_2;
    int h = h_2;

    return g;
}
// binary polynomial상에서의 모듈러 곱셈 역원 구하기
int bin_inv(int num) {
    return bin_ext_euclid(num, IP);
}
// carry 발생 시 처리 함수
bool carry(int a) {
    if (a & 0x100)
        return true;
    else
        return false;
}
// binary polynomial상에서의 모듈러 곱셈
int bin_mul(int a, int b, int n) {
    int buf = n & 0xff;

    int f[8] = { 0 };
    f[0] = a;
    for (int i = 1; i < 8; i++) {
        f[i] = f[i - 1] << 1;
        if (carry(f[i])) {
            f[i] &= 0xff;
            f[i] ^= buf;
        }
    }

    int res = 0;
    for (int i = 0; i < 8; i++) {
        int  mask = 1 << i;
        if ((b & mask) != 0)
            res ^= f[i];
    }

    return res;
}