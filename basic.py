"""
Python 기초

여러줄 주석
"""
# 한줄 주석

# 모듈을 import 하는 두가지 방법
import random
from pprint import pprint


def print_func(param1):
    pprint(param1)
    param2 = 1 + 2
    param2 += param1
    # + 뿐만 아니라 -, *, /, //, % 사용 가능. Python 3.x에서 두 정수에 대해 /는 소수점계산, //는 몫 구하는 연산으로 나뉨

if __name__ == "__main__":
    val1 = 10
    val2 = 'apple'
    val3 = None

    rnd = random.randint(0, 9)

    print_func([val1, val2, val3, rnd])  # array type
    print_func((val1 + rnd, val2, val3))  # tuple type
    print_func({val1, val2, val3, rnd})  # set type

