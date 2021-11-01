"""
polynomial representation

For example, f(z) = z^5 + z^2 + 1 <=> f = 0b100101

"""
# 12150981_7week

def get_poly_str(f):
    polys = []
    for i, v in enumerate(reversed(bin(f)[2:])):
        if v == '1':
            polys.insert(0, (i, v))
    return " + ".join(["z^{}".format(i) for i, v in polys])

"""
Multiplication for binary polynomials

the case of GF(2^8) and the number of bits of n is 9-bits. (e.g. AES)

"""


def carry(a):
    if a & 0x100:
        return True
    else:
        return False


def bin_mul(a, b, n):
    # buf = 0  # pre-computation for mod operation
    # for i in reversed(range(9)):  # from 8 down to 0
    #     mask = 1 << i
    #     if n & mask is not 0:
    #         buf = n & ((mask << 1) - 1)
    #         break
    buf = n & 0xff  # pre-computation for mod operation (simple)

    f = [0] * 8  # pre-computation table for `a`
    f[0] = a
    for i in range(1, 8):
        f[i] = f[i-1] << 1
        if carry(f[i]):
            f[i] &= 0xff
            f[i] ^= buf

    res = 0
    for i in range(8):
        mask = 1 << i
        if b & mask is not 0:
            res ^= f[i]

    return res


m = 32  # 32bit
"""
degree of binary polynomials
"""


def deg(bp):
    for i in reversed(range(m)):  # from m-1 down to 0
        if (bp & (1 << i)) != 0:
            return i
    return 0


"""
Extended Euclidean Algorithm for binary polynomials(Iterative version)

return (d, g, h) such that a * g + b * h = d = gcd(a, b)
loop invariant :
a * g_1 + b * h_1 = u
a * g_2 + b * h_2 = v
"""


def bin_ext_euclid(a, b):
    u = a
    v = b

    g_1 = 1
    g_2 = 0
    h_1 = 0
    h_2 = 1

    j = -1
    while u!=0:
        j = deg(u) - deg(v)
        if j<0:
            tmp = u
            u = v
            v = tmp

            tmp = g_1
            g_1 = g_2
            g_2 = tmp

            tmp = h_1
            h_1 = h_2
            h_2 = tmp

            j = -j

        u = u ^ (v << j)
        g_1 = g_1 ^ (g_2 << j)
        h_1 = h_1 ^ (h_2 << j)

    d = v
    g = g_2
    h = h_2

    return d, g, h


"""
Inversion for binary polynomials using extended euclidean algorithm

returns a^-1 mod n. (n should be irreducible.)
"""


def bin_inv(a, n):
    d, g, h = bin_ext_euclid(a, n)
    return g


if __name__ == "__main__":
    print("deg(10) = {}".format(deg(10)))
    # f(z) = z^8 + z^4 + z^3 + z + 1. f(z) is irreducible.
    print(get_poly_str(0b100011011))
    # the example on 4th slide
    print(get_poly_str(bin_mul(0b01010111, 0b10000011, 0b100011011)))
    # Inversion test
    d, g, h = bin_ext_euclid(128, 0b100011011)
    print(d, "|", get_poly_str(g), "|", get_poly_str(h))
    print(get_poly_str(bin_inv(128, 0b100011011)))
    print(get_poly_str(bin_mul(128, 131, 0b100011011)))
