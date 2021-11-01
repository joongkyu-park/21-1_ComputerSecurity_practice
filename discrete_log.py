"""
Discrete logarithm

a: given number
b: base
m: modulus

log_{b,m}(a)

"""

from exponentiation import exp


def dlog(a, b, n):
    for e in range(1, n):  # from 1 to n-1
        try_value = exp(a, e, n)
        if b == try_value:
            return e
    return None


if __name__ == "__main__":
    base = 3
    modulus = 65537  # 2 ** 16 + 1

    expos = [
        15797,
        23340,
        12447,
        18783,
        63199,
        1,
        2,
        65535,
    ]

    try_count = 100
    try:
        for expo in expos:
            # expo = random.randint(1, modulus-1)
            powed = exp(base, expo, modulus)
            loged = dlog(base, powed, modulus)
            print("({},{},{},{},{}),".format(expo, base, powed, loged, modulus))
            assert expo == loged
        print(">> All the tests passed <<")
    except:
        print("expo({}) and loged({}) is not equal".format(expo, loged))
