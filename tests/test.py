import unittest
import random
import struct
import xxhash

def digest_to_hex(XXHxx):
    if XXHxx.digest_size == 4:
        d = struct.unpack("<I", XXHxx.digest())[0]
    else:
        d = struct.unpack("<Q", XXHxx.digest())[0]
    hd = hex(d)[2:].rstrip('L')
    padding = '0' * (2*XXHxx.digest_size-len(hd))
    return padding + hd

class TestXXHASH(unittest.TestCase):

    def test_xxh32(self):
        self.assertEqual(xxhash.xxh32('a'), 1426945110)
        self.assertEqual(xxhash.xxh32('a', 0), 1426945110)
        self.assertEqual(xxhash.xxh32('a', 1), 4111757423)

    def test_xxh64(self):
        self.assertEqual(xxhash.xxh64('a'), 15154266338359012955)
        self.assertEqual(xxhash.xxh64('a', 0), 15154266338359012955)
        self.assertEqual(xxhash.xxh64('a', 1), 16051599287423682246)

    def test_XXH32(self):
        x = xxhash.XXH32()

        self.assertEqual(x.name, 'XXH32')
        self.assertEqual(x.digest_size, 4)
        self.assertEqual(x.block_size, 16)

        x.update('a')
        self.assertEqual(digest_to_hex(x), '550d7456')
        self.assertEqual(x.hexdigest(),    '550d7456')
        x.update('b')
        self.assertEqual(digest_to_hex(x), '4999fc53')
        self.assertEqual(x.hexdigest(),    '4999fc53')

        y = x.copy()
        y.update('c')
        self.assertEqual(digest_to_hex(y), '32d153ff')
        self.assertEqual(y.hexdigest(),    '32d153ff')

        x = xxhash.XXH32('abc')
        self.assertEqual(digest_to_hex(x), '32d153ff')
        self.assertEqual(x.hexdigest(),    '32d153ff')

        seed = random.randint(0, 2**32)
        x = xxhash.XXH32(start=seed)
        x.update('abc')
        self.assertNotEqual(x.hexdigest(), y.hexdigest())

    def test_XXH64(self):
        x = xxhash.XXH64()

        self.assertEqual(x.name, 'XXH64')
        self.assertEqual(x.digest_size, 8)
        self.assertEqual(x.block_size, 32)

        x.update('a')
        self.assertEqual(digest_to_hex(x), 'd24ec4f1a98c6e5b')
        self.assertEqual(x.hexdigest(),    'd24ec4f1a98c6e5b')
        x.update('b')
        self.assertEqual(digest_to_hex(x), '65f708ca92d04a61')
        self.assertEqual(x.hexdigest(),    '65f708ca92d04a61')

        y = x.copy()
        y.update('c')
        self.assertEqual(digest_to_hex(y), '44bc2cf5ad770999')
        self.assertEqual(y.hexdigest(),    '44bc2cf5ad770999')

        x = xxhash.XXH64('abc')
        self.assertEqual(digest_to_hex(x), '44bc2cf5ad770999')
        self.assertEqual(x.hexdigest(),    '44bc2cf5ad770999')

        seed = random.randint(0, 2**32)
        x = xxhash.XXH64(start=seed)
        x.update('abc')
        self.assertNotEqual(x.hexdigest(), y.hexdigest())


if __name__ == '__main__':
    unittest.main()
