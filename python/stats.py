#!/usr/bin/env python
"""
-- Statistics Class
-- 
-- Copyright (c) 2017-2018, Mark K. Gardner
-- All rights reserved.
-- 
-- Note: computation of mean and variance is based on incremental computation from
--       https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
--       (Should probably switch to Welford's algorithm at that URL.)
"""

import math

class Stats:
    '''
    Class of objects that compute descriptive statistics.
    '''
    def  __init__(self):
        '''
        Initialize internal state prior to use.
        '''
        self.reset()

    def __str__(self):
        '''
        Convert to string representation.
        '''
        return str(self.__dict__)

    def add(self, value):
        '''
        Update the statistics to include the value.
        '''
        self._minimum = min(self._minimum, value)
        self._maximum = max(self._maximum, value)
        if self._count is 0:
            self._K = value
        self._count += 1
        self._Ex += value - self._K
        self._Ex2 += (value - self._K) * (value - self._K)
        self._sum += value

    def remove(self, value):
        '''
        Update the statistics to remove the value.

        Note: no record of the values being added is kept and hence accurately
              adjusting the statistics when removing a value is impossible.
              Throw an exception rather than give possibly inaccurate results.
        '''
        self.reset()
        raise StatsException('the removal of values is not implemented')

    def count(self):
        '''
        Return the sample count.
        '''
        return self._count

    def sum(self):
        '''
        Return the sum of the values seen.
        '''
        if self._count < 1:
            raise StatsException(Stats._need1ErrStr % 'sum')
        return self._sum

    def min(self):
        '''
        Return the minimum value seen.
        '''
        if self._count < 1:
            raise StatsException(Stats._need1ErrStr % 'minimum')
        return self._minimum

    def max(self):
        '''
        Return the maxium value seen.
        '''
        if self._count < 1:
            raise StatsException(Stats._need1ErrStr % 'maximum')
        return self._maximum

    def mean(self):
        '''
        Return the mean of the values.
        '''
        if self._count < 1:
            raise StatsException(Stats._need1ErrStr % 'mean')
        return self._K + self._Ex / self._count

    def var(self):
        '''
        Return the variance of the values.
        '''
        if self._count < 2:
            raise StatsException(Stats._need2ErrStr % 'variance')
        return (self._Ex2 - (self._Ex * self._Ex) / self._count) / (self._count - 1)

    def stddev(self):
        '''
        Return the standard deviation of the values.
        '''
        if self._count < 2:
            raise StatsException(Stats._need2ErrStr % 'stddev')
        return math.sqrt(self.var())

    def stderr(self):
        '''
        Return the standard error of the values.
        '''
        if self._count < 2:
            raise StatsException(Stats._need2ErrStr % 'stderr')
        return math.sqrt(self.var() / self._count)

    def conf(self, level = 0.95):
        '''
        Return the (half) width of the confidence interval centered
        on the mean at the required confidence level.
        '''
        if self._count < 2:
            raise StatsException(Stats._need2ErrStr % 'confidence interval')
        centered = (1.0 - level) / 2
        if self._count > 1 and self._count < 30:
            fudge = self.T(centered, self._count - 1)
        else:
            fudge = self.Z(centered)
        return fudge * self.stderr()

    def reset(self):
        '''
        Reset internal state.
        '''
        self._count = 0
        self._minimum = float('inf')
        self._maximum = -float('inf')
        self._K = 0.0
        self._Ex = 0.0
        self._Ex2 = 0.0
        self._sum = 0.0

    _need1ErrStr = '%s is undefined for less than one sample'
    _need2ErrStr = '%s is undefined for less than two samples'

    def Z(self, p):
        '''
        Compute gaussian/normal distribution Z().
        See p. 276 of "Simulating Computer Systems" by M. H. MacDougall.
        '''
        q = p
        if p > 0.5:
            q = 1 - p
        z1 = math.sqrt(-2.0 * math.log(q))
        n = (0.010328 * z1 + 0.802853) * z1 + 2.515517
        d = ((0.0013080 * z1 + 0.189269) * z1 + 1.43278) * z1 + 1
        z1 -= n / d
        if p > 0.5:
            z1 = -z1
        return z1

    def T(self, p, ndf):
        '''
        Compute Student's T distribution.
        See p. 276 of "Simulating Computer Systems" by M. H. MacDougall.
        '''
        h = [0.0, 0.0, 0.0, 0.0]
        x = 0.0
        z1 = math.fabs(self.Z(p))
        z2 = z1 * z1
        h[0] = 0.25 * z1 * (z2 + 1.0)
        h[1] = 0.010416667 * z1 * ((5.0 * z2 + 16.0) * z2 + 3.0)
        h[2] = 0.002604167 * z1 * ((3.0 * z2 + 19.0) * z2 - 15.0)
        h[3] = z1 * ((((79.0*z2 + 776.0)* z2 + 1482.0) * z2 - 1920.0) * z2 - 945.0)
        h[3] *= 0.000010851

        for i in 3, 2, 1, 0:
            x = (x + h[i]) / ndf
        z1 += x
        if p > 0.5:
            z1 = -z1
        return z1

class StatsException(Exception):
    '''
    Class of exceptions that occur while computing statistics.
    '''
    pass


if __name__ == '__main__':
    """
    Unit test for Stats class
    """

    import stats
    import unittest

    class TestStats(unittest.TestCase):

        def test_empty(self):
            s = stats.Stats()
            self.assertEqual(s.count(), 0)
            with self.assertRaises(stats.StatsException):
                _ = s.min()
            with self.assertRaises(stats.StatsException):
                _ = s.max()
            with self.assertRaises(stats.StatsException):
                _ = s.mean()
            with self.assertRaises(stats.StatsException):
                _ = s.var()
            with self.assertRaises(stats.StatsException):
                _ = s.stddev()
            with self.assertRaises(stats.StatsException):
                _ = s.stderr()
            with self.assertRaises(stats.StatsException):
                _ = s.conf()

        def test_add1(self):
            s = stats.Stats()
            s.add(1.0)
            self.assertEqual(s.count(), 1)
            self.assertEqual(s.min(), 1.0)
            self.assertEqual(s.max(), 1.0)
            self.assertEqual(s.mean(), 1.0)
            with self.assertRaises(stats.StatsException):
                _ = s.var()
            with self.assertRaises(stats.StatsException):
                _ = s.stddev()
            with self.assertRaises(stats.StatsException):
                _ = s.stderr()
            with self.assertRaises(stats.StatsException):
                _ = s.conf()

        def test_remove(self):
            s = stats.Stats()
            s.add(1.0)
            with self.assertRaises(stats.StatsException):
                s.remove(2.0)  # should cause an error
            self.test_empty()

        def test_addMany(self):
            s = stats.Stats()
            s.add(1.0)
            s.add(2.0)
            s.add(3.0)
            s.add(4.0)
            self.assertEqual(s.count(), 4)
            self.assertEqual(s.min(), 1.0)
            self.assertEqual(s.max(), 4.0)
            self.assertEqual(s.mean(), 2.5)
            self.assertAlmostEqual(s.var(), 1.6666667)
            self.assertAlmostEqual(s.stddev(), 1.2909944)
            self.assertAlmostEqual(s.stderr(), 0.6454972)
            self.assertAlmostEqual(s.conf(), 2.0039382)

        def test_differentConfidenceLevel(self):
            s = stats.Stats()
            s.add(1.0)
            s.add(2.0)
            s.add(3.0)
            s.add(4.0)
            self.assertAlmostEqual(s.conf(0.99), 3.4905535)

    unittest.main()
